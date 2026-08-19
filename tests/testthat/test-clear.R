test_that("clear() resets native registrations", {
  clear()
  selectivity_logistic_create(
    inflection_point = 2,
    slope = 1,
    inflection_point_estimation_type = "fixed_effects"
  )

  expect_gt(
    native_information_parameter_counts()[["fixed_effects_parameters"]],
    0L
  )
  expect_null(clear())
  expect_equal(native_get_fixed(), numeric())
  expect_equal(native_get_random(), numeric())
  expect_equal(unname(native_information_model_counts()), rep(0L, 5L))
})
