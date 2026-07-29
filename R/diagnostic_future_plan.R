#' Set future plan for diagnostic helpers
#'
#' @keywords internal
set_diagnostic_future_plan <- function(n_cores) {
  previous_plan <- future::plan()
  if (n_cores == 1L) {
    future::plan(future::sequential)
  } else {
    future::plan(future::multisession, workers = n_cores)
  }
  invisible(previous_plan)
}
