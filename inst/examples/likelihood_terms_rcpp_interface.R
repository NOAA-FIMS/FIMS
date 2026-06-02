# Direct Rcpp likelihood-term interface example.
#
# This script calls the Rcpp module methods on a CatchAtAge object directly.
# It is intentionally small so developers can see the lower-level control
# surface without extra helper layers.

library(FIMS)

clear()

model <- methods::new(CatchAtAge)

model$UseLikelihoodTerms(TRUE)
stopifnot(model$UsesLikelihoodTerms())

likelihood_terms <- data.frame(
  name = model$LikelihoodTermNames(),
  type = model$LikelihoodTermTypes(),
  source_id = model$LikelihoodTermSourceIds(),
  stringsAsFactors = FALSE
)

stopifnot(model$LikelihoodTermCount() == nrow(likelihood_terms))

print(likelihood_terms)

model$UseLikelihoodTerms(FALSE)
stopifnot(!model$UsesLikelihoodTerms())

clear()
