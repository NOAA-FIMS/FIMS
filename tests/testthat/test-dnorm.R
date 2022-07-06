library(Rcpp)

testthat("dnorm",{

  #generate data using R stats:rnorm
  set.seed(123)
  #simulate normal data
  y <- stats::rnorm(100)

  #create a fims Rcpp object
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  #initialize the Dnorm module
  dnorm_ <- new(fims$TMBDnormDistribution)
  #populate class members
  dnorm_$x <- y
  dnorm_$mean <- 0
  dnorm_$sd <- 1
  #evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), sum(stats::dnorm(y, 0, 1, true)))

})
