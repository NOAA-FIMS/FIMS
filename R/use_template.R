#' Create tests/testthat/test-*.R test file
#'
#' This helper function creates a new test file for testthat using a template
#' available within the templates folder of this package. The test file is
#' created under the "tests/testthat" directory with a name based on the input
#' argument `name`. If the test file already exists, an error message is
#' returned and no changes are made.
#'
#' @param name A character string providing the name of the R function that you
#'   want to test. The name will be used to create the file name, i.e.,
#'   tests/testthat/test-{name}.R. If `name` is not specified, the function
#'   will not be able to create a file name and an error will be returned.
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
#' # Create a new test file named "test-new_function" for `new_function()`
#' \dontrun{
#' FIMS:::use_testthat_template("new_function")
#' }
use_testthat_template <- function(name) {
  path <- file.path("tests", "testthat", paste0("test-", name, ".R"))
  if (!file.exists(path)) {
    usethis::use_template(
      template = "testthat_template.R",
      save_as = path,
      package = "FIMS"
    )
  } else {
    cli::cli_abort("{path} already exists.")
  }
  invisible(TRUE)
}
