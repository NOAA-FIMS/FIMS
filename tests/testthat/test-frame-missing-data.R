test_that("missing-data expansion preserves duplicate and missing-unit keys", {
  #' @description Missing annual and composition rows are generated once per key, with missing units matched and unrelated observation types excluded.
  data <- tibble::tibble(
    type = c("catch", "catch", "catch", "age_comp", "age_comp", "index"),
    fleet = c("a", "a", "b", "a", "a", "unused"),
    unit = c("mt", "mt", NA, "proportion", "proportion", "number"),
    timing = c(1, 1, 2, 1, 1, 1),
    age = c(NA, NA, NA, 1, 2, NA),
    metadata = letters[1:6]
  )
  annual <- FIMS:::create_missing_data(data, timings = 1:2, types = "catch")
  expect_equal(annual, tibble::tibble(
    type = c("catch", "catch"), fleet = c("a", "b"),
    unit = c("mt", NA), timing = c(2, 1), observed = c(-999, -999)
  ))
  ages <- FIMS:::create_missing_data(data, bins = 1:2, timings = 1:2,
    column = "age", types = "age_comp")
  expect_equal(ages, tibble::tibble(
    type = rep("age_comp", 2), fleet = rep("a", 2),
    unit = rep("proportion", 2), timing = rep(2L, 2),
    age = 1:2, observed = rep(-999, 2)
  ))
  expect_equal(nrow(FIMS:::create_missing_data(
    data, timings = 1:2, types = "length_comp"
  )), 0)
})
