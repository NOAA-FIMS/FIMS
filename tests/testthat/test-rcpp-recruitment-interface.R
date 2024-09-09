library(testthat)
test_that("Recruitment input settings work as expected", {
  # Create recruitment
  recruitment <- new(BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000000.0
  spawns <- 9.55784 * 10^6
  ssb0 <- 0.0102562

  recruitment$logit_steep[1]$value <- -log(1.0 - h) + log(h - 0.2)
  recruitment$logit_steep[1]$min <- 0.21
  recruitment$logit_steep[1]$max <- 1.0
  recruitment$logit_steep[1]$is_random_effect <- TRUE
  recruitment$logit_steep[1]$estimated <- TRUE
  recruitment$log_rzero[1]$value <- log(r0)

  expect_equal(recruitment$get_id(), 1)
  expect_equal(recruitment$logit_steep[1]$value, 0.78845736)
  expect_equal(recruitment$logit_steep[1]$min, 0.21)
  expect_equal(recruitment$logit_steep[1]$max, 1.0)
  expect_true(recruitment$logit_steep[1]$is_random_effect)
  expect_true(recruitment$logit_steep[1]$estimated)
  expect_equal(recruitment$log_rzero[1]$value, log(1000000.0))


  expect_equal(object = recruitment$evaluate(spawns, ssb0), expected = 1090802.68)

  log_devs <- c(-1.0, 2.0, 3.0)
  recruitment$log_devs <- methods::new(ParameterVector, log_devs, length(log_devs))


  expected_lpdf <- sum(log(stats::dnorm(log_devs, 0, 0.7)))


  clear()
})
