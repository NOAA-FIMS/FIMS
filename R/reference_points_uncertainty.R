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
  restore_fn <- attr(input_fn, "restore_live_state", exact = TRUE)
  batch_env <- attr(input_fn, "evaluator_env", exact = TRUE)
  if (is.environment(batch_env) && is.function(restore_fn)) {
    batch_env$in_batch <- TRUE
    on.exit({
      batch_env$in_batch <- FALSE
      restore_fn()
    }, add = TRUE)
  }
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

#' Build a single-evaluation live-model snapshot and trajectory evaluator
#'
#' Construct a parameter-draw evaluator for [calculate_stock_status_uncertainty()]
#' and [calculate_reference_points_uncertainty()] that updates a live
#' `CatchAtAge` model, evaluates the model once to obtain both the historical
#' trajectory and the reference-year snapshot, and restores the original live
#' model state on exit.
#'
#' @inheritParams get_stock_status
#' @return A function `function(par, random = NULL)` returning a reference-point
#'   input snapshot with an embedded `historical` data frame for the selected
#'   population.
#' @details
#' Each call sets the supplied fixed-effect vector `par` and random-effect
#' vector `random` (defaulting to the fitted random effects when `random` is
#' `NULL`), then calls `model$reference_point_inputs()` once. That single model
#' evaluation refreshes all parameter transformations, historical spawning
#' biomass, historical `f_multiplier * sum_v(Fmort[v, t])`, reference-year
#' biology, fleet shares, selectivity, and the initial stock-recruit baseline
#' `phi0`. When called outside a batch uncertainty run, the evaluator restores
#' the pre-call fixed and random parameters and refreshes the live model before
#' returning; during a batch uncertainty run, the outer uncertainty routine
#' restores and refreshes the original model state on completion or error.
#' @export
make_reference_point_evaluator <- function(model, population = 1L, year = NULL,
                                           reference_year = year,
                                           fleet_shares = NULL,
                                           include_in_msy = NULL,
                                           fit = NULL,
                                           calendar_years = NULL) {
  if (is.FIMSFit(model)) {
    if (is.null(fit)) fit <- model
    model <- get_input(fit)[["model"]]
  }
  if (!methods::is(model, "Rcpp_CatchAtAge")) {
    stop("model must be a live CatchAtAge object or FIMSFit with a live model", call. = FALSE)
  }
  if (is.null(reference_year) || !valid_positive_index(population) ||
    !valid_positive_index(reference_year)) {
    stop("population and year (or reference_year) must be positive integer indices", call. = FALSE)
  }
  if (!is.null(fit) && !is.FIMSFit(fit)) {
    stop("fit must be a FIMSFit object", call. = FALSE)
  }
  population <- as.integer(population)
  reference_year <- as.integer(reference_year)
  if (is.null(fleet_shares)) fleet_shares <- numeric()
  if (!is.numeric(fleet_shares) || any(!is.finite(fleet_shares))) {
    stop("fleet_shares must be finite numeric values", call. = FALSE)
  }

  orig_fixed <- get_fixed()
  orig_random <- get_random()
  cpp_fixed_names <- if (length(orig_fixed)) {
    names(get_parameter_names(as.list(orig_fixed)))
  } else {
    NULL
  }
  cpp_random_names <- if (length(orig_random)) {
    names(get_random_names(as.list(orig_random)))
  } else {
    NULL
  }
  held_random <- if (!is.null(fit)) {
    extract_fit_parameter_vectors(
      fit, orig_fixed = orig_fixed, orig_random = orig_random
    )$random
  } else {
    orig_random
  }

  state_env <- new.env(parent = emptyenv())
  state_env$in_batch <- FALSE
  state_env$orig_fixed <- orig_fixed
  state_env$orig_random <- orig_random

  restore_fn <- function() {
    restore_live_model_state(
      state_env$orig_fixed, state_env$orig_random,
      model = model, population = population, year = reference_year
    )
  }

  evaluator <- function(par, random = NULL) {
    if (!isTRUE(state_env$in_batch)) {
      pre_fixed <- get_fixed()
      pre_random <- get_random()
      on.exit(
        restore_live_model_state(
          pre_fixed, pre_random, model = model,
          population = population, year = reference_year
        ),
        add = TRUE
      )
    }
    target_random <- if (is.null(random)) held_random else random
    expanded <- expand_live_parameters(
      par_fixed = par, par_random = target_random,
      orig_fixed = orig_fixed, orig_random = orig_random,
      cpp_fixed_names = cpp_fixed_names, cpp_random_names = cpp_random_names,
      fit = fit
    )
    if (length(expanded$fixed)) set_fixed(expanded$fixed)
    if (length(expanded$random)) set_random(expanded$random)
    inputs <- model$reference_point_inputs(population, reference_year, fleet_shares)
    if (!is.null(include_in_msy)) {
      inputs <- apply_include_in_msy(inputs, include_in_msy)
    }
    if (!is.null(calendar_years)) {
      inputs$calendar_years <- calendar_years
    }
    inputs
  }

  attr(evaluator, "restore_live_state") <- restore_fn
  attr(evaluator, "evaluator_env") <- state_env
  attr(evaluator, "held_random") <- held_random
  attr(evaluator, "fit") <- fit
  evaluator
}

validate_joint_precision_draws <- function(sdreport, n_draws, fit = NULL) {
  mu_fixed <- sdreport$par.fixed
  mu_random <- sdreport$par.random
  if (is.null(mu_random) || !is.numeric(mu_random) || !length(mu_random) ||
    any(!is.finite(mu_random))) {
    stop("Joint fixed/random draws require finite sdreport$par.random", call. = FALSE)
  }
  if (is.null(sdreport$jointPrecision)) {
    stop("Joint fixed/random draws require sdreport$jointPrecision or random_draws", call. = FALSE)
  }
  n_fixed <- length(mu_fixed)
  n_random <- length(mu_random)
  n_total <- n_fixed + n_random
  joint_prec <- tryCatch(as.matrix(sdreport$jointPrecision), error = function(e) {
    stop("sdreport$jointPrecision must be convertible to a numeric matrix", call. = FALSE)
  })
  if (!is.matrix(joint_prec) || !is.numeric(joint_prec) ||
    !identical(dim(joint_prec), c(n_total, n_total)) ||
    any(!is.finite(joint_prec)) || !isSymmetric(joint_prec)) {
    stop(
      "sdreport$jointPrecision must be a finite symmetric matrix matching par.fixed and par.random",
      call. = FALSE
    )
  }

  fixed_idx <- seq_len(n_fixed)
  random_idx <- seq.int(n_fixed + 1L, n_total)
  if (!is.null(fit)) {
    if (!is.FIMSFit(fit)) stop("fit must be a FIMSFit object", call. = FALSE)
    obj <- get_obj(fit)
    last_par <- obj[["env"]][["last.par.best"]]
    obj_rand <- obj[["env"]][["random"]]
    if (length(last_par) != n_total || length(obj_rand) != n_random) {
      stop("jointPrecision dimensions do not match fit parameter counts", call. = FALSE)
    }
    random_idx <- as.integer(obj_rand)
    fixed_idx <- setdiff(seq_len(n_total), random_idx)
    if (length(fixed_idx) != n_fixed ||
      max(abs(unname(last_par[fixed_idx]) - unname(mu_fixed))) > 1e-5 ||
      max(abs(unname(last_par[random_idx]) - unname(mu_random))) > 1e-5) {
      stop("jointPrecision parameter order does not match fit and sdreport", call. = FALSE)
    }
    if (!is.null(names(obj[["par"]])) && !is.null(names(mu_fixed)) &&
      !identical(names(obj[["par"]]), names(mu_fixed))) {
      stop("sdreport$par.fixed names do not match fit parameter order", call. = FALSE)
    }
  }

  expected_names <- character(n_total)
  if (!is.null(names(mu_fixed)) && !is.null(names(mu_random))) {
    expected_names[fixed_idx] <- names(mu_fixed)
    expected_names[random_idx] <- names(mu_random)
  } else {
    expected_names <- NULL
  }
  block_names <- character(n_total)
  block_names[fixed_idx] <- "p"
  block_names[random_idx] <- "re"

  check_dimnames <- function(nms) {
    if (is.null(nms)) return(TRUE)
    identical(nms, expected_names) || identical(nms, block_names)
  }
  if (!check_dimnames(rownames(joint_prec)) || !check_dimnames(colnames(joint_prec))) {
    stop("jointPrecision names must match fixed and random parameters in order", call. = FALSE)
  }

  joint_root <- tryCatch(chol(joint_prec), error = function(e) {
    stop("jointPrecision must be positive definite", call. = FALSE)
  })
  z <- matrix(stats::rnorm(n_draws * n_total), nrow = n_total, ncol = n_draws)
  delta <- t(backsolve(joint_root, z))
  mu_joint <- numeric(n_total)
  mu_joint[fixed_idx] <- mu_fixed
  mu_joint[random_idx] <- mu_random
  joint_draws <- sweep(delta, 2, mu_joint, "+")
  fixed_draws <- joint_draws[, fixed_idx, drop = FALSE]
  colnames(fixed_draws) <- names(mu_fixed)
  random_draws <- joint_draws[, random_idx, drop = FALSE]
  colnames(random_draws) <- names(mu_random)
  list(fixed = fixed_draws, random = random_draws)
}

#' Propagate parameter uncertainty into annual MSY-relative stock status
#'
#' Draw correlated parameter vectors from a fitted TMB report, evaluate both the
#' historical trajectory and the reference-year MSY benchmark once per draw, and
#' calculate pointwise equal-tail intervals and standard deviations for annual
#' `sb_over_sb_msy` and `f_over_f_msy`.
#'
#' @param sdreport A report from [get_sdreport()] containing `par.fixed`,
#'   `cov.fixed`, and `pdHess = TRUE`, or a [FIMSFit] object.
#' @param input_fn Optional function taking `par` (and optional `random`) on the
#'   estimation scale and returning an input snapshot with historical series for
#'   [calculate_stock_status()]. When `NULL`, `model` (or a [FIMSFit] passed as
#'   `sdreport` or `fit`) is used with [make_reference_point_evaluator()].
#' @param n_draws Positive integer number of draws, at least two.
#' @param level Central interval probability, strictly between zero and one
#'   (default `0.95`).
#' @inheritParams get_stock_status
#' @param include_random Logical (default `FALSE`). When `FALSE`, fixed effects
#'   are drawn from `sdreport$cov.fixed` with fitted random effects held fixed,
#'   and intervals are labeled `"conditional_on_random_effects"`. When `TRUE`,
#'   paired joint fixed- and random-effect draws are sampled from
#'   `sdreport$jointPrecision` (or supplied via `parameter_draws` and
#'   `random_draws`) and labeled `"joint_fixed_and_random_effects"`.
#' @param parameter_draws Optional numeric matrix of fixed-effect parameter
#'   draws (`n_draws x length(par.fixed)`).
#' @param random_draws Optional numeric matrix of paired random-effect draws
#'   (`n_draws x length(par.random)`).
#' @return A list containing:
#'   \describe{
#'     \item{`estimate`}{Point-estimate result from [calculate_stock_status()]
#'       evaluated at the fitted parameters.}
#'     \item{`summary`}{Per-year data frame with point estimates, equal-tail
#'       bounds (`*_lower`, `*_median`, `*_upper`), draw-based standard
#'       deviations (`*_sd`), usable and excluded draw counts (`*_n_usable`,
#'       `*_n_excluded`), and `conditioning` for `sb_over_sb_msy` and
#'       `f_over_f_msy`.}
#'     \item{`ratio_summary`}{Long-format per-year summary keyed by
#'       `quantity` (`"sb_over_sb_msy"` and `"f_over_f_msy"`), reporting
#'       `estimate`, `lower`, `median`, `upper`, `sd`, `n_usable`,
#'       `n_excluded`, `n_success`, `n_failed`, and `conditioning`.}
#'     \item{`parameter_draws`}{Matrix of fixed-effect draws.}
#'     \item{`random_draws`}{Matrix of random-effect draws when joint sampling
#'       is used, or `NULL` when conditional on fitted random effects.}
#'     \item{`draws`}{Draw-by-year data frame of numerators, denominators,
#'       ratios, statuses, and per-ratio diagnostics.}
#'     \item{`benchmark_draws`}{Draw-level equilibrium benchmark table.}
#'     \item{`draw_diagnostics`}{One row per draw (`1:n_draws`) recording
#'       evaluation outcome, solver status, collapse, usability flags, exclusion
#'       reasons, and error messages.}
#'     \item{`errors`}{Data frame with one row per draw that threw an error.}
#'     \item{`level`}{Interval probability level.}
#'     \item{`conditioning`}{`"conditional_on_random_effects"` or
#'       `"joint_fixed_and_random_effects"`.}
#'     \item{`uncertainty_description`}{Text description of the uncertainty
#'       scope and conditioning.}
#'     \item{`reference`}{Reference-year configuration and benchmark metadata.}
#'     \item{`conventions`}{Conventions and interpretation caveats.}
#'   }
#' @details
#' For each parameter draw, the historical trajectory and the selected
#' reference-year snapshot (biology, selectivity, fleet allocation, and initial
#' stock-recruit baseline `phi0`) are evaluated together, MSY is solved once for
#' that draw, and each annual numerator is divided by its matching draw's
#' denominator (`SB[t] / SB_MSY` and `F_eff[t] / F_MSY`). This pairing preserves
#' covariance between numerators and denominators and across years.
#'
#' Intervals based on `sdreport$cov.fixed` hold random effects at their fitted
#' values and are labeled `"conditional_on_random_effects"`. Setting
#' `include_random = TRUE` (or supplying paired `parameter_draws` and
#' `random_draws`) propagates joint fixed- and random-effect estimation
#' uncertainty through the same evaluator, verifying parameter order against
#' `fit` and `sdreport`. Both modes represent estimation uncertainty only and
#' exclude future process or observation variation.
#'
#' Failed draws, unconverged MSY candidates (including `upper_bound`), and
#' nonfinite or nonpositive denominators produce `NA_real_` ratios for the
#' affected quantities with explicit diagnostic reasons. Valid zero ratios are
#' retained, and interval bounds and standard deviations are calculated directly
#' from the retained ratio draws.
#' @export
#' @examples
#' set.seed(12)
#' report <- list(
#'   pdHess = TRUE,
#'   par.fixed = c(log_rzero = log(1000), log_f = log(0.3)),
#'   cov.fixed = matrix(
#'     c(0.02, -0.005, -0.005, 0.01), 2, 2,
#'     dimnames = rep(list(c("log_rzero", "log_f")), 2)
#'   )
#' )
#' status_fn <- function(par) {
#'   rzero <- exp(par[["log_rzero"]])
#'   f_hist <- c(0.8, 1.0) * exp(par[["log_f"]])
#'   list(
#'     natural_mortality = c(0.2, 0.2), weight = c(1, 1),
#'     maturity = c(0, 1), proportion_female = c(1, 1),
#'     fleets = list(list(share = 1, selectivity = c(1, 1), weight = c(1, 1))),
#'     recruitment = list(
#'       type = "beverton_holt", rzero = rzero, steepness = 0.75,
#'       phi0 = 1 / expm1(0.2)
#'     ),
#'     population_id = 1L,
#'     year_index = 2L,
#'     historical = data.frame(
#'       population_id = c(1L, 1L, 1L),
#'       year_index = 1:3,
#'       calendar_year = 2021:2023,
#'       spawning_biomass = rzero * c(2.4, 1.9, 1.7) * exp(-0.5 * c(f_hist, f_hist[2])),
#'       f_multiplier = c(1, 1, NA_real_),
#'       fleet_fishing_mortality = c(f_hist, NA_real_),
#'       effective_fishing_mortality = c(f_hist, NA_real_)
#'     )
#'   )
#' }
#' calculate_stock_status_uncertainty(report, status_fn, n_draws = 50)$summary
calculate_stock_status_uncertainty <- function(sdreport,
                                               input_fn = NULL,
                                               n_draws = 1000L,
                                               level = 0.95,
                                               model = NULL,
                                               population = NULL,
                                               year = NULL,
                                               reference_year = year,
                                               years = NULL,
                                               calendar_years = NULL,
                                               include_terminal_biomass = TRUE,
                                               fleet_shares = NULL,
                                               include_in_msy = NULL,
                                               include_random = FALSE,
                                               parameter_draws = NULL,
                                               random_draws = NULL,
                                               fit = NULL,
                                               spr_targets = 0.4,
                                               max_f = 5,
                                               tolerance = 1e-8,
                                               max_iterations = 100L,
                                               grid_intervals = 100L) {
  if (is.FIMSFit(sdreport)) {
    if (is.null(fit)) fit <- sdreport
    sdreport <- get_sdreport(fit)
    if (is.null(model)) model <- get_input(fit)[["model"]]
  }
  if (!is.null(fit) && !is.FIMSFit(fit)) {
    stop("fit must be a FIMSFit object", call. = FALSE)
  }
  if (is.null(input_fn)) {
    if (is.null(model) && !is.null(fit)) {
      model <- get_input(fit)[["model"]]
    }
    if (is.null(model)) {
      stop("Supply input_fn or a live CatchAtAge model / FIMSFit", call. = FALSE)
    }
    pop_eval <- if (is.null(population)) 1L else population
    input_fn <- make_reference_point_evaluator(
      model = model, population = pop_eval, reference_year = reference_year,
      fleet_shares = fleet_shares, include_in_msy = include_in_msy,
      fit = fit, calendar_years = calendar_years
    )
  }
  if (!is.function(input_fn)) stop("input_fn must be a function", call. = FALSE)
  if (!is.logical(include_random) || length(include_random) != 1L ||
    is.na(include_random)) {
    stop("include_random must be TRUE or FALSE", call. = FALSE)
  }
  if (!is.numeric(level) || length(level) != 1L || !is.finite(level) ||
    level <= 0 || level >= 1) {
    stop("level must be in (0, 1)", call. = FALSE)
  }

  mu <- sdreport$par.fixed
  covariance <- sdreport$cov.fixed
  if (!isTRUE(sdreport$pdHess) || !is.numeric(mu) || !length(mu) ||
    any(!is.finite(mu)) || !is.matrix(covariance) ||
    !is.numeric(covariance) ||
    !identical(dim(covariance), c(length(mu), length(mu))) ||
    any(!is.finite(covariance)) || !isSymmetric(covariance)) {
    stop(
      "sdreport must have a positive Hessian and finite, matching fixed-effect covariance",
      call. = FALSE
    )
  }
  if ((!is.null(rownames(covariance)) && !identical(rownames(covariance), names(mu))) ||
    (!is.null(colnames(covariance)) && !identical(colnames(covariance), names(mu)))) {
    stop("covariance names must match par.fixed in order", call. = FALSE)
  }

  if (is.null(fit)) {
    fit <- attr(input_fn, "fit", exact = TRUE)
  }
  mu_random <- if (!is.null(sdreport$par.random) && length(sdreport$par.random) > 0L) {
    sdreport$par.random
  } else {
    attr(input_fn, "held_random", exact = TRUE)
  }

  if (!is.null(parameter_draws)) {
    if (!is.matrix(parameter_draws) || !is.numeric(parameter_draws) ||
      nrow(parameter_draws) < 2L || ncol(parameter_draws) != length(mu) ||
      any(!is.finite(parameter_draws))) {
      stop(
        "parameter_draws must be a finite numeric matrix with at least two rows and length(par.fixed) columns",
        call. = FALSE
      )
    }
    if (!is.null(colnames(parameter_draws)) && !is.null(names(mu)) &&
      !identical(colnames(parameter_draws), names(mu))) {
      stop("parameter_draws column names must match par.fixed in order", call. = FALSE)
    }
    colnames(parameter_draws) <- names(mu)
    n_draws <- nrow(parameter_draws)
    if (!is.null(random_draws)) {
      if (is.null(mu_random) || !length(mu_random)) {
        stop("random_draws supplied but model has no random effects", call. = FALSE)
      }
      if (!is.matrix(random_draws) || !is.numeric(random_draws) ||
        nrow(random_draws) != n_draws || ncol(random_draws) != length(mu_random) ||
        any(!is.finite(random_draws))) {
        stop(
          "random_draws must be a finite numeric matrix matching parameter_draws rows and par.random columns",
          call. = FALSE
        )
      }
      if (!is.null(colnames(random_draws)) && !is.null(names(mu_random)) &&
        !identical(colnames(random_draws), names(mu_random))) {
        stop("random_draws column names must match par.random in order", call. = FALSE)
      }
      colnames(random_draws) <- names(mu_random)
      include_random <- TRUE
    } else if (include_random) {
      stop(
        "Supply random_draws when parameter_draws is provided with include_random = TRUE",
        call. = FALSE
      )
    }
  } else {
    if (!is.numeric(n_draws) || length(n_draws) != 1L ||
      !is.finite(n_draws) || n_draws < 2 || n_draws != floor(n_draws) ||
      n_draws > .Machine$integer.max) {
      stop("n_draws must be an integer of at least two", call. = FALSE)
    }
    n_draws <- as.integer(n_draws)
    if (!is.null(random_draws)) {
      stop("Supply parameter_draws together with random_draws", call. = FALSE)
    }
  }

  # Validate covariance / joint precision before running evaluations.
  root <- tryCatch(chol(covariance), error = function(e) {
    stop("fixed-effect covariance must be positive definite", call. = FALSE)
  })
  if (include_random && is.null(parameter_draws)) {
    if (is.null(sdreport$par.random) || !length(sdreport$par.random) ||
      any(!is.finite(sdreport$par.random))) {
      stop("Joint fixed/random draws require finite sdreport$par.random", call. = FALSE)
    }
    if (is.null(sdreport$jointPrecision)) {
      stop("Joint fixed/random draws require sdreport$jointPrecision or random_draws", call. = FALSE)
    }
  }

  restore_fn <- attr(input_fn, "restore_live_state", exact = TRUE)
  batch_env <- attr(input_fn, "evaluator_env", exact = TRUE)
  if (is.environment(batch_env) && is.function(restore_fn)) {
    batch_env$in_batch <- TRUE
    on.exit({
      batch_env$in_batch <- FALSE
      restore_fn()
    }, add = TRUE)
  }

  call_input_fn <- function(par_vec, rand_vec = NULL) {
    if (!is.null(rand_vec) && length(formals(input_fn)) >= 2L) {
      input_fn(par_vec, rand_vec)
    } else {
      input_fn(par_vec)
    }
  }

  evaluate_draw <- function(par_vec, rand_vec = NULL) {
    draw_inputs <- call_input_fn(par_vec, rand_vec)
    calculate_stock_status(
      inputs = draw_inputs, population = population,
      reference_year = reference_year, years = years,
      calendar_years = calendar_years,
      include_terminal_biomass = include_terminal_biomass,
      spr_targets = spr_targets, msy = TRUE, max_f = max_f,
      tolerance = tolerance, max_iterations = max_iterations,
      grid_intervals = grid_intervals
    )
  }

  estimate <- evaluate_draw(mu, mu_random)

  if (is.null(parameter_draws)) {
    if (include_random) {
      joint <- validate_joint_precision_draws(sdreport, n_draws = n_draws, fit = fit)
      parameters <- joint$fixed
      random_draws <- joint$random
    } else {
      parameters <- sweep(
        matrix(stats::rnorm(n_draws * length(mu)), nrow = n_draws) %*% root,
        2, mu, "+"
      )
      colnames(parameters) <- names(mu)
      random_draws <- NULL
    }
  } else {
    parameters <- parameter_draws
  }

  conditioning <- if (include_random) {
    "joint_fixed_and_random_effects"
  } else {
    "conditional_on_random_effects"
  }
  uncertainty_description <- if (include_random) {
    "Paired joint fixed- and random-effect parameter uncertainty; excludes future process and observation variation"
  } else if (!is.null(mu_random) && length(mu_random) > 0L) {
    "Fixed-effect parameter uncertainty from sdreport$cov.fixed conditional on held fitted random effects; excludes random-effect estimation uncertainty and future process or observation variation"
  } else {
    "Fixed-effect parameter uncertainty from sdreport$cov.fixed; excludes future process and observation variation"
  }

  rows <- vector("list", n_draws)
  benchmark_rows <- vector("list", n_draws)
  diag_rows <- vector("list", n_draws)
  errors <- data.frame(draw = integer(), message = character(), stringsAsFactors = FALSE)
  est_table <- estimate$stock_status
  est_bench <- estimate$reference_points$reference_points

  for (i in seq_len(n_draws)) {
    par_i <- stats::setNames(parameters[i, ], names(mu))
    rand_i <- if (!is.null(random_draws)) {
      stats::setNames(random_draws[i, ], names(mu_random))
    } else {
      mu_random
    }
    result <- tryCatch(evaluate_draw(par_i, rand_i), error = function(e) e)
    if (inherits(result, "error")) {
      msg <- conditionMessage(result)
      errors <- rbind(
        errors,
        data.frame(draw = i, message = msg, stringsAsFactors = FALSE)
      )
      diag_rows[[i]] <- data.frame(
        draw = i, evaluated = FALSE, status = "error", collapsed = NA,
        sb_msy = NA_real_, f_msy = NA_real_,
        sb_usable = FALSE, f_usable = FALSE,
        sb_reason = "error", f_reason = "error",
        error_message = msg, stringsAsFactors = FALSE
      )
    } else {
      table <- result$stock_status
      bench <- result$reference_points$reference_points
      if (!identical(table$population_id, est_table$population_id) ||
        !identical(table$year_index, est_table$year_index) ||
        !identical(table$calendar_year, est_table$calendar_year) ||
        !identical(table$reference_year, est_table$reference_year) ||
        !identical(bench$reference_point, est_bench$reference_point) ||
        !identical(names(bench), names(est_bench))) {
        stop(
          "input_fn must preserve population_id, year_index, calendar_year, reference points, and fleet IDs across draws",
          call. = FALSE
        )
      }
      table$draw <- rep(i, nrow(table))
      rows[[i]] <- table
      bench$draw <- rep(i, nrow(bench))
      benchmark_rows[[i]] <- bench
      ref <- result$reference
      diag_rows[[i]] <- data.frame(
        draw = i, evaluated = TRUE, status = ref$status,
        collapsed = ref$collapsed, sb_msy = ref$sb_msy, f_msy = ref$f_msy,
        sb_usable = identical(ref$sb_diagnostic, "ok"),
        f_usable = identical(ref$f_diagnostic, "ok"),
        sb_reason = ref$sb_diagnostic, f_reason = ref$f_diagnostic,
        error_message = NA_character_, stringsAsFactors = FALSE
      )
    }
  }

  draws <- do.call(rbind, rows)
  if (is.null(draws)) {
    draws <- est_table[FALSE, , drop = FALSE]
    draws$draw <- integer()
  }
  rownames(draws) <- NULL

  benchmark_draws <- do.call(rbind, benchmark_rows)
  if (is.null(benchmark_draws)) {
    benchmark_draws <- est_bench[FALSE, , drop = FALSE]
    benchmark_draws$draw <- integer()
  }
  rownames(benchmark_draws) <- NULL

  draw_diagnostics <- do.call(rbind, diag_rows)
  rownames(draw_diagnostics) <- NULL

  probs <- c((1 - level) / 2, 0.5, (1 + level) / 2)
  n_years_out <- nrow(est_table)
  sb_lower <- sb_median <- sb_upper <- sb_sd <- rep(NA_real_, n_years_out)
  f_lower <- f_median <- f_upper <- f_sd <- rep(NA_real_, n_years_out)
  sb_n_usable <- f_n_usable <- rep(0L, n_years_out)
  sb_n_excluded <- f_n_excluded <- rep(as.integer(n_draws), n_years_out)
  ratio_rows <- vector("list", 2L * n_years_out)

  for (k in seq_len(n_years_out)) {
    pop_k <- est_table$population_id[k]
    yr_k <- est_table$year_index[k]
    mask <- draws$population_id == pop_k & draws$year_index == yr_k

    sb_vals <- draws$sb_over_sb_msy[mask]
    sb_vals <- sb_vals[is.finite(sb_vals)]
    if (length(sb_vals) >= 1L) {
      q_sb <- stats::quantile(sb_vals, probs, names = FALSE)
      sb_lower[k] <- q_sb[1]
      sb_median[k] <- q_sb[2]
      sb_upper[k] <- q_sb[3]
    }
    if (length(sb_vals) >= 2L) sb_sd[k] <- stats::sd(sb_vals)
    sb_n_usable[k] <- length(sb_vals)
    sb_n_excluded[k] <- as.integer(n_draws - length(sb_vals))

    f_vals <- draws$f_over_f_msy[mask]
    f_vals <- f_vals[is.finite(f_vals)]
    if (length(f_vals) >= 1L) {
      q_f <- stats::quantile(f_vals, probs, names = FALSE)
      f_lower[k] <- q_f[1]
      f_median[k] <- q_f[2]
      f_upper[k] <- q_f[3]
    }
    if (length(f_vals) >= 2L) f_sd[k] <- stats::sd(f_vals)
    f_n_usable[k] <- length(f_vals)
    f_n_excluded[k] <- as.integer(n_draws - length(f_vals))

    ratio_rows[[2L * k - 1L]] <- data.frame(
      population_id = pop_k,
      year_index = yr_k,
      calendar_year = est_table$calendar_year[k],
      reference_year = est_table$reference_year[k],
      quantity = "sb_over_sb_msy",
      estimate = est_table$sb_over_sb_msy[k],
      lower = sb_lower[k],
      median = sb_median[k],
      upper = sb_upper[k],
      sd = sb_sd[k],
      n_usable = sb_n_usable[k],
      n_excluded = sb_n_excluded[k],
      n_success = sb_n_usable[k],
      n_failed = sb_n_excluded[k],
      conditioning = conditioning,
      stringsAsFactors = FALSE
    )
    ratio_rows[[2L * k]] <- data.frame(
      population_id = pop_k,
      year_index = yr_k,
      calendar_year = est_table$calendar_year[k],
      reference_year = est_table$reference_year[k],
      quantity = "f_over_f_msy",
      estimate = est_table$f_over_f_msy[k],
      lower = f_lower[k],
      median = f_median[k],
      upper = f_upper[k],
      sd = f_sd[k],
      n_usable = f_n_usable[k],
      n_excluded = f_n_excluded[k],
      n_success = f_n_usable[k],
      n_failed = f_n_excluded[k],
      conditioning = conditioning,
      stringsAsFactors = FALSE
    )
  }

  summary_df <- data.frame(
    population_id = est_table$population_id,
    year_index = est_table$year_index,
    calendar_year = est_table$calendar_year,
    reference_year = est_table$reference_year,
    spawning_biomass = est_table$spawning_biomass,
    effective_fishing_mortality = est_table$effective_fishing_mortality,
    sb_msy = est_table$sb_msy,
    f_msy = est_table$f_msy,
    sb_over_sb_msy = est_table$sb_over_sb_msy,
    sb_over_sb_msy_lower = sb_lower,
    sb_over_sb_msy_median = sb_median,
    sb_over_sb_msy_upper = sb_upper,
    sb_over_sb_msy_sd = sb_sd,
    sb_over_sb_msy_n_usable = sb_n_usable,
    sb_over_sb_msy_n_excluded = sb_n_excluded,
    f_over_f_msy = est_table$f_over_f_msy,
    f_over_f_msy_lower = f_lower,
    f_over_f_msy_median = f_median,
    f_over_f_msy_upper = f_upper,
    f_over_f_msy_sd = f_sd,
    f_over_f_msy_n_usable = f_n_usable,
    f_over_f_msy_n_excluded = f_n_excluded,
    status = est_table$status,
    conditioning = rep(conditioning, n_years_out),
    stringsAsFactors = FALSE
  )
  rownames(summary_df) <- NULL

  ratio_summary_df <- do.call(rbind, ratio_rows)
  rownames(ratio_summary_df) <- NULL

  list(
    estimate = estimate,
    summary = summary_df,
    ratio_summary = ratio_summary_df,
    parameter_draws = parameters,
    random_draws = random_draws,
    draws = draws,
    benchmark_draws = benchmark_draws,
    draw_diagnostics = draw_diagnostics,
    errors = errors,
    level = level,
    conditioning = conditioning,
    uncertainty_description = uncertainty_description,
    reference = estimate$reference,
    conventions = estimate$conventions
  )
}

#' @rdname calculate_stock_status_uncertainty
#' @export
get_stock_status_uncertainty <- function(model, population = 1L, year = NULL,
                                         reference_year = year,
                                         sdreport = NULL,
                                         n_draws = 1000L, level = 0.95,
                                         years = NULL, calendar_years = NULL,
                                         include_terminal_biomass = TRUE,
                                         fleet_shares = NULL,
                                         include_in_msy = NULL,
                                         include_random = FALSE,
                                         parameter_draws = NULL,
                                         random_draws = NULL,
                                         fit = NULL,
                                         spr_targets = 0.4, max_f = 5,
                                         tolerance = 1e-8,
                                         max_iterations = 100L,
                                         grid_intervals = 100L) {
  if (is.FIMSFit(model)) {
    if (is.null(fit)) fit <- model
    if (is.null(sdreport)) sdreport <- get_sdreport(fit)
    model <- get_input(fit)[["model"]]
  }
  if (is.null(sdreport) && !is.null(fit)) {
    sdreport <- get_sdreport(fit)
  }
  if (is.null(sdreport)) {
    stop("Supply sdreport or a FIMSFit object containing sdreport", call. = FALSE)
  }
  evaluator <- make_reference_point_evaluator(
    model = model, population = population, reference_year = reference_year,
    fleet_shares = fleet_shares, include_in_msy = include_in_msy,
    fit = fit, calendar_years = calendar_years
  )
  calculate_stock_status_uncertainty(
    sdreport = sdreport, input_fn = evaluator, n_draws = n_draws,
    level = level, model = model, population = population,
    reference_year = reference_year, years = years,
    calendar_years = calendar_years,
    include_terminal_biomass = include_terminal_biomass,
    fleet_shares = fleet_shares, include_in_msy = include_in_msy,
    include_random = include_random, parameter_draws = parameter_draws,
    random_draws = random_draws, fit = fit, spr_targets = spr_targets,
    max_f = max_f, tolerance = tolerance, max_iterations = max_iterations,
    grid_intervals = grid_intervals
  )
}

