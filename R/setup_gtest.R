#' Set up your local environment to run the google tests locally
#'
#' Intended for developers to set up their local environment prior to running
#' the integration tests.
#'
#' @keywords gtest_helper
#' @examples \dontrun{
#' setup_gtest()
#' }
#' @export
setup_gtest <- function() {
  # bind om_output and om_input locally to the function to avoid
  # R CMD Check note: no visible binding for global variable
  om_output <- om_input <- NULL
  # loop over iterations within the model comparison project output
  # currently don't need, but may need once we need more than one file.
  for (c_case in 0:2) {
    for (i_iter in 1) {
      # read Rdata file into workspace
      # temporarily use a scenario from the model comparison project that is
      # not deterministic
      github_dir <- paste0(
        "https://github.com/NOAA-FIMS/",
        "Age_Structured_Stock_Assessment_Model_Comparison/raw/main/",
        "FIMS_integration_test_data/FIMS_C",
        c_case,
        "/output/OM/"
      )
      Rdata_file <- paste0("OM", i_iter, ".RData") # e.g. OM1.Rdata
      # this loads the file directly from github
      # (which was easier to figure out than downloading the Rdata first)
      load(url(paste0(github_dir, Rdata_file)))
      # write json file
      output_name <- paste0("C", c_case, "_om_output", i_iter, ".json")
      input_name <- paste0("C", c_case, "_om_input", i_iter, ".json")
      json_folder <- file.path(
        "tests",
        "integration",
        "FIMS-deterministic-inputs"
      )
      if (!dir.exists(json_folder)) {
        dir.create(json_folder)
      }
      jsonlite::write_json(
        x = om_output,
        path = file.path(json_folder, output_name),
        pretty = TRUE
      )
      jsonlite::write_json(
        x = om_input,
        path = file.path(json_folder, input_name),
        pretty = TRUE
      )
    }
  }

  TRUE
}

#' Setup and run the google test suite
#'
#' Intended for developers to set up their local environment and run the google
#' test suite from R.
#'
#' @inheritParams run_gtest
#'
#' @keywords gtest_helper
#' @export
setup_and_run_gtest <- function(...) {
  setup_gtest()
  run_gtest(...)
  TRUE
}

#' Run the google test suite
#'
#' Intended for developers to run the google test suite from R.
#'
#' @param ... Additional arguments to `ctest --test-dir build` such as
#'   `"--rerun-failed --output-on-failure"`.
#'
#' @keywords gtest_helper
#' @export
run_gtest <- function(...) {
  system("cmake -S . -B build -G Ninja")
  system("cmake --build build")
  system(paste("ctest --test-dir build", ...))
  TRUE
}
