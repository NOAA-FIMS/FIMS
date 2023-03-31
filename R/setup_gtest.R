#' function for setting up your local environment to run the gtest
#' integration test locally. Intended for developers.
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
      github_dir <- paste0("https://github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison/raw/master/FIMS_integration_test_data/FIMS_C", c_case, "/output/OM/")
      Rdata_file <- paste0("OM", i_iter, ".RData") # e.g. OM1.Rdata
      # this loads the file directly from github
      # (which was easier to figure out than downloading the Rdata first)
      load(url(paste0(github_dir, Rdata_file)))
      # write json file
      outputname <- paste0("C", c_case, "_om_output", i_iter, ".json")
      inputname <- paste0("C", c_case, "_om_input", i_iter, ".json")
      json_folder <- file.path("tests", "integration", "inputs", "FIMS-deterministic")
      if (!dir.exists(json_folder)) dir.create(json_folder)
      jsonlite::write_json(
        x = om_output, path = file.path(json_folder, outputname),
        pretty = TRUE
      )
      jsonlite::write_json(
        x = om_input, path = file.path(json_folder, inputname),
        pretty = TRUE
      )
    }
  }

  TRUE
}

#' setup and run google test suite
#'
#' Developer function for setup and running google test suite from R
#' @param ... additional arguments to \code{ctest --test-dir build}
#' such as "--rerun-failed --output-on-failure"
#' @export
setup_and_run_gtest <- function(...) {
  setup_gtest()
  system("cmake -S . -B build -G Ninja")
  system("cmake --build build")
  system(paste0("ctest --test-dir build", ...))
  TRUE
}

#' run google test suite
#'
#' Developer function for running google test suite from R
#' @param ... additional arguments to \code{ctest --test-dir build}
#' such as "--rerun-failed --output-on-failure"
#' @export
run_gtest <- function(...) {
  system("cmake -S . -B build -G Ninja")
  system("cmake --build build")
  system(paste0("ctest --test-dir build", ...))
  TRUE
}
