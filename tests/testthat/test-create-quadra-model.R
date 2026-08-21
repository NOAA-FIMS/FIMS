test_that("CreateQuadraModel constructs a FIMS model with Quadra", {
  clear()
  on.exit(clear())

  selectivity <- methods::new(LogisticSelectivity)
  selectivity$inflection_point[1]$value <- 10
  selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  selectivity$slope[1]$value <- 0.2
  selectivity$slope[1]$estimation_type$set("fixed_effects")

  expect_true(CreateQuadraModel())
})

test_that("TMB remains constructible after a Quadra model", {
  clear()
  on.exit(clear())

  expect_true(CreateQuadraModel())
  clear()
  expect_true(CreateTMBModel())
})
