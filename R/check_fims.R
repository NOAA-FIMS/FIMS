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
#' # TODO: add examples of strings passed to `...`
#' setup_and_run_gtest()
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
#' @param ... Additional arguments to `ctest --test-dir build` such as
#'   `"--rerun-failed --output-on-failure"`.
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

#' Style the C++ files
#'
#' Style the C++ files using clang. This function is currently in development.
#' @keywords developer
#' @examples
#' \dontrun{
#' style_cpp()
#' }
# TODO: add a function to style the C++ files using clang-format
style_cpp <- function() {
  # Does not currently work
}


#' Run all styling, documentation, and testing for FIMS repository
#'
#' FIMS consists of both C++ and R code but no function exists to style,
#' document, and test both file types. This function first works with the C++
#' code and then moves onto the R code. For each file type it styles the files,
#' e.g., puts in appropriate spacing between brackets; then it build the
#' documentation, e.g., [roxygen2::roxygenize()]; and then it tests the
#' functions.
#'
#' @return
#' An invisible list is returned with the following four items:
#' \itemize{
#'   \item{`style_r`}{Contains the report from [styler::style_pkg()].}
#'   \item{`spelling_code`}{Contains a 2-column data frame of words that are
#'   spelled incorrectly in the code but not in the excluded word list.}
#'   \item{`spelling_package`}{Contains a 2-column data frame of words that need
#'   to be added to the WORDLIST file. This will be an empty data frame if no
#'   words are spelled incorrectly.}
#'   \item{`spelling_word_list`}{A vector of strings indicating the words that
#'   are included in the WORDLIST file and are ignored when performing a spell
#'   check.}
#' }
#' @keywords developer
#' @examples
#' \dontrun{
#' check_results <- check_fims()
#' }
check_fims <- function() {
  # Check you are actually in the FIMS directory
  directory <- getwd()
  if (basename(directory) != "FIMS") {
    cli::cli_abort(c(
      "Did you mean to run this function outside of the FIMS clone?",
      i = "Your working directory is {.var {directory}}.",
      x = "Your working directory does not end in FIMS.",
      i = "You must be in a clone of FIMS to run this function."
    ))
  }

  # Get the word list to run spelling::spell_check_files() later
  word_list <- spelling::get_wordlist()

  # C++
  ## spelling
  # These results are included in the general spell check for all code below
  # and is not separated between C++ and R like the other sections.

  ## style
  style_cpp()

  ## document
  # This is done in `setup_and_run_gtest()`

  ## test
  setup_and_run_gtest()

  # R
  ## style
  results_style_r <- styler::style_pkg()

  ## document
  ### spelling
  # Need to exclude non-code files like pdf and data
  code_files <- list.files(
    c("R", "tests", file.path("inst", "include")),
    recursive = TRUE,
    full.names = TRUE,
    pattern = "\\.cpp|\\.hpp|\\.md|\\.R$|\\.Rmd|\\.txt"
  )
  results_spelling <- spelling::spell_check_files(
    code_files,
    ignore = word_list
  )

  md_vignette_files <- dir(
    file.path("vignettes"),
    pattern = "\\.md$",
    full.names = TRUE
  )
  if (length(md_vignette_files) > 0) {
    # Cannot spell check the package with vignettes turned on because you have
    # a markdown file in the vignettes directory. This is a known issue with the
    # spelling package when one of the FIMS vignettes is rendered because it
    # cannot spell check the special characters printed via cli when printing a
    # tibble like the "i" and checkmarks.
    cli::cli_alert_info("Skipping spell check for vignettes.")
    cli::cli_alert_info(
      "Remove {.var {md_vignette_files}} to run spell check on vignettes."
    )
  }
  results_spelling_package <- spelling::spell_check_package(
    pkg = ".",
    vignettes = ifelse(length(md_vignette_files) > 0, FALSE, TRUE),
    use_wordlist = TRUE
  )

  ### man
  # You have to generate new manual files before building the website otherwise
  # it might fail.
  devtools::document()

  ### pkgdown
  # TODO: think about adding the arguments devel, new_process, and install
  #       because the package will already be installed
  pkgdown::build_site()

  # The following steps are printed to the screen for the user to do themselves
  # because it was too buggy to include them in the function.
  cli::cli_bullets(c(
    "i" = "You still need to run the following commands to finish the check:",
    "*" = "{.code devtools::test()}, which is shorter than running CRAN checks",
    "*" = "{.code devtools::check()} to test building the package for CRAN",
    "*" = "{.code report <- covr::report()}"
  ))

  invisible(list(
    style_r = results_style_r,
    spelling_code = results_spelling,
    spelling_package = results_spelling_package,
    spelling_word_list = word_list
  ))
}
