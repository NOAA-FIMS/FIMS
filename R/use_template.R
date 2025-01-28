#' Create R test file
#' 
#' This helper function creates a new test file using a template if it doesn't already exist. 
#' The test file is created under the "tests/testthat" directory with a name based on 
#' the input argument \code{name}. If the test file already exists at the 
#' specified path, an error message is raised and no changes are made.
#' 
#' @param name A character string to specify the name of the test file. The name 
#' should be used to create the file (with a \code{test-} prefix). If \code{name} 
#' is \code{NULL}, the function will not generate a name and will throws an error.
#' 
#' @return Returns \code{invisible(TRUE)} to allow for chaining of commands.
#' 
#' @examples
#' # Create a new test file named "my-test"
#' \dontrun{
#' FIMS:::use_testthat_template("my-test")
#' }
use_testthat_template <- function(name){
  path <- file.path("tests", "testthat", paste0("test-", name, ".R"))
  if (!file.exists(path)) {
    usethis::use_template("testthat_template.R", save_as = path, package = "FIMS")
  } else {
    cli::cli_abort("{path} already exists.")
  }
  invisible(TRUE)
}
