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
TMB::compile(paste0(path, "/test_dnorm_distribution.cpp"), flags = "-DTMB_MODEL")
TMB::compile(paste0(path, "/test_dlnorm_distribution.cpp"), flags = "-DTMB_MODEL")
TMB::compile(paste0(path, "/test_dmultinom_distribution.cpp"), flags = "-DTMB_MODEL")

test_that("dnorm unit test", {

  # setwd(project_path)
  # on.exit(setwd(old_wd), add = TRUE)

  # dnorm unit test
  # load test
  dyn.load(dynlib(paste0(path, "/test_dnorm_distribution")))
  set.seed(123)
  #Simulate new data with R
  y = rnorm(10, 5, 3)
  #Calculate negative log-likelihood with R dnorm
  nll = -sum(dnorm(y, 5,3, TRUE))
  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(y =y),
                  parameters = list(p = c(5, log(3))),
                  DLL = "test_dnorm_distribution")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  dyn.unload(dynlib(paste0(path, "/test_dnorm_distribution")))
  file.remove(paste0(path, "/", dynlib("test_dnorm_distribution")))
  file.remove( paste0(path, "/test_dnorm_distribution.o"))

})

test_that("dlnorm unit test", {

  # setwd(project_path)
  # on.exit(setwd(old_wd), add = TRUE)

  # dlnorm unit test
  # load test
  dyn.load(dynlib(paste0(path, "/test_dlnorm_distribution")))
  set.seed(123)
  #Simulate new data with R
  y = rlnorm(10, 2, 1)
  #Calculate negative log-likelihood with R dlnorm
  nll = -sum(dlnorm(y, 2, 1, TRUE))
  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(logy = y),
                  parameters = list(p = c(2, log(1))),
                  DLL = "test_dlnorm_distribution")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  dyn.unload(dynlib(paste0(path, "/test_dlnorm_distribution")))
  file.remove(paste0(path, "/", dynlib("test_dlnorm_distribution")))
  file.remove( paste0(path, "/test_dlnorm_distribution.o"))
})

test_that("dmultinom unit test", {

  # setwd(project_path)
  # on.exit(setwd(old_wd), add = TRUE)

  # # dmultinom unit test
  # # load test
  dyn.load(dynlib(paste0(path, "/test_dmultinom_distribution")))
  set.seed(123)
  #Simulate new data with R
  p = (1:10)/sum(1:10)
  x = rmultinom(1, 100, p)
  #Calculate negative log-likelihood with R dnmultinom
  nll = -dmultinom(x, 100,p, TRUE)
  #Initialize TMB model object with true values
  mod = MakeADFun(data = list(x =x),
                  parameters = list(p = p),
                  DLL = "test_dmultinom_distribution")
  #Compare R nll to TMB nll
  expect_equal(nll, mod$fn())

  dyn.unload(dynlib(paste0(path, "/test_dmultinom_distribution")))
  file.remove(paste0(path, "/", dynlib("test_dmultinom_distribution")))
  file.remove( paste0(path, "/test_dmultinom_distribution.o"))
})
