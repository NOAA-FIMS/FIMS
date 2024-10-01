# Prepare initial input
prepare_input <- function(
    data,
    selectivity = c("LogisticSelectivity"),
    recruitment_form = c("BevertonHoltRecruitment"),
    recruitment_distribution = c("TMBDnormDistribution"),
    growth = c("EWAAgrowth"),
    maturity = c("LogisticMaturity")) {

  fishing_fleet_names <- dplyr::filter(
    .data = as.data.frame(data@data),
    type == "landings"
  ) |>
    dplyr::distinct(name) |>
    dplyr::pull(name)

  if (length(selectivity) != length(fishing_fleet_names)) {
    cli::cli_abort(c(
      "Number of selectivity forms must match the number of fleets:",
      "i" = "There {?is/are} {length(fishing_fleet_names)} fleet{?s}.",
      "x" = "{length(selectivity)} selectivity form{?s} {?has/have} been provided."
    ))
  }

  parameter_input <- list()

  for (i in 1:length(selectivity)) {
    selectivity_temp <- prepare_selectivity_default(selectivity[i])
    parameter_input <- c(parameter_input, selectivity_temp)
  }

  recruitment_temp <- prepare_recruitment_default(
    recruitment_form = recruitment_form,
    recruitment_distribution = recruitment_distribution,
    data = data
  )
  parameter_input <- c(parameter_input, recruitment_temp)

  growth_temp <- prepare_growth_default(
    form = growth,
    data = data
  )
  parameter_input <- c(parameter_input, growth_temp)

  maturity_temp <- prepare_maturity_default(form = maturity)
  parameter_input <- c(parameter_input, maturity_temp)

  parameter_input_list <- do.call(c, unlist(parameter_input, recursive=FALSE))
  # parameter_input_dataframe <- as.data.frame(parameter_input_vector)
  # return(parameter_input_dataframe)
  return(parameter_input_list)
}

prepare_Logistic_default <- function() {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(
    inflection_point = list(
      value = 2, estimated = TRUE
    ),
    slope = list(
      value = 1, estimated = TRUE
    )
  )
  return(default)
}

prepare_DoubleLogistic_default <- function() {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(
    inflection_point = list(
      value = 2, estimated = TRUE
    ),
    slope = list(
      value = 1, estimated = TRUE
    )
  )
  return(default)
}

prepare_EWAAgrowth_default <- function(data) {
  default <- list(
    ages = data@ages,
    weights = rep(0.001, data@n_ages)
  )
  return(default)
}

prepare_selectivity_default <- function(form) {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(switch(form,
    "LogisticSelectivity" = prepare_Logistic_default(),
    "DoubleLogisticSelectivity" = prepare_DoubleLogistic_default()
  ))
  names(default) <- form
  return(default)
}

prepare_maturity_default <- function(form) {
  # Can pass data to this function in the future to
  # dynamically set up initial values
  default <- list(switch(form,
    "LogisticMaturity" = prepare_Logistic_default()
  ))
  names(default) <- form
  return(default)
}

prepare_growth_default <- function(form, data) {
  default <- list(switch(form,
    "EWAAgrowth" = prepare_EWAAgrowth_default(data)
  ))
  names(default) <- form
  return(default)
}

prepare_BevertonHoltRecruitment_default <- function(data) {
  default <- list(
    log_rzero = list(
      value = log(1e+06), estimated = TRUE
    ),
    logit_steep = list(
      value = -log(1.0 - 0.75) + log(0.75 - 0.2), estimated = TRUE
    ),
    log_devs = list(
      value = rep(0.01, data@n_years),
      nyears = data@n_years,
      estimated = TRUE
    )
  )
  return(default)
}

prepare_TMBDnormDistribution_default <- function(data) {
  default <- list(
    log_sd = list(
      value = log(0.4), estimated = FALSE
    ),
    x = list(value = 0),
    expected_values = list(value = 0)
  )
  return(default)
}

prepare_recruitment_default <- function(recruitment_form,
                                        recruitment_distribution,
                                        data) {
  recruitment_default <- list(switch(recruitment_form,
    "BevertonHoltRecruitment" = prepare_BevertonHoltRecruitment_default(data)
  ))
  names(recruitment_default) <- recruitment_form

  if (recruitment_default[[recruitment_form]]$log_devs$estimated == TRUE) {
    distribution_default <- list(switch(recruitment_distribution,
      "TMBDnormDistribution" = prepare_TMBDnormDistribution_default(data)
    ))
    names(distribution_default) <- paste0("Recruitment", recruitment_distribution)
  }

  default <- c(recruitment_default, distribution_default)
  return(default)
}
# modifies a particular element of the current input from prepare_input()
update_input <- function(parameter_input, ...) {
  current_input <- parameter_input
  updated_input <- list(...)
  parameter_vector <- as.vector(parameter_input)
  new_input <- `[<-`(parameter_input, names(updated_input), updated_input)
  return(new_input)
}

set_input <- function(data, parameter_input){

}

# example
data(data_mile1)
data <- FIMS::FIMSFrame(data_mile1)

parameter_input <- prepare_input(
  data = data,
  selectivity = "LogisticSelectivity",
  recruitment_form = "BevertonHoltRecruitment",
  recruitment_distribution = "TMBDnormDistribution",
  growth = "EWAAgrowth",
  maturity = "LogisticMaturity"
)


updated_input <- update_input(
  parameter_input = parameter_input,
  LogisticMaturity.slope.value = 1.5,
  LogisticMaturity.slope.estimated = FALSE
)
