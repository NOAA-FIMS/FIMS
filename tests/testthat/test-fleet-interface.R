data(package = "FIMS")

test_that("Fleet: selectivity IDs can be added to the
fleet module", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Create selectivity for fleet 1
  selectivity_fleet1 <- new(fims$LogisticSelectivity)
  expect_equal((selectivity_fleet1$get_id()), 1)

  # Create selectivity for fleet 2
  selectivity_fleet2 <- new(fims$LogisticSelectivity)
  expect_equal((selectivity_fleet2$get_id()), 2)

  # Add selectivity to fleet
  fleet1 <- new(fims$Fleet)
  fleet2 <- new(fims$Fleet)

  # Expect code produces no output, error, message, or warnings
  expect_silent(fleet1$SetSelectivity(selectivity_fleet1$get_id()))
  expect_silent(fleet2$SetSelectivity(selectivity_fleet2$get_id()))

  # Expect code produces error when ID of selectivity is a character string
  expect_error(fleet1$SetSelectivity("id"))

  fims$clear()
})


test_that("Fleet: SetAgeCompLikelihood works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetAgeCompLikelihood(1))

  fims$clear()
})

test_that("Fleet: SetIndexLikelihood works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetIndexLikelihood(1))

  fims$clear()
})

test_that("Fleet: SetObservedAgeCompData works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetObservedAgeCompData(1))

  fims$clear()
})

test_that("Fleet: SetObservedIndexData works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet <- new(fims$Fleet)

  expect_silent(fleet$SetObservedIndexData(1))

  fims$clear()
})

test_that("Fleet: evaluate_index_nll works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet_ <- new(fims$Fleet)
  nyears <- 10
  nages <- 12
  fleet_$nyears = nyears
  fleet_$nages <- nages
  fleet_$log_Fmort <- log(rep(1.0, 10))
  fleet_$estimate_F <- TRUE
  fleet_$random_F <- FALSE
  fleet_$log_q <- log(1.0)
  fleet_$estimate_q <- FALSE
  fleet_$random_q <- FALSE
  fleet_$log_obs_error$value <- log(sqrt(log(0.5^2 + 1)))
  fleet_$log_obs_error$estimated <- FALSE
  
  survey_fleet_index <- new(fims$Index, nyears)
  survey_fleet_index$SetIndexLikelihood(1)
  survey_fleet_index$SetObservedIndexData(survey_fleet_index$get_id())
  fleet_$SetObservedIndexData(survey_fleet_index$get_id())

  survey_fleet_index$index_data <- seq_len(1,10,length.out=nyears)

  expect_equal(0.0, fleet_$evaluate_index_nll())
  fims$clear()
})

test_that("Fleet: evaluate_age_comp_nll works", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  fleet_ <- new(fims$Fleet)
nages <- 12
nyears <- 30
  fleet_$nages <- nages
  fleet_$nyears <- nyears
  fleet_$log_Fmort <- log(om_output$f)
  fleet_$estimate_F <- TRUE
  fleet_$random_F <- FALSE
  fleet_$log_q <- log(1.0)
  fleet_$estimate_q <- FALSE
  fleet_$random_q <- FALSE
  fleet_$log_obs_error$value <- log(sqrt(log(1.0^2 + 1)))
  fleet_$log_obs_error$estimated <- FALSE
  fleet_$SetAgeCompLikelihood(1)

  fishing_fleet_age_comp <- new(fims$AgeComp, length(catch), om_input$nages)
  fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * em_input$n.L$fleet1
  fleet_$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())
  expect_equal(0.0, fleet_$evaluate_age_comp_nll())
  fims$clear()
})

