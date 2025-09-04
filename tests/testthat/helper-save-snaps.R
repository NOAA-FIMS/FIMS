#' Save a unnested tibble as a CSV file and return the file path.
#' 
#' @description This function saves a given tibble as a CSV file in a temporary
#' location. It is used to facilitate testing by providing a consistent file path
#' for snapshot comparisons (e.g., testthat::expect_snapshot_file()).
#' @param data A tibble to save as a CSV file.
#' @return The file path of the saved CSV file.
save_csv <- function(data) {
  path <- tempfile(fileext = ".csv")
  write.csv(data, path)
  path
}