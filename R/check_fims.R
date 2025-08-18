#' Set up your local environment to run the google tests locally
#'
#' Intended for developers to set up their local environment prior to running
#' the integration tests.
#'
#' @keywords developer
#' @examples \dontrun{
#' setup_gtest()
#' }
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
      r_data_file <- paste0("OM", i_iter, ".RData") # e.g. OM1.Rdata
      # this loads the file directly from github
      # (which was easier to figure out than downloading the Rdata first)
      load(url(paste0(github_dir, r_data_file)))
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
#' @keywords developer
#' @examples
#' \dontrun{
#' setup_and_run_gtest("--rerun-failed --output-on-failure")
#' }
setup_and_run_gtest <- function(...) {
  setup_gtest()
  run_gtest(...)
  TRUE
}

#' Run the google test suite
#'
#' Intended for developers to run the google test suite from R.
#'
#' @details
#' The output from running the tests should look something like the following:
#' ```bash
#' Internal ctest changing into directory: C:/github/NOAA-FIMS/FIMS/build
#' Test project C:/github_repos/NOAA-FIMS_org/FIMS/build
#'     Start 1: dlognorm.use_double_inputs
#' 1/5 Test #1: dlognorm.use_double_inputs .......   Passed    0.04 sec
#'     Start 2: dlognorm.use_int_inputs
#' 2/5 Test #2: dlognorm.use_int_inputs ..........   Passed    0.04 sec
#'     Start 3: modelTest.eta
#' 3/5 Test #3: modelTest.eta ....................   Passed    0.04 sec
#'     Start 4: modelTest.nll
#' 4/5 Test #4: modelTest.nll ....................   Passed    0.04 sec
#'     Start 5: modelTest.evaluate
#' 5/5 Test #5: modelTest.evaluate ...............   Passed    0.04 sec
#'
#' 100% tests passed, 0 tests failed out of 5
#' ```
#'
#' If a test fails, the output will look something like the following:
#' ```bash
#' Internal ctest changing into directory: C:/github/NOAA-FIMS/FIMS/build
#' Test project C:/github/NOAA-FIMS/FIMS/build
#'     Start 1: dlognorm.use_double_inputs
#' 1/7 Test #1: dlognorm.use_double_inputs .......   Passed    0.04 sec
#'     Start 2: dlognorm.use_int_inputs
#' 2/7 Test #2: dlognorm.use_int_inputs ..........   Passed    0.04 sec
#'     Start 3: modelTest.eta
#' 3/7 Test #3: modelTest.eta ....................   Passed    0.04 sec
#'     Start 4: modelTest.nll
#' 4/7 Test #4: modelTest.nll ....................   Passed    0.04 sec
#'     Start 5: modelTest.evaluate
#' 5/7 Test #5: modelTest.evaluate ...............   Passed    0.04 sec
#'     Start 6: dlognormTest.DoubleInput
#' 6/7 Test #6: dlognormTest.DoubleInput .........   Passed    0.04 sec
#'     Start 7: dlognormTest.IntInput
#' 7/7 Test #7: dlognormTest.IntInput ............***Failed    0.04 sec
#'
#' 86% tests passed, 1 tests failed out of 7
#'
#' Total Test time (real) =   0.28 sec
#'
#' The following tests FAILED:
#'           7 - dlognormTest.IntInput (Failed)
#' Errors while running CTest
#' Output from these tests are in: C:/github/NOAA-FIMS/FIMS/build/Testing/Temporary/LastTest.log
#' Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
#' ```
#' @param ... Additional arguments to `ctest --test-dir build` such as
#'   `"--rerun-failed --output-on-failure"` or `--parallel 16` if you want it to
#'   run on multiple cores.
#'
#' @keywords developer
run_gtest <- function(...) {
  system("cmake -S . -B build -G Ninja")
  system("cmake --build build")
  system(paste("ctest --test-dir build", ...))
  TRUE
}

#' Run R integration tests
#'
#' Intended for developers to run the R integration tests.
#' @keywords developer
#' @examples
#' \dontrun{
#' run_r_integration_tests()
#' }
run_r_integration_tests <- function() {
  devtools::test(filter = "integration")
  devtools::test(filter = "parallel")
}

#' Run R unit tests
#'
#' Intended for developers to run the R unit tests.
#' @keywords developer
#' @examples
#' \dontrun{
#' run_r_unit_tests()
#' }
run_r_unit_tests <- function() {
  # Exclude the files returned from the second set because we do not want
  # integration tests which includes integration and parallel tests
  unit_tests <- setdiff(
    list.files(
      path = testthat::test_path(),
      pattern = "^test-.*\\.R$"
    ),
    list.files(
      path = testthat::test_path(),
      pattern = "^test-integration.*|^test-parallel.*"
    )
  )
  # Remove "test-" and ".R" from the file names
  test_files <- gsub("^test-|\\.R$", "", unit_tests)
  # Run unit tests
  # TODO: use purrr::pwalk() to run the tests in parallel.
  purrr::walk(
    test_files,
    \(x) {
      devtools::test(filter = x)
    }
  )
}

#' Remove test data
#'
#' Intended for developers to remove test data to run the tests from fresh.
#' Developers should run this function before testing if changes affect FIMS
#' input or output.
#'
#' @keywords developer
#' @examples \dontrun{
#' remove_test_data()
#' }
remove_test_data <- function() {
  # List the data files starting with "integration-"
  data_to_keep <- list.files(
    path = testthat::test_path("fixtures"),
    pattern = "^integration[-_].*\\.RD",
    full.names = TRUE
  )

  # List all data files
  all_files <- list.files(
    path = testthat::test_path("fixtures"),
    pattern = "\\.RDS$|\\.RData$",
    full.names = TRUE
  )

  # Remove the files that are not data_to_keep
  unlink(
    setdiff(all_files, data_to_keep),
    recursive = TRUE,
    force = TRUE
  )
}
