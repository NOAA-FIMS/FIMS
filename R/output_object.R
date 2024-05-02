#' Create FIMS Output Object
#'
#' @return The FIMS output S4 object of class FIMSOutput.
#' @examples
#' create_fims_output()
create_fims_output <- function() {

}

# This file defines a parent class and its children by
# (1) setting the class;
# (2) defining methods, using setMethod();
# (3) setting the validators; and
# (4) establishing the constructors (i.e., functions called by users)
# where only the constructors are documented using roxygen.

# setClass: ----
# Classes are not currently exported, and therefore, do not need documentation.
# See the following link if we do want to document them in the future:
# https://stackoverflow.com/questions/7368262/how-to-properly-document-s4-class-slots-using-roxygen2

setClass(
  Class = "FIMSOutput",
  slots = c(
    estimates = "tibble",
    fits = "tibble",
    tmb = "list",
    sdreport = "list",
    call = "call",
    timestamp = "vector",
    version = "list"
  )
)
