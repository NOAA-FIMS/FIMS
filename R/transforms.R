#' Transform a bounded value to the real line
#'
#' @param a,b Numeric scalar lower and upper bounds.
#' @param x Numeric scalar to transform.
#' @return A numeric scalar.
#' @export
logit <- function(a, b, x) {
  if (length(a) != 1L || length(b) != 1L || length(x) != 1L) {
    stop("Expecting a single value for `a`, `b`, and `x`.", call. = FALSE)
  }
  log((x - a) / (b - x))
}

#' Transform a real value to a bounded interval
#'
#' @inheritParams logit
#' @return A numeric scalar.
#' @export
inv_logit <- function(a, b, x) {
  if (length(a) != 1L || length(b) != 1L || length(x) != 1L) {
    stop("Expecting a single value for `a`, `b`, and `x`.", call. = FALSE)
  }
  a + (b - a) / (1 + exp(-x))
}
