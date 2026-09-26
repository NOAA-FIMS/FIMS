#' Label values for convergence messages
#'
#' Returns one label per value so that convergence messages can point at a
#' specific parameter or derived quantity.
#'
#' @param values A vector of values, e.g., gradients or standard errors.
#' @param labels A character vector of labels with the same length as
#'   `values`, or `NULL`.
#' @param fallback A string used to build labels, e.g., `"p[2]"`, when
#'   `labels` is `NULL` or does not line up with `values`.
#' @return
#' A character vector the same length as `values`. Repeated labels, e.g., the
#' rows of an ADREPORT vector, are indexed within their name, e.g.,
#' `"biomass[3]"`. FIMS parameter names are made readable with
#' `readable_parameter_labels()`, and braces are escaped so the labels can be
#' placed in cli messages.
#' @noRd
label_values <- function(values, labels = NULL, fallback = "p") {
  n <- length(values)
  if (n == 0) {
    return(character(0))
  }
  # A length mismatch means the labels cannot be trusted to line up, so index
  # the fallback rather than risk naming the wrong parameter. The fallback is
  # always indexed so a message never shows a bare "p".
  if (is.null(labels) || length(labels) != n) {
    return(sprintf("%s[%d]", fallback, seq_len(n)))
  }
  labels <- as.character(labels)
  # A missing label cannot be indexed within its name, so fall back per value
  labels[is.na(labels)] <- sprintf("%s[%d]", fallback, which(is.na(labels)))
  position <- stats::ave(seq_len(n), labels, FUN = seq_along)
  repeated <- duplicated(labels) | duplicated(labels, fromLast = TRUE)
  ifelse(repeated, sprintf("%s[%d]", labels, position), labels) |>
    readable_parameter_labels() |>
    # Labels are formatted with cli::format_inline() and then passed to
    # cli::cli_warn(), which interpolates `{}` a second time. Fleet names come
    # from the user, so a brace in a name would otherwise be evaluated as code.
    gsub(pattern = "([{}])", replacement = "\\1\\1")
}

#' Make FIMS parameter names readable
#'
#' FIMS parameter names have the form `module_name.module_id.label.
#' parameter_id`, e.g., `"Selectivity.2.slope.47"`, which is hard to read in a
#' warning. This splits them into the same pieces as the columns returned by
#' [get_estimates()] so users can find the row.
#'
#' @param labels A character vector of labels.
#' @param module_links A tibble with the columns `fleet`, `module_name`,
#'   `module_id`, and `describes`, i.e., the `"module_links"` attribute of the
#'   list returned by [initialize_fims()], or `NULL`. When supplied, the fleet
#'   name is added to fleet, selectivity, and data-distribution parameters, and
#'   process-distribution parameters say what they describe, e.g.,
#'   `"dnorm 7 (Recruitment log_devs): log_sd (parameter_id 763)"`.
#' @return
#' A character vector the same length as `labels`, e.g.,
#' `"Selectivity 2 (survey1): slope (parameter_id 47)"`, or without the fleet
#' when it is unknown. Labels that do not follow the FIMS pattern, e.g.,
#' `"p[2]"` or `"biomass[3]"`, are returned unchanged, so the function can be
#' applied more than once.
#' @noRd
readable_parameter_labels <- function(labels, module_links = NULL) {
  # Same pieces as reshape_tmb_estimates(), but only for labels that have all
  # four so other labels, e.g., "p[2]" or already readable ones, are unchanged
  fims_pattern <- "^([^.]+)\\.([0-9]+)\\.([^.]+)\\.([0-9]+)$"
  is_fims_name <- grepl(fims_pattern, labels)
  if (!any(is_fims_name)) {
    return(labels)
  }
  fims_names <- labels[is_fims_name]
  module_name <- sub(fims_pattern, "\\1", fims_names)
  module_id <- sub(fims_pattern, "\\2", fims_names)
  label <- sub(fims_pattern, "\\3", fims_names)
  parameter_id <- sub(fims_pattern, "\\4", fims_names)

  alias <- rep(NA_character_, length(fims_names))
  if (!is.null(module_links) && nrow(module_links) > 0) {
    # Distribution modules share one id counter in C++, so any distribution
    # prefix is looked up under the same name
    lookup_name <- ifelse(
      module_name %in% c("dnorm", "dlnorm", "dmultinom"),
      "distribution",
      module_name
    )
    link_keys <- paste(
      module_links[["module_name"]],
      module_links[["module_id"]]
    )
    fleet <- module_links[["fleet"]]
    describes <- module_links[["describes"]]
    link_text <- paste(
      ifelse(is.na(fleet), "", fleet),
      ifelse(is.na(describes), "", describes)
    ) |>
      trimws()
    # A module listed twice is ambiguous, so it is left without an alias
    ambiguous <- duplicated(link_keys) | duplicated(link_keys, fromLast = TRUE)
    link_text[ambiguous | link_text == ""] <- NA_character_
    alias <- link_text[match(paste(lookup_name, module_id), link_keys)]
  }
  module <- ifelse(
    is.na(alias),
    paste(module_name, module_id),
    paste0(module_name, " ", module_id, " (", alias, ")")
  )
  labels[is_fims_name] <- sprintf(
    "%s: %s (parameter_id %s)",
    module, label, parameter_id
  )
  labels
}

#' Check convergence of nlminb optimization
#'
#' Checks the convergence of the nlminb optimization by evaluating the
#' convergence code and maximum gradient. If convergence issues are detected,
#' appropriate warnings are issued and the fit is returned for diagnostic
#' purposes. If the optimizer converged but the maximum gradient is above
#' certain thresholds, warnings are issued about potential convergence concerns.
#'
#' @param input The FIMS input object used for fitting, containing model
#' configuration and data.
#' @param obj The TMB object used for fitting, containing the model environment
#' and random effects.
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
#' If convergence issues are detected, a FIMSFit object is returned for
#' diagnostics. Otherwise, the function returns NULL.
#' @noRd
check_mle_convergence <- function(
  input,
  obj,
  opt,
  maxgrad,
  gradient = NULL,
  parameter_names = NULL
) {
  # Check convergence status
  convergence_issues <- c()
  convergence_warnings <- c()

  # which.max() drops NA and NaN, so an all-NaN gradient would give an empty
  # label; Inf is kept because it is the most informative gradient to name
  largest_gradient_message <- NULL
  if (length(gradient) > 0 && any(!is.na(gradient))) {
    largest_gradient_label <- label_values(gradient, parameter_names)[
      which.max(abs(gradient))
    ]
    largest_gradient_message <- cli::format_inline(
      "Largest absolute gradient is on {.val {largest_gradient_label}}."
    )
  }

  # Check 1: nlminb convergence flag
  if (opt[["convergence"]] != 0) {
    convergence_message <- if (!is.null(opt[["message"]])) {
      c(
        cli::format_inline("Convergence code = {.val {opt[['convergence']]}}."),
        cli::format_inline("Message = {.val {opt[['message']]}}.")
      )
    } else {
      cli::format_inline("Convergence code = {.val {opt[['convergence']]}}.")
    }
    convergence_issues <- c(
      convergence_issues,
      convergence_message,
      largest_gradient_message
    )
  } else {
    # if optimizer converged, check the gradient to see if it is close enough
    # to zero
    # Check 2: Maximum gradient threshold (warning only)
    if (maxgrad > 1) {
      convergence_issues <- c(
        convergence_issues,
        cli::format_inline(
          "Maximum absolute gradient
          ({.val {format(maxgrad, scientific = TRUE)}})
          is higher than {.val {1}}. Model does not seem converged."
        ),
        largest_gradient_message
      )
    } else if (maxgrad > 0.01) {
      convergence_warnings <- c(
        convergence_warnings,
        cli::format_inline(
          "Maximum absolute gradient
          ({.val {format(maxgrad, scientific = TRUE)}})
          is higher than {.val {0.01}}. Model might not be converged."
        ),
        largest_gradient_message
      )
    }
  }

  # If optimizer did not converge, skip sdreport, warn, and return fit for
  # diagnostics
  if (length(convergence_issues) > 0) {
    warning_bullets <- c(
      "x" = "Optimization failed convergence checks.",
      setNames(convergence_issues, rep("i", length(convergence_issues))),
      "i" = "Skipping sdreport. Consider adjusting control parameters
        (eval.max, iter.max) or model structure.",
      "i" = "Model fit returned for diagnostic purposes only. Results are not
        reliable."
    )
    cli::cli_warn(warning_bullets)
  }

  if (length(convergence_warnings) > 0) {
    warning_bullets <- c(
      "!" = "Optimization resulted in high gradients.",
      setNames(convergence_warnings, rep("i", length(convergence_warnings))),
      "i" = "Results may be less reliable than results with a smaller gradient,
        where the target gradient is close to {.val {0}}.",
      "i" = "Consider adjusting model structure, control parameters, or
        starting values.",
      "i" = "Model fit returned for diagnostic purposes only. Results might not
        be reliable."
    )
    cli::cli_warn(warning_bullets)
    fit <- FIMSFit(
      input = input,
      obj = obj,
      opt = opt,
      sdreport = list(),
      run_time = c(
        time_optimization = as.difftime(0, units = "secs"),
        time_sdreport = as.difftime(0, units = "secs"),
        time_total = as.difftime(0, units = "secs")
      )
    )
    print(fit)
    return(fit)
  }
}

#' Check convergence of sdreport and standard errors
#'
#' Checks the convergence of the sdreport step by evaluating the positive
#' definiteness of the Hessian, the presence of NA standard errors, and the
#' condition number of the Hessian. If convergence issues are detected
#' (e.g., non-positive definite Hessian, NA standard errors), appropriate
#' warnings are issued and the fit is returned for diagnostic purposes.
#' If the Hessian is near singular (high condition number), a warning is issued
#' about potential unreliability of standard errors and MLEs.
#' @inheritParams check_mle_convergence
#' @param sdreport The sdreport output from TMB, containing standard errors and
#' Hessian information.
#' @param random_effects_names A character vector of random-effect names, e.g.,
#' from [get_random_names()], in the same order as the random effects in
#' `sdreport`. If `NULL`, labels such as `"re[2]"` are used.
#' @return
#' If convergence issues are detected, a FIMSFit object is returned for
#' diagnostics. Otherwise, the function returns NULL and allows the fitting
#' process to continue to sdreport.
#' @noRd
check_sdreport_convergence <- function(
  input,
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
  summary_labels <- function(summary_matrix, names, fallback) {
    labels <- if (is.null(names)) rownames(summary_matrix) else names
    label_values(summary_matrix[, "Std. Error"], labels, fallback)
  }
  format_na_se_issue <- function(std_errors, labels, noun) {
    is_na <- is.na(std_errors)
    na_se <- sum(is_na)
    if (na_se == 0) {
      return(NULL)
    }
    # Long models can have hundreds of NA standard errors, so only name a few
    shown <- utils::head(labels[is_na], 5)
    n_more <- na_se - length(shown)
    # qty() is needed because cli otherwise pluralizes on the length of `noun`
    message <- cli::format_inline(
      "{na_se} {noun}{cli::qty(na_se)}{?s} {?has/have} NA standard
      error{?s}: {.val {shown}}"
    )
    if (n_more > 0) {
      message <- paste0(message, " (", n_more, " more not shown)")
    }
    paste0(message, ".")
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
    fit <- FIMSFit(
      input = input,
      obj = obj,
      opt = opt,
      sdreport = sdreport,
      run_time = c(
        time_optimization = as.difftime(0, units = "secs"),
        time_sdreport = as.difftime(0, units = "secs"),
        time_total = as.difftime(0, units = "secs")
      )
    )
    print(fit)
    return(fit)
  }

  # Check 2: Validate standard errors
  # Safely extract fixed effects summary and check for issues
  se_check_result <- tryCatch(
    {
      se_issues <- c()

      fixed_summary <- summary(sdreport, "fixed")

      if (!is.null(fixed_summary) && nrow(fixed_summary) > 0) {
        issue <- format_na_se_issue(
          fixed_summary[, "Std. Error"],
          summary_labels(fixed_summary, parameter_names, "p"),
          "fixed effect"
        )
        if (!is.null(issue)) {
          se_issues <- c(se_issues, issue)
        }
      }

      if (has_random_effects) {
        random_summary <- summary(sdreport, "random")
        if (!is.null(random_summary) && nrow(random_summary) > 0) {
          issue <- format_na_se_issue(
            random_summary[, "Std. Error"],
            summary_labels(random_summary, random_effects_names, "re"),
            "random effect"
          )
          if (!is.null(issue)) {
            se_issues <- c(se_issues, issue)
          }
        }
      }


      derived_summary <- summary(sdreport, "report")
      if (!is.null(derived_summary) && nrow(derived_summary) > 0) {
        issue <- format_na_se_issue(
          derived_summary[, "Std. Error"],
          summary_labels(derived_summary, NULL, "report"),
          "derived value"
        )
        if (!is.null(issue)) {
          se_issues <- c(se_issues, issue)
        }
      }
      list(issues = se_issues)
    },
    error = function(e) {
      list(issues = c("Unable to extract summary from sdreport"))
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
        labels <- if (has_random_effects) {
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
            data.frame(label = labels[ranked], std_error = std_errors[ranked]),
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
              std_error <- format(
                largest_se[["std_error"]][i],
                scientific = TRUE
              )
              cli::format_inline(
                "{i}. {.val {largest_se[['label']][i]}}: {.val {std_error}}"
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

        list(warnings = warning_bullets)
      } else {
        list(warnings = c())
      }
    },
    error = function(e) {
      list(
        warnings = c("Unable to extract Hessian for condition number check.")
      )
    }
  )

  # Issues and warnings are reported independently: NA standard errors are
  # often a symptom of a near-singular Hessian, so the conditioning warning is
  # most useful exactly when NA standard errors are present.
  if (length(se_check_result[["issues"]]) > 0) {
    cli::cli_warn(c(
      "x" = "sdreport convergence issues detected:",
      setNames(
        se_check_result[["issues"]],
        rep("i", length(se_check_result[["issues"]]))
      )
    ))
  }
  if (length(hessian_check_result[["warnings"]]) > 0) {
    cli::cli_warn(c(
      "!" = "Large condition number detected in Hessian; the matrix may be near singular.",
      setNames(
        hessian_check_result[["warnings"]],
        rep("i", length(hessian_check_result[["warnings"]]))
      )
    ))
  }
}
