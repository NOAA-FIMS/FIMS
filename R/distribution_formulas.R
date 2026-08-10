#' Lognormal distribution family
#'
#' @param link Link function. The native interface currently supports `"log"`.
#' @return A family descriptor used when creating FIMS configurations.
#' @export
lognormal <- function(link = "log") {
  link <- match.arg(link, "log")
  structure(
    list(family = "lognormal", link = link),
    class = "family"
  )
}

#' Multinomial distribution family
#'
#' @param link Link function. The native interface currently supports
#'   `"logit"`.
#' @return A family descriptor used when creating FIMS configurations.
#' @export
multinomial <- function(link = "logit") {
  link <- match.arg(link, "logit")
  structure(
    list(family = "multinomial", link = link),
    class = "family"
  )
}
