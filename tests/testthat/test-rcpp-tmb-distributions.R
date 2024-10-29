test_that("normal_lpdf", {
  ## several important cases to test depending on the dimensions
  ## of the inputs

  ## Single value, e.g. a prior on a parameter
  # generate data using R stats::rnorm
  set.seed(123)

  # simulate normal data with scalar input
  y <- stats::rnorm(1)
  # create a fims Rcpp object
  # initialize the Dnorm module
  dnorm_ <- new(TMBDnormDistribution)
  # populate class members
  dnorm_$x <- new(ParameterVector, y, 1)
  dnorm_$expected_values <- new(ParameterVector, 0, 1)
  dnorm_$log_sd <- new(ParameterVector, log(1), 1)
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), stats::dnorm(y, 0, 1, TRUE))
  clear()

  ## A vector of state variables, but scalar arguments, e.g., a
  ## random effect vector
  # simulate normal data
  y <- stats::rnorm(10)
  # create a fims Rcpp object
  # initialize the Dnorm module
  dnorm_ <- new(TMBDnormDistribution)
  # populate class members
  dnorm_$x <- new(ParameterVector, y, 10)
  dnorm_$expected_values <- new(ParameterVector, 0, 1)
  dnorm_$log_sd <- new(ParameterVector, log(1), 1)
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), sum(stats::dnorm(y, 0, 1, TRUE)))
  clear()

  ## Vectors of state variables (x) and arguments, e.g., a
  ## index likelihood vector
  # simulate normal data
  y <- stats::rnorm(10)
  # create a fims Rcpp object
  # initialize the Dnorm module
  dnorm_ <- new(TMBDnormDistribution)
  # populate class members
  dnorm_$x <- new(ParameterVector, y, 10)
  dnorm_$expected_values <- new(ParameterVector, rep(0, 10), 10)
  dnorm_$log_sd <- new(ParameterVector, rep(log(1), 10), 10)
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), sum(stats::dnorm(y, 0, 1, TRUE)))
  clear()

  ## It should error out when there is a dimension mismatch
  # comment out until error checking is fixed
  # y <- stats::rnorm(10)
  # # create a fims Rcpp object
  # # initialize the Dnorm module
  # dnorm_ <- new(TMBDnormDistribution)
  # # populate class members
  # dnorm_$x <- new(FIMS:::ParameterVector, y, 10)
  # dnorm_$expected_values <- new(FIMS:::ParameterVector, 0, 11)
  # dnorm_$log_sd <- new(FIMS:::ParameterVector, log(1), 3)
  # clear()
})

test_that("lognormal_lpdf", {
  ## several important cases to test depending on the dimensions
  ## of the inputs

  ## Single value, e.g. a prior on a parameter
  # generate data using R stats::rlnorm
  set.seed(123)
  # simulate lognormal data
  y <- stats::rlnorm(n = 1, meanlog = 0, sdlog = 1)

  # create a fims Rcpp object
  # initialize the Dlnorm module
  dlnorm_ <- new(TMBDlnormDistribution)
  # populate class members
  dlnorm_$x <- new(ParameterVector, y, 1)
  dlnorm_$expected_values <- new(ParameterVector, 0, 1)
  dlnorm_$log_sd <- new(ParameterVector, log(1), 1)
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(), stats::dlnorm(y, 0, 1, TRUE) + log(y))
  clear()

  ## A vector of state variables, but scalar arguments, e.g., a
  ## random effect vector
  y <- stats::rlnorm(n = 10, meanlog = 0, sdlog = 1)

  # create a fims Rcpp object
  # initialize the Dlnorm module
  dlnorm_ <- new(TMBDlnormDistribution)
  # populate class members
  dlnorm_$x <- new(ParameterVector, y, 10)
  dlnorm_$expected_values <- new(ParameterVector, 0, 1)
  dlnorm_$log_sd <- new(ParameterVector, log(1), 1)
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(), sum(stats::dlnorm(y, 0, 1, TRUE)) + sum(log(y)))
  clear()


  ## Vectors of state variables (x) and arguments, e.g., a
  ## index likelihood vector
  y <- stats::rlnorm(n = 10, meanlog = 0, sdlog = 1)

  # create a fims Rcpp object
  # initialize the Dlnorm module
  dlnorm_ <- new(TMBDlnormDistribution)
  # populate class members
  dlnorm_$x <- new(ParameterVector, y, 10)
  dlnorm_$expected_values <- new(ParameterVector, rep(0, 10), 10)
  dlnorm_$log_sd <- new(ParameterVector, rep(log(1), 10), 10)
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(), sum(stats::dlnorm(y, 0, 1, TRUE)) + sum(log(y)))
  clear()

  ## It should error out when there is a dimension mismatch
  # comment out until error checking is fixed
  # y <- stats::rlnorm(n = 10, meanlog = 0, sdlog = 1)
  #
  # # create a fims Rcpp object
  # # initialize the Dlnorm module
  # dlnorm_ <- new(TMBDlnormDistribution)
  # # populate class members
  # dlnorm_$x <- new(ParameterVector, y, 10)
  # dlnorm_$expected_values <- new(ParameterVector, 0, 11)
  # dlnorm_$log_sd <- new(ParameterVector, log(1), 3)
  # clear()
})

test_that("multinomial_lpdf", {
  # generate data using R stats:rnorm
  set.seed(123)
  p <- (1:10) / sum(1:10)
  x <- t(stats::rmultinom(1, 100, p))
  # create a fims Rcpp object
  # initialize the Dmultinom module
  dmultinom_ <- new(TMBDmultinomDistribution)
  # populate class members
  dmultinom_$expected_values <- new(ParameterVector, p, 10)
  dmultinom_$dims <- c(1, 10)
  dmultinom_$x <- new(ParameterVector, as.vector(x), 10)
  # evaluate the density and compare with R
  expect_equal(
    dmultinom_$evaluate(),
    stats::dmultinom(x = x, prob = p, log = TRUE)
  )

  clear()
})
