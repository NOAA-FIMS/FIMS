#' Should FIMS be verbose?
#'
#' Verbosity is set globally for FIMS using
#' `options(rlib_message_verbosity = "quiet")` to stop the printing of messages
#' from `cli::cli_inform()`. Using a global option allows for verbose to not
#' have to be an argument to every function. All `cli::cli_abort()` messages are
#' printed to the console no matter what the global option is set to.
#'
#' @return
#' A logical is returned where `TRUE` ensures messages from `cli::cli_inform()`
#' are printed to the console.
#'
#' @examples
#' fims:::is_fims_verbose()
is_fims_verbose <- function() {
  verbose_option <- getOption("rlib_message_verbosity", default = "default")
  verbose_boolean <- ifelse(
    verbose_option %in% c("default", "verbose"),
    TRUE,
    FALSE
  )
  return(verbose_boolean)
}
