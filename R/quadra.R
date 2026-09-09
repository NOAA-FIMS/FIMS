#' Evaluate an XPtr model with Quadra
#'
#' @param fixed,random Numeric fixed- and random-effect parameter vectors, in
#'   the order returned by [get_fixed()] and [get_random()].
#' @return A list containing the joint negative log likelihood and its gradient
#'   (fixed effects followed by random effects).
#' @details Call [CreateTMBModel()] after assembling or changing modules.
#'   Evaluation uses the joint likelihood, without a Laplace correction.
#' @export
quadra_evaluate <- function(fixed = get_fixed(), random = get_random()) {
  check_quadra_parameters(fixed, random)
  .Call("fims_call_quadra_evaluate", as.numeric(fixed), as.numeric(random),
    PACKAGE = "FIMS"
  )
}

check_quadra_parameters <- function(fixed, random) {
  if (is.null(.fims_registry[["model_handle"]])) {
    cli::cli_abort("Build the model with {.fn CreateTMBModel} before using Quadra.")
  }
  for (values in list(fixed, random)) {
    if (!is.numeric(values) || any(!is.finite(values))) {
      cli::cli_abort("Quadra parameters must be finite numeric vectors.")
    }
  }
}

#' Optimize an XPtr model with Quadra
#'
#' @inheritParams quadra_evaluate
#' @param method `"joint"` estimates fixed and random effects together;
#'   `"laplace"` integrates random effects using a Laplace approximation.
#' @param max_iterations Positive integer iteration limit.
#' @param gradient_tolerance Positive convergence tolerance.
#' @return Estimates and optimizer diagnostics. The objective and gradient
#'   correspond to the selected method.
#' @export
quadra_fit <- function(fixed = get_fixed(), random = get_random(),
                       method = c("joint", "laplace"),
                       max_iterations = 1000L, gradient_tolerance = 1e-5) {
  method <- match.arg(method)
  check_quadra_parameters(fixed, random)
  if (length(max_iterations) != 1L || !is.finite(max_iterations) ||
    max_iterations < 1 || max_iterations > .Machine$integer.max ||
    max_iterations != floor(max_iterations)) {
    cli::cli_abort("{.arg max_iterations} must be a positive integer.")
  }
  if (length(gradient_tolerance) != 1L || !is.finite(gradient_tolerance) ||
    gradient_tolerance <= 0) {
    cli::cli_abort("{.arg gradient_tolerance} must be one positive number.")
  }
  if (!length(c(fixed, random))) {
    cli::cli_abort("Quadra must have at least one parameter to optimize.")
  }
  .Call("fims_call_quadra_fit", as.numeric(fixed), as.numeric(random), method,
    as.integer(max_iterations), as.numeric(gradient_tolerance),
    PACKAGE = "FIMS"
  )
}

#' Joint parameter uncertainty from Quadra
#'
#' @inheritParams quadra_evaluate
#' @return A `quadra_sdreport` with the joint Hessian, covariance, parameter
#'   standard errors, gradient, and positive-definiteness diagnostic.
#' @details This inverts the joint Hessian. It is not the Hessian of the
#'   Laplace objective and does not include derived-quantity standard errors.
#' @export
quadra_sdreport <- function(fixed = get_fixed(), random = get_random()) {
  check_quadra_parameters(fixed, random)
  result <- .Call("fims_call_quadra_sdreport", as.numeric(fixed), as.numeric(random),
    PACKAGE = "FIMS"
  )
  fixed_names <- names(get_parameter_names(fixed))
  names(result$par.fixed) <- fixed_names
  names(result$par.random) <- names(get_random_names(random))
  dimnames(result$cov.fixed) <- list(fixed_names, fixed_names)
  class(result) <- c("quadra_sdreport", "list")
  result
}

#' Summarize Quadra parameter uncertainty
#' @param object A `quadra_sdreport`.
#' @param select Which estimates to return.
#' @param ... Unused.
#' @return A matrix with estimate and standard-error columns.
#' @export
summary.quadra_sdreport <- function(object,
                                    select = c("all", "fixed", "random", "report"),
                                    ...) {
  select <- match.arg(select)
  fixed <- cbind(
    Estimate = object$par.fixed,
    `Std. Error` = utils::head(object$std.error, length(object$par.fixed))
  )
  random <- cbind(
    Estimate = object$par.random,
    `Std. Error` = utils::tail(object$std.error, length(object$par.random))
  )
  empty <- matrix(numeric(), 0, 2,
    dimnames = list(NULL, c("Estimate", "Std. Error"))
  )
  switch(select,
    fixed = fixed,
    random = random,
    report = empty,
    all = rbind(fixed, random)
  )
}

# Internal adapter retaining FIMSFit's TMB reporting interface.
fit_fims_quadra <- function(input, get_sd, save_sd, optimize, control,
                            method, gradient_tolerance) {
  if (length(input$map)) {
    cli::cli_abort("Quadra fitting does not yet support parameter maps.")
  }
  if (!identical(attr(input, "model_handle"), .fims_registry[["model_handle"]]) &&
    !is.null(attr(input, "model_handle"))) {
    cli::cli_abort("This input belongs to a model that has been rebuilt.")
  }
  start <- Sys.time()
  result <- if (optimize) {
    quadra_fit(
      input$parameters$p, input$parameters$re, method,
      if (is.null(control$iter.max)) 1000L else control$iter.max,
      gradient_tolerance
    )
  } else {
    NULL
  }
  time_optimization <- Sys.time() - start
  if (optimize) {
    input$parameters <- list(p = result$par, re = result$random)
  }
  input$backend <- "quadra"
  input$quadra_method <- method
  # For joint fits, hold the fitted random effects at their joint estimates
  # when asking TMB for reports; do not reoptimize them with a Laplace objective.
  joint <- method == "joint"
  report_map <- if (joint && length(input$parameters$re)) {
    list(re = factor(rep(NA_integer_, length(input$parameters$re))))
  } else {
    NULL
  }
  obj <- TMB::MakeADFun(
    data = list(), parameters = input$parameters,
    map = report_map, random = if (joint) NULL else "re", DLL = "FIMS", silent = TRUE
  )
  if (!optimize) {
    return(FIMSFit(input, obj))
  }
  opt <- list(
    par = result$par, objective = result$objective,
    convergence = if (isTRUE(result$converged)) 0L else 1L,
    iterations = result$iterations, message = result$message
  )
  obj$fn(opt$par)
  start_sd <- Sys.time()
  sd <- if (get_sd) {
    if (joint) quadra_sdreport(result$par, result$random) else TMB::sdreport(obj)
  } else {
    list()
  }
  time_sdreport <- Sys.time() - start_sd
  fit <- FIMSFit(input, obj, opt, if (save_sd) sd else list(),
    run_time = c(
      time_optimization = time_optimization,
      time_sdreport = time_sdreport, time_total = Sys.time() - start
    )
  )
  fit@gradient <- utils::head(as.numeric(result$gradient), length(result$par))
  fit@max_gradient <- if (length(result$gradient)) max(abs(result$gradient)) else 0
  fit
}

#' Evaluate the joint objective without automatic differentiation
#'
#' @inheritParams quadra_evaluate
#' @return The scalar joint negative log likelihood.
#' @details Uses the model templated on double, without a derivative tape.
#'   Call [CreateTMBModel()] after assembling or changing modules.
#' @export
quadra_objective <- function(fixed = get_fixed(), random = get_random()) {
  check_quadra_parameters(fixed, random)
  .Call("fims_call_quadra_objective", as.numeric(fixed), as.numeric(random),
    PACKAGE = "FIMS"
  )
}

#' Evaluate the joint gradient with Quadra
#'
#' @inheritParams quadra_evaluate
#' @return A numeric gradient, fixed effects followed by random effects.
#' @details Uses the Quadra first-order tape. The forward value required for
#'   differentiation is computed internally; only the gradient is returned.
#' @export
quadra_gradient <- function(fixed = get_fixed(), random = get_random()) {
  quadra_evaluate(fixed, random)$gradient
}
