library(testthat)
test_that("Recruitment input settings work as expected", {
  # Create recruitment
  recruitment <- new(BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000000.0
  spawns <- 9.55784 * 10^6
  ssb0 <- 0.0102562


  recruitment$logit_steep$value <- -log(1.0 - h) + log(h - 0.2)
  recruitment$logit_steep$min <- 0.21
  recruitment$logit_steep$max <- 1.0
  recruitment$logit_steep$is_random_effect <- TRUE
  recruitment$logit_steep$estimated <- TRUE
  recruitment$log_rzero$value <- log(r0)
  recruitment$log_sigma_recruit$value <- log(0.7)

  expect_equal(recruitment$get_id(), 1)
  expect_equal(recruitment$logit_steep$value, 0.78845736)
  expect_equal(recruitment$logit_steep$min, 0.21)
  expect_equal(recruitment$logit_steep$max, 1.0)
  expect_true(recruitment$logit_steep$is_random_effect)
  expect_true(recruitment$logit_steep$estimated)
  expect_equal(recruitment$log_rzero$value, log(1000000.0))


  expect_equal(object = recruitment$evaluate(spawns, ssb0), expected = 1090802.68)

  log_devs <- c(-1.0, 2.0, 3.0)
  recruitment$log_devs <- log_devs


  expected_nll <- -sum(log(stats::dnorm(log_devs, 0, 0.7)))


  recruitment$estimate_log_devs <- FALSE
  expect_equal(recruitment$evaluate_nll(), 0.0)

  recruitment$estimate_log_devs <- TRUE
  expect_equal(recruitment$evaluate_nll(), expected = expected_nll)

  clear()
})
