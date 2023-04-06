library(testthat)
test_that("Recruitment input settings work as expected", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create recruitment
  recruitment <- new(fims$BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000000.0
  spawns <- 9.55784 * 10^6
  ssb0 <- 0.0102562


  recruitment$steep$value <- h
  recruitment$steep$min <- 0.21
  recruitment$steep$max <- 1.0
  recruitment$steep$is_random_effect <- TRUE
  recruitment$steep$estimated <- TRUE
  recruitment$rzero$value <- r0
  recruitment$log_sigma_recruit$value <- log(0.7)
  recruitment$recruitment_bias_adj <- rep(1.0, 3)
  recruitment$deviations <- devs
  recruitment$use_bias_correction <- FALSE

  expect_equal(recruitment$get_id(), 1)
  expect_equal(recruitment$steep$value, 0.75)
  expect_equal(recruitment$steep$min, 0.21)
  expect_equal(recruitment$steep$max, 1.0)
  expect_true(recruitment$steep$is_random_effect)
  expect_true(recruitment$steep$estimated)
  expect_equal(recruitment$rzero$value, 1000000.0)
  

  expect_equal(object = recruitment$evaluate(spawns, ssb0), expected = 1090802.68)

  devs <- c(1.0, 2.0, 3.0)


  expected_nll <- -sum(log(stats::dnorm(log(devs), 0, 0.7)))


  recruitment$estimate_deviations <- FALSE
  expect_equal(recruitment$evaluate_nll(), 0.0)

  recruitment$estimate_deviations <- TRUE
  expect_equal(recruitment$evaluate_nll(), expected = expected_nll)

  recruitment$use_bias_correction <- TRUE
  recruitment$recruitment_bias_adj <- rep(0.245, 3)
  expected_nll <- -sum(log(stats::dnorm(log(devs), -0.245, 0.7)))
  expect_equal(recruitment$evaluate_nll(), expected = expected_nll)

  recruitment$use_bias_correction <- TRUE
  recruitment$recruitment_bias_adj <- c(0.245, 0.2, 0.1)
  expected_nll <- -sum(log(stats::dnorm(log(devs), c(-0.245, -0.2, -0.1), 0.7)))
  expect_equal(recruitment$evaluate_nll(), expected = expected_nll)

  fims$clear()
})
