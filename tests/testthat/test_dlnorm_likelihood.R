library(testthat)
library(TMB)

test_that("dlnorm unit test", {

  project_path <- find.package("FIMS")
  old_wd <- getwd()
  setwd(project_path)


  if (!file.exists(file.path(project_path, "inst"))) {
    path <- file.path("extdata", "TMB_tests", "likelihoods")
  } else {
    path <- file.path("inst", "extdata", "TMB_tests", "likelihoods")
  }


  #compile and load test
  TMB::compile(paste0(path, "/test_dlnorm_likelihood.cpp"), flags = "-DTMB_MODEL")
  dyn.load(dynlib(paste0(path, "/test_dlnorm_likelihood")))
  set.seed(123)
  #Simulate new data with R
  y = rlnorm(10, 2, 1)
  #Calculate negative log-likelihood with R stats::dlnorm
  nll = -sum(dlnorm(y, 2, 1, TRUE))
  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(logy = y),
                  parameters = list(p = c(2, log(1))),
                  DLL = "test_dlnorm_likelihood")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  #cleanup
  on.exit(dyn.unload(dynlib(paste0(path, "/test_dlnorm_likelihood"))), add = TRUE)
  on.exit(
    file.remove(paste0(path, "/", dynlib("test_dlnorm_likelihood")), add = TRUE)
  )
  on.exit(file.remove( paste0(path, "/test_dlnorm_likelihood.o")), add = TRUE)
  on.exit(setwd(old_wd), add = TRUE)
})
