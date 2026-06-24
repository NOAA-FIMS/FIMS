# rcpp likelihood ----

test_that("NormalLikelihood works with correct inputs", {
  likelihood <- methods::new(NormalLikelihood)
  observed <- c(1, 2)
  expected <- c(0.5, 1.5)

  likelihood$observed_values[] <- observed
  likelihood$expected_values[] <- expected
  likelihood$log_sd[] <- log(2)

  #' @description Test that NormalLikelihood evaluates negative log likelihood values.
  expect_equal(
    likelihood$evaluate(),
    -sum(stats::dnorm(observed, expected, 2, log = TRUE))
  )
  expect_equal(likelihood$nll_components$size(), length(observed))
  clear()
})

test_that("NormalLikelihood links RealVector inputs", {
  likelihood <- methods::new(NormalLikelihood)
  input <- methods::new(RealVector)

  input[] <- 1
  likelihood$expected_values[] <- 0
  likelihood$log_sd[] <- 0
  likelihood$set_real_input(input, "prior")

  #' @description Test that NormalLikelihood can evaluate a linked RealVector input.
  expect_equal(likelihood$evaluate(), -stats::dnorm(1, 0, 1, log = TRUE))
  clear()
})

test_that("NormalLikelihood links RealVector expected values", {
  likelihood <- methods::new(NormalLikelihood)
  expected <- methods::new(RealVector)

  likelihood$observed_values[] <- 1
  expected[] <- 0
  likelihood$log_sd[] <- 0
  likelihood$set_real_expected_input(expected)

  #' @description Test that NormalLikelihood can evaluate linked RealVector expected values.
  expect_equal(likelihood$evaluate(), -stats::dnorm(1, 0, 1, log = TRUE))
  clear()
})

test_that("LognormalLikelihood works with correct inputs", {
  likelihood <- methods::new(LognormalLikelihood)
  observed <- exp(c(1, 2))
  expected <- c(0.5, 1.5)

  likelihood$observed_values[] <- observed
  likelihood$expected_values[] <- expected
  likelihood$log_sd[] <- log(2)

  #' @description Test that LognormalLikelihood evaluates negative log likelihood values.
  expect_equal(
    likelihood$evaluate(),
    -sum(stats::dlnorm(observed, expected, 2, log = TRUE))
  )
  clear()
})

test_that("GammaLikelihood works with correct inputs", {
  likelihood <- methods::new(GammaLikelihood)
  observed <- 4
  mean_value <- 3
  sd_value <- 1.5
  shape <- (mean_value / sd_value)^2
  scale <- (sd_value^2) / mean_value

  likelihood$observed_values[] <- observed
  likelihood$expected_values[] <- mean_value
  likelihood$log_sd[] <- log(sd_value)

  #' @description Test that GammaLikelihood evaluates negative log likelihood values.
  expect_equal(
    likelihood$evaluate(),
    -stats::dgamma(observed, shape = shape, scale = scale, log = TRUE)
  )
  clear()
})

test_that("InvGammaLikelihood works with correct inputs", {
  likelihood <- methods::new(InvGammaLikelihood)
  observed <- 2
  shape <- 3
  scale <- 4
  log_density <- -shape * log(scale) - lgamma(shape) -
    (shape + 1) * log(observed) - 1 / (scale * observed)

  likelihood$observed_values[] <- observed
  likelihood$log_shape[] <- log(shape)
  likelihood$log_scale[] <- log(scale)

  #' @description Test that InvGammaLikelihood evaluates negative log likelihood values.
  expect_equal(likelihood$evaluate(), -log_density)
  clear()
})

test_that("MultinomialLikelihood works with correct inputs", {
  likelihood <- methods::new(MultinomialLikelihood)
  observed <- c(2, 1, 1)
  probabilities <- c(0.5, 0.25, 0.25)

  likelihood$observed_values[] <- observed
  likelihood$expected_values[] <- probabilities
  likelihood$dims[] <- c(1, 3)

  #' @description Test that MultinomialLikelihood evaluates negative log likelihood values.
  expect_equal(
    likelihood$evaluate(),
    -stats::dmultinom(observed, prob = probabilities, log = TRUE)
  )
  clear()
})
