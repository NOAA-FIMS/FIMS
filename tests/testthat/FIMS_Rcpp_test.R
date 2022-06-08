testthat("Rcpp interface works for modules", {
library(Rcpp)

fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

a <- new(fims$Parameter, .1)
expect_success(beverton_holt <- new(fims$BevertonHoltRecruitment))
expect_success(logistic_selectivity <- new(fims$LogisticSelectivity))
logistic_selectivity$slope$value <- .7
logistic_selectivity$median$value <- 5.0

expect_equal(logistic_selectivity$slope$value, 0.7)
expect_equal(logistic_selectivity$get_id(), 0)
})
