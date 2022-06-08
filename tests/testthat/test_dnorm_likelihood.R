library(testthat)
library(TMB)

test_that("dnorm unit test", {

  project_path <- find.package("FIMS")
  old_wd <- getwd()
  setwd(project_path)


  if (!file.exists(file.path(project_path, "inst"))) {
    path <- file.path("extdata", "TMB_tests", "likelihoods")
  } else {
    path <- file.path("inst", "extdata", "TMB_tests", "likelihoods")
  }


  #compile and load test
  TMB::compile(paste0(path, "/test_dnorm_likelihood.cpp"), flags = "-DTMB_MODEL")
  dyn.load(dynlib(paste0(path, "/test_dnorm_likelihood")))
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
  on.exit(dyn.unload(dynlib(paste0(path, "/test_dnorm_likelihood"))), add = TRUE)
  on.exit(
    file.remove(paste0(path, "/", dynlib("test_dnorm_likelihood")), add = TRUE)
  )
  on.exit(file.remove( paste0(path, "/test_dnorm_likelihood.o")), add = TRUE)
  on.exit(setwd(old_wd), add = TRUE)
})
