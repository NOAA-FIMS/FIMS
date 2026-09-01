# Serialized model fits are caches, not canonical test inputs. Refresh them
# whenever their package or schema contract differs from the code under test.
integration_fixtures_are_current <- function() {
  fit_path <- testthat::test_path("fixtures", "fit_age_length_comp.RDS")
  parameters_path <- testthat::test_path(
    "fixtures", "parameters_model_comparison_project.RDS"
  )

  if (!file.exists(fit_path) || !file.exists(parameters_path)) {
    return(FALSE)
  }

  tryCatch(
    {
      fit <- readRDS(fit_path)
      parameters <- readRDS(parameters_path)
      fit_slots <- methods::slotNames(fit)

      all(c("run_time", "version", "model_output") %in% fit_slots) &&
        identical(
          as.character(methods::slot(fit, "version")),
          as.character(utils::packageVersion("FIMS"))
        ) &&
        "timing" %in% names(parameters)
    },
    error = function(...) FALSE
  )
}

if (!integration_fixtures_are_current()) {
  suppressWarnings(
    suppressMessages(
      prepare_test_data()
    )
  )
}
