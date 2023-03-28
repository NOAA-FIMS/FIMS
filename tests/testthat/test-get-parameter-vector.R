testthat("test get parameter vector",{

  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
    # Create selectivity
  selectivity <- new(fims$LogisticSelectivity)
  selectivity$median$value <- 10.0
  selectivity$median$min <- 8.0
  selectivity$median$max <- 12.0
  selectivity$median$is_random_effect <- TRUE
  selectivity$median$estimated <- TRUE
  selectivity$slope$value <- 0.2

  p <- get_parameter_function()
}
)
