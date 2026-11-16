# Source benchmark.R, sensitivity.R and information.R first. Prior pilots are unchanged.
calibration_intervals <- function(truth, estimate, se, level = .95) {
  metrics <- recovery_metrics(truth, estimate, se, level)
  valid <- is.finite(truth) & is.finite(estimate) & is.finite(se) & se > 0
  z <- qnorm((1 + level) / 2)
  metrics$mean_width <- if (any(valid)) mean(2 * z * se[valid]) else NA_real_
  metrics$mean_squared_standardized_error <- if (any(valid))
    mean(((estimate[valid] - truth[valid]) / se[valid])^2) else NA_real_
  metrics
}

calibration_report <- function(fixture, operating, sd_mode, selected = NULL) {
  stopifnot(sd_mode %in% c("known", "estimated"))
  started <- proc.time()[["elapsed"]]
  warnings <- character()
  withCallingHandlers({
    design <- data.frame(processes = "R_F_M", initialization = "normal", sd_mode = sd_mode)
    setup <- recovery_fit_input(fixture, operating, design)
    input <- setup$input
    positions <- integer()
    if (sd_mode == "estimated") {
      stopifnot(!is.null(selected), nrow(selected) == 1L, is.finite(selected$objective))
      names <- names(get_parameter_names(as.list(input$parameters$p)))
      densities <- list(R = attr(input, "gaussian_recruitment"),
        F = attr(input, "fishing_processes")$fleet1, M = attr(input, "gaussian_mortality"))
      positions <- vapply(densities, function(density) {
        idx <- which(startsWith(names, paste0("gaussian_process.", density$get_id(), ".log_sd.")))
        stopifnot(length(idx) == 1L); idx
      }, integer(1))
      input$parameters$p[positions] <- log(c(selected$R_sd, selected$F_sd, selected$M_sd))
    }
    obj <- TMB::MakeADFun(data = list(), parameters = input$parameters,
      map = input$map, random = "re", DLL = "FIMS", silent = TRUE)
    value <- as.numeric(obj$fn(obj$par))
    gradient <- as.numeric(obj$gr(obj$par))
    joint <- obj$env$last.par
    inner <- as.numeric(obj$env$f(joint, order = 1))[obj$env$random]
    random <- joint[obj$env$random]
    random_names <- names(get_random_names(as.list(input$parameters$re)))
    difference <- if (sd_mode == "estimated") value - selected$objective else 0
    stopifnot(is.finite(difference), abs(difference) < 1e-5)
    projected <- sensitivity_projected_gradient(obj$par, gradient)
    optimizer_valid <- (sd_mode == "known" || isTRUE(selected$valid)) &&
      all(is.finite(inner)) && max(abs(inner)) < 1e-3 &&
      all(is.finite(projected)) && (!length(projected) || max(abs(projected)) < 1e-3)
    sd_error <- NA_character_
    sdr <- tryCatch(TMB::sdreport(obj, par.fixed = obj$par),
      error = function(e) {sd_error <<- conditionMessage(e); NULL})
    se <- rep(NA_real_, length(random))
    if (!is.null(sdr$diag.cov.random)) {
      stopifnot(length(sdr$par.random) == length(random), length(sdr$diag.cov.random) == length(random))
      random <- as.numeric(sdr$par.random)
      se <- sqrt(sdr$diag.cov.random)
    }
    sigma <- if (sd_mode == "known") fixture$sigma else c(R = selected$R_sd, F = selected$F_sd, M = selected$M_sd)
    boundary <- sd_mode == "estimated" && any(abs(log(sigma) - log(.001)) < 1e-3 | abs(log(sigma)) < 1e-3)
    pd_hessian <- !is.null(sdr) && isTRUE(sdr$pdHess)
    # Regularity is a reporting stratum, not a claim of interval validity.
    regular <- optimizer_valid && pd_hessian && !boundary && all(is.finite(se) & se > 0)
    labels <- c(R = "log_devs", F = "log_Fmort", M = "log_M")
    states <- do.call(rbind, lapply(names(labels), function(key) {
      idx <- which(grepl(paste0(".", labels[[key]], "."), random_names, fixed = TRUE))
      rows <- fixture$rows[[key]]
      stopifnot(length(idx) == length(rows))
      data.frame(process = key, timing = fixture$parameters$timing[rows],
        age = fixture$parameters$age[rows], truth = operating$truth[[key]],
        estimate = random[idx], se = se[idx])
    }))
    metrics <- do.call(rbind, lapply(names(labels), function(key) {
      rows <- states[states$process == key, ]
      cbind(process = key, calibration_intervals(rows$truth, rows$estimate, rows$se))
    }))
    hyperparameters <- data.frame()
    if (sd_mode == "estimated") for (key in names(labels)) {
      idx <- positions[[key]]
      hyper_se <- if (is.null(sdr$cov.fixed)) NA_real_ else sqrt(diag(sdr$cov.fixed))[idx]
      estimate <- sigma[[key]]
      hyperparameters <- rbind(hyperparameters, data.frame(process = key,
        truth = fixture$sigma[[key]], estimate = estimate, log_se = hyper_se,
        boundary = abs(log(estimate) - log(.001)) < 1e-3 || abs(log(estimate)) < 1e-3,
        covered = if (is.finite(hyper_se) && hyper_se > 0)
          abs(log(estimate) - log(fixture$sigma[[key]])) <= qnorm(.975) * hyper_se else NA))
    }
    diagnostics <- data.frame(error = NA_character_, objective = value,
      optimizer_valid = optimizer_valid, pd_hessian = pd_hessian, boundary = boundary,
      M_boundary = sd_mode == "estimated" &&
        (abs(log(sigma[["M"]]) - log(.001)) < 1e-3 || abs(log(sigma[["M"]])) < 1e-3),
      regular = regular, sdreport_error = sd_error,
      max_projected_gradient = if (length(projected)) max(abs(projected)) else 0,
      max_inner_gradient = max(abs(inner)), objective_rebuild_difference = difference,
      R_sd = sigma[["R"]], F_sd = sigma[["F"]], M_sd = sigma[["M"]],
      n_random = length(random), n_outer = length(obj$par),
      uncertainty_seconds = proc.time()[["elapsed"]] - started,
      warnings = paste(unique(warnings), collapse = " | "))
    list(diagnostics = diagnostics, states = states, state_metrics = metrics,
         hyperparameters = hyperparameters)
  }, warning = function(w) {warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning")})
}

calibration_summaries <- function(tables) {
  diagnostics <- tables$diagnostics
  for (name in c("error", "optimizer_valid", "pd_hessian", "boundary", "M_boundary", "regular", "sdreport_error"))
    if (!name %in% names(diagnostics)) diagnostics[[name]] <- NA
  diagnostics$optimizer_valid[is.na(diagnostics$optimizer_valid)] <- FALSE
  diagnostics$regular[is.na(diagnostics$regular)] <- FALSE
  keys <- c("true_M_sd", "sd_mode")
  fit_summary <- diagnostics |>
    dplyr::group_by(dplyr::across(dplyr::all_of(keys))) |>
    dplyr::summarise(planned_fits = dplyr::n(), errors = sum(!is.na(.data$error)),
      valid_optimizers = sum(.data$optimizer_valid),
      sdreport_failures = sum(!is.na(.data$sdreport_error)),
      boundary_fits = sum(.data$boundary, na.rm = TRUE),
      M_boundary_fits = sum(.data$M_boundary, na.rm = TRUE), regular_fits = sum(.data$regular),
      .groups = "drop")
  tag <- diagnostics[c("replicate", keys, "optimizer_valid", "regular", "boundary")]
  metrics <- tables$state_metrics
  summary <- data.frame()
  if (nrow(metrics)) {
    metrics <- dplyr::left_join(metrics, tag, by = c("replicate", keys))
    # Keep all computed intervals, and show interior/regular results separately.
    expanded <- dplyr::bind_rows(dplyr::mutate(metrics, stratum = "all"),
      dplyr::mutate(metrics[metrics$regular, ], stratum = "regular"),
      dplyr::mutate(metrics[!metrics$regular, ], stratum = "nonregular"))
    summary <- expanded |>
      dplyr::group_by(dplyr::across(dplyr::all_of(c(keys, "process", "stratum")))) |>
      dplyr::summarise(fits = dplyr::n(), interval_fits = sum(is.finite(.data$coverage)),
        mean_rmse = mean(.data$rmse),
        mean_coverage = if (any(is.finite(.data$coverage))) mean(.data$coverage, na.rm = TRUE) else NA_real_,
        coverage_mcse = if (sum(is.finite(.data$coverage)) > 1L)
          sd(.data$coverage, na.rm = TRUE) / sqrt(sum(is.finite(.data$coverage))) else NA_real_,
        mean_width = if (any(is.finite(.data$mean_width))) mean(.data$mean_width, na.rm = TRUE) else NA_real_,
        .groups = "drop")
  }
  hyper <- tables$hyperparameters
  hyper_summary <- data.frame()
  if (nrow(hyper)) {
    hyper <- dplyr::left_join(hyper, tag, by = c("replicate", keys))
    expanded <- dplyr::bind_rows(dplyr::mutate(hyper, stratum = "all"),
      dplyr::mutate(hyper[hyper$regular, ], stratum = "regular"),
      dplyr::mutate(hyper[!hyper$regular, ], stratum = "nonregular"))
    hyper_summary <- expanded |>
      dplyr::group_by(dplyr::across(dplyr::all_of(c(keys, "process", "stratum")))) |>
      dplyr::group_modify(function(rows, key) {
        n <- sum(!is.na(rows$covered)); covered <- sum(rows$covered, na.rm = TRUE)
        interval <- if (n) binom.test(covered, n)$conf.int else c(NA_real_, NA_real_)
        data.frame(fits = nrow(rows), interval_fits = n, covered = covered,
          coverage = if (n) covered / n else NA_real_,
          coverage_lower = interval[1], coverage_upper = interval[2],
          mean_sd = mean(rows$estimate), bias = mean(rows$estimate - rows$truth))
      }) |>
      dplyr::ungroup()
  }
  list(diagnostics = diagnostics, fit_summary = fit_summary, state_summary = summary,
       hyperparameter_summary = hyper_summary)
}

run_calibration_benchmark <- function(replicates = 10L, years = 18L, seed = 4301L,
                                      mortality_sds = c(.025, .05, .1),
                                      starts = c(.05, .2), checkpoint = NULL) {
  stopifnot(replicates >= 1L, replicates == as.integer(replicates),
    seed >= 0, seed + 100000L + replicates < .Machine$integer.max,
    length(mortality_sds) > 0L, !anyDuplicated(mortality_sds),
    all(is.finite(mortality_sds)), all(mortality_sds > 0 & mortality_sds < 1),
    length(starts) > 0L, all(is.finite(starts)), all(starts >= .001 & starts <= 1))
  all_results <- list(); attempts <- list(); operating_models <- list()
  design <- information_design()[4, , drop = FALSE]
  for (true_sd in mortality_sds) {
    fixture <- recovery_fixture(years)
    fixture$sigma[["M"]] <- true_sd
    for (replicate in seq_len(replicates)) {
      message(sprintf("M truth=%.3f replicate=%d/%d", true_sd, replicate, replicates))
      paired <- information_operating_models(fixture, seed + replicate - 1L,
        seed + 100000L + replicate - 1L, design)
      operating <- paired$models$both
      metadata <- data.frame(true_M_sd = true_sd, replicate = replicate, seed = seed + replicate - 1L)
      operating_models[[paste(true_sd, replicate, sep = "_")]] <- list(
        truth = paired$truth, data = get_data(operating$data),
        observation_pool = paired$observation_pool, rng = paired$rng)
      candidates <- lapply(starts, function(start) {
        message(sprintf("  estimated SD start=%.3f", start))
        started <- proc.time()[["elapsed"]]
        result <- tryCatch(sensitivity_fit(fixture, operating, "normal", start),
          error = function(e) data.frame(error = conditionMessage(e), objective = NA_real_,
            convergence = NA_integer_, max_projected_gradient = NA_real_, max_inner_gradient = NA_real_,
            elapsed_seconds = proc.time()[["elapsed"]] - started))
        cbind(start_sd = start, result)
      })
      candidates <- dplyr::bind_rows(candidates)
      candidates$valid <- sensitivity_valid(candidates)
      candidates <- candidates[order(!candidates$valid, candidates$objective, na.last = TRUE), ]
      candidates$selected <- seq_len(nrow(candidates)) == 1L
      attempts[[length(attempts) + 1L]] <- cbind(metadata[rep(1L, nrow(candidates)), ], candidates)
      for (mode in c("known", "estimated")) {
        message("  uncertainty: ", mode)
        report <- tryCatch(calibration_report(fixture, operating, mode,
          if (mode == "estimated") candidates[1L, ] else NULL),
          error = function(e) list(diagnostics = data.frame(error = conditionMessage(e),
            optimizer_valid = FALSE, pd_hessian = NA, boundary = NA, regular = FALSE,
            sdreport_error = NA_character_)))
        meta <- cbind(metadata, sd_mode = mode)
        all_results[[length(all_results) + 1L]] <- lapply(report, function(table)
          if (nrow(table)) cbind(meta[rep(1L, nrow(table)), ], table) else NULL)
        gc(verbose = FALSE)
      }
      if (!is.null(checkpoint)) saveRDS(list(results = all_results,
        attempts = dplyr::bind_rows(attempts), operating_models = operating_models), checkpoint)
    }
  }
  tables <- lapply(c("diagnostics", "states", "state_metrics", "hyperparameters"), function(name)
    dplyr::bind_rows(lapply(all_results, `[[`, name)))
  names(tables) <- c("diagnostics", "states", "state_metrics", "hyperparameters")
  tables$attempts <- dplyr::bind_rows(attempts)
  summaries <- calibration_summaries(tables)
  list(configuration = list(replicates = replicates, years = years, seed = seed,
    mortality_sds = mortality_sds, starts = starts, design = design,
    initial = "proper-normal", R_sd = .2, F_sd = .15,
    session = capture.output(sessionInfo())), operating_models = operating_models,
    tables = tables, summaries = summaries)
}
