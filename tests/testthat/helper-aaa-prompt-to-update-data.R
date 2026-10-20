if (interactive()) {
  cli::cli_alert_warning(
    "Did you modify code affecting {.var data_big} generation, parameter setup, model fitting, or {.fn get_estimates}?"
  )

  cli::cli_alert_info(
    "If yes, run `source(file.path('data-raw', 'data_big.R')); source(file.path('data-raw', 'save_internal_data.R'))` to update the package data."
  )
}
