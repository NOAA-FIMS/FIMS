# Source after library(FIMS). This is a scientific benchmark, not a public API.
recovery_fixture <- function(years = 6L) {
  stopifnot(length(years) == 1L, is.finite(years), years >= 3L,
            years <= 30L, years == as.integer(years))
  raw <- subset(FIMS::data_big, is.na(timing) | timing <= years | type == "weight_at_age")
  raw$timing[raw$type == "weight_at_age"] <- NA_integer_
  # Controlled sampling design rather than the near-exact demonstration catches.
  raw$uncertainty[raw$type == "catch"] <- "~ dlnorm(meanlog = log_catch_expected, sdlog = 0.1)"
  raw$uncertainty[raw$type == "index"] <- "~ dlnorm(meanlog = log_index_expected, sdlog = 0.2)"
  raw$uncertainty[raw$type == "age_comp"] <- "~ dmultinom(prob = agecomp_proportion, size = 200)"
  raw$uncertainty[raw$type == "length_comp"] <- "~ dmultinom(prob = lengthcomp_proportion, size = 200)"
  data <- FIMSFrame(unique(raw))
  parameters <- suppressMessages(setup_default_parameters(data))
  parameters$estimation_type[] <- "constant"
  rows <- list(R = which(parameters$module_name %in% "Recruitment" & parameters$label %in% "log_devs"),
               F = which(parameters$module_name %in% "Fleet" & parameters$fleet %in% "fleet1" &
                           parameters$label %in% "log_Fmort"),
               M = which(parameters$module_name %in% "Population" & parameters$label %in% "log_M"))
  parameters$value[rows$R] <- 0
  parameters$value[rows$F] <- log(0.2)
  parameters$value[rows$M] <- log(0.2)
  parameters$value[parameters$module_name %in% "Recruitment" & parameters$label %in% "log_sd"] <- log(0.2)
  parameters$estimation_type[unlist(rows)] <- "random_effects"
  list(data = data, parameters = parameters, rows = rows, sigma = c(R = 0.2, F = 0.15, M = 0.05),
       initial = list(type = "normal", mean = log(0.2), sd = 0.25))
}

recovery_operating_model <- function(fixture, seed) {
  walk <- function(sd) list(type = "rw1", sd = sd, estimation_type = "constant", initial = fixture$initial)
  simulation <- simulate_fims(fixture$parameters, fixture$data,
    list(type = "iid"), seed = seed, observation_error = TRUE,
    fishing_process = list(fleet1 = walk(fixture$sigma[["F"]])),
    mortality_process = walk(fixture$sigma[["M"]]))
  draw <- simulation$replicates[[1L]]
  raw <- simulation$configuration$data
  idx <- raw$type %in% c("catch", "index", "age_comp", "length_comp")
  stopifnot(sum(idx) == nrow(draw$observations), all(is.finite(draw$observations$simulated)))
  # Preserve dimensions and uncertainty formulas; replace only observations.
  raw$observed[idx] <- draw$observations$simulated
  list(data = FIMSFrame(raw), truth = list(R = draw$recruitment$value,
    F = draw$fishing_mortality$log_Fmort, M = draw$natural_mortality$log_M),
    simulation = simulation)
}

recovery_design <- function() {
  design <- expand.grid(processes = c("R", "F", "M", "R_F_M"),
    initialization = c("fixed", "normal"), sd_mode = c("known", "estimated"),
    stringsAsFactors = FALSE)
  design[design$processes != "R" | design$initialization == "fixed", , drop = FALSE]
}

recovery_fit_input <- function(fixture, operating, design) {
  parameters <- fixture$parameters
  parameters$estimation_type[] <- "constant"
  rec <- parameters$module_name %in% "Recruitment"
  parameters$distribution[rec] <- NA_character_
  parameters$distribution_type[rec] <- NA_character_
  active <- strsplit(design$processes, "_", fixed = TRUE)[[1L]]
  initial <- if (design$initialization == "fixed") list(type = "fixed") else fixture$initial
  role <- if (design$sd_mode == "estimated") "fixed_effects" else "constant"
  walk <- function(key) list(type = "rw1",
    sd = if (design$sd_mode == "known") fixture$sigma[[key]] else 0.1,
    estimation_type = role, initial = initial)
  for (key in names(fixture$rows)) {
    idx <- fixture$rows[[key]]
    parameters$value[idx] <- operating$truth[[key]]
    if (key %in% active) {
      parameters$estimation_type[idx] <- "random_effects"
      # Start away from truth. Only oracle-conditioned states retain true values.
      parameters$value[idx] <- if (key == "R") 0.05 else log(0.18)
      if (key != "R" && design$initialization == "fixed") {
        first <- parameters$timing[idx] == get_start_year(fixture$data)
        parameters$estimation_type[idx[first]] <- "constant"
        parameters$value[idx[first]] <- operating$truth[[key]][first]
      }
    }
  }
  if ("R" %in% active) {
    states <- fixture$rows$R
    parameters$distribution_type[states] <- "process"
    parameters$distribution[states] <- "Dnorm"
    sd <- which(rec & parameters$label %in% "log_sd")
    parameters$value[sd] <- log(if (design$sd_mode == "known") fixture$sigma[["R"]] else 0.1)
    parameters$estimation_type[sd] <- role
    parameters$distribution_type[sd] <- "process"
    parameters$distribution[sd] <- "Dnorm"
  }
  input <- initialize_fims(parameters, operating$data,
    recruitment_process = if ("R" %in% active) list(type = "iid") else NULL,
    fishing_process = if ("F" %in% active) list(fleet1 = walk("F")) else NULL,
    mortality_process = if ("M" %in% active) walk("M") else NULL)
  list(input = input, parameters = parameters, active = active)
}

recovery_metrics <- function(truth, estimate, se, level = 0.95) {
  stopifnot(length(truth) == length(estimate), length(se) == length(truth))
  z <- qnorm((1 + level) / 2)
  usable <- is.finite(truth) & is.finite(estimate)
  intervals <- usable & is.finite(se) & se > 0
  data.frame(n = length(truth), n_estimates = sum(usable), n_intervals = sum(intervals),
    bias = if (any(usable)) mean(estimate[usable] - truth[usable]) else NA_real_,
    rmse = if (any(usable)) sqrt(mean((estimate[usable] - truth[usable])^2)) else NA_real_,
    coverage = if (any(intervals)) mean(abs(estimate[intervals] - truth[intervals]) <= z * se[intervals]) else NA_real_,
    mean_se = if (any(intervals)) mean(se[intervals]) else NA_real_)
}

# Laplace/Gaussian marginal summaries on the log scale; no rounding via JSON.
recovery_fit <- function(fixture, operating, design) {
  warnings <- character()
  started <- proc.time()[["elapsed"]]
  withCallingHandlers({
    setup <- recovery_fit_input(fixture, operating, design)
    obj <- TMB::MakeADFun(data = list(), parameters = setup$input$parameters,
      map = setup$input$map, random = "re", DLL = "FIMS", silent = TRUE)
    fixed_names <- names(get_parameter_names(as.list(setup$input$parameters$p)))
    random_names <- names(get_random_names(as.list(setup$input$parameters$re)))
    # Only innovation SDs are outer parameters. Bounds expose, rather than hide,
    # variance collapse; estimates near either limit are marked in diagnostics.
    stopifnot(length(fixed_names) == length(obj$par), all(grepl("log_sd", fixed_names)))
    opt <- if (length(obj$par)) nlminb(obj$par, obj$fn, obj$gr,
      lower = log(0.001), upper = log(1), control = list(iter.max = 300, eval.max = 600)) else
        list(par = obj$par, objective = obj$fn(obj$par), convergence = 0L,
             message = "No outer parameters", iterations = 0L)
    value <- obj$fn(opt$par)
    gradient <- as.numeric(obj$gr(opt$par))
    par <- obj$env$last.par.best
    # The no-outer-parameter case still needs an inner-mode stationarity check.
    inner_gradient <- as.numeric(obj$env$f(par, order = 1))[obj$env$random]
    random <- par[obj$env$random]
    sd_error <- NA_character_
    sdr <- tryCatch(TMB::sdreport(obj, par.fixed = opt$par, getJointPrecision = TRUE),
      error = function(e) {sd_error <<- conditionMessage(e); NULL})
    se <- if (is.null(sdr) || is.null(sdr$diag.cov.random)) rep(NA_real_, length(random)) else
      sqrt(sdr$diag.cov.random)
    # Full joint precision is in original parameter order (TMB sdreport source).
    covariance <- if (!is.null(sdr$jointPrecision)) tryCatch(
      solve(as.matrix(sdr$jointPrecision)), error = function(e) NULL) else NULL
    correlation <- if (!is.null(covariance) && all(diag(covariance) > 0))
      cov2cor(covariance)[obj$env$random, obj$env$random, drop = FALSE] else NULL
    labels <- c(R = "log_devs", F = "log_Fmort", M = "log_M")
    group <- vapply(random_names, function(name) {
      hits <- names(labels)[vapply(labels, function(label) grepl(paste0(".", label, "."), name, fixed = TRUE), logical(1))]
      stopifnot(length(hits) == 1L); hits
    }, character(1))
    states <- do.call(rbind, lapply(setup$active, function(key) {
      idx <- fixture$rows[[key]]
      estimated <- setup$parameters$estimation_type[idx] == "random_effects"
      selected <- which(group == key)
      stopifnot(length(selected) == sum(estimated))
      data.frame(process = key, timing = setup$parameters$timing[idx][estimated],
        age = setup$parameters$age[idx][estimated], truth = operating$truth[[key]][estimated],
        estimate = as.numeric(random[selected]), se = se[selected])
    }))
    state_metrics <- do.call(rbind, lapply(setup$active, function(key) {
      rows <- states[states$process == key, ]
      cbind(process = key, recovery_metrics(rows$truth, rows$estimate, rows$se))
    }))
    # Hyperparameter ownership uses the actual instantiated process IDs.
    hypers <- data.frame()
    for (key in setup$active) if (design$sd_mode == "estimated") {
      density <- if (key == "R") attr(setup$input, "gaussian_recruitment") else if (key == "M")
        attr(setup$input, "gaussian_mortality") else attr(setup$input, "fishing_processes")$fleet1
      position <- which(startsWith(fixed_names, paste0("gaussian_process.", density$get_id(), ".log_sd.")))
      stopifnot(length(position) == 1L)
      fixed_se <- if (is.null(sdr)) NA_real_ else sqrt(diag(sdr$cov.fixed))[position]
      hypers <- rbind(hypers, data.frame(process = key, truth = fixture$sigma[[key]],
        estimate = exp(opt$par[position]), log_estimate = opt$par[position], log_se = fixed_se,
        boundary = abs(opt$par[position] - log(0.001)) < 1e-3 || abs(opt$par[position]) < 1e-3,
        covered = if (is.finite(fixed_se) && fixed_se > 0)
          abs(opt$par[position] - log(fixture$sigma[[key]])) <= qnorm(0.975) * fixed_se else NA))
    }
    pairs <- combn(c("R", "F", "M"), 2, simplify = FALSE)
    correlations <- do.call(rbind, lapply(pairs, function(pair) {
      block <- if (!is.null(correlation)) correlation[group == pair[1], group == pair[2], drop = FALSE] else numeric()
      data.frame(pair = paste(pair, collapse = "_"),
        max_abs_correlation = if (length(block) && all(is.finite(block))) max(abs(block)) else NA_real_)
    }))
    diagnostics <- data.frame(convergence = opt$convergence, objective = value,
      max_outer_gradient = if (length(gradient)) max(abs(gradient)) else 0,
      max_inner_gradient = max(abs(inner_gradient)),
      error = NA_character_, sd_boundary = nrow(hypers) > 0L && any(hypers$boundary),
      pd_hessian = !is.null(sdr) && isTRUE(sdr$pdHess),
      sdreport_error = sd_error, n_fixed = length(obj$par), n_random = length(random),
      elapsed_seconds = proc.time()[["elapsed"]] - started,
      optimizer_message = opt$message, warnings = paste(unique(warnings), collapse = " | "))
    list(diagnostics = diagnostics, states = states, state_metrics = state_metrics,
         hyperparameters = hypers, correlations = correlations)
  }, warning = function(w) {warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning")})
}

run_recovery_benchmark <- function(replicates = 3L, years = 6L, seed = 4301L,
                                    design = recovery_design()) {
  stopifnot(replicates >= 1L, replicates == as.integer(replicates),
            seed >= 0, seed + replicates < .Machine$integer.max)
  fixture <- recovery_fixture(years)
  results <- list(); truth <- list()
  for (replicate in seq_len(replicates)) {
    operating <- recovery_operating_model(fixture, seed + replicate - 1L)
    truth[[replicate]] <- list(states = operating$truth, data = get_data(operating$data),
                               rng = operating$simulation$rng)
    for (i in seq_len(nrow(design))) {
      case <- design[i, , drop = FALSE]
      message(sprintf("replicate %d/%d: %s, %s initialization, %s SD", replicate,
        replicates, case$processes, case$initialization, case$sd_mode))
      started <- proc.time()[["elapsed"]]
      fit <- tryCatch(recovery_fit(fixture, operating, case), error = function(e)
        list(diagnostics = data.frame(error = conditionMessage(e),
          elapsed_seconds = proc.time()[["elapsed"]] - started)))
      metadata <- cbind(replicate = replicate, seed = seed + replicate - 1L, case)
      results[[length(results) + 1L]] <- lapply(fit, function(table)
        if (nrow(table)) cbind(metadata[rep(1L, nrow(table)), , drop = FALSE], table) else NULL)
      # Keep only plain snapshots across native model lifetimes; no manual FreeADFun.
      gc(verbose = FALSE)
    }
  }
  tables <- lapply(c("diagnostics", "states", "state_metrics", "hyperparameters", "correlations"),
    function(name) dplyr::bind_rows(lapply(results, `[[`, name)))
  names(tables) <- c("diagnostics", "states", "state_metrics", "hyperparameters", "correlations")
  list(configuration = list(replicates = replicates, years = years, seed = seed,
    design = design, sigma = fixture$sigma, initial = fixture$initial,
    parameters = fixture$parameters, RNG_kind = RNGkind(),
    package_version = as.character(packageVersion("FIMS")), TMB_version = as.character(packageVersion("TMB")),
    session = capture.output(sessionInfo())), truth = truth, tables = tables)
}

recovery_summaries <- function(result) {
  diagnostics <- result$tables$diagnostics
  for (name in c("error", "convergence", "max_outer_gradient", "max_inner_gradient", "pd_hessian", "sd_boundary"))
    if (!name %in% names(diagnostics)) diagnostics[[name]] <- NA
  diagnostics$qualified <- with(diagnostics,
    is.na(error) & !is.na(convergence) & convergence == 0 &
      is.finite(max_outer_gradient) & max_outer_gradient < 1e-3 &
      is.finite(max_inner_gradient) & max_inner_gradient < 1e-3 &
      !is.na(pd_hessian) & pd_hessian & !is.na(sd_boundary) & !sd_boundary)
  keys <- c("processes", "initialization", "sd_mode")
  status <- diagnostics |>
    dplyr::group_by(dplyr::across(dplyr::all_of(keys))) |>
    dplyr::summarise(planned_fits = dplyr::n(), errors = sum(!is.na(.data$error)),
      qualified_fits = sum(.data$qualified), boundary_fits = sum(.data$sd_boundary, na.rm = TRUE),
      median_seconds = median(.data$elapsed_seconds), .groups = "drop")
  tag <- diagnostics[c("replicate", keys, "qualified")]
  state <- result$tables$state_metrics
  if (nrow(state)) state <- state |>
    dplyr::left_join(tag, by = c("replicate", keys)) |>
    dplyr::group_by(dplyr::across(dplyr::all_of(c(keys, "process", "qualified")))) |>
    dplyr::summarise(fits = dplyr::n(), interval_fits = sum(is.finite(.data$coverage)),
      mean_bias = mean(.data$bias), mean_rmse = mean(.data$rmse),
      mean_pointwise_coverage = if (any(is.finite(.data$coverage))) mean(.data$coverage, na.rm = TRUE) else NA_real_,
      # Across-replicate SE, not a binomial calculation treating cells as independent.
      coverage_mcse = if (sum(is.finite(.data$coverage)) > 1L)
        sd(.data$coverage, na.rm = TRUE) / sqrt(sum(is.finite(.data$coverage))) else NA_real_,
      mean_se = if (any(is.finite(.data$mean_se))) mean(.data$mean_se, na.rm = TRUE) else NA_real_,
      .groups = "drop")
  hyper <- result$tables$hyperparameters
  if (nrow(hyper)) hyper <- hyper |>
    dplyr::left_join(tag, by = c("replicate", keys)) |>
    dplyr::group_by(dplyr::across(dplyr::all_of(c(keys, "process", "qualified")))) |>
    dplyr::summarise(fits = dplyr::n(), boundary_fits = sum(.data$boundary),
      mean_sd = mean(.data$estimate), bias = mean(.data$estimate - .data$truth),
      rmse = sqrt(mean((.data$estimate - .data$truth)^2)),
      interval_fits = sum(!is.na(.data$covered)),
      coverage = if (any(!is.na(.data$covered))) mean(.data$covered, na.rm = TRUE) else NA_real_,
      .groups = "drop")
  list(diagnostics = diagnostics, fit_summary = status, state_summary = state, hyperparameter_summary = hyper)
}
