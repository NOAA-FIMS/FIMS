# Source the existing recovery modules through mode-handoff.R first.
# Benchmark only: retain the historical drivers and their source fingerprints.
dm_composition_spec <- function(operating, concentration, estimated = FALSE) {
  if (is.infinite(concentration)) return(NULL)
  raw <- get_data(operating$data)
  result <- list()
  for (fleet in unique(raw$fleet[raw$type %in% c("age_comp", "length_comp")])) {
    types <- unique(raw$type[raw$fleet == fleet & raw$type %in% c("age_comp", "length_comp")])
    result[[fleet]] <- stats::setNames(lapply(types, function(type)
      list(type = "dirichlet_multinomial", concentration = concentration,
        estimation_type = if (estimated) "fixed_effects" else "constant")), types)
  }
  result
}

# Same snapshot contract as mode_fit_snapshot, with explicit alternate latent starts
# and parameter ownership. No public fit or persistence API is changed.
dm_fit_snapshot <- function(fixture, operating, start_sd, shifted, composition = NULL) {
  setup <- mode_model_input(fixture, operating, start_sd, composition)
  input <- setup$input
  fixed <- names(get_parameter_names(as.list(input$parameters$p)))
  random <- names(get_random_names(as.list(input$parameters$re)))
  dispersion <- grepl(".log_concentration.", fixed, fixed = TRUE)
  lower <- ifelse(dispersion, log(.1), log(.001))
  upper <- ifelse(dispersion, log(1e5), 0)
  densities <- list(R = attr(input, "gaussian_recruitment"),
    F = attr(input, "fishing_processes")$fleet1, M = attr(input, "gaussian_mortality"))
  positions <- vapply(densities, function(density) {
    idx <- which(startsWith(fixed, paste0("gaussian_process.", density$get_id(), ".log_sd.")))
    stopifnot(length(idx) == 1L); idx
  }, integer(1))
  if (shifted) input$parameters$re[] <- input$parameters$re + .15 * sin(seq_along(input$parameters$re))
  obj <- TMB::MakeADFun(data = list(), parameters = input$parameters, random = "re", DLL = "FIMS", silent = TRUE)
  opt <- nlminb(obj$par, obj$fn, obj$gr, lower = lower, upper = upper,
    control = list(iter.max = 300, eval.max = 600))
  first <- as.numeric(obj$fn(opt$par))
  gradient <- as.numeric(obj$gr(opt$par))
  value <- as.numeric(obj$fn(opt$par))
  joint <- as.numeric(obj$env$last.par)
  full <- obj$env$parList(par = joint)
  inner <- as.numeric(obj$env$f(joint, order = 1))[obj$env$random]
  projected <- sensitivity_projected_gradient(opt$par, gradient, lower, upper)
  valid <- all(is.finite(c(value, first, projected, inner))) && abs(value - first) < 1e-5 &&
    opt$convergence == 0 && max(abs(projected)) < 1e-3 && max(abs(inner)) < 1e-3
  snapshot <- list(p = as.numeric(full$p), re = as.numeric(full$re), joint = joint,
    fixed_layout = mode_parameter_layout(fixed), random_layout = mode_parameter_layout(random),
    random_indices = obj$env$random, objective = value,
    joint_objective = as.numeric(obj$env$f(joint, order = 0)),
    parameters = setup$parameters, fixture_parameters = fixture$parameters, initial = fixture$initial,
    truth = operating$truth, data = get_data(operating$data), start_sd = start_sd,
    composition_likelihood = composition, lower = lower, upper = upper,
    optimizer_code = opt$convergence, optimizer_message = opt$message, optimizer_valid = valid,
    evaluation_drift = value - first, max_projected_gradient = max(abs(projected)),
    max_inner_gradient = max(abs(inner)), mode_scope = "local conditional solution; global optimality not established")
  owner <- rep(NA_character_, length(fixed)); owner[positions] <- names(positions)
  compositions <- attr(input, "composition_distributions")
  for (type in names(compositions)) for (fleet in names(compositions[[type]])) {
    density <- compositions[[type]][[fleet]]
    if (is.null(density)) next
    idx <- which(vapply(strsplit(fixed, ".", fixed = TRUE), function(parts)
      parts[2] == as.character(density$get_id()) && parts[3] == "log_concentration", logical(1)))
    owner[idx] <- paste("concentration", fleet, type, sep = "/")
  }
  stopifnot(!anyNA(owner))
  list(snapshot = snapshot, point = data.frame(parameter = owner, value = snapshot$p,
    lower = lower, upper = upper), diagnostics = data.frame(objective = value,
      optimizer_code = opt$convergence, optimizer_valid = valid,
      max_projected_gradient = snapshot$max_projected_gradient,
      max_inner_gradient = snapshot$max_inner_gradient,
      boundary = any(abs(snapshot$p - lower) < 1e-3 | abs(snapshot$p - upper) < 1e-3)))
}

# Summarize replicate-level metrics: correlated age/year cells are not independent
# Monte Carlo replicates. Missing/failed reports remain in planned denominators.
dm_mean_mcse <- function(x) {
  x <- x[is.finite(x)]
  c(n = length(x), mean = if (length(x)) mean(x) else NA_real_,
    mcse = if (length(x) > 1L) sd(x) / sqrt(length(x)) else NA_real_)
}

dm_validation_summaries <- function(tables) {
  reports <- tables$reports
  fits <- dplyr::bind_rows(lapply(split(reports, interaction(reports$scenario, reports$case, reports$warm, drop = TRUE)), function(x)
    data.frame(scenario = x$scenario[1], case = x$case[1], warm = x$warm[1], planned = nrow(x),
      optimizer_valid = sum(x$optimizer_valid %in% TRUE), consistent = sum(x$status == "consistent_local_report"),
      eligible = sum(x$eligible), regular = sum(x$regular), boundary = sum(x$boundary %in% TRUE))))
  metrics <- tables$metrics
  states <- dplyr::bind_rows(lapply(split(metrics, interaction(metrics$scenario, metrics$case, metrics$process, drop = TRUE)), function(x)
    dplyr::bind_rows(lapply(c("eligible", "regular"), function(stratum) {
      z <- x[x[[stratum]], , drop = FALSE]
      dplyr::bind_rows(lapply(c("coverage", "rmse", "bias", "mean_width"), function(metric) {
        s <- dm_mean_mcse(z[[metric]])
        data.frame(scenario = x$scenario[1], case = x$case[1], process = x$process[1],
          stratum = stratum, metric = metric, planned = nrow(x), n = s[["n"]],
          mean = s[["mean"]], mcse = s[["mcse"]])
      }))
    }))))
  # Paired contrasts require eligible reports on BOTH models of a dataset.
  paired <- list()
  for (scenario in unique(metrics$scenario)) for (process in unique(metrics$process)) {
    base <- metrics[metrics$scenario == scenario & metrics$process == process & metrics$case == "multinomial" & metrics$eligible, ]
    for (case in c("known_dm", "estimated_dm")) {
      other <- metrics[metrics$scenario == scenario & metrics$process == process & metrics$case == case & metrics$eligible, ]
      both <- merge(base, other, by = "replicate", suffixes = c("_base", "_dm"))
      for (metric in c("coverage", "rmse")) {
        s <- dm_mean_mcse(both[[paste0(metric, "_dm")]] - both[[paste0(metric, "_base")]])
        paired[[length(paired) + 1L]] <- data.frame(scenario = scenario, process = process,
          comparison = paste(case, "minus multinomial"), metric = metric,
          n = s[["n"]], mean_difference = s[["mean"]], mcse = s[["mcse"]])
      }
    }
  }
  points <- tables$points
  hyper <- dplyr::bind_rows(lapply(split(points, interaction(points$scenario, points$case, points$parameter, drop = TRUE)), function(x)
    dplyr::bind_rows(lapply(c("eligible", "regular"), function(stratum) {
      z <- x[x[[stratum]], , drop = FALSE]
      finite_truth <- is.finite(z$truth)
      error <- z$value[finite_truth] - log(z$truth[finite_truth])
      est <- dm_mean_mcse(z$estimate); bias <- dm_mean_mcse(error)
      cover <- dm_mean_mcse(as.numeric(z$covered[finite_truth]))
      data.frame(scenario = x$scenario[1], case = x$case[1], parameter = x$parameter[1],
        stratum = stratum, planned = nrow(x), n = est[["n"]], mean_estimate = est[["mean"]],
        estimate_mcse = est[["mcse"]], mean_log_error = bias[["mean"]], log_error_mcse = bias[["mcse"]],
        log_rmse = if (length(error)) sqrt(mean(error^2)) else NA_real_,
        coverage_n = cover[["n"]], coverage = cover[["mean"]], coverage_mcse = cover[["mcse"]],
        boundary = sum(z$boundary))
    }))))
  list(fit_summary = fits, state_summary = states, paired_summary = dplyr::bind_rows(paired), hyperparameter_summary = hyper)
}

run_dm_validation <- function(replicates = 5L, years = 18L, seed = 4301L, checkpoint = NULL) {
  stopifnot(replicates >= 1L, replicates == as.integer(replicates))
  fixture <- recovery_fixture(years)
  starts <- expand.grid(start_sd = c(.05, .2), shifted = c(FALSE, TRUE))
  design <- robustness_design()
  attempts <- reports <- metrics <- points <- snapshots <- inputs <- list()
  for (replicate in seq_len(replicates)) {
    operating <- robustness_operating_models(fixture, seed + replicate - 1L)
    inputs[[replicate]] <- list(truth = operating$truth, data = lapply(operating$models, function(x) get_data(x$data)))
    for (i in seq_len(nrow(design))) {
      scenario <- design$scenario[i]; truth_kappa <- design$concentration[i]
      model <- operating$models[[scenario]]
      cases <- if (is.finite(truth_kappa)) c("multinomial", "known_dm", "estimated_dm") else c("multinomial", "estimated_dm")
      for (case in cases) {
        message(sprintf("replicate=%d/%d scenario=%s case=%s", replicate, replicates, scenario, case))
        meta <- data.frame(replicate = replicate, seed = seed + replicate - 1L, scenario = scenario, case = case)
        composition <- if (case == "multinomial") NULL else dm_composition_spec(model,
          if (case == "known_dm") truth_kappa else 2000, case == "estimated_dm")
        candidates <- list(); diagnostics <- list()
        for (j in seq_len(nrow(starts))) {
          warnings <- character(); started <- proc.time()[["elapsed"]]
          fit <- withCallingHandlers(tryCatch(dm_fit_snapshot(fixture, model, starts$start_sd[j], starts$shifted[j], composition),
            error = identity), warning = function(w) {warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning")})
          candidates[j] <- list(if (inherits(fit, "error")) NULL else fit)
          row <- if (inherits(fit, "error")) data.frame(objective = NA_real_, optimizer_valid = FALSE,
            error = conditionMessage(fit)) else fit$diagnostics
          diagnostics[[j]] <- cbind(meta, start_id = j, starts[j, ], row,
            warnings = paste(unique(warnings), collapse = " | "), elapsed_seconds = proc.time()[["elapsed"]] - started)
          gc(verbose = FALSE)
        }
        attempt <- dplyr::bind_rows(diagnostics)
        rank <- order(!attempt$optimizer_valid, attempt$objective, na.last = TRUE)
        selected <- rank[1L]; attempt$selected <- seq_len(nrow(attempt)) == selected
        attempts[[length(attempts) + 1L]] <- attempt
        finite <- is.finite(attempt$objective) & attempt$optimizer_valid
        spread <- if (any(finite)) diff(range(attempt$objective[finite])) else NA_real_
        fit <- candidates[[selected]]
        for (warm in c(FALSE, TRUE)) {
          warnings <- character()
          report <- withCallingHandlers(tryCatch({
            if (is.null(fit)) stop("No fitted snapshot")
            mode_report_snapshot(fixture, model, fit$snapshot, warm)
          }, error = identity), warning = function(w) {warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning")})
          if (inherits(report, "error")) report <- list(diagnostics = data.frame(warm = warm,
            status = "report_error", error = conditionMessage(report), optimizer_valid = FALSE,
            pd_hessian = FALSE, boundary = NA), states = data.frame(), state_metrics = data.frame())
          d <- report$diagnostics
          d$eligible <- isTRUE(d$optimizer_valid) && isTRUE(d$pd_hessian) &&
            d$status == "consistent_local_report" && nrow(report$states) > 0 &&
            all(is.finite(report$states$se) & report$states$se > 0)
          d$regular <- d$eligible && !isTRUE(d$boundary)
          reports[[length(reports) + 1L]] <- cbind(meta, selected_start = selected, valid_start_objective_spread = spread,
            warnings = paste(unique(warnings), collapse = " | "), d)
          if (warm) {
            # A planned row for every process, even if uncertainty was withheld.
            m <- if (nrow(report$state_metrics)) merge(data.frame(process = c("R", "F", "M")), report$state_metrics, by = "process", all.x = TRUE) else
              data.frame(process = c("R", "F", "M"), coverage = NA_real_, rmse = NA_real_, bias = NA_real_, mean_width = NA_real_)
            metrics[[length(metrics) + 1L]] <- cbind(meta, eligible = d$eligible, regular = d$regular, m)
            if (!is.null(fit)) {
              p <- fit$point
              p$truth <- ifelse(grepl("concentration", p$parameter), truth_kappa, fixture$sigma[match(p$parameter, names(fixture$sigma))])
              p$estimate <- exp(p$value); p$boundary <- abs(p$value - p$lower) < 1e-3 | abs(p$value - p$upper) < 1e-3
              p$log_se <- if (!is.null(report$fixed_covariance)) sqrt(diag(report$fixed_covariance)) else NA_real_
              p$covered <- is.finite(p$truth) & is.finite(p$log_se) & p$log_se > 0 & abs(p$value - log(p$truth)) <= qnorm(.975) * p$log_se
              p$covered[!is.finite(p$truth) | !is.finite(p$log_se)] <- NA
              points[[length(points) + 1L]] <- cbind(meta, eligible = d$eligible, regular = d$regular, p)
              snapshots[[paste(replicate, scenario, case, sep = "/")]] <- fit$snapshot
            }
          }
        }
        if (!is.null(checkpoint)) saveRDS(list(inputs = inputs, snapshots = snapshots,
          attempts = attempts, reports = reports, metrics = metrics, points = points), checkpoint)
      }
    }
  }
  tables <- lapply(list(attempts = attempts, reports = reports, metrics = metrics, points = points), dplyr::bind_rows)
  list(configuration = list(replicates = replicates, years = years, seed = seed, design = design, starts = starts,
    estimated_concentration_start = 2000, concentration_bounds = c(.1, 1e5), process_sd_bounds = c(.001, 1),
    true_process_sd = fixture$sigma, latent_shift = ".15 * sin(index), all random states",
    concentration_ownership = "independent per fleet/composition type, constant across years",
    interval_scope = "local fitted log-states, not forecasts or derived quantities"),
    inputs = inputs, snapshots = snapshots, tables = tables, summaries = dm_validation_summaries(tables))
}
