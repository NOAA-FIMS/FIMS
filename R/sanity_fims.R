#' Check whether a fitted FIMS model is ready to use
#'
#' @description
#' Prints the result of each convergence check that [fit_fims()] stored on
#' the fit, similar to `sdmTMB::sanity()`. The checks and thresholds are the
#' ones [fit_fims()] warns with:
#' * `optimizer`: the nlminb convergence code is 0.
#' * `max_gradient`: the maximum absolute gradient is below 0.001. It fails
#'   above 0.01, and then sdreport is skipped.
#' * `nll`: the objective, the total negative log-likelihood, and each
#'   negative log-likelihood component are finite.
#' * `hessian`: the Hessian is positive definite.
#' * `standard_errors`: no standard error is NA.
#' * `large_standard_errors`: no fixed or random effect has a standard error
#'   above 1000.
#' * `condition_number`: the condition number of the Hessian is at or below
#'   1e5.
#'
#' The checks are read from the fit rather than recomputed, so this works
#' after [clear()].
#'
#' @param fit An object of class `FIMSFit` returned by [fit_fims()].
#' @return
#' Invisibly, a list with `all_ok`, which is `TRUE` when every check is
#' `"OK"`, and `checks`, the tibble returned by [get_convergence()].
#' @export
#' @keywords fit_fims
#' @examples
#' \dontrun{
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#' fit <- setup_default_parameters(data = data_4_model) |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#' sanity <- sanity_fims(fit)
#' sanity[["all_ok"]]
#' }
sanity_fims <- function(fit) {
  if (!is.FIMSFit(fit)) {
    cli::cli_abort(c(
      "x" = "{.var fit} must be a {.cls FIMSFit} object returned by
        {.fn fit_fims}.",
      "i" = "{.var fit} has class {.cls {class(fit)}}."
    ))
  }
  checks <- get_convergence(fit)
  if (NROW(checks) == 0) {
    cli::cli_bullets(c(
      "x" = "No convergence checks were run because the model was not
        optimized.",
      "i" = "Use {.code fit_fims(optimize = TRUE)}."
    ))
    return(invisible(list(all_ok = FALSE, checks = checks)))
  }

  cli::cli_rule(left = "FIMS sanity check")
  purrr::pwalk(
    checks[c("check", "severity", "parameter", "message")],
    function(check, severity, parameter, message) {
      bullet <- switch(severity,
        "OK" = "v",
        "NOTE" = "i",
        "WARN" = "!",
        "FAIL" = "x"
      )
      # The parameter of an OK check, e.g., the largest gradient of a
      # converged fit, is not a problem, so it is left out.
      text <- if (is.na(parameter) || severity == "OK") {
        "{.strong {check}}: {message}"
      } else {
        "{.strong {check}}: {message} ({parameter})"
      }
      cli::cli_bullets(stats::setNames(text, bullet))
    }
  )
  cli::cli_rule()

  flagged_checks <- checks[["check"]][
    checks[["severity"]] %in% c("WARN", "FAIL")
  ]
  skipped_checks <- checks[["check"]][checks[["severity"]] == "NOTE"]
  all_ok <- all(checks[["severity"]] == "OK")
  if (all_ok) {
    cli::cli_alert_success("All {nrow(checks)} checks are OK.")
  }
  if (length(flagged_checks) > 0) {
    cli::cli_alert_danger(
      "{length(flagged_checks)} of {nrow(checks)} checks need attention:
      {.val {flagged_checks}}."
    )
  }
  # A check that did not run, e.g., with get_sd = FALSE, is not a problem with
  # the fit, but all_ok stays FALSE because the check was not passed.
  if (length(skipped_checks) > 0) {
    cli::cli_alert_info(
      "{length(skipped_checks)} check{?s} did not run: {.val {skipped_checks}}."
    )
  }
  invisible(list(all_ok = all_ok, checks = checks))
}
