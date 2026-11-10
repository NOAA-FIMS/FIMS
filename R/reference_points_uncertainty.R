#' Propagate parameter uncertainty into equilibrium reference points
#'
#' Draw correlated fixed effects from the multivariate normal approximation in
#' a fitted TMB report, then calculate reference points for each draw.
#'
#' @param sdreport A report from [get_sdreport()] containing `par.fixed`,
#'   `cov.fixed`, and `pdHess = TRUE`.
#' @param input_fn Function taking a named parameter vector on the estimation
#'   scale and returning an input list for [calculate_reference_points()].
#'   Apply transformations (for example, exp for log R0) and rebuild all affected
#'   biology, selectivity, fleet shares, and recruitment baseline in this function.
#'   It must not mutate a live model. Parameters absent from the vector are held
#'   fixed. Conditional random effects and process variation are not sampled.
#' @param n_draws Positive integer number of draws, at least two.
#' @param level Central interval probability, strictly between zero and one.
#' @param ... Solver arguments passed to [calculate_reference_points()].
#' @return A list containing `estimate` at the fitted parameters, `summary`
#'   (median and equal-tail bounds conditional on converged solutions), named
#'   `parameter_draws`, `draws` with solver status and collapse flags, and `errors`
#'   with one row per failed draw. Summary rows report successful and failed
#'   counts for each quantity. Collapsed but converged equilibria are included.
#' @details
#' These are approximate parameter uncertainty intervals, not prediction
#' intervals. They rely on the local normal approximation being appropriate.
#' Parameter correlations are preserved; covariance must be positive definite.
#' Use [set.seed()] before calling for reproducibility. Invalid draws are retained
#' as errors and are never silently resampled. Inspect failures and boundary
#' statuses before interpreting intervals. No refitting is performed.
#' @export
calculate_reference_points_uncertainty <- function(sdreport, input_fn,
                                                  n_draws = 1000L,
                                                  level = 0.95, ...) {
  if (!is.function(input_fn)) stop("input_fn must be a function", call. = FALSE)
  if (!is.numeric(n_draws) || length(n_draws) != 1L ||
    !is.finite(n_draws) || n_draws < 2 || n_draws != floor(n_draws) ||
    n_draws > .Machine$integer.max) {
    stop("n_draws must be an integer of at least two", call. = FALSE)
  }
  if (!is.numeric(level) || length(level) != 1L || !is.finite(level) ||
    level <= 0 || level >= 1) stop("level must be in (0, 1)", call. = FALSE)
  mu <- sdreport$par.fixed
  covariance <- sdreport$cov.fixed
  if (!isTRUE(sdreport$pdHess) || !is.numeric(mu) || !length(mu) ||
    any(!is.finite(mu)) || !is.matrix(covariance) ||
    !is.numeric(covariance) ||
    !identical(dim(covariance), c(length(mu), length(mu))) ||
    any(!is.finite(covariance)) || !isSymmetric(covariance)) {
    stop("sdreport must have a positive Hessian and finite, matching fixed-effect covariance", call. = FALSE)
  }
  # Do not silently reorder a covariance matrix relative to its parameter vector.
  if ((!is.null(rownames(covariance)) && !identical(rownames(covariance), names(mu))) ||
    (!is.null(colnames(covariance)) && !identical(colnames(covariance), names(mu)))) {
    stop("covariance names must match par.fixed in order", call. = FALSE)
  }
  root <- tryCatch(chol(covariance), error = function(e) {
    stop("fixed-effect covariance must be positive definite", call. = FALSE)
  })
  evaluate <- function(par) calculate_reference_points(input_fn(par), ...)
  estimate <- evaluate(mu)
  parameters <- sweep(matrix(stats::rnorm(n_draws * length(mu)), nrow = n_draws) %*%
    root, 2, mu, "+")
  colnames(parameters) <- names(mu)
  rows <- vector("list", n_draws)
  errors <- data.frame(draw = integer(), message = character())
  for (i in seq_len(n_draws)) {
    result <- tryCatch(evaluate(stats::setNames(parameters[i, ], names(mu))),
      error = function(e) e)
    if (inherits(result, "error")) {
      errors <- rbind(errors, data.frame(draw = i, message = conditionMessage(result)))
    } else {
      table <- result$reference_points
      if (!identical(table$reference_point, estimate$reference_points$reference_point) ||
        !identical(names(table), names(estimate$reference_points))) {
        stop("input_fn must preserve reference points and fleet IDs across draws", call. = FALSE)
      }
      table$draw <- rep(i, nrow(table))
      rows[[i]] <- table
    }
  }
  draws <- do.call(rbind, rows)
  if (is.null(draws)) {
    draws <- estimate$reference_points[FALSE, , drop = FALSE]
    draws$draw <- integer()
  }
  fields <- c("fishing_mortality", "achieved_spr", "spawning_biomass",
    "total_biomass", "recruitment", "objective_yield", "total_yield",
    grep("^fleet_yield_", names(estimate$reference_points), value = TRUE))
  summaries <- list()
  for (reference in unique(estimate$reference_points$reference_point)) {
    for (field in fields) {
      values <- if (is.null(draws)) numeric() else draws[[field]][
        draws$reference_point == reference & draws$status == "converged"]
      values <- values[is.finite(values)]
      bounds <- if (length(values)) stats::quantile(values,
        c((1 - level) / 2, 0.5, (1 + level) / 2), names = FALSE) else rep(NA_real_, 3)
      summaries[[length(summaries) + 1L]] <- data.frame(
        reference_point = reference, quantity = field,
        lower = bounds[1], median = bounds[2], upper = bounds[3],
        n_success = length(values), n_failed = n_draws - length(values))
    }
  }
  list(estimate = estimate, summary = do.call(rbind, summaries),
    parameter_draws = parameters, draws = draws, errors = errors, level = level)
}
