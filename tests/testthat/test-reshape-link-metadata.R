test_that("link metadata does not change reshaped parameter estimates", {
  output <- get_model_output(FIMS::fit_big_optimized)
  baseline <- FIMS:::reshape_json_estimates(output)
  configuration <- jsonlite::fromJSON(output, simplifyVector = FALSE)
  configuration$recruitment_links <- list(list(allocations = list(
    list(population_id = 1, fraction = 0.4), list(population_id = 2, fraction = 0.6)
  )))
  configuration$fleet_observation_links <- list(list(
    target_id = 3, source_ids = c(1L, 2L), use_catch = TRUE
  ))
  configuration$fleet_parameter_links <- list(list(
    parameter = "log_Fmort", owner_id = 1, follower_ids = 2L
  ))
  #' @description Configuration-only link metadata is ignored when reshaping parameter and derived-quantity modules.
  expect_equal(
    FIMS:::reshape_json_estimates(jsonlite::toJSON(configuration, auto_unbox = TRUE, null = "null", digits = NA)),
    baseline
  )
})

test_that("fresh pooled model exports reach get_estimates with link metadata", {
  on.exit(clear())
  clear()
  data <- FIMSFrame(data_big)
  fit <- setup_default_parameters(data) |>
    initialize_fims(data = data) |>
    fit_fims(optimize = FALSE)
  #' @description The complete export-to-estimates path used by data regeneration and vignettes accepts empty link configurations.
  expect_s3_class(get_estimates(fit), "tbl_df")
  configuration <- jsonlite::fromJSON(get_model_output(fit), simplifyVector = FALSE)
  expect_true(all(c("recruitment_links", "fleet_observation_links") %in% names(configuration)))
})


test_that("empty JSON parameter sections terminate without producing rows", {
  #' @description Optional empty parameter modules, including tidyr's scalar NA placeholder, do not hang or create fake parameters.
  for (section in list(NULL, list(), NA, list(NA), list(list()))) {
    expect_equal(nrow(FIMS:::dimension_folded_to_tibble(section)), 0L)
  }
})
