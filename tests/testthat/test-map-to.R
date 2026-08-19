test_that("`map_to()` aliases fixed-effect VariableVectors", {
  skip_if_not(exists("map_to", envir = asNamespace("FIMS"), inherits = FALSE))
  clear()
  selectivity <- methods::new(LogisticSelectivity)
  selectivity$inflection_point[1]$value <- 10
  selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  selectivity$slope[1]$value <- 0.2
  selectivity$slope[1]$estimation_type$set("fixed_effects")

  map_to(
    selectivity$slope$get_id(),
    selectivity$inflection_point$get_id()
  )
  CreateTMBModel()

  expect_equal(get_fixed(), 10)
  expect_equal(length(names(get_parameter_names(as.list(get_fixed())))), 1L)
  clear()
})

test_that("`map_to()` aliases random-effect VariableVectors", {
  skip_if_not(exists("map_to", envir = asNamespace("FIMS"), inherits = FALSE))
  clear()
  selectivity <- methods::new(LogisticSelectivity)
  selectivity$inflection_point[1]$value <- 10
  selectivity$inflection_point[1]$estimation_type$set("random_effects")
  selectivity$slope[1]$value <- 0.2
  selectivity$slope[1]$estimation_type$set("random_effects")

  map_to(
    selectivity$slope$get_id(),
    selectivity$inflection_point$get_id()
  )
  CreateTMBModel()

  expect_equal(get_random(), 10)
  expect_equal(length(names(get_random_names(as.list(get_random())))), 1L)
  clear()
})

test_that("`map_to()` validates mappings", {
  skip_if_not(exists("map_to", envir = asNamespace("FIMS"), inherits = FALSE))
  clear()
  selectivity <- methods::new(LogisticSelectivity)
  id <- selectivity$slope$get_id()
  map_to(id, id)
  expect_error(CreateTMBModel(), "cannot be mapped to itself")
  clear()

  selectivity <- methods::new(LogisticSelectivity)
  selectivity$slope$resize(2)
  map_to(
    selectivity$slope$get_id(),
    selectivity$inflection_point$get_id()
  )
  expect_error(CreateTMBModel(), "must have the same length")
  clear()
})
