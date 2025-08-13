#' Create tests/gtest/test_*.cpp test file and register it in CMakeLists.txt
#'
#' This helper function generates a GoogleTest (gtest) C++ template file for a
#' given function and appends lines to `CMakeLists.txt` to register the test.
#'
#' @param name A string representing the combined name for the C++ test file
#'   and CMake executable target. It must follow the format
#'   `FileName_ClassName_FunctionName`, where
#'   - `FileName` is the C++ source file name (e.g., "Logistic")
#'   - `ClassName` is the C++ class name (e.g., "LogisticSelectivity")
#'   - `FunctionName` is the C++ function name (e.g., "Evaluate")
#'   and use underscores to separate each component. For example:
#'   `Logistic_LogisticSelectivity_Evaluate`.
#'
#'   If the function is not a member of a class, use a placeholder for
#'   `ClassName`. For example: `FIMSMath_ClassName_Logistic`.
#'
#'   The default is `FileName_ClassName_FunctionName`.
#'
#' @return
#' If successful, this function invisibly returns `TRUE` to allow for the
#' chaining of commands. If the function is unsuccessful, an error message is
#' returned.
#'
#' Three messages are also returned from the usethis package, which is used by
#' this function. The first states where the FIMS project is on your computer.
#' The second states the file path of the newly created file. The file will not
#' be automatically opened. The third states the test has been registered in
#' `tests/gtest/CMakeLists.txt`.
#'
#' @examples
#' \dontrun{
#' # Create a new test file named
#' # "test_Logistic_LogisticSelectivity_Evaluate.cpp" for
#' # `LogisticSelectivity::evaluate()` in
#' # `inst/include/population_dynamics/selectivity/functors/logistic.hpp`.
#'
#' FIMS:::use_gtest_template(name = "Logistic_LogisticSelectivity_Evaluate")
#' }
#'
#' @keywords developer
use_gtest_template <- function(name = "FileName_ClassName_FunctionName") {
  cmakelist_path <- file.path("tests", "gtest", "CMakeLists.txt")
  # Check if the CMakeLists.txt file exists
  if (!file.exists(cmakelist_path)) {
    cli::cli_abort(
      c("{.file {cmakelist_path}} does not exist.",
        "i" = "Please ensure that the CMakeLists.txt file is present in the
        {.file tests/gtest} directory."
      )
    )
  }

  # TODO: add the ability to add a function to a file that already exists
  # TODO: add the ability to also pass the arguments for the function or find
  #       them within the code base and ensure that the template includes the
  #       necessary structure for each input argument
  # TODO: Change the paste calls to use glue::glue() to increase readability

  # Validate the name format
  name_parts_list <- strsplit(x = name, split = "_")[[1]]
  if (length(name_parts_list) != 3) {
    cli::cli_abort(
      c("Invalid {.var name} format.",
        "i" = "Expected format: {.val FileName_ClassName_FunctionName}.",
        "x" = "Received name: {.val {name}}."
      )
    )
  }
  # Extract parts from the name
  file_name <- name_parts_list[[1]][1]
  class_name <- name_parts_list[[1]][2]
  function_name <- name_parts_list[[1]][3]

  # Create the test file
  path <- file.path("tests", "gtest", paste0("test_", name, ".cpp"))
  if (!file.exists(path)) {
    usethis::use_template(
      template = "gtest_template.cpp",
      save_as = path,
      data = list(
        class_name = class_name,
        function_name = function_name
      ),
      package = "FIMS"
    )
  } else {
    cli::cli_abort("{.file {path}} already exists.")
  }

  # Register the test in tests/gtest/CMakeLists.txt
  # Open in append mode
  CON <- file(cmakelist_path, "a")
  on.exit(close(CON), add = TRUE)
  writeLines(
    c(
      paste0("\n\n# test_", name, ".cpp"),
      paste0("add_executable(", name),
      paste0("  test_", name, ".cpp"),
      paste0(")\n"),
      paste0("target_link_libraries(", name),
      paste0("  gtest_main"),
      paste0("  fims_test"),
      paste0(")\n"),
      paste0("gtest_discover_tests(", name, ")")
    ),
    CON
  )
  cli::cli_alert_success(
    "Registering test {.val {name}} in {.file {cmakelist_path}}."
  )

  invisible(TRUE)
}
