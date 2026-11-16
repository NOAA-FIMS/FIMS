# Source benchmark.R first. Keep the original pilot's source and results intact.
sensitivity_objective <- function(fixture, operating, initialization, start_sd,
                                   profile_sd = NULL) {
  stopifnot(initialization %in% c("fixed", "normal"),
    length(start_sd) == 1L, is.finite(start_sd), start_sd >= 0.001, start_sd <= 1)
  if (!is.null(profile_sd)) stopifnot(length(profile_sd) == 1L,
    is.finite(profile_sd), profile_sd >= 0.001, profile_sd <= 1)
  design <- data.frame(processes = "R_F_M", initialization = initialization, sd_mode = "estimated")
  setup <- recovery_fit_input(fixture, operating, design)
  input <- setup$input
  full_names <- names(get_parameter_names(as.list(input$parameters$p)))
  owners <- list(R = attr(input, "gaussian_recruitment"),
                F = attr(input, "fishing_processes")$fleet1,
                M = attr(input, "gaussian_mortality"))
  positions <- vapply(owners, function(density) {
    idx <- which(startsWith(full_names, paste0("gaussian_process.", density$get_id(), ".log_sd.")))
    stopifnot(length(idx) == 1L); idx
  }, integer(1))
  stopifnot(length(full_names) == 3L, length(unique(positions)) == 3L)
  input$parameters$p[] <- log(start_sd)
  if (!is.null(profile_sd)) {
    input$parameters$p[positions[["M"]]] <- log(profile_sd)
    mapping <- seq_along(input$parameters$p)
    mapping[positions[["M"]]] <- NA_integer_
    input$map <- list(p = factor(mapping))
  }
  obj <- TMB::MakeADFun(data = list(), parameters = input$parameters,
    map = input$map, random = "re", DLL = "FIMS", silent = TRUE)
  list(obj = obj, positions = positions, input = input, setup = setup)
}

# KKT stationarity on a declared box; a variance-boundary optimum remains visible.
sensitivity_projected_gradient <- function(par, gradient, lower = log(0.001), upper = 0,
                                           tolerance = 1e-6) {
  stopifnot(length(par) == length(gradient))
  projected <- gradient
  projected[par <= lower + tolerance & gradient > 0] <- 0
  projected[par >= upper - tolerance & gradient < 0] <- 0
  projected
}

sensitivity_fit <- function(fixture, operating, initialization, start_sd, profile_sd = NULL) {
  started <- proc.time()[["elapsed"]]
  warnings <- character()
  withCallingHandlers({
    setup <- sensitivity_objective(fixture, operating, initialization, start_sd, profile_sd)
    obj <- setup$obj
    opt <- nlminb(obj$par, obj$fn, obj$gr, lower = log(0.001), upper = 0,
      control = list(iter.max = 300, eval.max = 600))
    objective <- as.numeric(obj$fn(opt$par))
    gradient <- as.numeric(obj$gr(opt$par))
    # Snapshot the mode at this candidate, not a cached best from a different evaluation.
    joint <- obj$env$last.par
    inner <- as.numeric(obj$env$f(joint, order = 1))[obj$env$random]
    full <- obj$env$parList(par = joint)
    sigma <- exp(full$p[setup$positions])
    random_names <- names(get_random_names(as.list(full$re)))
    estimates <- full$re[grepl(".log_M.", random_names, fixed = TRUE)]
    idx <- fixture$rows$M
    estimated <- setup$setup$parameters$estimation_type[idx] == "random_effects"
    stopifnot(length(estimates) == sum(estimated))
    errors <- estimates - operating$truth$M[estimated]
    projected <- sensitivity_projected_gradient(opt$par, gradient)
    data.frame(error = NA_character_, objective = objective, convergence = opt$convergence,
      max_gradient = max(abs(gradient)), max_projected_gradient = max(abs(projected)),
      max_inner_gradient = max(abs(inner)), R_sd = sigma[1], F_sd = sigma[2], M_sd = sigma[3],
      R_boundary = abs(log(sigma[1]) - log(0.001)) < 1e-3 || abs(log(sigma[1])) < 1e-3,
      F_boundary = abs(log(sigma[2]) - log(0.001)) < 1e-3 || abs(log(sigma[2])) < 1e-3,
      M_boundary = abs(log(sigma[3]) - log(0.001)) < 1e-3 || abs(log(sigma[3])) < 1e-3,
      M_rmse = sqrt(mean(errors^2)), M_bias = mean(errors),
      n_random = length(full$re), n_outer = length(opt$par),
      elapsed_seconds = proc.time()[["elapsed"]] - started,
      message = opt$message, warnings = paste(unique(warnings), collapse = " | "))
  }, warning = function(w) {warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning")})
}

sensitivity_valid <- function(table) {
  with(table, is.na(error) & is.finite(objective) & !is.na(convergence) & convergence == 0 &
    is.finite(max_projected_gradient) & max_projected_gradient < 1e-3 &
    is.finite(max_inner_gradient) & max_inner_gradient < 1e-3)
}

sensitivity_summaries <- function(attempts) {
  attempts$valid <- sensitivity_valid(attempts)
  best <- attempts |>
    dplyr::group_by(.data$years, .data$replicate, .data$seed, .data$initialization, .data$profile_sd) |>
    dplyr::arrange(dplyr::desc(.data$valid), .data$objective, .by_group = TRUE) |>
    dplyr::mutate(attempts = dplyr::n(), valid_starts = sum(.data$valid),
      objective_spread = if (any(.data$valid))
        max(.data$objective[.data$valid]) - min(.data$objective[.data$valid]) else NA_real_) |>
    dplyr::slice_head(n = 1L) |>
    dplyr::ungroup()
  # Each grid point reoptimizes R/F SDs AND all latent states. Delta NLL references
  # the best valid free or profile result for the same dataset/initialization.
  best <- best |>
    dplyr::group_by(.data$years, .data$replicate, .data$initialization) |>
    dplyr::mutate(reference_objective = if (any(.data$valid)) min(.data$objective[.data$valid]) else NA_real_,
      delta_nll = ifelse(.data$valid, .data$objective - .data$reference_objective, NA_real_)) |>
    dplyr::ungroup()
  list(attempts = attempts, best = best)
}

run_sensitivity_benchmark <- function(horizons = c(6L, 18L), replicates = 3L, seed = 4301L,
                                      starts = c(0.025, 0.1, 0.35),
                                      grid = c(0.001, 0.005, 0.01, 0.025, 0.05, 0.1, 0.2, 0.4),
                                      profile_replicates = 1L, checkpoint = NULL) {
  stopifnot(length(horizons) > 0L, !anyDuplicated(horizons),
    replicates >= 1L, replicates == as.integer(replicates),
    profile_replicates >= 0L, profile_replicates <= replicates,
    profile_replicates == as.integer(profile_replicates),
    seed >= 0, seed + replicates < .Machine$integer.max,
    length(starts) > 0L, all(is.finite(starts)), all(starts >= 0.001 & starts <= 1),
    length(grid) > 0L, all(is.finite(grid)), all(grid >= 0.001 & grid <= 1))
  results <- list(); operating_models <- list()
  for (years in horizons) {
    fixture <- recovery_fixture(years)
    for (replicate in seq_len(replicates)) {
      # Same seed labels across horizons, but draw layouts differ: trajectories
      # are not nested prefixes. Only fits within a horizon/dataset are paired.
      operating <- recovery_operating_model(fixture, seed + replicate - 1L)
      operating_models[[paste(years, replicate, sep = "_")]] <- list(
        truth = operating$truth, data = get_data(operating$data), rng = operating$simulation$rng)
      for (initialization in c("fixed", "normal")) {
        profiles <- c(NA_real_, if (replicate <= profile_replicates) grid)
        for (profile in profiles) for (start in starts) {
          message(sprintf("years=%d replicate=%d initial=%s M_SD=%s start=%.3f",
            years, replicate, initialization, if (is.na(profile)) "free" else profile, start))
          started <- proc.time()[["elapsed"]]
          fit <- tryCatch(sensitivity_fit(fixture, operating, initialization, start,
            if (is.na(profile)) NULL else profile), error = function(e) data.frame(
              error = conditionMessage(e), objective = NA_real_, convergence = NA_integer_,
              max_projected_gradient = NA_real_, max_inner_gradient = NA_real_,
              elapsed_seconds = proc.time()[["elapsed"]] - started))
          results[[length(results) + 1L]] <- cbind(years = years, replicate = replicate,
            seed = seed + replicate - 1L, initialization = initialization,
            profile_sd = profile, start_sd = start, fit)
          gc(verbose = FALSE)
        }
      }
      # Persist plain completed attempts for long experiments, including failures.
      if (!is.null(checkpoint)) saveRDS(list(attempts = dplyr::bind_rows(results),
        operating_models = operating_models), checkpoint)
    }
  }
  tables <- sensitivity_summaries(dplyr::bind_rows(results))
  list(configuration = list(horizons = horizons, replicates = replicates, seed = seed,
    starts = starts, grid = grid, profile_replicates = profile_replicates,
    truth_sigma = c(R = 0.2, F = 0.15, M = 0.05),
    session = capture.output(sessionInfo())), operating_models = operating_models, tables = tables)
}
