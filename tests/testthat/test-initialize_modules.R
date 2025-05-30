# Instructions ----
#' This file follows the format generated by FIMS:::use_testthat_template().
#' Necessary tests include input and output (IO) correctness [IO
#' correctness], edge-case handling [Edge handling], and built-in errors and
#' warnings [Error handling]. See `?FIMS:::use_testthat_template` for more
#' information. Every test should have a @description tag that takes up just
#' one line, which will be used in the bookdown report of {testthat} results.

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

test_that("initialize_fims works with correct inputs", {
  #' @description Test that [initialize_fims()] returns a list with one
  #' element named parameters.
  result <- initialize_fims(parameters = default_parameters, data = data)
  expect_type(result, "list")
  expect_named(result, "parameters")
  expect_equal(length(result), 1)
  clear()

  #' @description Test that [initialize_comp()] works for AgeComp and returns
  #' an S4 object.
  result <- initialize_comp(
    data = data,
    fleet_name = "fleet1",
    type = "AgeComp"
  )
  expect_type(result, "S4")
  #' @description Test that [initialize_comp()] works for AgeComp and contains
  #' the correct names inside the rcpp object.
  expect_no_error(result[["age_comp_data"]])
  expect_null(result[["length_comp_data"]])
  expect_true(
    all(c("age_comp_data", "initialize", "finalize", ".pointer") %in%
      names(result))
  )
  #' @description Test that the age-composition data in the returned object from
  #' [initialize_comp()] has the correct values.
  expect_equal(
    result$age_comp_data$toRVector(),
    data |>
      get_data() |>
      dplyr::filter(type == "age", name == "fleet1") |>
      dplyr::mutate(out = value * uncertainty) |>
      dplyr::pull(out)
  )
  clear()

  #' @description Test that [initialize_fims()] works for LengthComp and
  #' returns an S4 object.
  result <- initialize_comp(
    data = data,
    fleet_name = "fleet1",
    type = "LengthComp"
  )
  expect_type(result, "S4")
  #' @description Test that [initialize_fims()] works for LengthComp and
  #' contains correct function.
  expect_no_error(result[["length_comp_data"]])
  expect_null(result[["age_comp_data"]])
  expect_true(
    all(c("length_comp_data", "initialize", "finalize", ".pointer") %in%
      names(result))
  )
  #' @description Test that the length-composition data in the returned object
  #' from [initialize_comp()] has the correct values.
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

test_that("initialize_fims returns correct error messages", {
  #' @description Test that [initialize_fims()] handles missing parameters
  #' input correctly.
  expect_error(
    initialize_fims(data = data),
    "argument must be a non-missing list."
  )
  clear()

  #' @description Test that [initialize_fims()] handles non-list parameters
  #' input correctly.
  expect_error(
    initialize_fims(parameters = "not_a_list", data = data),
    "argument must be a non-missing list."
  )
  clear()

  #' @description Test that [initialize_fims()] fails when no fleets are
  #' provided.
  parameters_no_fleets <- default_parameters
  parameters_no_fleets[["modules"]][["fleets"]] <- NULL
  expect_error(
    initialize_fims(parameters = parameters_no_fleets, data = data),
    "No fleets found in the provided"
  )
  clear()

  #' @description Test that [initialize_comp()] correctly returns error on
  #' unknown fleet_name.
  expect_error(
    initialize_comp(
      data = data,
      fleet_name = "unknownfleet",
      type = "AgeComp"
    ),
    "Fleet `unknownfleet` not found in the data object."
  )
  clear()

  #' @description Test that [initialize_comp()] correctly returns error on
  #' unknown type.
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
