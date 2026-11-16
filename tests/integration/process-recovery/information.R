# Source benchmark.R and sensitivity.R first. Older pilots remain unchanged.
information_design <- function() {
  data.frame(design = c("baseline", "survey", "compositions", "both"),
    index_sd = c(0.2, 0.05, 0.2, 0.05), composition_size = c(200L, 200L, 2000L, 2000L))
}

information_operating_models <- function(fixture, process_seed, observation_seed,
                                         design = information_design()) {
  stopifnot(nrow(design) > 0L, !anyNA(design), !anyDuplicated(design$design),
    all(is.finite(design$index_sd) & design$index_sd > 0),
    all(is.finite(design$composition_size) & design$composition_size > 0 &
      design$composition_size == floor(design$composition_size)))
  walk <- function(sd) list(type = "rw1", sd = sd, estimation_type = "constant", initial = fixture$initial)
  simulation <- simulate_fims(fixture$parameters, fixture$data, list(type = "iid"),
    seed = process_seed, observation_error = FALSE,
    fishing_process = list(fleet1 = walk(fixture$sigma[["F"]])),
    mortality_process = walk(fixture$sigma[["M"]]))
  draw <- simulation$replicates[[1L]]
  truth <- list(R = draw$recruitment$value, F = draw$fishing_mortality$log_Fmort,
                M = draw$natural_mortality$log_M)
  raw <- get_data(fixture$data)
  indices <- which(raw$type %in% c("catch", "index", "age_comp", "length_comp"))
  obs <- draw$observations
  stopifnot(length(indices) == nrow(obs), all(obs$model_bin), all(is.finite(obs$expected)),
    all(vapply(c("fleet", "type", "timing", "age", "length"), function(key)
      identical(raw[[key]][indices], obs[[key]]), logical(1))))
  had_seed <- exists(".Random.seed", .GlobalEnv, inherits = FALSE)
  saved_seed <- if (had_seed) get(".Random.seed", .GlobalEnv) else NULL
  on.exit({
    if (had_seed) assign(".Random.seed", saved_seed, .GlobalEnv)
    else if (exists(".Random.seed", .GlobalEnv, inherits = FALSE)) rm(".Random.seed", envir = .GlobalEnv)
  }, add = TRUE)
  set.seed(observation_seed)
  scalar <- which(obs$type %in% c("catch", "index"))
  stopifnot(all(obs$family[scalar] == "dlnorm"))
  normals <- rnorm(length(scalar))
  comp <- which(obs$type %in% c("age_comp", "length_comp"))
  stopifnot(all(obs$unit[comp] == "proportion"))
  groups <- split(comp, interaction(obs$fleet[comp], obs$type[comp], obs$timing[comp], drop = TRUE))
  # One categorical pool per observation. Larger samples include the smaller
  # sample exactly; all prefixes have the declared multinomial distribution.
  pools <- lapply(groups, function(idx) {
    prob <- obs$expected[idx]
    stopifnot(all(prob >= 0), abs(sum(prob) - 1) < 1e-6)
    sample.int(length(idx), max(design$composition_size), replace = TRUE, prob = prob)
  })
  models <- lapply(seq_len(nrow(design)), function(i) {
    scenario <- design[i, ]
    values <- obs$expected
    sd <- ifelse(obs$type[scalar] == "index", scenario$index_sd, 0.1)
    values[scalar] <- exp(obs$location[scalar] + sd * normals)
    for (j in seq_along(groups)) {
      idx <- groups[[j]]
      counts <- tabulate(pools[[j]][seq_len(scenario$composition_size)], nbins = length(idx))
      values[idx] <- counts / scenario$composition_size
    }
    data <- raw
    data$observed[indices] <- values
    data$uncertainty[data$type == "index"] <- sprintf(
      "~ dlnorm(meanlog = log_index_expected, sdlog = %.17g)", scenario$index_sd)
    for (type in c("age_comp", "length_comp")) {
      link <- if (type == "age_comp") "agecomp_proportion" else "lengthcomp_proportion"
      data$uncertainty[data$type == type] <- sprintf("~ dmultinom(prob = %s, size = %d)",
        link, scenario$composition_size)
    }
    list(data = FIMSFrame(data), truth = truth)
  })
  names(models) <- design$design
  list(models = models, truth = truth, predictions = obs,
    observation_pool = list(scalar_indices = scalar, standard_normals = normals,
      composition_groups = groups, categorical_draws = pools),
    rng = list(process_seed = process_seed, observation_seed = observation_seed, kind = RNGkind()))
}

information_summaries <- function(attempts) {
  # Keep separate likelihood references for different observation datasets.
  pieces <- lapply(split(attempts, attempts$design), sensitivity_summaries)
  tables <- lapply(c("attempts", "best"), function(name)
    dplyr::bind_rows(lapply(pieces, `[[`, name)))
  names(tables) <- c("attempts", "best")
  for (name in c("M_boundary", "M_sd", "M_rmse"))
    if (!name %in% names(tables$best)) tables$best[[name]] <- NA
  free <- tables$best[is.na(tables$best$profile_sd), ]
  tables$summary <- free |>
    dplyr::group_by(.data$design, .data$initialization) |>
    dplyr::summarise(planned_datasets = dplyr::n(), valid_fits = sum(.data$valid),
      M_boundary_fits = sum(.data$M_boundary & .data$valid, na.rm = TRUE),
      mean_M_sd = if (any(.data$valid)) mean(.data$M_sd[.data$valid]) else NA_real_,
      mean_M_rmse = if (any(.data$valid)) mean(.data$M_rmse[.data$valid]) else NA_real_,
      .groups = "drop")
  tables
}

run_information_benchmark <- function(years = 18L, replicates = 3L, seed = 4301L,
                                      starts = c(0.05, 0.2),
                                      design = information_design(),
                                      initializations = c("fixed", "normal"),
                                      profile_designs = c("baseline", "both"),
                                      grid = c(.001, .01, .05, .1, .2, .4),
                                      checkpoint = NULL) {
  stopifnot(replicates >= 1L, replicates == as.integer(replicates),
    seed >= 0, seed + 100000 + replicates < .Machine$integer.max,
    length(starts) > 0L, all(is.finite(starts)), all(starts >= .001 & starts <= 1),
    all(profile_designs %in% design$design), length(initializations) > 0L,
    all(initializations %in% c("fixed", "normal")), !anyDuplicated(initializations),
    all(is.finite(grid)), all(grid >= .001 & grid <= 1))
  fixture <- recovery_fixture(years)
  results <- list(); operating_models <- list()
  for (replicate in seq_len(replicates)) {
    paired <- information_operating_models(fixture, seed + replicate - 1L,
      seed + 100000L + replicate - 1L, design)
    operating_models[[replicate]] <- list(truth = paired$truth,
      data = lapply(paired$models, function(x) get_data(x$data)),
      predictions = paired$predictions, observation_pool = paired$observation_pool, rng = paired$rng)
    for (name in design$design) for (initialization in initializations) {
      profiles <- c(NA_real_, if (replicate == 1L && name %in% profile_designs) grid)
      for (profile in profiles) for (start in starts) {
        message(sprintf("replicate=%d design=%s initial=%s M_SD=%s start=%.3f",
          replicate, name, initialization, if (is.na(profile)) "free" else profile, start))
        started <- proc.time()[["elapsed"]]
        fit <- tryCatch(sensitivity_fit(fixture, paired$models[[name]], initialization,
          start, if (is.na(profile)) NULL else profile), error = function(e) data.frame(
            error = conditionMessage(e), objective = NA_real_, convergence = NA_integer_,
            max_projected_gradient = NA_real_, max_inner_gradient = NA_real_,
            elapsed_seconds = proc.time()[["elapsed"]] - started))
        results[[length(results) + 1L]] <- cbind(years = years, replicate = replicate,
          seed = seed + replicate - 1L, design = name, initialization = initialization,
          profile_sd = profile, start_sd = start, fit)
        gc(verbose = FALSE)
      }
    }
    if (!is.null(checkpoint)) saveRDS(list(attempts = dplyr::bind_rows(results),
      operating_models = operating_models), checkpoint)
  }
  list(configuration = list(years = years, replicates = replicates, seed = seed,
    observation_seed_offset = 100000L, starts = starts, design = design,
    initializations = initializations, profile_designs = profile_designs, grid = grid,
    truth_sigma = fixture$sigma, initial_rule = fixture$initial,
    session = capture.output(sessionInfo())), operating_models = operating_models,
    tables = information_summaries(dplyr::bind_rows(results)))
}
