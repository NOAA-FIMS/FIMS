test_that("TMB map names omit fixed parameters and collapse shared levels", {
  parameter_names <- c("alpha", "beta", "gamma", "delta")
  map <- factor(c(1, 1, NA, 2))

  expect_equal(
    FIMS:::tmb_mapped_parameter_names(parameter_names, map),
    c("alpha", "delta")
  )
})

test_that("TMB map names preserve all names without a map", {
  parameter_names <- c("alpha", "beta")
  expect_equal(
    FIMS:::tmb_mapped_parameter_names(parameter_names),
    parameter_names
  )
})

test_that("TMB map names validate the map", {
  expect_error(
    FIMS:::tmb_mapped_parameter_names(c("alpha", "beta"), c(1, 2)),
    "must be a factor"
  )
  expect_error(
    FIMS:::tmb_mapped_parameter_names(c("alpha", "beta"), factor(1)),
    "one entry per FIMS parameter"
  )
})

test_that("TMB MakeADFun and FIMS names use the same mapped indexing", {
  clear()
  on.exit(clear(), add = TRUE)

  selectivity <- methods::new(LogisticSelectivity)
  selectivity$inflection_point[1]$value <- 10
  selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  selectivity$slope[1]$value <- 0.2
  selectivity$slope[1]$estimation_type$set("fixed_effects")
  CreateTMBModel()

  parameters <- list(p = get_fixed(), re = get_random())
  map <- list(p = factor(c(1, 1)))
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = parameters,
    map = map,
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  full_names <- names(get_parameter_names(as.list(parameters$p)))
  mapped_names <- FIMS:::tmb_mapped_parameter_names(full_names, map$p)

  expect_length(obj$par, 1L)
  expect_length(mapped_names, length(obj$par))
  expect_identical(mapped_names, full_names[1L])
})
