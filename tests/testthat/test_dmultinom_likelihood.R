library(testthat)
library(TMB)

test_that("dmultinom unit test", {

  project_path <- find.package("FIMS")
  old_wd <- getwd()
  setwd(project_path)


  if (!file.exists(file.path(project_path, "inst"))) {
    path <- file.path("extdata", "TMB_tests", "likelihoods")
  } else {
    path <- file.path("inst", "extdata", "TMB_tests", "likelihoods")
  }


  #compile and load test
  TMB::compile(paste0(path, "/test_dmultinom_likelihood.cpp"), flags = "-DTMB_MODEL")
  dyn.load(dynlib(paste0(path, "/test_dmultinom_likelihood")))
  set.seed(123)
  #Simulate new data with R
  p = (1:10)/sum(1:10)
  x = rmultinom(1, 100, p)
  #Calculate negative log-likelihood with R stats::dnmultinom
  nll = -dmultinom(x, 100,p, TRUE)
  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(x =x),
                  parameters = list(p = p),
                  DLL = "test_dmultinom_likelihood")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  #cleanup
  on.exit(dyn.unload(dynlib(paste0(path, "/test_dmultinom_likelihood"))), add = TRUE)
  on.exit(
    file.remove(paste0(path, "/", dynlib("test_dmultinom_likelihood")), add = TRUE)
  )
  on.exit(file.remove( paste0(path, "/test_dmultinom_likelihood.o")), add = TRUE)
  on.exit(setwd(old_wd), add = TRUE)
})
