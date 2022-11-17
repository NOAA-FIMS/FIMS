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
})
