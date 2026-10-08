#' Check model configuration and convergence sanity
#'
#' @description
#' Evaluates whether a fitted FIMS model converged and has well-behaved
#' parameter estimates, negative log-likelihoods, and variance calculations.
#' Similar to `sdmTMB::sanity()`, this function inspects the optimizer
#' convergence status, negative log-likelihoods (including named components),
#' maximum gradient, Hessian positive definiteness, condition number, and
#' standard errors, providing an actionable diagnostic summary.
#'
#' @param object A fitted model object of class `FIMSFit` returned from
#'   [fit_fims()].
#' @param gradient_thresh Numeric threshold for the maximum absolute gradient.
#'   Default is `0.001`.
#' @param condition_thresh Numeric threshold for the condition number of the
#'   Hessian matrix. Default is `1e5`.
#' @param big_se Numeric threshold above which standard errors are flagged as
#'   suspiciously large. Default is `1e3`.
#' @param verbose Logical indicating whether to print the formatted diagnostic
#'   report to the console. Default is `TRUE`.
#' @param ... Additional arguments passed to methods.
#'
#' @return An invisible list of class `"sanity_fims"` containing:
#' \describe{
#'   \item{`all_ok`}{Logical; `TRUE` if all convergence, likelihood, and variance checks pass.}
#'   \item{`optimized`}{Logical; `TRUE` if the model was optimized.}
#'   \item{`nlminb_ok`}{Logical; `TRUE` if `nlminb` convergence code is 0.}
#'   \item{`nll_ok`}{Logical; `TRUE` if marginal, total, and all NLL components are finite.}
#'   \item{`gradients_ok`}{Logical; `TRUE` if maximum gradient is below `gradient_thresh`.}
#'   \item{`hessian_ok`}{Logical; `TRUE` if the Hessian is positive definite.}
#'   \item{`condition_ok`}{Logical; `TRUE` if the Hessian condition number is below `condition_thresh`.}
#'   \item{`se_ok`}{Logical; `TRUE` if standard errors are finite and reasonable.}
#'   \item{`se_na_ok`}{Logical; `TRUE` if no standard errors are NA, NaN, or Inf.}
#'   \item{`se_magnitude_ok`}{Logical; `TRUE` if no standard errors exceed `big_se`.}
#'   \item{`marginal_nll`}{Numeric; the marginal negative log-likelihood from the optimizer.}
#'   \item{`total_nll`}{Numeric; the total joint negative log-likelihood from `report$jnll`.}
#'   \item{`nll_components`}{Named numeric vector of individual negative log-likelihood components.}
#'   \item{`max_gradient`}{Numeric; the maximum absolute gradient.}
#'   \item{`condition_number`}{Numeric; condition number of the Hessian matrix.}
#'   \item{`largest_gradient_parameter`}{Character; name of the parameter with largest gradient.}
#'   \item{`top_gradient_parameters`}{Named numeric vector; top 2-3 parameters ranked by absolute gradient.}
#'   \item{`issues`}{Character vector describing failed checks.}
#'   \item{`warnings`}{Character vector describing warning conditions.}
#'   \item{`details`}{A list with details on problematic parameters (NA SEs, large SEs, top gradients, optimizer messages, bad NLLs).}
#' }
#'
#' @export
#' @keywords diagnostics
#' @examples
#' \dontrun{
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#' fit <- setup_default_parameters(data = data_4_model) |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#'
#' # Run sanity check
#' sanity_fims(fit)
#'
#' # Inspect named NLL components
#' sanity_fims(fit)$nll_components
#'
#' # Check programmatically
#' checks <- sanity_fims(fit, verbose = FALSE)
#' if (checks$all_ok) {
#'   message("Model is converged and ready for inference!")
#' }
#' }

sanity_fims <- function(object, ...) {
  UseMethod("sanity_fims")
}

#' @export
sanity_fims.default <- function(object, ...) {
  cli::cli_abort(
    "No {.fn sanity_fims} method for object of class {.cls {class(object)}}."
  )
}

#' @rdname sanity_fims
#' @export
sanity_fims.FIMSFit <- function(
  object,
  gradient_thresh = 0.001,
  condition_thresh = 1e5,
  big_se = 1e3,
  verbose = TRUE,
  ...
) {
  opt <- get_opt(object)
  sdreport <- get_sdreport(object)
  report <- get_report(object)
  obj <- get_obj(object)
  max_grad <- get_max_gradient(object)
  gradient <- get_gradient(object)

  has_opt <- length(opt) > 0 && !is.null(opt[["convergence"]])
  has_sd <- inherits(sdreport, "sdreport") && length(sdreport) > 0
  has_random <- length(obj[["env"]][["random"]]) > 0

  issues <- character()
  warnings <- character()

  # 1. Model optimization status
  optimized <- has_opt
  if (!optimized) {
    issues <- c(issues, "Model was not optimized (optimize = FALSE).")
    res <- list(
      all_ok = FALSE,
      optimized = FALSE,
      nlminb_ok = FALSE,
      nll_ok = FALSE,
      gradients_ok = FALSE,
      hessian_ok = FALSE,
      condition_ok = FALSE,
      se_ok = FALSE,
      se_na_ok = FALSE,
      se_magnitude_ok = FALSE,
      marginal_nll = NA_real_,
      total_nll = if (!is.null(report[["jnll"]])) report[["jnll"]] else NA_real_,
      nll_components = numeric(0),
      max_gradient = NA_real_,
      condition_number = NA_real_,
      largest_gradient_parameter = NA_character_,
      top_gradient_parameters = stats::setNames(numeric(0), character(0)),
      issues = issues,
      warnings = warnings,
      details = list(
        opt_message = "Model was not optimized.",
        nll_components = numeric(0),
        bad_nll_components = character(),
        top_gradient_parameters = stats::setNames(numeric(0), character(0)),
        na_se_parameters = character(),
        large_se_parameters = character()
      )
    )
    class(res) <- "sanity_fims"
    if (verbose) {
      print(res)
    }
    return(invisible(res))
  }

  # 2. nlminb convergence
  nlminb_ok <- isTRUE(opt[["convergence"]] == 0L)
  opt_msg <- if (!is.null(opt[["message"]])) opt[["message"]] else ""
  if (!nlminb_ok) {
    issues <- c(
      issues,
      cli::format_inline(
        "Optimizer did not converge (code {.val {opt[['convergence']]}}: {.val {opt_msg}}). Bummer."
      )
    )
  }

  # 3. Negative log-likelihood checks (marginal, joint, and named components)
  marginal_nll <- if (has_opt && !is.null(opt[["objective"]])) opt[["objective"]] else NA_real_
  total_nll <- if (!is.null(report[["jnll"]])) report[["jnll"]] else NA_real_
  nll_comp <- report[["nll_components"]]
  has_nll_comp <- !is.null(nll_comp) && length(nll_comp) > 0

  # Name the NLL components using fleet and density metadata
  if (has_nll_comp) {
    comp_names <- extract_nll_component_names(
      model_output = get_model_output(object),
      n_components = length(nll_comp)
    )
    names(nll_comp) <- comp_names
  } else {
    nll_comp <- numeric(0)
  }

  marginal_finite <- is.finite(marginal_nll)
  total_finite <- is.na(total_nll) || is.finite(total_nll)
  comp_finite <- if (has_nll_comp) all(is.finite(nll_comp)) else TRUE

  nll_ok <- marginal_finite && total_finite && comp_finite
  bad_nll_components <- character()

  if (!marginal_finite) {
    issues <- c(
      issues,
      cli::format_inline("Marginal negative log-likelihood is not finite ({.val {marginal_nll}}).")
    )
  }

  if (!is.na(total_nll) && !is.finite(total_nll)) {
    issues <- c(
      issues,
      cli::format_inline("Total negative log-likelihood (jnll) is not finite ({.val {total_nll}}).")
    )
  }

  if (has_nll_comp) {
    bad_idx <- which(!is.finite(nll_comp))
    if (length(bad_idx) > 0) {
      bad_nll_components <- names(nll_comp)[bad_idx]
      issues <- c(
        issues,
        cli::format_inline(
          "{length(bad_idx)} NLL component{?s} {?has/have} non-finite values: {.val {bad_nll_components}}."
        )
      )
    }

    extreme_idx <- which(is.finite(nll_comp) & abs(nll_comp) > 1e7)
    if (length(extreme_idx) > 0) {
      warnings <- c(
        warnings,
        cli::format_inline(
          "{length(extreme_idx)} NLL component{?s} {?has/have} extremely large values (> 1e7): {.val {names(nll_comp)[extreme_idx]}}."
        )
      )
    }
  }

  # In fixed-effects models, marginal NLL and total NLL should match
  if (!has_random && marginal_finite && !is.na(total_nll) && is.finite(total_nll)) {
    if (abs(marginal_nll - total_nll) > 0.01) {
      warnings <- c(
        warnings,
        cli::format_inline(
          "Marginal NLL ({.val {round(marginal_nll, 4)}}) and total NLL ({.val {round(total_nll, 4)}}) differ by {.val {round(abs(marginal_nll - total_nll), 4)}} in a fixed-effects model."
        )
      )
    }
  }

  # 4. Maximum gradient check
  param_names <- names(obj[["par"]])
  largest_grad_name <- NA_character_
  top_gradient_parameters <- stats::setNames(numeric(0), character(0))
  if (length(gradient) > 0 && any(!is.na(gradient))) {
    labeled_grad <- label_values(gradient, param_names)
    valid_idx <- which(!is.na(gradient))
    ord <- valid_idx[order(abs(gradient[valid_idx]), decreasing = TRUE)]
    largest_grad_name <- labeled_grad[ord[1]]
    n_top <- min(3L, length(ord))
    top_indices <- ord[seq_len(n_top)]
    top_gradient_parameters <- stats::setNames(abs(gradient[top_indices]), labeled_grad[top_indices])
  }

  gradients_ok <- !is.na(max_grad) && isTRUE(max_grad < gradient_thresh)
  if (!gradients_ok) {
    grad_str <- format(max_grad, scientific = TRUE, digits = 3)
    n_culprits <- length(top_gradient_parameters)
    culprit_label <- if (n_culprits > 1) "culprits" else "culprit"
    top_str <- if (n_culprits > 0) {
      paste0(
        '"', names(top_gradient_parameters), '" (',
        format(unname(top_gradient_parameters), scientific = TRUE, digits = 3), ")",
        collapse = ", "
      )
    } else if (!is.na(largest_grad_name)) {
      paste0('"', largest_grad_name, '"')
    } else {
      "unknown"
    }

    if (!is.na(max_grad) && max_grad <= 0.01) {
      warnings <- c(
        warnings,
        cli::format_inline(
          "Maximum gradient is {.val {grad_str}} (> {.val {gradient_thresh}}). Top gradient {culprit_label}: {top_str}."
        )
      )
    } else {
      issues <- c(
        issues,
        cli::format_inline(
          "Maximum gradient is {.val {grad_str}} (> 0.01). Top gradient {culprit_label}: {top_str}."
        )
      )
    }
  }

  # 5. Hessian positive definite
  hessian_ok <- has_sd && isTRUE(sdreport[["pdHess"]])
  if (!hessian_ok) {
    if (!has_sd) {
      issues <- c(issues, "Standard error report (sdreport) is missing or was not calculated.")
    } else {
      issues <- c(issues, "Hessian is not positive definite; standard errors are unavailable or unreliable.")
    }
  }

  # 6. Condition number check
  condition_number <- tryCatch(
    {
      hessian <- if (has_random) {
        obj[["env"]]$spHess(random = TRUE)
      } else {
        as.matrix(obj[["he"]](opt[["par"]]))
      }
      kappa(hessian)
    },
    error = function(e) {
      # Fallback to cov.fixed if Hessian evaluation fails (e.g. after clear())
      if (has_sd && !is.null(sdreport[["cov.fixed"]]) && is.matrix(sdreport[["cov.fixed"]])) {
        tryCatch(kappa(sdreport[["cov.fixed"]]), error = function(e2) NA_real_)
      } else {
        NA_real_
      }
    }
  )

  condition_ok <- is.finite(condition_number) && (condition_number <= condition_thresh)
  if (is.finite(condition_number) && !condition_ok) {
    cond_str <- format(condition_number, scientific = TRUE, digits = 3)
    warnings <- c(
      warnings,
      cli::format_inline(
        "Hessian condition number is {.val {cond_str}} (> {.val {condition_thresh}}; weak parameter identification)."
      )
    )
  }

  # 7. Standard errors check
  na_se_params <- character()
  large_se_params <- character()

  if (has_sd) {
    # Check fixed effects
    fixed_summary <- tryCatch(
      suppressWarnings(summary(sdreport, "fixed")),
      error = function(e) NULL
    )
    if (!is.null(fixed_summary) && nrow(fixed_summary) > 0 && "Std. Error" %in% colnames(fixed_summary)) {
      fixed_se <- fixed_summary[, "Std. Error"]
      pref_names <- if (!is.null(names(sdreport[["par.fixed"]]))) names(sdreport[["par.fixed"]]) else param_names
      fixed_labels <- label_values(fixed_se, pref_names, fallback = "p")

      bad_na <- is.na(fixed_se) | is.nan(fixed_se) | is.infinite(fixed_se) | fixed_se < 0
      if (any(bad_na)) {
        na_se_params <- c(na_se_params, fixed_labels[bad_na])
      }
      bad_large <- !bad_na & (fixed_se > big_se)
      if (any(bad_large)) {
        large_se_params <- c(large_se_params, fixed_labels[bad_large])
      }
    }

    # Check random effects if present
    if (has_random) {
      random_summary <- tryCatch(
        suppressWarnings(summary(sdreport, "random")),
        error = function(e) NULL
      )
      if (!is.null(random_summary) && nrow(random_summary) > 0 && "Std. Error" %in% colnames(random_summary)) {
        random_se <- random_summary[, "Std. Error"]
        pref_re_names <- names(sdreport[["par.random"]])
        re_labels <- label_values(random_se, pref_re_names, fallback = "re")

        bad_na_re <- is.na(random_se) | is.nan(random_se) | is.infinite(random_se) | random_se < 0
        if (any(bad_na_re)) {
          na_se_params <- c(na_se_params, re_labels[bad_na_re])
        }
        bad_large_re <- !bad_na_re & (random_se > big_se)
        if (any(bad_large_re)) {
          large_se_params <- c(large_se_params, re_labels[bad_large_re])
        }
      }
    }

    # Check derived quantities
    has_report <- "value" %in% names(sdreport) && length(sdreport[["value"]]) > 0
    if (has_report) {
      report_summary <- tryCatch(
        suppressWarnings(summary(sdreport, "report")),
        error = function(e) NULL
      )
      if (!is.null(report_summary) && nrow(report_summary) > 0 && "Std. Error" %in% colnames(report_summary)) {
        report_se <- report_summary[, "Std. Error"]
        rep_labels <- label_values(report_se, rownames(report_summary), fallback = "report")
        bad_na_rep <- is.na(report_se) | is.nan(report_se) | is.infinite(report_se) | report_se < 0
        if (any(bad_na_rep)) {
          na_se_params <- c(na_se_params, rep_labels[bad_na_rep])
        }
      }
    }
  }

  se_na_ok <- length(na_se_params) == 0
  se_magnitude_ok <- length(large_se_params) == 0
  se_ok <- has_sd && se_na_ok && se_magnitude_ok

  if (has_sd) {
    if (!se_na_ok) {
      n_bad <- length(na_se_params)
      shown <- utils::head(na_se_params, 3)
      more_msg <- if (n_bad > 3) paste0(" (", n_bad - 3, " more not shown)") else ""
      issues <- c(
        issues,
        cli::format_inline(
          "{n_bad} parameter{?s} or derived quantit{?y/ies} {?has/have} NA/NaN standard error: {.val {shown}}{more_msg}."
        )
      )
    }
    if (!se_magnitude_ok) {
      n_large <- length(large_se_params)
      shown_l <- utils::head(large_se_params, 3)
      more_l_msg <- if (n_large > 3) paste0(" (", n_large - 3, " more not shown)") else ""
      warnings <- c(
        warnings,
        cli::format_inline(
          "{n_large} parameter{?s} {?has/have} large standard error (> {.val {big_se}}): {.val {shown_l}}{more_l_msg}."
        )
      )
    }
  }

  all_ok <- nlminb_ok && nll_ok && gradients_ok && hessian_ok && isTRUE(condition_ok) && se_ok

  res <- list(
    all_ok = all_ok,
    optimized = optimized,
    nlminb_ok = nlminb_ok,
    nll_ok = nll_ok,
    gradients_ok = gradients_ok,
    hessian_ok = hessian_ok,
    condition_ok = condition_ok,
    se_ok = se_ok,
    se_na_ok = se_na_ok,
    se_magnitude_ok = se_magnitude_ok,
    marginal_nll = marginal_nll,
    total_nll = total_nll,
    nll_components = nll_comp,
    max_gradient = max_grad,
    condition_number = condition_number,
    largest_gradient_parameter = largest_grad_name,
    top_gradient_parameters = top_gradient_parameters,
    issues = issues,
    warnings = warnings,
    details = list(
      opt_message = opt_msg,
      nll_components = nll_comp,
      bad_nll_components = bad_nll_components,
      top_gradient_parameters = top_gradient_parameters,
      na_se_parameters = na_se_params,
      large_se_parameters = large_se_params,
      gradient_thresh = gradient_thresh,
      condition_thresh = condition_thresh,
      big_se = big_se
    )
  )
  class(res) <- "sanity_fims"

  if (verbose) {
    print(res)
  }

  invisible(res)
}

#' Print sanity check results
#'
#' @param x An object of class `"sanity_fims"` returned by [sanity_fims()].
#' @param ... Unused.
#'
#' @return Invisibly returns `x`.
#' @exportS3Method base::print
print.sanity_fims <- function(x, ...) {
  cli::cli_rule(left = "FIMS Model Sanity Check")

  if (!isTRUE(x[["optimized"]])) {
    cli::cli_alert_danger("Model was not optimized ({.code optimize = FALSE}) 😭.")
    cli::cli_rule()
    return(invisible(x))
  }

  # 1. Optimizer convergence
  if (isTRUE(x[["nlminb_ok"]])) {
    msg <- if (nzchar(x[["details"]][["opt_message"]])) {
      paste0(" (", x[["details"]][["opt_message"]], ")")
    } else {
      ""
    }
    cli::cli_alert_success("Optimizer converged ({.code convergence = 0}){msg}. Yay! 🎉.")
  } else {
    cli::cli_alert_danger("Optimizer failed to converge: {x[['details']][['opt_message']]}. It gave up the chase, and now you get to go on one to figure out why 🏃‍♀️.")
  }

  # 2. Negative log-likelihood
  if (isTRUE(x[["nll_ok"]])) {
    m_val <- format(x[["marginal_nll"]], digits = 5)
    t_val <- if (!is.na(x[["total_nll"]])) format(x[["total_nll"]], digits = 5) else m_val
    cli::cli_alert_success(
      "Negative log-likelihood is finite (marginal = {m_val}, total = {t_val})."
    )
    nll_comps <- x[["nll_components"]]
    if (length(nll_comps) > 0) {
      cli::cli_alert_success(
        "All {length(nll_comps)} NLL component{?s} are finite: {.val {names(nll_comps)}}."
      )
    }
  } else {
    bad_comps <- x[["details"]][["bad_nll_components"]]
    if (length(bad_comps) > 0) {
      cli::cli_alert_danger(
        "Non-finite NLL component{?s} detected: {.val {bad_comps}}."
      )
    } else {
      cli::cli_alert_danger("Non-finite negative log-likelihood detected (infinite sadness 😢).")
    }
  }

  # 3. Maximum gradient and top culprits
  max_grad <- x[["max_gradient"]]
  grad_thresh <- x[["details"]][["gradient_thresh"]]
  top_grads <- x[["top_gradient_parameters"]]
  n_culprits <- length(top_grads)
  culprit_label <- if (n_culprits > 1) "culprits" else "culprit"

  top_str <- if (n_culprits > 0) {
    paste0(
      '"', names(top_grads), '" (',
      format(unname(top_grads), scientific = TRUE, digits = 3), ")",
      collapse = ", "
    )
  } else if (!is.na(x[["largest_gradient_parameter"]])) {
    paste0('"', x[["largest_gradient_parameter"]], '"')
  } else {
    "unknown"
  }

  if (isTRUE(x[["gradients_ok"]])) {
    grad_str <- format(max_grad, scientific = TRUE, digits = 3)
    cli::cli_alert_success(
      "Maximum absolute gradient is {grad_str} (<= {grad_thresh})."
    )
  } else if (!is.na(max_grad) && max_grad <= 0.01) {
    grad_str <- format(max_grad, scientific = TRUE, digits = 3)
    cli::cli_alert_warning(
      "Maximum absolute gradient is {grad_str} (> {grad_thresh}). Top gradient {culprit_label}: {top_str}."
    )
  } else {
    grad_str <- if (is.na(max_grad)) "NA" else format(max_grad, scientific = TRUE, digits = 3)
    cli::cli_alert_danger(
      "Maximum absolute gradient is {grad_str} (> 0.01). Top gradient {culprit_label}: {top_str}."
    )
  }

  # 4. Hessian positive definite
  if (isTRUE(x[["hessian_ok"]])) {
    cli::cli_alert_success("Hessian is positive definite. Proper bowl-shaped curvature!")
  } else {
    cli::cli_alert_danger("Hessian is not positive definite; standard errors are unavailable or unreliable.")
  }

  # 5. Condition number
  cond_num <- x[["condition_number"]]
  cond_thresh <- x[["details"]][["condition_thresh"]]
  if (is.finite(cond_num)) {
    cond_str <- format(cond_num, scientific = TRUE, digits = 3)
    if (isTRUE(x[["condition_ok"]])) {
      cli::cli_alert_success("Hessian condition number is {cond_str} (<= {cond_thresh}).")
    } else {
      cli::cli_alert_warning(
        "Hessian condition number is {cond_str} (> {cond_thresh}; parameters are gossiping behind your back -- weak identification)."
      )
    }
  }

  # 6. Standard errors
  big_se_val <- x[["details"]][["big_se"]]
  if (isTRUE(x[["se_ok"]])) {
    cli::cli_alert_success("Standard errors are finite and reasonable (<= {big_se_val}). No runaway uncertainties.")
  } else {
    if (!isTRUE(x[["se_na_ok"]])) {
      n_na <- length(x[["details"]][["na_se_parameters"]])
      shown <- utils::head(x[["details"]][["na_se_parameters"]], 3)
      cli::cli_alert_danger(
        "{n_bad_label(n_na)} NA/NaN standard error: {.val {shown}}.",
        wrap = TRUE
      )
    }
    if (!isTRUE(x[["se_magnitude_ok"]])) {
      n_l <- length(x[["details"]][["large_se_parameters"]])
      shown_l <- utils::head(x[["details"]][["large_se_parameters"]], 3)
      cli::cli_alert_warning(
        "{n_l} {cli::qty(n_l)}parameter{?s} {?has/have} huge standard error (> {big_se_val}): {.val {shown_l}} (essentially unbounded).",
        wrap = TRUE
      )
    }
  }

  # Summary conclusion
  cli::cli_rule()
  if (isTRUE(x[["all_ok"]])) {
    cli::cli_alert_success("All sanity checks passed! Model looks fit as a fiddle 💪!")
  } else {
    n_issues <- length(x[["issues"]])
    n_warnings <- length(x[["warnings"]])
    cli::cli_alert_danger(
      "Sanity checks flagged {n_issues} {cli::qty(n_issues)}issue{?s} and {n_warnings} {cli::qty(n_warnings)}warning{?s}. Proceed with caution (and perhaps more coffee ☕)."
    )
  }

  invisible(x)
}

# Helper for pluralizing error count label
n_bad_label <- function(n) {
  if (n == 1) "1 parameter or derived quantity has" else paste(n, "parameters or derived quantities have")
}

#' Check if an object is of class `sanity_fims`
#'
#' @param x An object to check.
#' @return Logical `TRUE` if `x` inherits from `"sanity_fims"`, `FALSE` otherwise.
#' @export
#' @keywords diagnostics
is.sanity_fims <- function(x) {
  inherits(x, "sanity_fims")
}

#' Extract names for negative log-likelihood components
#'
#' Maps the unnamed vector `report$nll_components` to descriptive names by
#' parsing fleet and density component definitions from the model JSON output.
#' In C++, `nll_components` evaluates density components in the following
#' order: priors (ordered by module id), random effects (ordered by module id),
#' and data likelihoods (ordered by module id).
#'
#' @param model_output A JSON string returned from [get_model_output()].
#' @param n_components Optional integer specifying expected number of components.
#' @return A character vector of component names, matching the length and order of
#'   `report$nll_components`.
#' @noRd
extract_nll_component_names <- function(model_output, n_components = NULL) {
  if (is.null(model_output) || !is.character(model_output) || !nzchar(model_output)) {
    if (!is.null(n_components) && n_components > 0) {
      return(paste0("component_", seq_len(n_components)))
    }
    return(character())
  }

  json_list <- tryCatch(
    jsonlite::fromJSON(model_output, simplifyVector = FALSE),
    error = function(e) NULL
  )
  if (is.null(json_list) || is.null(json_list[["density_components"]])) {
    if (!is.null(n_components) && n_components > 0) {
      return(paste0("component_", seq_len(n_components)))
    }
    return(character())
  }

  # Build data_id -> name map from fleets
  data_id_map <- list()
  if (!is.null(json_list[["fleets"]])) {
    for (flt in json_list[["fleets"]]) {
      flt_name <- if (!is.null(flt[["fleet"]])) flt[["fleet"]] else paste0("fleet_", flt[["module_id"]])
      if (!is.null(flt[["data_ids"]])) {
        for (dtype in names(flt[["data_ids"]])) {
          did <- as.character(flt[["data_ids"]][[dtype]])
          data_id_map[[did]] <- paste(flt_name, dtype, sep = "_")
        }
      }
    }
  }

  # Fallback data map from data entries
  if (!is.null(json_list[["data"]])) {
    for (d in json_list[["data"]]) {
      did <- as.character(d[["id"]])
      if (is.null(data_id_map[[did]])) {
        data_id_map[[did]] <- if (!is.null(d[["name"]])) d[["name"]] else paste0("data_", did)
      }
    }
  }

  # Density components in C++ order: priors, random_effects, data
  dcs <- json_list[["density_components"]]
  mod_ids <- vapply(
    dcs,
    function(x) if (!is.null(x[["module_id"]])) as.integer(x[["module_id"]]) else 0L,
    integer(1)
  )
  dcs <- dcs[order(mod_ids)]

  input_types <- vapply(
    dcs,
    function(x) if (!is.null(x[["input_type"]])) x[["input_type"]] else "data",
    character(1)
  )

  priors <- dcs[input_types == "prior"]
  re <- dcs[input_types == "random_effects"]
  dat <- dcs[input_types == "data"]
  ordered_dcs <- c(priors, re, dat)

  comp_names <- character(length(ordered_dcs))
  prior_idx <- 1L
  re_idx <- 1L

  for (i in seq_along(ordered_dcs)) {
    dc <- ordered_dcs[[i]]
    itype <- if (!is.null(dc[["input_type"]])) dc[["input_type"]] else "data"
    if (itype == "data") {
      obs_id <- as.character(dc[["observed_data_id"]])
      name_val <- data_id_map[[obs_id]]
      if (!is.null(name_val)) {
        comp_names[i] <- name_val
      } else {
        comp_names[i] <- paste0("data_", obs_id)
      }
    } else if (itype == "random_effects") {
      comp_names[i] <- paste0("re_", if (length(re) > 1) re_idx else "recruitment")
      re_idx <- re_idx + 1L
    } else if (itype == "prior") {
      comp_names[i] <- paste0("prior_", if (length(priors) > 1) prior_idx else "parameter")
      prior_idx <- prior_idx + 1L
    } else {
      comp_names[i] <- paste0("component_", i)
    }
  }

  comp_names <- make.unique(comp_names, sep = "_")

  if (!is.null(n_components) && length(comp_names) != n_components) {
    return(paste0("component_", seq_len(n_components)))
  }

  comp_names
}
