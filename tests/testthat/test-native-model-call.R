test_that("native information state can be cleared and inspected", {
  expect_null(native_clear())
  expect_equal(native_get_fixed(), numeric())
  expect_equal(native_get_random(), numeric())
  expect_equal(native_get_parameter_names(), character())
  expect_equal(
    unname(native_information_model_counts()),
    rep(0L, 5L)
  )
})

test_that("native model assembly requires a population", {
  native_clear()
  expect_error(
    native_create_model(),
    "No populations are registered"
  )
})
