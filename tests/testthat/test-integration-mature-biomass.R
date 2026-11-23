# Mature population biomass through the public R interface.
testthat::skip_if(
  Sys.getenv("RUN_SLOW_TESTS") != "true",
  "Skipping: RUN_SLOW_TESTS is not set to true."
)

data("data_big")
data_4_model <- FIMSFrame(data_big)
default_parameters <- setup_default_parameters(data = data_4_model)

report_for_maturity <- function(inflection) {
  on.exit(clear())
  parameters <- default_parameters |>
    dplyr::mutate(value = dplyr::if_else(
      module_name == "Maturity" & label == "inflection_point",
      inflection, value
    ))
  fit <- parameters |>
    initialize_fims(data = data_4_model) |>
    fit_fims(optimize = FALSE)
  get_report(fit)
}

test_that("fully mature populations contribute their full biomass", {
  report <- report_for_maturity(-100)
  #' @description Test that full maturity makes spawning biomass equal to total biomass in fished and unfished populations.
  expect_equal(report$spawning_biomass[[1]], report$biomass[[1]])
  expect_equal(
    report$unfished_spawning_biomass[[1]], report$unfished_biomass[[1]]
  )
})

test_that("later maturity reduces spawning biomass at fixed initial abundance", {
  early <- report_for_maturity(2)
  late <- report_for_maturity(5)
  #' @description Test that delaying maturity changes spawning biomass while retaining first-year abundance and biomass.
  expect_equal(early$biomass[[1]][1], late$biomass[[1]][1])
  expect_gt(early$spawning_biomass[[1]][1], late$spawning_biomass[[1]][1])
  expect_gt(late$spawning_biomass[[1]][1], 0)
  expect_lt(early$spawning_biomass[[1]][1], early$biomass[[1]][1])
})
