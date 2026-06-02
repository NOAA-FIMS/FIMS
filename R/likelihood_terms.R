#' Mirrored likelihood-term controls
#'
#' These helpers provide a small R-facing interface for the mirrored
#' likelihood-term path exposed by supported model objects.
#'
#' @param model A FIMS model object, such as [CatchAtAge].
#' @param value A logical value indicating whether mirrored likelihood terms
#'   should be used for model evaluation.
#'
#' @return
#' [use_likelihood_terms()] returns `model` invisibly.
#' [uses_likelihood_terms()] returns a logical scalar.
#' [get_likelihood_terms()] returns a data frame with columns `name`, `type`,
#' and `source_id`.
#'
#' @name likelihood_terms
#' @keywords likelihood
NULL

#' @rdname likelihood_terms
#' @export
use_likelihood_terms <- function(model, value = TRUE) {
  check_likelihood_terms_model(model)
  if (!is.logical(value) || length(value) != 1L || is.na(value)) {
    stop("`value` must be TRUE or FALSE.", call. = FALSE)
  }
  model$UseLikelihoodTerms(value)
  invisible(model)
}

#' @rdname likelihood_terms
#' @export
uses_likelihood_terms <- function(model) {
  check_likelihood_terms_model(model)
  model$UsesLikelihoodTerms()
}

#' @rdname likelihood_terms
#' @export
get_likelihood_terms <- function(model) {
  check_likelihood_terms_model(model)
  data.frame(
    name = model$LikelihoodTermNames(),
    type = model$LikelihoodTermTypes(),
    source_id = model$LikelihoodTermSourceIds(),
    stringsAsFactors = FALSE
  )
}

check_likelihood_terms_model <- function(model) {
  supported <- tryCatch(
    {
      model$UsesLikelihoodTerms()
      TRUE
    },
    error = function(e) FALSE
  )
  if (!supported) {
    stop(
      "`model` does not support mirrored likelihood-term controls.",
      call. = FALSE
    )
  }
  invisible(model)
}
