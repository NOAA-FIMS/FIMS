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
  dnorm_ <- methods::new(DnormDistribution)
  # populate class members
  dnorm_$x[1]$value <- y
  dnorm_$expected_values[1]$value <- 0
  dnorm_$log_sd[1]$value <- log(1)
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), stats::dnorm(y, 0, 1, TRUE))
  clear()

  ## A vector of state variables, but scalar arguments, e.g., a
  ## random effect vector
  # simulate normal data
  y <- stats::rnorm(10)
  # create a fims Rcpp object
  # initialize the Dnorm module
  dnorm_ <- methods::new(DnormDistribution)
  # populate class members
  dnorm_$x$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dnorm_$x[x]$value <- y[x]
  )
  dnorm_$expected_values[1]$value <- 0
  dnorm_$log_sd[1]$value <- log(1)
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), sum(stats::dnorm(y, 0, 1, TRUE)))
  clear()

  ## Vectors of state variables (x) and arguments, e.g., a
  ## index likelihood vector
  # simulate normal data
  y <- stats::rnorm(10)
  # create a fims Rcpp object
  # initialize the Dnorm module
  dnorm_ <- methods::new(DnormDistribution)
  # populate class members
  dnorm_$x$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dnorm_$x[x]$value <- y[x]
  )
  dnorm_$expected_values$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dnorm_$expected_values[x]$value <- 0
  )
  dnorm_$log_sd$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dnorm_$log_sd[x]$value <- log(1)
  )
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(), sum(stats::dnorm(y, 0, 1, TRUE)))
  clear()

  ## It should error out when there is a dimension mismatch
  # comment out until error checking is fixed
  # y <- stats::rnorm(10)
  # # create a fims Rcpp object
  # # initialize the Dnorm module
  # dnorm_ <- methods::new(DnormDistribution)
  # # populate class members
  # dnorm_$x <- methods::new(FIMS:::ParameterVector, y, 10)
  # dnorm_$expected_values <- methods::new(FIMS:::ParameterVector, 0, 11)
  # dnorm_$log_sd <- methods::new(FIMS:::ParameterVector, log(1), 3)
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
  dlnorm_ <- methods::new(DlnormDistribution)
  # populate class members
  dlnorm_$x[1]$value <- y
  dlnorm_$expected_values[1]$value <- 0
  dlnorm_$log_sd[1]$value <- log(1)
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(), stats::dlnorm(y, 0, 1, TRUE) + log(y))
  clear()

  ## A vector of state variables, but scalar arguments, e.g., a
  ## random effect vector
  y <- stats::rlnorm(n = 10, meanlog = 0, sdlog = 1)

  # create a fims Rcpp object
  # initialize the Dlnorm module
  dlnorm_ <- methods::new(DlnormDistribution)
  # populate class members
  dlnorm_$x$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dlnorm_$x[x]$value <- y[x]
  )
  dlnorm_$expected_values[1]$value <- 0
  dlnorm_$log_sd[1]$value <- log(1)
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(), sum(stats::dlnorm(y, 0, 1, TRUE)) + sum(log(y)))
  clear()


  ## Vectors of state variables (x) and arguments, e.g., a
  ## index likelihood vector
  y <- stats::rlnorm(n = 10, meanlog = 0, sdlog = 1)

  # create a fims Rcpp object
  # initialize the Dlnorm module
  dlnorm_ <- methods::new(DlnormDistribution)
  # populate class members
  dlnorm_$x$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dlnorm_$x[x]$value <- y[x]
  )
  dlnorm_$expected_values$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dlnorm_$expected_values[x]$value <- 0
  )
  dlnorm_$log_sd$resize(length(y))
  purrr::walk(
    seq_along(y),
    \(x) dlnorm_$log_sd[x]$value <- log(1)
  )
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(), sum(stats::dlnorm(y, 0, 1, TRUE)) + sum(log(y)))
  clear()

  ## It should error out when there is a dimension mismatch
  # comment out until error checking is fixed
  # y <- stats::rlnorm(n = 10, meanlog = 0, sdlog = 1)
  #
  # # create a fims Rcpp object
  # # initialize the Dlnorm module
  # dlnorm_ <- methods::new(DlnormDistribution)
  # # populate class members
  # dlnorm_$x <- methods::new(ParameterVector, y, 10)
  # dlnorm_$expected_values <- methods::new(ParameterVector, 0, 11)
  # dlnorm_$log_sd <- methods::new(ParameterVector, log(1), 3)
  # clear()
})

test_that("multinomial_lpdf", {
  # generate data using R stats:rnorm
  set.seed(123)
  p <- (1:10) / sum(1:10)
  x_values <- t(stats::rmultinom(1, 100, p))
  # create a fims Rcpp object
  # initialize the Dmultinom module
  dmultinom_ <- methods::new(DmultinomDistribution)
  # populate class members
  dmultinom_$dims$resize(2)
  dmultinom_$dims$set(0, 1)
  dmultinom_$dims$set(1, length(p))
  dmultinom_$expected_values$resize(length(p))
  purrr::walk(
    seq_along(p),
    \(x) dmultinom_$expected_values[x]$value <- p[x]
  )

  dmultinom_$x$resize(length(p))
  purrr::walk(
    seq_along(p),
    \(x) dmultinom_$x[x]$value <- x_values[x]
  )

  # evaluate the density and compare with R
  expect_equal(
    dmultinom_$evaluate(),
    stats::dmultinom(x = x_values, prob = p, log = TRUE)
  )

  clear()
})
