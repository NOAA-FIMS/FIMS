#' Label values for convergence messages
#'
#' Returns one label per value so that convergence messages can point at a
#' specific parameter or derived quantity.
#'
#' @param values A vector of values, e.g., gradients or standard errors.
#' @param value_labels A character vector of labels with the same length as
#'   `values`, or `NULL`.
#' @param fallback A string used to build labels, e.g., `"p[2]"`, when
#'   `value_labels` is `NULL` or does not line up with `values`.
#' @return
#' A character vector the same length as `values`. Repeated labels, e.g., the
#' rows of an ADREPORT vector, are indexed within their name, e.g.,
#' `"biomass[3]"`.
#' @noRd
label_values <- function(values, value_labels = NULL, fallback = "p") {
  n <- length(values)
  if (n == 0) {
    return(character(0))
  }
  # Labels that do not line up with the values could name the wrong parameter
  if (is.null(value_labels) || length(value_labels) != n) {
    return(sprintf("%s[%d]", fallback, seq_len(n)))
  }
  position <- stats::ave(seq_len(n), value_labels, FUN = seq_along)
  repeated <- duplicated(value_labels) |
    duplicated(value_labels, fromLast = TRUE)
  ifelse(repeated, sprintf("%s[%d]", value_labels, position), value_labels)
}

#' Build rows of the convergence table
#'
#' Returns the rows stored in the `convergence` slot of a FIMSFit object, so
#' the result of each check can be read after the warnings have scrolled by,
#' e.g., in a simulation with hundreds of fits.
#'
#' @param check A character vector naming each check, e.g., `"max_gradient"`.
#' @param severity A character vector with one of `"OK"`, `"NOTE"`, `"WARN"`,
#'   or `"FAIL"` per check. `"NOTE"` marks a check that did not run.
#' @param value The value that was checked, e.g., the maximum gradient.
#' @param threshold The threshold that `value` was compared with.
#' @param parameter The parameter the check points to, e.g., the parameter
#'   with the largest gradient.
#' @param message A short description of the result.
#' @return
#' A tibble with one row per check and the columns `check`, `severity`,
#' `value`, `threshold`, `parameter`, and `message`.
#' @noRd
convergence_check <- function(
  check,
  severity,
  value = NA_real_,
  threshold = NA_real_,
  parameter = NA_character_,
  message = NA_character_
) {
  tibble::tibble(
    check = check,
    severity = severity,
    value = as.numeric(value),
    threshold = as.numeric(threshold),
    parameter = as.character(parameter),
    message = as.character(message)
  )
}

#' Check convergence of nlminb optimization
#'
#' Checks the convergence of the nlminb optimization by evaluating the
#' convergence code and maximum gradient. A non-zero convergence code or a
#' maximum gradient above 1 gives a warning that the optimization failed. A
#' maximum gradient above 0.01 gives a warning that the model might not be
#' converged.
#'
#' @param opt The optimization output from nlminb, containing convergence
#' information.
#' @param maxgrad The maximum absolute gradient from the optimization, used to
#' assess convergence quality.
#' @param gradient The gradient vector of the fixed effects at the optimum, used
#' to name the parameter with the largest absolute gradient. The default of
#' `NULL` leaves the name out of the messages.
#' @param parameter_names A character vector of fixed-effect names, e.g., from
#' [get_parameter_names()], in the same order as `gradient`. If `NULL`, labels
#' such as `"p[2]"` are used.
#' @return
#' A tibble from `convergence_check()` with the rows `"optimizer"` and
#' `"max_gradient"`. The `threshold` of `"max_gradient"` is 1 when it fails
#' and 0.01 otherwise. [fit_fims()] skips sdreport when `"max_gradient"` fails.
#' @noRd
check_mle_convergence <- function(
  opt,
  maxgrad,
  gradient = NULL,
  parameter_names = NULL
) {
  optimizer_failed <- opt[["convergence"]] != 0
  # A gradient that is not finite cannot be compared with the thresholds and
  # means the fit failed.
  gradient_severity <- if (!is.finite(maxgrad) || maxgrad > 1) {
    "FAIL"
  } else if (maxgrad > 0.01) {
    "WARN"
  } else {
    "OK"
  }

  # which.max() drops NA and NaN, so an all-NaN gradient would give an empty
  # label
  largest_gradient_message <- NULL
  largest_gradient_label <- NA_character_
  if (length(gradient) > 0 && any(!is.na(gradient))) {
    largest_gradient_label <- label_values(gradient, parameter_names)[
      which.max(abs(gradient))
    ]
    largest_gradient_message <- cli::format_inline(
      "Largest absolute gradient is on {.val {largest_gradient_label}}."
    )
  }

  convergence_issues <- c()
  if (optimizer_failed) {
    convergence_issues <- c(
      convergence_issues,
      cli::format_inline("Convergence code = {.val {opt[['convergence']]}}."),
      if (!is.null(opt[["message"]])) {
        cli::format_inline("Message = {.val {opt[['message']]}}.")
      }
    )
  }
  if (gradient_severity == "FAIL") {
    convergence_issues <- c(
      convergence_issues,
      cli::format_inline(
        "Maximum absolute gradient
        ({.val {format(maxgrad, scientific = TRUE)}})
        is higher than {.val {1}} or not finite. Model does not seem converged."
      )
    )
  } else if (gradient_severity == "WARN") {
    convergence_issues <- c(
      convergence_issues,
      cli::format_inline(
        "Maximum absolute gradient
        ({.val {format(maxgrad, scientific = TRUE)}})
        is higher than {.val {0.01}}. Model might not be converged."
      )
    )
  }
  convergence_issues <- c(convergence_issues, largest_gradient_message)

  if (optimizer_failed || gradient_severity == "FAIL") {
    cli::cli_warn(c(
      "x" = "Optimization failed convergence checks.",
      stats::setNames(
        convergence_issues,
        rep("i", length(convergence_issues))
      ),
      # Standard errors are still worth having when only the convergence code
      # failed, e.g., a false convergence with a small gradient.
      "i" = if (gradient_severity == "FAIL") {
        "Skipping sdreport. Consider adjusting control parameters
        (eval.max, iter.max) or model structure."
      } else {
        "Consider adjusting control parameters (eval.max, iter.max) or model
        structure."
      },
      "i" = "Model fit returned for diagnostic purposes only. Results are not
        reliable."
    ))
  } else if (gradient_severity == "WARN") {
    cli::cli_warn(c(
      "!" = "Optimization resulted in high gradients.",
      stats::setNames(
        convergence_issues,
        rep("i", length(convergence_issues))
      ),
      "i" = "Results may be less reliable than results with a smaller gradient,
        where the target gradient is close to {.val {0}}.",
      "i" = "Consider adjusting model structure, control parameters, or
        starting values.",
      "i" = "Model fit returned for diagnostic purposes only. Results might not
        be reliable."
    ))
  }

  invisible(dplyr::bind_rows(
    convergence_check(
      check = "optimizer",
      severity = if (optimizer_failed) "FAIL" else "OK",
      value = opt[["convergence"]],
      threshold = 0,
      message = if (is.null(opt[["message"]])) NA else opt[["message"]]
    ),
    convergence_check(
      check = "max_gradient",
      severity = gradient_severity,
      value = maxgrad,
      threshold = if (gradient_severity == "FAIL") 1 else 0.01,
      parameter = largest_gradient_label,
      message = switch(gradient_severity,
        "FAIL" = "Maximum absolute gradient is above 1 or not finite.",
        "WARN" = "Maximum absolute gradient is above 0.01.",
        "OK" = "Maximum absolute gradient is at or below 0.01."
      )
    )
  ))
}

#' Check convergence of sdreport and standard errors
#'
#' Checks the convergence of the sdreport step by evaluating the positive
#' definiteness of the Hessian, the presence of NA standard errors, and the
#' condition number of the Hessian. A Hessian that is not positive definite or
#' NA standard errors give a warning that the standard errors failed. A
#' condition number above the threshold gives a warning that the Hessian may be
#' near singular.
#' @inheritParams check_mle_convergence
#' @param obj The TMB object used for fitting, used to compute the Hessian.
#' @param sdreport The sdreport output from TMB, containing standard errors and
#' Hessian information.
#' @param random_effects_names A character vector of random-effect names, e.g.,
#' from [get_random_names()], in the same order as the random effects in
#' `sdreport`. If `NULL`, labels such as `"re[2]"` are used.
#' @return
#' A tibble from `convergence_check()` with the row `"hessian"` and, when the
#' Hessian is positive definite, the rows `"standard_errors"` and
#' `"condition_number"`.
#' @noRd
check_sdreport_convergence <- function(
  obj,
  opt,
  sdreport,
  parameter_names = NULL,
  random_effects_names = NULL
) {
  condition_number_threshold <- 1e5
  has_random_effects <- length(obj[["env"]][["random"]]) > 0
  # TMB labels every fixed effect "p" and every random effect "re", so prefer
  # the FIMS names and only fall back to the summary row names without them.
  summary_labels <- function(summary_matrix, preferred_labels, fallback) {
    row_labels <- if (is.null(preferred_labels)) {
      rownames(summary_matrix)
    } else {
      preferred_labels
    }
    label_values(summary_matrix[, "Std. Error"], row_labels, fallback)
  }
  format_na_se_issue <- function(std_errors, value_labels, noun) {
    is_na <- is.na(std_errors)
    na_se <- sum(is_na)
    if (na_se == 0) {
      return(NULL)
    }
    # Long models can have hundreds of NA standard errors, so only name a few
    shown <- utils::head(value_labels[is_na], 5)
    n_more <- na_se - length(shown)
    # qty() is needed because cli otherwise pluralizes on the length of `noun`
    issue_text <- cli::format_inline(
      "{na_se} {noun}{cli::qty(na_se)}{?s} {?has/have} NA standard
      error{?s}: {.val {shown}}"
    )
    if (n_more > 0) {
      issue_text <- paste0(issue_text, " (", n_more, " more not shown)")
    }
    paste0(issue_text, ".")
  }

  # Check 1: Hessian is invertible (positive definite)
  if (!sdreport[["pdHess"]]) {
    # Skip further checks if Hessian is not positive definite
    # Warn and return output early
    cli::cli_warn(c(
      "x" = "Standard error calculations failed convergence checks:",
      "i" = "Hessian is not positive definite, which may indicate convergence
        issues or model misspecification.",
      "i" = "Standard errors cannot be reliably calculated, and MLEs may be
        unreliable.",
      "i" = "Consider simplifying the model, improving data quality, or fixing
        poorly informed parameters.",
      "i" = "Model fit returned for diagnostic purposes only. Results are not
        reliable."
    ))
    # Skip the rest of the sdreport checks
    return(invisible(convergence_check(
      check = "hessian",
      severity = "FAIL",
      message = "Hessian is not positive definite."
    )))
  }

  # Check 2: Validate standard errors
  # Safely extract fixed effects summary and check for issues
  se_check_result <- tryCatch(
    {
      se_issues <- c()
      na_se_labels <- character(0)

      fixed_summary <- summary(sdreport, "fixed")

      if (!is.null(fixed_summary) && nrow(fixed_summary) > 0) {
        fixed_labels <- summary_labels(fixed_summary, parameter_names, "p")
        issue <- format_na_se_issue(
          fixed_summary[, "Std. Error"],
          fixed_labels,
          "fixed effect"
        )
        if (!is.null(issue)) {
          se_issues <- c(se_issues, issue)
          na_se_labels <- c(
            na_se_labels,
            fixed_labels[is.na(fixed_summary[, "Std. Error"])]
          )
        }
      }

      if (has_random_effects) {
        random_summary <- summary(sdreport, "random")
        if (!is.null(random_summary) && nrow(random_summary) > 0) {
          random_labels <- summary_labels(
            random_summary,
            random_effects_names,
            "re"
          )
          issue <- format_na_se_issue(
            random_summary[, "Std. Error"],
            random_labels,
            "random effect"
          )
          if (!is.null(issue)) {
            se_issues <- c(se_issues, issue)
            na_se_labels <- c(
              na_se_labels,
              random_labels[is.na(random_summary[, "Std. Error"])]
            )
          }
        }
      }


      derived_summary <- summary(sdreport, "report")
      if (!is.null(derived_summary) && nrow(derived_summary) > 0) {
        derived_labels <- summary_labels(derived_summary, NULL, "report")
        issue <- format_na_se_issue(
          derived_summary[, "Std. Error"],
          derived_labels,
          "derived value"
        )
        if (!is.null(issue)) {
          se_issues <- c(se_issues, issue)
          na_se_labels <- c(
            na_se_labels,
            derived_labels[is.na(derived_summary[, "Std. Error"])]
          )
        }
      }
      list(issues = se_issues, na_se_labels = na_se_labels)
    },
    error = function(e) {
      list(
        issues = c("Unable to extract summary from sdreport"),
        na_se_labels = NULL
      )
    }
  )

  # Check 3: Condition number of covariance matrix (warning)
  # Rank the same block of parameters the Hessian below covers (random effects
  # when there are any, otherwise fixed effects). Derived quantities are left
  # out because their standard errors are on the scale of the output, e.g.,
  # numbers at age, and would crowd out the parameters. This is kept separate
  # from the Hessian tryCatch so a failed ranking does not hide the condition
  # number.
  effect_type <- if (has_random_effects) "random" else "fixed"
  largest_se <- tryCatch(
    {
      ranked_summary <- summary(sdreport, effect_type)
      if (is.null(ranked_summary) || nrow(ranked_summary) == 0) {
        NULL
      } else {
        ranked_labels <- if (has_random_effects) {
          summary_labels(ranked_summary, random_effects_names, "re")
        } else {
          summary_labels(ranked_summary, parameter_names, "p")
        }
        std_errors <- ranked_summary[, "Std. Error"]
        # NA and NaN standard errors are reported by Check 2 and would
        # otherwise be listed here as the "largest"; Inf is kept and ranks first
        ranked <- order(std_errors, decreasing = TRUE)
        ranked <- ranked[!is.na(std_errors[ranked])]
        if (length(ranked) == 0) {
          NULL
        } else {
          utils::head(
            data.frame(
              label = ranked_labels[ranked],
              std_error = std_errors[ranked]
            ),
            2
          )
        }
      }
    },
    error = function(e) NULL
  )

  # Safely extract hessian and check condition number
  hessian_check_result <- tryCatch(
    {
      if (has_random_effects) {
        hessian <- obj[["env"]]$spHess(random = TRUE)
      } else {
        hessian <- as.matrix(obj$he(opt[["par"]]))
      }
      # Compare condition number to threshold
      condition_number <- kappa(hessian)

      if (condition_number > condition_number_threshold) {
        n_show <- if (is.null(largest_se)) 0 else nrow(largest_se)
        largest_se_messages <- if (n_show > 0) {
          vapply(
            seq_len(n_show),
            function(i) {
              cli::format_inline(
                "{i}. {.val {largest_se[['label']][i]}}:
                {.val {format(largest_se[['std_error']][i], scientific = TRUE)}}"
              )
            },
            character(1)
          )
        } else {
          character(0)
        }

        warning_bullets <- c(
          cli::format_inline(
            "Condition number of Hessian ({.val {format(condition_number, scientific = TRUE)}}) exceeds threshold of {.val {condition_number_threshold}}."
          ),
          "This suggests the model is weakly identified and results may be unreliable.",
          "Consider simplifying the model, improving data quality, or fixing poorly informed parameters."
        )

        if (n_show > 0) {
          warning_bullets <- c(
            warning_bullets,
            cli::format_inline(
              "Among {effect_type} effects, the {n_show} largest standard error
              value{?s} {?is/are}:"
            ),
            largest_se_messages
          )
        } else {
          warning_bullets <- c(
            warning_bullets,
            "Unable to rank parameters by standard error."
          )
        }

        warning_bullets <- c(
          warning_bullets,
          "Standard errors and MLEs may be unreliable."
        )

        list(warnings = warning_bullets, condition_number = condition_number)
      } else {
        list(warnings = c(), condition_number = condition_number)
      }
    },
    error = function(e) {
      list(
        warnings = c("Unable to extract Hessian for condition number check."),
        condition_number = NA_real_
      )
    }
  )

  # Issues and warnings are reported independently: NA standard errors are
  # often a symptom of a near-singular Hessian, so the conditioning warning is
  # most useful exactly when NA standard errors are present.
  if (length(se_check_result[["issues"]]) > 0) {
    cli::cli_warn(c(
      "x" = "sdreport convergence issues detected:",
      stats::setNames(
        se_check_result[["issues"]],
        rep("i", length(se_check_result[["issues"]]))
      )
    ))
  }
  if (length(hessian_check_result[["warnings"]]) > 0) {
    cli::cli_warn(c(
      "!" = "Large condition number detected in Hessian; the matrix may be near singular.",
      stats::setNames(
        hessian_check_result[["warnings"]],
        rep("i", length(hessian_check_result[["warnings"]]))
      )
    ))
  }

  # NULL when the summary could not be extracted, so the count is unknown
  na_se_labels <- se_check_result[["na_se_labels"]]
  condition_number <- hessian_check_result[["condition_number"]]
  condition_severity <- if (is.na(condition_number)) {
    "NOTE"
  } else if (condition_number > condition_number_threshold) {
    "WARN"
  } else {
    "OK"
  }
  invisible(dplyr::bind_rows(
    convergence_check(
      check = "hessian",
      severity = "OK",
      message = "Hessian is positive definite."
    ),
    convergence_check(
      check = "standard_errors",
      severity = if (length(se_check_result[["issues"]]) > 0) "FAIL" else "OK",
      value = if (is.null(na_se_labels)) NA else length(na_se_labels),
      threshold = 0,
      parameter = if (length(na_se_labels) > 0) {
        paste(utils::head(na_se_labels, 5), collapse = ", ")
      } else {
        NA
      },
      # The stored message is read outside the console, e.g., saved to a file,
      # so it is kept free of terminal colors.
      message = if (length(se_check_result[["issues"]]) > 0) {
        cli::ansi_strip(paste(se_check_result[["issues"]], collapse = " "))
      } else {
        "No standard errors are NA."
      }
    ),
    convergence_check(
      check = "condition_number",
      severity = condition_severity,
      value = condition_number,
      threshold = condition_number_threshold,
      parameter = if (is.null(largest_se)) NA else largest_se[["label"]][1],
      message = switch(condition_severity,
        "NOTE" = "Unable to extract the Hessian for the condition number.",
        "WARN" = "Condition number of the Hessian is above the threshold.",
        "OK" = "Condition number of the Hessian is at or below the threshold."
      )
    )
  ))
}
