#' Save a unnested tibble as a CSV file and return the file path.
#'
#' @description This function saves a given tibble as a CSV file in a temporary location. It's used to facilitate testing by providing a consistent file path for snapshot comparisons (e.g., testthat::expect_snapshot_file()).
#' @param data A tibble to save as a CSV file.
#' @return The file path of the saved CSV file.
save_csv <- function(data) {
  path <- tempfile(fileext = ".csv")
  result <- tryCatch(
    {
      write.csv(data, path, row.names = FALSE)
      return(path)
    },
    error = function(e) {
    message("ERROR MESSAGE: ", conditionMessage(e))
    message("CALL: ")
    print(e$call)
    log_error(paste("Error in save_csv:", conditionMessage(e)))
    stop(e)  #rethrow so testthat still fails
    }
)
  # write.csv(data, path)
  path
}
