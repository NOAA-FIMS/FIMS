library(Rcpp)

test_that("dnorm", {
  # generate data using R stats:rnorm
  set.seed(123)
  # simulate normal data
  y <- stats::rnorm(1)

  # create a fims Rcpp object
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  # initialize the Dnorm module
  dnorm_ <- new(fims$TMBDnormDistribution)
  # populate class members
  dnorm_$x$value <- y
  dnorm_$mean$value <- 0
  dnorm_$sd$value <- 1
  # evaluate the density and compare with R
  expect_equal(dnorm_$evaluate(TRUE), stats::dnorm(y, 0, 1, TRUE))

  fims$clear()
})

test_that("dlnorm", {
  # generate data using R stats:rnorm
  set.seed(123)
  # simulate lognormal data
  y <- stats::rlnorm(n = 1, meanlog = 0, sdlog = 1)

  # create a fims Rcpp object
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  # initialize the Dnorm module
  dlnorm_ <- new(fims$TMBDlnormDistribution)
  # populate class members
  dlnorm_$x$value <- y
  dlnorm_$meanlog$value <- 0
  dlnorm_$sdlog$value <- 1
  dlnorm_$do_bias_correction <- FALSE
  # evaluate the density and compare with R
  expect_equal(dlnorm_$evaluate(TRUE), stats::dlnorm(y, 0, 1, TRUE))
  expect_equal(dlnorm_$evaluate(FALSE), stats::dlnorm(y, 0, 1, FALSE))

  fims$clear()
})

test_that("dmultinom", {
  # generate data using R stats:rnorm
  set.seed(123)
  p <- (1:10) / sum(1:10)
  x <- stats::rmultinom(1, 100, p)

  # create a fims Rcpp object
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  # initialize the Dmultinom module
  dmultinom_ <- new(fims$TMBDmultinomDistribution)
  # populate class members
  dmultinom_$x <- x
  dmultinom_$p <- p
  # evaluate the density and compare with R
  expect_equal(
    dmultinom_$evaluate(TRUE),
    stats::dmultinom(x = x, prob = p, log = TRUE)
  )
  expect_equal(
    dmultinom_$evaluate(FALSE),
    stats::dmultinom(x = x, prob = p, log = FALSE)
  )

  fims$clear()
})
