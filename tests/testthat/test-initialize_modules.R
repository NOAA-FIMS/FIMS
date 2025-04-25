# test_initialize_modules ----
## Setup ----
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

## IO correctness ----

test_that("initialize_fims works", {
  #' @description initialize_fims works
  result <- initialize_fims(parameters = default_parameters, data = data)

  expect_type(result, "list")
  expect_named(result, "parameters")
  clear()
})

test_that("initialize_comp works for type = AgeComp", {
  result <- initialize_comp(
    data = data,
    fleet_name = "fleet1",
    type = "AgeComp"
  )
  #' @description initialize_fims works for AgeComp: returns an S4 object
  expect_type(result, "S4")
  #' @description initialize_fims works for AgeComp: class contains correct function
  expect_no_error(result$age_comp_data)
  expect_true("age_comp_data" %in% names(result))
  #' @description initialize_fims works: class does not contain the other type of
  #'   function
  expect_error(result$length_comp_data)
  expect_false("length_comp_data" %in% names(result))

  expect_equal(
    result$age_comp_data$toRVector(),
    data |>
      get_data() |>
      dplyr::filter(type == "age", name == "fleet1") |>
      dplyr::mutate(out = value * uncertainty) |>
      dplyr::pull(out)
  )

  clear()
})

test_that("initialize_comp works for type = LengthComp", {
  result <- initialize_comp(
    data = data,
    fleet_name = "fleet1",
    type = "LengthComp"
  )
  #' @description initialize_fims works for LengthComp: returns an S4 object
  expect_type(result, "S4")
  #' @description initialize_fims works for LengthComp: class contains correct function
  expect_no_error(result$length_comp_data)
  expect_true("length_comp_data" %in% names(result))
  #' @description initialize_fims works: class does not contain the other type of
  #'   function
  expect_error(result$age_comp_data)

  expect_equal(
    result$length_comp_data$toRVector(),
    data |>
      get_data() |>
      dplyr::filter(type == "length", name == "fleet1") |>
      dplyr::mutate(out = value * uncertainty) |>
      dplyr::pull(out)
  )

  clear()
})

## Edge handling ----

## Error handling ----

test_that("initialize_fims handles missing parameters input correctly", {
  #' @description initialize_fims handles missing parameters input correctly
  expect_error(
    initialize_fims(data = data),
    "argument must be a non-missing list."
  )
  clear()
})

test_that("initialize_fims handles non-list parameters input correctly", {
  #' @description initialize_fims handles non-list parameters input correctly
  expect_error(
    initialize_fims(parameters = "not_a_list", data = data),
    "argument must be a non-missing list."
  )
  clear()
})

test_that("initialize_fims fails when no fleets are provided", {
  #' @description initialize_fims fails when no fleets are provided
  parameters_no_fleets[["modules"]][["fleets"]] <- NULL
  expect_error(
    initialize_fims(parameters = parameters_no_fleets, data = data),
    "No fleets found in the provided"
  )
  clear()
})

test_that("initialize_comp correctly returns error on unknown fleet_name", {
  expect_error(
    initialize_comp(
      data = data,
      fleet_name = "unknownfleet",
      type = "AgeComp"
    ),
    "Fleet `unknownfleet` not found in the data object."
  )
  clear()
})

test_that("initialize_comp correctly returns error on unknown type", {
  expect_error(
    initialize_comp(
      data = data,
      fleet_name = "fleet1",
      type = "unknown"
    ),
    "should be one of"
  )

  clear()
})

# TODO: most lines with no coverage are error checks that are not verified to 
# work via the tests.