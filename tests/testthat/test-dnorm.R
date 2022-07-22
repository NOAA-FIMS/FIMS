library(Rcpp)

test_that("dnorm", {

  # generate data using R stats:rnorm
  set.seed(123)
  # simulate normal data
  y <- stats::rnorm(1)

  # create a fims Rcpp object
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  # initialize the Dnorm module
  dnorm_ <- new(fims$TMBDnormDistribution)
  # populate class members
  dnorm_$x$value <- y
  dnorm_$mean$value <- 0
  dnorm_$sd$value <- 1
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(TRUE), stats::dnorm(y, 0, 1, TRUE))
})
