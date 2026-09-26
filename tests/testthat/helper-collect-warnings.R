#' Collect the messages of every warning raised by an expression.
#'
#' @description This function evaluates an expression, muffles each warning it raises, and returns their messages. It is used instead of testthat::expect_warning(), which stops at the first warning, when a test needs to check more than one warning.
#' @param expr An expression to evaluate.
#' @return A character vector of warning messages, in the order they were raised.
collect_warnings <- function(expr) {
  messages <- character()
  withCallingHandlers(
    expr,
    warning = function(w) {
      messages <<- c(messages, conditionMessage(w))
      invokeRestart("muffleWarning")
    }
  )
  messages
}
