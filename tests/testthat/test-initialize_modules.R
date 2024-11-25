data("data_mile1")
data <- FIMSFrame(data_mile1)

fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Index = "DlnormDistribution",
    AgeComp = "DmultinomDistribution"
  )
)

fleets = list(fleet1 = fleet1, survey1 = survey1)

default_parameters <- create_default_parameters(data, fleets = fleets)

test_that("initialize_fims handles missing parameters input correctly", {
  expect_error(
    initialize_fims(data = data),
    "argument must be a non-missing list."
  )
  clear()
})

test_that("initialize_fims handles non-list parameters input correctly", {
  expect_error(
    initialize_fims(parameters = "not_a_list", data = data),
    "argument must be a non-missing list."
  )
  clear()
})

test_that("initialize_fims validates data input correctly", {
  expect_error(
    initialize_fims(parameters = default_parameters, data = "not_an_s4_object"),
    "argument must be an object created by."
  )
  clear()
})

test_that("initialize_fims fails when no fleets are provided", {
  parameters_no_fleets <- default_parameters
  parameters_no_fleets[["modules"]][["fleets"]] <- NULL
  expect_error(
    initialize_fims(parameters = parameters_no_fleets, data = data),
    "No fleets found in the provided"
  )
  clear()
})

test_that("initialize_fims works", {

  result <- initialize_fims(parameters = default_parameters, data = data)

  expect_type(result, "list")
  expect_named(result, "p")
  clear()
})

test_that("initialize_fims clears previous FIMS settings before initializing", {
  mock_clear <- mockery::mock()
  mockery::stub(initialize_fims, "clear", mock_clear)

  initialize_fims(parameters = default_parameters, data = data)

  mockery::expect_called(mock_clear, 1)
  clear()
})

