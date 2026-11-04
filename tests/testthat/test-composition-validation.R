test_that("composition validation preserves tolerance and unit checks", {
  #' @description Valid proportions, counts, missing groups, and tolerance boundaries retain their validation results; invalid groups retain diagnostic context.
  data <- tibble::tibble(type = "age_comp", fleet = "survey",
    timing = 3, unit = "proportion", observed = c(0.25, 0.75), age = 1:2)
  expect_identical(FIMS:::validate_composition_data(data), 0)
  data$observed <- c(0.25, 0.7505)
  expect_identical(FIMS:::validate_composition_data(data), 0)
  data$unit <- "number"
  data$observed <- c(25, 75)
  expect_identical(FIMS:::validate_composition_data(data), 0)
  data$observed <- c(-999, -999)
  expect_identical(FIMS:::validate_composition_data(data), 0)
  data$unit <- "proportion"
  data$observed <- c(0.2, 0.7)
  diagnostics <- capture.output(
    result <- FIMS:::validate_composition_data(data), type = "message"
  )
  expect_identical(result, 1)
  expect_match(paste(diagnostics, collapse = "\n"), "survey")
  expect_match(paste(diagnostics, collapse = "\n"), "not 1.0", fixed = TRUE)
  data$unit <- c("number", "proportion")
  diagnostics <- capture.output(
    result <- FIMS:::validate_composition_data(data), type = "message"
  )
  expect_identical(result, 1)
  expect_match(paste(diagnostics, collapse = "\n"), "only be one unit", fixed = TRUE)
})
