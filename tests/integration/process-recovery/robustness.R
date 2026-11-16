# Source the prior four benchmark modules first. No production likelihood changes.
robustness_design <- function(size = 2000L) {
  stopifnot(length(size) == 1L, is.finite(size), size > 0, size == floor(size))
  concentration <- c(Inf, 2000, 200)
  inflation <- ifelse(is.infinite(concentration), 1, (size + concentration) / (1 + concentration))
  data.frame(scenario = c("multinomial", "moderate", "strong"), concentration = concentration,
    sample_size = size, variance_inflation = inflation, effective_size = size / inflation)
}

# Gamma quantiles allow common random numbers across concentration settings.
robustness_probabilities <- function(probability, concentration, uniforms) {
  stopifnot(length(probability) > 0L, all(is.finite(probability)), all(probability >= 0),
    abs(sum(probability) - 1) < 1e-6, length(concentration) == 1L,
    !is.na(concentration), concentration > 0, length(uniforms) == length(probability),
    all(is.finite(uniforms)), all(uniforms > 0 & uniforms < 1))
  probability <- probability / sum(probability)
  if (is.infinite(concentration)) return(probability)
  positive <- probability > 0
  gamma <- numeric(length(probability))
  gamma[positive] <- qgamma(uniforms[positive], shape = concentration * probability[positive], rate = 1)
  stopifnot(all(is.finite(gamma)), sum(gamma) > 0)
  gamma / sum(gamma)
}

robustness_counts <- function(probability, uniforms) {
  stopifnot(all(is.finite(probability)), all(probability >= 0), abs(sum(probability) - 1) < 1e-6,
    all(is.finite(uniforms)), all(uniforms > 0 & uniforms < 1))
  boundaries <- c(0, pmin(1, cumsum(probability / sum(probability))))
  boundaries[length(boundaries)] <- 1
  index <- findInterval(uniforms, boundaries, rightmost.closed = TRUE, all.inside = TRUE)
  tabulate(index, nbins = length(probability))
}

robustness_operating_models <- function(fixture, seed, design = robustness_design()) {
  stopifnot(nrow(design) > 0L, !anyNA(design), !anyDuplicated(design$scenario),
    length(unique(design$sample_size)) == 1L, all(design$sample_size == 2000L))
  # Reuse exactly the same process paths and catch/survey observations.
  paired <- information_operating_models(fixture, seed, seed + 100000L,
    information_design()[4, , drop = FALSE])
  raw <- get_data(paired$models$both$data)
  predictions <- paired$predictions
  indices <- which(raw$type %in% c("catch", "index", "age_comp", "length_comp"))
  comp <- which(predictions$type %in% c("age_comp", "length_comp"))
  groups <- split(comp, interaction(predictions$fleet[comp], predictions$type[comp],
    predictions$timing[comp], drop = TRUE))
  had_seed <- exists(".Random.seed", .GlobalEnv, inherits = FALSE)
  saved_seed <- if (had_seed) get(".Random.seed", .GlobalEnv) else NULL
  on.exit({
    if (had_seed) assign(".Random.seed", saved_seed, .GlobalEnv)
    else if (exists(".Random.seed", .GlobalEnv, inherits = FALSE)) rm(".Random.seed", envir = .GlobalEnv)
  }, add = TRUE)
  set.seed(seed + 200000L)
  pool <- lapply(groups, function(idx) list(gamma = runif(length(idx)),
    sample = runif(design$sample_size[1L])))
  models <- lapply(seq_len(nrow(design)), function(i) {
    data <- raw
    mixed <- vector("list", length(groups))
    for (j in seq_along(groups)) {
      idx <- groups[[j]]
      mixed[[j]] <- robustness_probabilities(predictions$expected[idx], design$concentration[i], pool[[j]]$gamma)
      counts <- robustness_counts(mixed[[j]], pool[[j]]$sample)
      data$observed[indices[idx]] <- counts / design$sample_size[i]
    }
    # All fitted formulas stay at the nominal multinomial sample size.
    list(data = FIMSFrame(data), truth = paired$truth, mixed_probabilities = mixed)
  })
  names(models) <- design$scenario
  list(models = models, truth = paired$truth, predictions = predictions, pool = pool,
    rng = c(paired$rng, list(composition_seed = seed + 200000L)))
}

robustness_summaries <- function(tables) {
  scenarios <- unique(tables$diagnostics$scenario)
  pieces <- lapply(scenarios, function(scenario) {
    subset <- lapply(tables[c("diagnostics", "state_metrics", "hyperparameters")],
      function(table) if (nrow(table)) table[table$scenario == scenario, , drop = FALSE] else table)
    output <- calibration_summaries(subset)
    lapply(output, function(table) {
      if (nrow(table) && !"scenario" %in% names(table)) table <- cbind(scenario = scenario, table)
      table
    })
  })
  names <- names(pieces[[1L]])
  result <- lapply(names, function(name) dplyr::bind_rows(lapply(pieces, `[[`, name)))
  stats::setNames(result, names)
}

run_robustness_benchmark <- function(replicates = 5L, years = 18L, seed = 4301L,
                                     starts = c(.05, .2), checkpoint = NULL) {
  stopifnot(replicates >= 1L, replicates == as.integer(replicates),
    seed >= 0, seed + 200000L + replicates < .Machine$integer.max,
    length(starts) > 0L, all(is.finite(starts)), all(starts >= .001 & starts <= 1))
  fixture <- recovery_fixture(years)
  design <- robustness_design()
  results <- list(); attempts <- list(); operating_models <- list()
  for (replicate in seq_len(replicates)) {
    operating <- robustness_operating_models(fixture, seed + replicate - 1L, design)
    operating_models[[replicate]] <- list(truth = operating$truth,
      data = lapply(operating$models, function(x) get_data(x$data)), predictions = operating$predictions,
      mixed_probabilities = lapply(operating$models, `[[`, "mixed_probabilities"),
      pool = operating$pool, rng = operating$rng)
    for (scenario in design$scenario) {
      message(sprintf("replicate=%d/%d scenario=%s", replicate, replicates, scenario))
      model <- operating$models[[scenario]]
      metadata <- data.frame(replicate = replicate, seed = seed + replicate - 1L,
        scenario = scenario, true_M_sd = fixture$sigma[["M"]])
      candidates <- lapply(starts, function(start) {
        message(sprintf("  estimated SD start=%.3f", start))
        started <- proc.time()[["elapsed"]]
        fit <- tryCatch(sensitivity_fit(fixture, model, "normal", start), error = function(e)
          data.frame(error = conditionMessage(e), objective = NA_real_, convergence = NA_integer_,
            max_projected_gradient = NA_real_, max_inner_gradient = NA_real_,
            elapsed_seconds = proc.time()[["elapsed"]] - started))
        cbind(start_sd = start, fit)
      })
      candidates <- dplyr::bind_rows(candidates)
      candidates$valid <- sensitivity_valid(candidates)
      candidates <- candidates[order(!candidates$valid, candidates$objective, na.last = TRUE), ]
      candidates$selected <- seq_len(nrow(candidates)) == 1L
      attempts[[length(attempts) + 1L]] <- cbind(metadata[rep(1L, nrow(candidates)), ], candidates)
      for (mode in c("known", "estimated")) {
        message("  uncertainty: ", mode)
        report <- tryCatch(calibration_report(fixture, model, mode,
          if (mode == "estimated") candidates[1L, ] else NULL), error = function(e)
            list(diagnostics = data.frame(error = conditionMessage(e), optimizer_valid = FALSE,
              pd_hessian = NA, boundary = NA, M_boundary = NA, regular = FALSE, sdreport_error = NA_character_)))
        meta <- cbind(metadata, sd_mode = mode)
        results[[length(results) + 1L]] <- lapply(report, function(table)
          if (nrow(table)) cbind(meta[rep(1L, nrow(table)), ], table) else NULL)
        gc(verbose = FALSE)
      }
    }
    if (!is.null(checkpoint)) saveRDS(list(results = results, attempts = dplyr::bind_rows(attempts),
      operating_models = operating_models), checkpoint)
  }
  names <- c("diagnostics", "states", "state_metrics", "hyperparameters")
  tables <- lapply(names, function(name) dplyr::bind_rows(lapply(results, `[[`, name)))
  names(tables) <- names
  tables$attempts <- dplyr::bind_rows(attempts)
  list(configuration = list(replicates = replicates, years = years, seed = seed,
    starts = starts, design = design, true_M_sd = fixture$sigma[["M"]],
    initial = "proper-normal", fitted_composition_likelihood = "multinomial, nominal size 2000",
    session = capture.output(sessionInfo())), operating_models = operating_models,
    tables = tables, summaries = robustness_summaries(tables))
}
