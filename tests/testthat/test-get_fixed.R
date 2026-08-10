test_that("get_fixed() reads native fixed-effect registrations", {
  clear()
  selectivity_logistic_create(
    inflection_point = 2,
    slope = 1,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "constant"
  )

  expect_equal(get_fixed(), 2)
  clear()
  expect_equal(get_fixed(), numeric())
})
