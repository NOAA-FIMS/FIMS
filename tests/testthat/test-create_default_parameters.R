data("data1")
data <- FIMSFrame(data1)

fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Index = "DlnormDistribution",
    AgeComp = "DmultinomDistribution"
  )
)

fleets = list(fleet1 = fleet1, survey1 = survey1)

test_that("create_default_parameters handles empty data object", {
  empty_data <- NULL

  expect_error(create_default_parameters(empty_data, fleets = fleets))
})

test_that("create_default_parameters returns correct structure", {

  result <- create_default_parameters(data, fleets = fleets)

  expect_named(result, c("parameters", "modules"))
  expect_type(result[["parameters"]], "list")
  expect_type(result[["modules"]], "list")
  expect_named(result[["modules"]], c("fleets", "recruitment", "growth", "maturity"))
})

test_that("create_default_parameters detects missing fleet names", {
  invalid_fleet <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution"
    )
  )

  invalid_fleets = list(
    fleet1 = fleet1,
    survey1 = survey1,
    invalid_fleet = invalid_fleet
  )

  expect_error(
    create_default_parameters(data, invalid_fleets),
    "The following 1 fleet name is missing from the data: invalid_fleet"
  )
})
