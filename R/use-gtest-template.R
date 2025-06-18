#' Create a GoogleTest test file and register it in CMakeLists.txt
#'
#' This helper function generate a GoogleTest (gtest) C++ template file for a
#' given function and optionally appends test registration lines to a
#' `CMakeLists.txt` file.
#'
#' @param class_name A character string providing the name of the C++ class.
#' @param function_name A character string providing the name of the function
#' to test.
#' @param header_file_path A character string providing the path to the header
#' file that declares the function.
#' @param cmakelist_path A character string providing the path to the
#' `CMakeLists.txt` file where the test should be registered. If `NULL`, the
#' test will not be registered in CMake.
#'
#' @return
#' If successful, this function invisibly returns `TRUE` to allow for the
#' chaining of commands. If the function is unsuccessful, an error message is
#' returned.
#'
#' Two messages are also returned from the usethis package, which is used by
#' this function. The first states where the FIMS project is on your computer.
#' The second states the file path of the newly created file. The file will not
#' be automatically opened.
#'
#' @examples
#' # Create a new test file named "test_JasonParse_WriteToFile.cpp" for
#' `JasonParse::WriteToFile()`
#' \dontrun{
#' FIMS:::use_gtest_template(
#'   class_name = "JasonParse",
#'   function_name = "WriteToFile",
#'   header_file_path = "utilities/fims_json.hpp",
#'   cmakelist_path = "tests/gtest/CMakeLists.txt"
#' )
#' }
use_gtest_template <- function(class_name, function_name, header_file_path, cmakelist_path = NULL) {
  # TODO: add the ability to add a function to a file that already exists
  # TODO: add the ability to also pass the arguments for the function or find
  #       them within the code base and ensure that the template includes the
  #       necessary structure for each input argument
  path <- file.path("tests", "gtest", paste0("test_", class_name, "_", function_name, ".cpp"))
  if (!file.exists(path)) {
    function_name <- gsub("-", "_", function_name)
    usethis::use_template(
      template = "gtest_template.cpp",
      save_as = path,
      data = list(function_name = function_name, header_file = header_file_path),
      package = "FIMS"
    )
  } else {
    cli::cli_abort("{path} already exists.")
  }

  if (!is.null(cmakelist_path)) {
    # TODO: remove the line below after testing
    cmakelist_path <- file.path("tests", "gtest", "CMakeLists.txt")
    class_function <- paste0(class_name, "_", function_name)

    CON <- file(cmakelist_path, "a")
    writeLines(
      c(
        paste0("\n\n# test_", class_function, ".cpp"),
        paste0("add_executable(", class_function),
        paste0("  test_", class_function, ".cpp"),
        paste0(")\n"),
        paste0("target_link_libraries(", class_function),
        paste0("  gtest_main"),
        paste0("  fims_test"),
        paste0(")\n"),
        paste0("gtest_discover_tests(", class_function, ")")
      ),
      CON
    )
    close(CON)
    cli::cli_alert_info("Test has been registered in {cmakelist_path}.")
  } else {
    cli::cli_alert_info("No CMakeLists.txt file provided. Test will not be registered.")
  }
  invisible(TRUE)
}
