# Source benchmark.R, sensitivity.R, information.R, calibration.R and robustness.R.
# This pathway stores plain parameter/state snapshots across native lifetimes.
mode_parameter_layout <- function(names) {
  # Registry IDs change on reconstruction. Preserve the owner/field sequence.
  vapply(strsplit(names, ".", fixed = TRUE), function(x) {
    stopifnot(length(x) == 4L)
    paste(x[c(1L, 3L)], collapse = ".")
  }, character(1), USE.NAMES = FALSE)
}

mode_model_input <- function(fixture, operating, start_sd, composition_likelihood = NULL) {
  stopifnot(length(start_sd) == 1L, is.finite(start_sd), start_sd >= .001, start_sd <= 1)
  p <- fixture$parameters
  p$estimation_type[] <- "constant"
  rec <- p$module_name %in% "Recruitment"
  p$distribution[rec] <- NA_character_
  p$distribution_type[rec] <- NA_character_
  for (key in names(fixture$rows)) {
    idx <- fixture$rows[[key]]
    p$estimation_type[idx] <- "random_effects"
    p$value[idx] <- if (key == "R") .05 else log(.18)
  }
  idx <- fixture$rows$R
  p$distribution[idx] <- "Dnorm"
  p$distribution_type[idx] <- "process"
  sd <- which(rec & p$label %in% "log_sd")
  p$value[sd] <- log(start_sd)
  p$estimation_type[sd] <- "fixed_effects"
  p$distribution[sd] <- "Dnorm"
  p$distribution_type[sd] <- "process"
  walk <- list(type = "rw1", sd = start_sd, estimation_type = "fixed_effects", initial = fixture$initial)
  input <- initialize_fims(p, operating$data, recruitment_process = list(type = "iid"),
    fishing_process = list(fleet1 = walk), mortality_process = walk,
    composition_likelihood = composition_likelihood)
  list(input = input, parameters = p)
}

mode_fit_snapshot <- function(fixture, operating, start_sd = .2, composition_likelihood = NULL) {
  setup <- mode_model_input(fixture, operating, start_sd, composition_likelihood)
  input <- setup$input
  fixed_names <- names(get_parameter_names(as.list(input$parameters$p)))
  random_names <- names(get_random_names(as.list(input$parameters$re)))
  dispersion <- grepl(".log_concentration.", fixed_names, fixed = TRUE)
  lower <- ifelse(dispersion, log(.1), log(.001))
  upper <- ifelse(dispersion, log(1e5), 0)
  obj <- TMB::MakeADFun(data = list(), parameters = input$parameters, random = "re", DLL = "FIMS", silent = TRUE)
  opt <- nlminb(obj$par, obj$fn, obj$gr, lower = lower, upper = upper,
    control = list(iter.max = 300, eval.max = 600))
  value <- as.numeric(obj$fn(opt$par))
  gradient <- as.numeric(obj$gr(opt$par))
  initial_value <- value
  value <- as.numeric(obj$fn(opt$par))
  evaluation_drift <- value - initial_value
  joint <- as.numeric(obj$env$last.par)
  full <- obj$env$parList(par = joint)
  inner <- as.numeric(obj$env$f(joint, order = 1))[obj$env$random]
  projected <- sensitivity_projected_gradient(opt$par, gradient, lower, upper)
  snapshot <- list(p = as.numeric(full$p), re = as.numeric(full$re), joint = joint,
    fixed_layout = mode_parameter_layout(fixed_names), random_layout = mode_parameter_layout(random_names),
    random_indices = obj$env$random, objective = value,
    joint_objective = as.numeric(obj$env$f(joint, order = 0)),
    parameters = setup$parameters, fixture_parameters = fixture$parameters, initial = fixture$initial,
    truth = operating$truth, data = get_data(operating$data), start_sd = start_sd,
    composition_likelihood = composition_likelihood, lower = lower, upper = upper,
    optimizer_code = opt$convergence, optimizer_message = opt$message,
    optimizer_valid = abs(evaluation_drift) < 1e-5 && opt$convergence == 0 && max(abs(projected)) < 1e-3 && max(abs(inner)) < 1e-3,
    evaluation_drift = evaluation_drift,
    max_projected_gradient = max(abs(projected)), max_inner_gradient = max(abs(inner)),
    mode_scope = "local conditional solution; global optimality not established")
  list(snapshot = snapshot, point = data.frame(parameter = fixed_names, value = snapshot$p,
    lower = lower, upper = upper), diagnostics = data.frame(objective = value,
      optimizer_code = opt$convergence, optimizer_valid = snapshot$optimizer_valid,
      max_projected_gradient = snapshot$max_projected_gradient, max_inner_gradient = snapshot$max_inner_gradient,
      boundary = any(abs(snapshot$p - lower) < 1e-3 | abs(snapshot$p - upper) < 1e-3)))
}

mode_report_snapshot <- function(fixture, operating, snapshot, warm = TRUE,
                                  composition_likelihood = snapshot$composition_likelihood,
                                  tolerance = 1e-5, skip_delta_method = TRUE) {
  stopifnot(is.logical(warm), length(warm) == 1L, !is.na(warm), tolerance > 0)
  if (!identical(fixture$parameters, snapshot$fixture_parameters) || !identical(fixture$initial, snapshot$initial) ||
      !identical(operating$truth, snapshot$truth) || !identical(get_data(operating$data), snapshot$data) ||
      !identical(composition_likelihood, snapshot$composition_likelihood))
    stop("Mode snapshot does not match the model inputs or parameter layout")
  setup <- mode_model_input(fixture, operating, snapshot$start_sd, composition_likelihood)
  input <- setup$input
  fixed <- names(get_parameter_names(as.list(input$parameters$p)))
  random <- names(get_random_names(as.list(input$parameters$re)))
  if (!identical(setup$parameters, snapshot$parameters) ||
      !identical(get_data(operating$data), snapshot$data) ||
      !identical(composition_likelihood, snapshot$composition_likelihood) ||
      !identical(mode_parameter_layout(fixed), snapshot$fixed_layout) ||
      !identical(mode_parameter_layout(random), snapshot$random_layout))
    stop("Mode snapshot does not match the model inputs or parameter layout")
  if (length(snapshot$p) != length(input$parameters$p) ||
      length(snapshot$re) != length(input$parameters$re) ||
      length(snapshot$joint) != length(snapshot$p) + length(snapshot$re) ||
      any(!is.finite(c(snapshot$p, snapshot$re, snapshot$joint))))
    stop("Mode snapshot parameter lengths or values are invalid")
  input$parameters$p[] <- snapshot$p
  if (warm) input$parameters$re[] <- snapshot$re
  obj <- TMB::MakeADFun(data = list(), parameters = input$parameters, random = "re", DLL = "FIMS", silent = TRUE)
  stopifnot(identical(obj$env$random, snapshot$random_indices))
  value <- as.numeric(obj$fn(obj$par))
  before <- value - snapshot$objective
  joint_probe <- as.numeric(obj$env$f(snapshot$joint, order = 0)) - snapshot$joint_objective
  diagnostics <- data.frame(warm = warm, status = "before_report", objective_difference = before,
    joint_reference_difference = joint_probe, after_difference = NA_real_,
    optimizer_valid = snapshot$optimizer_valid, pd_hessian = NA, boundary =
      any(abs(snapshot$p - snapshot$lower) < 1e-3 | abs(snapshot$p - snapshot$upper) < 1e-3),
    max_mode_drift = NA_real_, mode_scope = snapshot$mode_scope)
  # An alternate stationary solution is reported, never quietly accepted.
  if (!is.finite(before) || abs(before) > tolerance || abs(joint_probe) > tolerance) {
    diagnostics$status <- "rebuild_mismatch"
    return(list(diagnostics = diagnostics, states = data.frame(), state_metrics = data.frame()))
  }
  sdr <- tryCatch(TMB::sdreport(obj, par.fixed = obj$par, skip.delta.method = skip_delta_method), error = identity)
  if (inherits(sdr, "error")) {
    diagnostics$status <- paste("sdreport_error", conditionMessage(sdr))
    return(list(diagnostics = diagnostics, states = data.frame(), state_metrics = data.frame()))
  }
  after <- as.numeric(obj$fn(obj$par)) - snapshot$objective
  diagnostics$after_difference <- after
  diagnostics$pd_hessian <- isTRUE(sdr$pdHess)
  diagnostics$max_mode_drift <- max(abs(as.numeric(sdr$par.random) - snapshot$re))
  if (!is.finite(after) || abs(after) > tolerance) {
    diagnostics$status <- "post_report_mismatch"
    return(list(diagnostics = diagnostics, states = data.frame(), state_metrics = data.frame()))
  }
  diagnostics$status <- "consistent_local_report"
  states <- do.call(rbind, lapply(c(R = "log_devs", F = "log_Fmort", M = "log_M"), function(label) {
    key <- names(c(R = "log_devs", F = "log_Fmort", M = "log_M"))[match(label, c("log_devs", "log_Fmort", "log_M"))]
    idx <- which(grepl(paste0(".", label, "."), random, fixed = TRUE))
    stopifnot(length(idx) == length(operating$truth[[key]]))
    data.frame(process = key, truth = operating$truth[[key]],
      estimate = as.numeric(sdr$par.random)[idx], se = sqrt(sdr$diag.cov.random)[idx])
  }))
  metrics <- do.call(rbind, lapply(c("R", "F", "M"), function(key) {
    rows <- states[states$process == key, ]
    cbind(process = key, calibration_intervals(rows$truth, rows$estimate, rows$se))
  }))
  list(diagnostics = diagnostics, states = states, state_metrics = metrics,
    fixed_covariance = sdr$cov.fixed, point = snapshot$p)
}
