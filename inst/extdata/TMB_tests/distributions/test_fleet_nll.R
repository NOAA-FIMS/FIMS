# open FIMS.proj and run locally from console using:
# source('inst/extdata/TMB_tests/distributions/test_distributions.R')

library(testthat)
library(TMB)

# project_path <- find.package("FIMS")
# old_wd <- getwd()
# setwd(project_path)

project_path <- getwd()

# if (!file.exists(file.path(project_path, "inst"))) {
#   path <- file.path("extdata", "TMB_tests", "distributions")
# } else {
   path <- file.path("inst", "extdata", "TMB_tests", "distributions")
# }


# compile test .cpp files from inst/extdata/TMB_tests/distributions
TMB::compile(paste0(path, "/test_fleet_index_nll.cpp"), flags = "-O1 -g -DTMB_MODEL", DLLFLAGS="")
TMB::compile(paste0(path, "/test_fleet_acomp_nll.cpp"), flags = "-O1 -g -DTMB_MODEL", DLLFLAGS="")

test_that("fleet index nll unit test", {

  # setwd(project_path)
  # on.exit(setwd(old_wd), add = TRUE)

  # # dmultinom unit test
  # # load test
  dyn.load(dynlib(paste0(path, "/test_fleet_index_nll")))

  set.seed(123)
  #Simulate new data with R

  #Simulate new data with R
  y = stats::rlnorm(10, 2, 1)
  #Calculate negative log-likelihood with R dlnorm
  nll = -sum(stats::dnorm(log(y), log(2), 1, TRUE))

  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(y = y),
                  parameters = list(p = rep(2,10), log_sd = log(1)),
                  DLL = "test_fleet_index_nll")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())
  dyn.unload(dynlib(paste0(path, "/test_fleet_index_nll")))    
  file.remove(paste0(path, "/", dynlib("test_fleet_index_nll")))
  file.remove(paste0(path, "/test_fleet_index_nll.o"))

})

test_that("fleet acomp nll unit test", {

  # setwd(project_path)
  # on.exit(setwd(old_wd), add = TRUE)

  # # dmultinom unit test
  # # load test
  dyn.load(dynlib(paste0(path, "/test_fleet_acomp_nll")))

  set.seed(123)
  p = (1:10)/sum(1:10)
  x = stats::rmultinom(1, 100, p)

  #Calculate negative log-likelihood with R dnmultinom
  nll = -stats::dmultinom(x, 100,p, TRUE)

  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(x = x),
                  parameters = list(p = p),
                  DLL = "test_fleet_acomp_nll")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  dyn.unload(dynlib(file.path(path, "test_fleet_acomp_nll")))
  file.remove(file.path(path, dynlib("test_fleet_acomp_nll")))
  file.remove(file.path(path, "test_fleet_acomp_nll.o"))

})
