testthat("test get parameter vector",{

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

  #test two modules
  fims$clear()

  selectivity <- new(fims$LogisticSelectivity)
  selectivity$median$value <- 10.0
  selectivity$median$min <- 8.0
  selectivity$median$max <- 12.0
  selectivity$median$is_random_effect <- FALSE
  selectivity$median$estimated <- TRUE
  selectivity$slope$value <- 0.2
  selectivity$slope$is_random_effect <- FALSE
  selectivity$slope$estimated <- TRUE
  recruitment <- new(fims$BevertonHoltRecruitment)
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

  fims$CreateTMBModel() 
  p <- fims$get_fixed()

  expect_equal(c(sel_parm, rec_parm), p)


}
)
