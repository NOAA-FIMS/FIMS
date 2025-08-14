#' @export
use_config_template <- function() {

  fishing_fleet_config <- dplyr::tribble(
    ~module_name, ~module_type, ~distribution_link, ~fleet_name, ~population_name, ~distribution_type, ~distribution,
    "Selectivity", "Logistic", NA_character_, "fleet1", NA_character_, NA_character_, NA_character_,
    "Data", "Landings", "Landings", "fleet1", NA_character_, "Data", "DlnormDistribution",
    "Data", "Index", "Index", "fleet1", NA_character_, "Data", "DlnormDistribution",
    "Data", "AgeComp", "AgeComp", "fleet1", NA_character_, "Data", "DmultinomDistribution",
    "Data", "LengthComp", "LengthComp", "fleet1", NA_character_, "Data", "DmultinomDistribution"
  )

  survey_fleet_config <- fishing_fleet_config |>
    dplyr::mutate(fleet_name = "survey1") |>
    # remove row: module_type = "Landings"
    dplyr::filter(module_type != "Landings")

  other_config <- dplyr::tribble(
    ~module_name, ~module_type, ~distribution_link, ~fleet_name, ~population_name, ~distribution_type, ~distribution,
    "Recruitment", "Beverton--Holt", "log_devs", NA_character_, NA_character_,"process", "DnormDistribution",
    "Growth", "EWAAgrowth", NA_character_, NA_character_, NA_character_, NA_character_, NA_character_,
    "Maturity", "Logistic", NA_character_, NA_character_, NA_character_, NA_character_, NA_character_
  )

  final_config <- dplyr::bind_rows(
    fishing_fleet_config,
    survey_fleet_config, 
    other_config
  ) |>
  tidyr::nest(.by = c(module_name, fleet_name, population_name))
}
