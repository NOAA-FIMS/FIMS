# plot_age_to_length_conversion ----
#' Tests fixed input and realized Growth-derived output plotting, fleet selection,
#' and informative errors for unsupported or incomplete inputs.


## Setup ----
data("data_big", package = "FIMS")

fixed_conversion_frame <- FIMS::FIMSFrame(data_big)

derived_conversion_output <- tibble::tibble(
  fleet = rep(c("fleet1", "survey1"), each = 4),
  label = "age_to_length_conversion_derived",
  year_i = 1L,
  age_i = rep(rep(1:2, each = 2), 2),
  length_i = rep(1:2, 4),
  estimated = rep(c(0.2, 0.8, 0.6, 0.4), 2)
)

## IO correctness ----
test_that("plot_age_to_length_conversion() plots supported inputs", {
  #' @description Test that fixed age-to-length conversion input returns a ggplot object.
  expect_s3_class(
    plot_age_to_length_conversion(fixed_conversion_frame),
    "ggplot"
  )

  #' @description Test that one fleet of derived long output returns a ggplot object.
  expect_s3_class(
    plot_age_to_length_conversion(
      dplyr::filter(
        derived_conversion_output,
        .data[["fleet"]] == "fleet1"
      )
    ),
    "ggplot"
  )
})

## Edge handling ----
test_that("plot_age_to_length_conversion() selects the requested derived fleet", {
  selected_fleet_plot <- plot_age_to_length_conversion(
    data = derived_conversion_output,
    fleet = "fleet1"
  )

  #' @description Test that selecting a fleet uses only that fleet's derived probabilities.
  expect_equal(
    selected_fleet_plot[["data"]][["value"]],
    c(0.2, 0.8, 0.6, 0.4)
  )
})

## Error handling ----
test_that("plot_age_to_length_conversion() gives clear fleet-selection errors", {
  #' @description Test that derived output with multiple fleets requires an explicit fleet.
  expect_error(
    plot_age_to_length_conversion(derived_conversion_output),
    "multiple fleets"
  )

  #' @description Test that an unavailable fleet produces an informative error.
  expect_error(
    plot_age_to_length_conversion(
      data = derived_conversion_output,
      fleet = "unknown_fleet"
    ),
    "not present"
  )

  #' @description Test that fleet selection is rejected for fixed conversion input.
  expect_error(
    plot_age_to_length_conversion(
      data = fixed_conversion_frame,
      fleet = "fleet1"
    ),
    "only applies"
  )

  #' @description Test that a FIMSFrame without conversion rows produces an informative error.
  expect_error(
    plot_age_to_length_conversion(
      FIMS::FIMSFrame(
        dplyr::filter(
          data_big,
          .data[["type"]] != "age_to_length_conversion"
        )
      )
    ),
    "has no"
  )

  #' @description Test that incomplete derived output produces an informative error.
  expect_error(
    plot_age_to_length_conversion(
      tibble::tibble(
        fleet = "fleet1",
        label = "age_to_length_conversion_derived"
      )
    ),
    "missing required columns"
  )
})
