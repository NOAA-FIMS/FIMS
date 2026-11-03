#' Keep a portable snapshot of a completed FIMS fit
#'
#' Retains numerical results without the live TMB objective, input module
#' objects, full covariance matrices, or duplicate JSON output. This is useful
#' when collecting many simulation or retrospective fits. The snapshot is a
#' plain list, not a `FIMSFit`, and cannot be used to refit or recompute uncertainty.
#'
#' This function does not mutate or free the original fit. Discard the original
#' fit and other references to its objective when finished; garbage collection
#' can then release its TMB allocations. FIMS also retains the current model
#' globally until the next initialization or an explicit [clear()] call. Only
#' call `clear()` once you no longer need that live model. Compact snapshots
#' remain usable after clearing FIMS and after saving/reloading with `saveRDS()`.
#'
#' @param fit A `FIMSFit` object.
#' @param reports Character vector of ordinary report names to retain. `NULL`
#'   retains all reports; `character(0)` retains none. Estimates and standard
#'   errors from [get_estimates()] are retained regardless of this selection.
#' @return A named list with `estimates`, `report`, `opt`, `diagnostics`,
#'   `uncertainty_reports`, and `version`. Diagnostics include the maximum and
#'   full parameter gradient, parameter counts and run times. Standard errors
#'   that were not calculated remain `NA` in `estimates`.
#' @examples
#' \dontrun{
#' result <- fit_fims(input, uncertainty_reports = c("spawning_biomass", "biomass")) |>
#'   compact_fims(reports = c("spawning_biomass", "biomass"))
#' saveRDS(result, "fit-results.rds")
#' }
#' @md
#' @export
compact_fims <- function(fit, reports = NULL) {
  if (!methods::is(fit, "FIMSFit")) {
    cli::cli_abort("fit must be a FIMSFit object.")
  }
  report <- get_report(fit)
  if (!is.null(reports)) {
    if (!is.character(reports) || !is.null(dim(reports)) ||
        anyNA(reports) || any(!nzchar(reports))) {
      cli::cli_abort("reports must be NULL or a character vector without missing or empty names.")
    }
    unknown <- setdiff(reports, names(report))
    if (length(unknown)) {
      cli::cli_abort("Unknown report name(s): {.val {unknown}}.")
    }
    report <- report[names(report) %in% reports]
  }
  selection <- get_obj(fit)$env$data$uncertainty_reports
  if (!is.null(selection)) selection <- as.character(unlist(selection, use.names = FALSE))
  list(
    estimates = get_estimates(fit),
    report = report,
    opt = get_opt(fit),
    diagnostics = list(
      max_gradient = fit@max_gradient,
      gradient = get_gradient(fit),
      number_of_parameters = fit@number_of_parameters,
      run_time = get_run_time(fit)
    ),
    uncertainty_reports = selection,
    version = get_version(fit)
  )
}
