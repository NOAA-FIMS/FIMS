# library(Rcpp)
# data(package = "FIMS")
#
# testthat("dnorm",{
#
#   #generate data using R stats:rnorm
#   set.seed(123)
#   y <- stats::rnorm(100)
#
#   fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
#   dnorm_like <- new(fims$TMBDnormLikelihood)
#   dnorm_like$x <- y
#   dnorm_like$mean <- 0
#   dnorm_like$sd <- 1
#   expect_equal(dnorm_like$evaluate(), sum(stats::dnorm(y, 0, 1, true)))
#
# })
