#' Check convergence of nlminb optimization
#'
#' Checks the convergence of the nlminb optimization by evaluating the
#' convergence code and maximum gradient. If convergence issues are detected,
#' appropriate warnings are issued and the fit is returned for diagnostic
#' purposes. If the optimizer converged but the maximum gradient is above
#' certain thresholds, warnings are issued about potential convergence concerns.
#'
#' @param input The FIMS input object used for fitting, containing model configuration and data.
#' @param obj The TMB object used for fitting, containing the model environment and random effects
#' @param opt The optimization output from nlminb, containing convergence information.
#' @param maxgrad The maximum absolute gradient from the optimization, used to assess convergence quality.
#' @param filename Optional filename to save the fit object if convergence issues are detected.
#' @return If convergence issues are detected, a FIMSFit object is returned for diagnostics. Otherwise, the function returns NULL and allows the fitting process to continue to sdreport.
#' @noRd
check_mle_convergence <- function(input, obj, opt, maxgrad, filename) {
  # Check convergence status
  convergence_issues <- c()
  convergence_warnings <- c()

  # Check 1: nlminb convergence flag
  if (opt[["convergence"]] != 0) {
    convergence_message <- if (!is.null(opt[["message"]])) {
      paste0("Convergence code = ", opt[["convergence"]], "; message = ", opt[["message"]])
    } else {
      paste0("Convergence code = ", opt[["convergence"]])
    }
    convergence_issues <- c(convergence_issues, convergence_message)
  } else {
    # if optimizer converged, check the gradient to see if it is close enough to zero
    # Check 2: Maximum gradient threshold (warning only)
    if (maxgrad > 1) {
      convergence_issues <- c(
        convergence_issues,
        paste0(
          "Maximum absolute gradient (", format(maxgrad, scientific = TRUE),
          ") is higher than ", 1,
          ". Model does not seem converged."
        )
      )
    } else if (maxgrad > 0.01) {
      convergence_warnings <- c(
        convergence_warnings,
        paste0(
          "Maximum absolute gradient (", format(maxgrad, scientific = TRUE),
          ") is higher than ", 0.01,
          ". Model might not be converged."
        )
      )
    }
  }

  # If optimizer did not converge, skip sdreport, warn, and return fit for diagnostics
  if (length(convergence_issues) > 0) {
    cli::cli_warn(c(
      "x" = "Optimization failed convergence checks.",
      setNames(convergence_issues, rep("i", length(convergence_issues))),
      "i" = "Skipping sdreport. Consider adjusting control parameters (eval.max, iter.max) or model structure.",
      "i" = "Model fit returned for diagnostic purposes only. Results are not reliable."
    ))
  }

  if (length(convergence_warnings) > 0) {
    cli::cli_warn(c(
      "!" = "Optimization resulted in high gradients.",
      setNames(convergence_warnings, rep("i", length(convergence_warnings))),
      "i" = "Model fit returned; results may be less reliable if the gradient is too far from zero.",
      "i" = "Consider adjusting model structure or starting values."
    ))
    fit <- FIMSFit(
      input = input,
      obj = obj,
      opt = opt,
      sdreport = list(),
      timing = c(
        time_optimization = as.difftime(0, units = "secs"),
        time_sdreport = as.difftime(0, units = "secs"),
        time_total = as.difftime(0, units = "secs")
      )
    )
    print(fit)
    if (!is.null(filename)) {
      cli::cli_warn(c(
        "i" = "Saving output to file is not yet implemented."
      ))
      # saveRDS(fit, file=file.path(input[["path"]], filename))
    }
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
#' @param sdreport The sdreport output from TMB, containing standard errors and Hessian information.
#' @return If convergence issues are detected, a FIMSFit object is returned for diagnostics. Otherwise, the function returns NULL and allows the fitting process to continue to sdreport.
#' @noRd
check_sdreport_convergence <- function(input, obj, opt, sdreport, filename) {
  CONDITION_NUMBER_THRESHOLD <- 1e5

  # Check 1: Hessian is invertible (positive definite)
  if (!sdreport[["pdHess"]]) {
    # Skip further checks if Hessian is not positive definite
    # Warn and return output early
    cli::cli_warn(c(
      "x" = "Standard error calculations failed convergence checks:",
      "i" = "Hessian is not positive definite, which may indicate convergence issues or model misspecification.",
      "i" = "Standard errors cannot be reliably calculated, and MLEs may be unreliable.",
      "i" = "Consider simplifying the model, improving data quality, or fixing poorly informed parameters.",
      "i" = "Model fit returned for diagnostic purposes only. Results are not reliable."
    ))
    # Skip the rest of the sdreport checks
    fit <- FIMSFit(
      input = input,
      obj = obj,
      opt = opt,
      sdreport = sdreport,
      timing = c(
        time_optimization = as.difftime(0, units = "secs"),
        time_sdreport = as.difftime(0, units = "secs"),
        time_total = as.difftime(0, units = "secs")
      )
    )
    print(fit)
    if (!is.null(filename)) {
      cli::cli_warn(c(
        "i" = "Saving output to file is not yet implemented."
      ))
      # saveRDS(fit, file=file.path(input[["path"]], filename))
    }
    return(fit)
  }

  # Check 2: Validate standard errors
  # Safely extract fixed effects summary and check for issues
  se_check_result <- tryCatch(
    {
      se_issues <- c()

      fixed_summary <- summary(sdreport, "fixed")

      if (!is.null(fixed_summary) && nrow(fixed_summary) > 0) {
        std_errors <- fixed_summary[, "Std. Error"]

        na_se <- sum(is.na(std_errors))
        if (na_se > 0) {
          se_issues <- c(
            se_issues,
            paste0(na_se, " fixed effect(s) have NA standard errors")
          )
        }
      }

      if (length(obj[["env"]][["random"]]) > 0) {
        random_summary <- summary(sdreport, "random")
        if (!is.null(random_summary) && nrow(random_summary) > 0) {
          std_errors <- random_summary[, "Std. Error"]

          na_se <- sum(is.na(std_errors))
          if (na_se > 0) {
            se_issues <- c(
              se_issues,
              paste0(na_se, " random effect(s) have NA standard errors")
            )
          }
        }
      }


      derived_summary <- summary(sdreport, "report")
      if (!is.null(derived_summary) && nrow(derived_summary) > 0) {
        std_errors <- derived_summary[, "Std. Error"]

        na_se <- sum(is.na(std_errors))
        if (na_se > 0) {
          se_issues <- c(
            se_issues,
            paste0(na_se, " derived value(s) have NA standard errors")
          )
        }
      }
      list(issues = se_issues)
    },
    error = function(e) {
      list(issues = c("Unable to extract summary from sdreport"))
    }
  )

  # Check 3: Condition number of covariance matrix (warning)
  # Safely extract hessian and check condition number
  hessian_check_result <- tryCatch(
    {
      if (length(obj[["env"]][["random"]]) > 0) {
        hessian <- obj[["env"]]$spHess(random = TRUE)
      } else {
        hessian <- as.matrix(obj$he(opt[["par"]]))
      }
      # Compare condition number to threshold
      condition_number <- kappa(hessian)
      sdr_mat <- summary(sdreport) |>
        as.data.frame() |>
        dplyr::arrange(desc(`Std. Error`)) |>
        head(2)


      if (condition_number > CONDITION_NUMBER_THRESHOLD) {
        list(warnings = c(
          paste0(
            "Condition number of Hessian (", format(condition_number, scientific = TRUE),
            ") exceeds threshold of ", CONDITION_NUMBER_THRESHOLD,
            "."
          ),
          "This suggests the model is weakly identified and results may be unreliable.",
          "Consider simplifying the model, improving data quality, or fixing poorly informed parameters.",
          "The two largest standard error values are for parameters: ",
          paste0("  - ", rownames(sdr_mat)[1], ": ", format(sdr_mat[1, "Std. Error"], scientific = TRUE)),
          paste0("  - ", rownames(sdr_mat)[2], ": ", format(sdr_mat[2, "Std. Error"], scientific = TRUE)),
          "Standard errors and MLEs may be unreliable."
        ))
      } else {
        list(warnings = c())
      }
    },
    error = function(e) {
      list(warnings = c("Unable to extract Hessian for condition number check"))
    }
  )

  # Separate issues and warnings
  if (length(se_check_result$issues) > 0) {
    cli::cli_warn(c(
      "x" = "sdreport convergence issues detected:",
      setNames(se_check_result$issues, rep("i", length(se_check_result$issues)))
    ))
  } else {
    if (length(hessian_check_result$warnings) > 0) {
      cli::cli_warn(c(
        "!" = "Large condition number detected in Hessian indicating it may be near singularity",
        setNames(hessian_check_result$warnings, rep("i", length(hessian_check_result$warnings)))
      ))
    }
  }
}
