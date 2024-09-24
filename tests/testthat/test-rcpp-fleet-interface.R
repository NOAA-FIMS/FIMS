test_that("Fleet: selectivity IDs can be added to the
fleet module", {
  # Create selectivity for fleet 1
  selectivity_fleet1 <- new(LogisticSelectivity)
  expect_equal((selectivity_fleet1$get_id()), 1)

  # Create selectivity for fleet 2
  selectivity_fleet2 <- new(LogisticSelectivity)
  expect_equal((selectivity_fleet2$get_id()), 2)

  # Add selectivity to fleet
  fleet1 <- new(Fleet)
  fleet2 <- new(Fleet)

  # Expect code produces no output, error, message, or warnings
  expect_silent(fleet1$SetSelectivity(selectivity_fleet1$get_id()))
  expect_silent(fleet2$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet1$SetSelectivity("id"))

  clear()
})

test_that("Fleet: length composition can be added to the
fleet module", {

   # set fishing fleet age comp data, need to set dimensions of age comps
  fishing_fleet_length_comp <- new(LengthComp, om_input$nyr, om_input$nages)
  fishing_fleet_length_comp$length_comp_data <- c(t(length_data)) * length(om_input_list$len_bins)

  # Add selectivity to fleet
  fleet1 <- new(Fleet)

  fleet1$nages <- om_input$nages
  fleet1$nyears <- om_input$nyr
  fleet1$nlengths <- length(om_input_list$len_bins)
  fleet1$age_length_conversion_matrix <- length_age_conversion #this should be a matrix of dim nages * nlengths
  fleet1$log_Fmort <- methods::new(ParameterVector, log(om_output$f), om_input$nyr)
  fleet1$log_Fmort$set_all_estimable(TRUE)
  fleet1$log_q <- log(1.0)
  fleet1$estimate_q <- FALSE
  fleet1$random_q <- FALSE

  # Set up fishery age composition data using the multinomial
  fishing_fleet_lengthcomp_distribution <- methods::new(TMBDmultinomDistribution)
  fishing_fleet_lengthcomp_distribution$set_observed_data(fishing_fleet_length_comp$get_id())
  fishing_fleet_lengthcomp_distribution$set_distribution_links("data", fishing_fleet$proportion_catch_numbers_at_length$get_id())

  clear()
})