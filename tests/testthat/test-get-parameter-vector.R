test_that("test get parameter vector", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  # Create selectivity
  selectivity <- new(fims$LogisticSelectivity)
  selectivity$median$value <- 10.0
  selectivity$median$min <- 8.0
  selectivity$median$max <- 12.0
  selectivity$median$is_random_effect <- FALSE
  selectivity$median$estimated <- TRUE
  selectivity$slope$value <- 0.2
  selectivity$slope$is_random_effect <- FALSE
  selectivity$slope$estimated <- TRUE

  fims$CreateTMBModel()
  p <- fims$get_fixed()
  sel_parm <- c(selectivity$median$value, selectivity$slope$value)
  expect_equal(sel_parm, p)

  # test fims clear
  fims$clear()
  p <- fims$get_fixed()
  expect_equal(numeric(0), p)
  fims$CreateTMBModel()
  p <- fims$get_fixed()
  expect_equal(numeric(0), p)

  ## Not working - fims2::CreateTMBModel() retains the defined module from fims?
  # test multiple modules
  fims$clear()
  p <- fims$get_fixed()
  expect_equal(numeric(0), p)
  fims2 <- Rcpp::Module("fims", PACKAGE = "FIMS")
  selectivity <- new(fims2$LogisticSelectivity)
  selectivity$median$value <- 11.0
  selectivity$median$min <- 8.0
  selectivity$median$max <- 12.0
  selectivity$median$is_random_effect <- FALSE
  selectivity$median$estimated <- TRUE
  selectivity$slope$value <- 0.5
  selectivity$slope$is_random_effect <- FALSE
  selectivity$slope$estimated <- TRUE
  sel_parm <- c(selectivity$median$value, selectivity$slope$value)
  recruitment <- new(fims2$BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000000.0
  spawns <- 9.55784 * 10^6
  ssb0 <- 0.0102562
  recruitment$steep$value <- h
  recruitment$steep$min <- 0.21
  recruitment$steep$max <- 1.0
  recruitment$steep$is_random_effect <- FALSE
  recruitment$steep$estimated <- TRUE
  recruitment$rzero$value <- r0
  recruitment$rzero$is_random_effect <- FALSE
  recruitment$rzero$estimated <- TRUE
  rec_parm <- c(h, r0)

  fims2$CreateTMBModel()
  p2 <- fims2$get_fixed()

  expect_equal(c(sel_parm, rec_parm), p2)
  fims2$clear()
})
