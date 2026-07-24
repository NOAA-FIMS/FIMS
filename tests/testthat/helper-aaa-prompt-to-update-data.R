# Helper script to prompt developer to update *_big dataset if relevant code has been modified

if (interactive()) {
  cli::cli_alert_warning(
    "Did you modify code affecting {.var data_big} generation, parameter setup, model fitting, or {.fn get_estimates}?"
  )

  cli::cli_alert_info(
    "If yes, run `source(file.path('data-raw', 'data_big.R'))` to update the dataset."
  )
}
