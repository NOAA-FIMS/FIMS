library(testthat)
library(TMB)

test_that("dnorm unit test", {

  #compile and load test
  TMB::compile("likelihoods/test_dnorm_likelihood.cpp", flags = "-DTMB_MODEL")
  dynlib(dyn.load("likelihoods/test_dnorm_likelihood"))
  set.seed(123)
  #Simulate new data with R
  y = rnorm(10, 5, 3)
  #Calculate negative log-likelihood with R stats::dnorm
  nll = -sum(dnorm(y, 5,3, TRUE))
  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(y =y),
                  parameters = list(p = c(5, log(3))),
                  DLL = "test_dnorm_likelihood")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  #cleanup
  on.exit(dynlib(dyn.unload("likelihoods/test_dnorm_likelihood")), add = TRUE)
  on.exit(file.remove("likelihoods/test_dnorm_likelihood.dll"), add = TRUE)
  on.exit(file.remove( "likelihoods/test_dnorm_likelihood.o"), add = TRUE)
})
