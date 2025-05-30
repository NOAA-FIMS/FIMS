# tests/testthat/test_test-initialize_modules.R

library(testthat)

test_that("initialize_fims returns a list with correct structure", {
  data <- FIMS::FIMSFrame(data1)
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )
  fleets <- list(fleet1 = fleet1, survey1 = survey1)
  default_parameters <- create_default_parameters(data, fleets = fleets)
  result <- initialize_fims(parameters = default_parameters, data = data)
  expect_type(result, "list")
  expect_named(result, "parameters")
  expect_equal(length(result), 1)
})

test_that("initialize_comp returns S4 object with correct slots for AgeComp", {
  data <- FIMS::FIMSFrame(data1)
  result <- initialize_comp(
    data = data,
    fleet_name = "fleet1",
    type = "AgeComp"
  )
  expect_type(result, "S4")
  expect_no_error(result[["age_comp_data"]])
  expect_null(result[["length_comp_data"]])
  expect_true(
    all(c("age_comp_data", "initialize", "finalize", ".pointer") %in%
      names(result))
  )
})

test_that("initialize_comp returns S4 object with correct slots for LengthComp", {
  data <- FIMS::FIMSFrame(data1)
  result <- initialize_comp(
    data = data,
    fleet_name = "fleet1",
    type = "LengthComp"
  )
  expect_type(result, "S4")
  expect_no_error(result[["length_comp_data"]])
  expect_null(result[["age_comp_data"]])
  expect_true(
    all(c("length_comp_data", "initialize", "finalize", ".pointer") %in%
      names(result))
  )
})

test_that("initialize_fims handles missing or invalid parameters", {
  data <- FIMS::FIMSFrame(data1)
  expect_error(
    initialize_fims(data = data),
    "argument must be a non-missing list."
  )
  expect_error(
    initialize_fims(parameters = "not_a_list", data = data),
    "argument must be a non-missing list."
  )
})

test_that("initialize_fims fails when no fleets are provided", {
  data <- FIMS::FIMSFrame(data1)
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )
  fleets <- list(fleet1 = fleet1, survey1 = survey1)
  default_parameters <- create_default_parameters(data, fleets = fleets)
  parameters_no_fleets <- default_parameters
  parameters_no_fleets[["modules"]][["fleets"]] <- NULL
  expect_error(
    initialize_fims(parameters = parameters_no_fleets, data = data),
    "No fleets found in the provided"
  )
})

test_that("initialize_comp returns error on unknown fleet_name", {
  data <- FIMS::FIMSFrame(data1)
  expect_error(
    initialize_comp(
      data = data,
      fleet_name = "unknownfleet",
      type = "AgeComp"
    ),
    "Fleet `unknownfleet` not found in the data object."
  )
})

test_that("initialize_comp returns error on unknown type", {
  data <- FIMS::FIMSFrame(data1)
  expect_error(
    initialize_comp(
      data = data,
      fleet_name = "fleet1",
      type = "unknown"
    ),
    "should be one of"
  )
})

test_that("initialize_fims returns error on unknown estimation_type", {
  data <- FIMS::FIMSFrame(data1)
  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )
  fleets <- list(fleet1 = fleet1, survey1 = survey1)
  default_parameters <- create_default_parameters(data, fleets = fleets)
  parameters_wrong_type <- default_parameters
  parameters_wrong_type[["parameters"]][["recruitment"]][["BevertonHoltRecruitment.log_devs.estimation_type"]] <- 'fixed.effects'
  expect_error(
    initialize_fims(parameters = parameters_wrong_type, data = data),
    "The estimation type entered: 'fixed.effects', is not one of the three options: constant, fixed_effects, random_effects"
  )
})