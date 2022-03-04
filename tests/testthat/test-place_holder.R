test_that("length computed correctly", {
  expect_success(expect_length(1, 1))
  expect_failure(expect_length(1, 2), "has length 1, not length 2.")
})
