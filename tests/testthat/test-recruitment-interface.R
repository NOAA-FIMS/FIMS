test_that("Recruitment input settings work as expected", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create recruitment
  recruitment <- new(fims$BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000.0
  spawns <- 500.0
  ssb0 <- 800.0

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

  answer <- (h*r0*0.75*spawns)/(0.2*r0*ssb0*(1-h)+spawns*(h-0.2))

  expect_equal(recruitment$evaluate(spawns, ssb0), answer)
})
