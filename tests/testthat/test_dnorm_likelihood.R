library(testthat)
library(TMB)

test_that("dnorm unit test", {
  compile("likelihoods/test_dnorm_likelihood.cpp", flags = "-DTMB_MODEL")
  dynlib(dyn.load("likelihoods/test_dnorm_likelihood"))
  set.seed(123)
  y = rnorm(10, 5, 3)
  nll = -sum(dnorm(y, 5,3, TRUE))
  mod = MakeADFun(data = list(y =y),
                  parameters = list(p = c(5, log(3))),
                  DLL = "test_dnorm_likelihood")
  expect_equal(nll, mod$fn())
})
