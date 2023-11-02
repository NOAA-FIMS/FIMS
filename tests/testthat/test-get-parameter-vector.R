test_that("test get parameter vector", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  # Create selectivity
  selectivity <- new(fims$LogisticSelectivity)
  selectivity$inflection_point$value <- 10.0
  selectivity$inflection_point$min <- 8.0
  selectivity$inflection_point$max <- 12.0
  selectivity$inflection_point$is_random_effect <- FALSE
  selectivity$inflection_point$estimated <- TRUE
  selectivity$slope$value <- 0.2
  selectivity$slope$is_random_effect <- FALSE
  selectivity$slope$estimated <- TRUE

  fims$CreateTMBModel()
  p <- fims$get_fixed()
  sel_parm <- c(selectivity$inflection_point$value, selectivity$slope$value)
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
  selectivity$inflection_point$value <- 11.0
  selectivity$inflection_point$min <- 8.0
  selectivity$inflection_point$max <- 12.0
  selectivity$inflection_point$is_random_effect <- FALSE
  selectivity$inflection_point$estimated <- TRUE
  selectivity$slope$value <- 0.5
  selectivity$slope$is_random_effect <- FALSE
  selectivity$slope$estimated <- TRUE
  sel_parm <- c(selectivity$inflection_point$value, selectivity$slope$value)
  recruitment <- new(fims2$BevertonHoltRecruitment)
  h <- 0.75
  r0 <- 1000000.0
  spawns <- 9.55784 * 10^6
  ssb0 <- 0.0102562
  recruitment$logit_steep$value <- -log(1.0 - h) + log(h - 0.2)
  recruitment$logit_steep$min <- 0.21
  recruitment$logit_steep$max <- 1.0
  recruitment$logit_steep$is_random_effect <- FALSE
  recruitment$logit_steep$estimated <- TRUE
  recruitment$log_rzero$value <- log(r0)
  recruitment$log_rzero$is_random_effect <- FALSE
  recruitment$log_rzero$estimated <- TRUE
  rec_parm <- c(-log(1.0 - h) + log(h - 0.2), log(r0))

  fims2$CreateTMBModel()
  p2 <- fims2$get_fixed()

  expect_equal(c(sel_parm, rec_parm), p2)
  fims2$clear()
})
