library(testthat)
test_that("Recruitment input settings work as expected", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create recruitment
  recruitment <- new(fims$BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000.0
  spawns <- 30.0
  ssb0 <- 100.0


  recruitment$steep$value <- h
  recruitment$steep$min <- 0.21
  recruitment$steep$max <- 1.0
  recruitment$steep$is_random_effect <- TRUE
  recruitment$steep$estimated <- TRUE
  recruitment$rzero$value <- r0

  expect_equal(recruitment$get_id(), 1)
  expect_equal(recruitment$steep$value, 0.75)
  expect_equal(recruitment$steep$min, 0.21)
  expect_equal(recruitment$steep$max, 1.0)
  expect_true(recruitment$steep$is_random_effect)
  expect_true(recruitment$steep$estimated)
  expect_equal(recruitment$rzero$value, 1000.0)

  expect_equal(object = recruitment$evaluate(spawns, ssb0), expected = 837.209300)

  devs1 <- c(1.0,2.0,3.0)
  devs <- c(2.3333333, 0.6666667, 1.6666667)


  recnll <- new(fims$RecruitmentNLL)
  recnll$log_sigma_recruit$value = log(0.7)
  recnll$recruitment_bias_adj = rep(1.0,3)
  recnll$recruitment_devs = devs1
  recnll$do_bias_correction = FALSE

  expected_nll <- -sum(log(stats::dnorm(devs1, 0, 0.7)))


  recnll$estimate_recruit_deviations = FALSE
  expect_equal(recnll$evaluate(), 0.0);

  recnll$estimate_recruit_deviations = TRUE
  recnll$evaluate()

  expect_equal(recnll$evaluate(), expected = expected_nll)


})
