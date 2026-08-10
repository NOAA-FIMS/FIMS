test_that("get_random() reads native random-effect registrations", {
  clear()
  selectivity_logistic_create(
    inflection_point = 2,
    slope = 1,
    inflection_point_estimation_type = "constant",
    slope_estimation_type = "random_effects"
  )

  expect_equal(get_random(), log(1))
  clear()
  expect_equal(get_random(), numeric())
})
