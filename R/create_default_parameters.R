# TODO: Document the names/items in each list that are returned
#' Create default parameters for a FIMS model
#'
#' @description
#' This function generates a Fisheries Integrated Modeling System (FIMS) model
#' configuration with detailed parameter specifications. This function takes a
#' high-level configuration `tibble` and generates the corresponding parameters
#' with default initial values and estimation settings required to build and run
#' the model.
#'
#' @details
#' The function processes the input `configurations` tibble, which defines the
#' modules for different model components (e.g., `"Selectivity"`, `"Recruitment"`).
#' For each module specified, it calls internal helper functions to create a
#' default set of parameters. For example, if a fleet's selectivity is configured
#' as `"Logistic"`, it generates initial values for `"inflection_point"` and
#' `"slope"`.
#'
#' @param configurations A tibble of model configurations. Typically created
#'   by [create_default_configurations()]. Users can modify this tibble
#'   to customize the model structure before generating default parameters.
#' @param data An S4 object. FIMS input data.
#' @return A `tibble` with default model parameters. The tibble has a nested
#'   structure with the following top-level columns.
#' \describe{
#'   \item{\code{model_family}:}{The specified model family (e.g.,
#'     "catch_at_age").}
#'   \item{\code{module_name}:}{The name of the FIMS module (e.g.,
#'     "Data", "Selectivity", "Recruitment", "Growth", "Maturity").}
#'   \item{\code{fleet_name}:}{The name of the fleet the module applies to. This
#'     will be `NA` for non-fleet-specific modules like "Recruitment".}
#'   \item{\code{data}:}{A list-column containing a `tibble` with detailed
#'     parameters. Unnesting this column reveals:
#'     \describe{
#'       \item{\code{module_type}:}{The specific type of the module (e.g.,
#'         "Logistic" for a "Selectivity" module).}
#'       \item{\code{label}:}{The name of the parameter (e.g., "inflection_point").}
#'       \item{\code{distribution_link}:}{The component the distribution module
#'         links to.}
#'       \item{\code{age}:}{The age the parameter applies to.}
#'       \item{\code{length}:}{The length bin the parameter applies to.}
#'       \item{\code{time}:}{The time step (i.e., year) the parameter applies to.}
#'       \item{\code{value}:}{The initial value of the parameter.}
#'       \item{\code{estimation_type}:}{The type of estimation (e.g.,
#'         "constant", "fixed_effects", "random_effects").}
#'       \item{\code{distribution_type}:}{The type of distribution (e.g., "Data",
#'         "process").}
#'       \item{\code{distribution}:}{The name of distribution (e.g.,
#'         "Dlnorm", `Dmultinom`).}
#'     }
#'   }
#' }
#' @export
#' @seealso
#' * [FIMSFrame()]
#' * [create_default_configurations()]
#' @examples
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data1")
#' fims_frame <- FIMSFrame(data1)
#'
#' # Create default configurations
#' default_configurations <- create_default_configurations(fims_frame)
#'
#' # Create default parameters
#' default_parameters <- create_default_parameters(
#'   configurations = default_configurations,
#'   data = fims_frame
#' ) |>
#'   tidyr::unnest(cols = data)
#'
#' # Update selectivity parameters for survey1
#' updated_parameters <- default_parameters |>
#'   dplyr::rows_update(
#'     tibble::tibble(
#'       fleet_name = "survey1",
#'       label = c("inflection_point", "slope"),
#'       value = c(1.5, 2)
#'     ),
#'     by = c("fleet_name", "label")
#'   )
#'
#' # Do the same as above except, model fleet1 with double logistic selectivity
#' # To see required parameters for double logistic selectivity, run
#' # show(DoubleLogisticSelectivity)
#' parameters_with_double_logistic <- default_configurations |>
#'   tidyr::unnest(cols = data) |>
#'   dplyr::rows_update(
#'     tibble::tibble(
#'       module_name = "Selectivity",
#'       fleet_name = "fleet1",
#'       module_type = "DoubleLogistic"
#'     ),
#'     by = c("module_name", "fleet_name")
#'   ) |>
#'   create_default_parameters(
#'     data = fims_frame
#'   )
#' }
create_default_parameters <- function(
  configurations,
  data
) {
  # FIXME: use default values if there are no fleets info passed into the
  # function or a fleet is not present but it has data? Maybe we don't want the
  # latter because it could be that we want to drop a fleet from a model but we
  # don't want to alter the data?

  # Check if configurations is a nested tibble. If so, unnest configurations
  if ("data" %in% names(configurations)) {
    unnested_configurations <- tidyr::unnest(configurations, cols = data)
  } else {
    unnested_configurations <- configurations
  }

  # Create fleet parameters
  fleet_names <- unnested_configurations |>
    dplyr::pull(fleet_name) |>
    na.omit() |>
    unique()
  fleet_temp <- purrr::map(
    fleet_names,
    function(fleet_name_i) {
      create_default_fleet(
        unnested_configurations = unnested_configurations,
        current_fleet_name = fleet_name_i,
        data = data
      )
    }
  ) |>
    # bind_rows now directly takes the list of tibbles from map()
    dplyr::bind_rows()

  # Create recruitment parameters
  recruitment_temp <- create_default_recruitment(
    unnested_configurations = unnested_configurations,
    data = data
  )

  # Create maturity parameters
  maturity_temp <- create_default_maturity(
    unnested_configurations = unnested_configurations,
    data = data
  )

  # Create population parameters
  # Handle population parameters based on recruitment form
  log_rzero <- recruitment_temp |>
    dplyr::filter(label == "log_rzero") |>
    dplyr::pull(value)

  population_temp <- create_default_Population(
    unnested_configurations = unnested_configurations,
    data,
    log_rzero = log_rzero
  )

  # Compile temps
  temp <- dplyr::bind_rows(
    fleet_temp,
    recruitment_temp,
    maturity_temp,
    population_temp
  )
  # Merge with configuration_unnest
  expanded_configurations <- dplyr::full_join(
    temp,
    unnested_configurations,
    by = c("module_name", "fleet_name", "module_type", "distribution_link")
  ) |>
    dplyr::mutate(
      model_family = dplyr::coalesce(model_family.y, model_family.x),
      distribution_type = dplyr::coalesce(distribution_type.y, distribution_type.x),
      distribution = dplyr::coalesce(distribution.y, distribution.x)
    ) |>
    dplyr::select(-dplyr::ends_with(c(".x", ".y"))) |>
    tidyr::fill(model_family) |>
    dplyr::select(
      model_family, module_name, module_type, label, distribution_link, dplyr::everything()
    ) |>
    tidyr::nest(.by = c(model_family, module_name, fleet_name))
}

#' Create default parameters for a FIMS model
#' @description
#' This function creates a template for default parameters used in a Fisheries
#' Integrated Modeling System (FIMS) model. The template includes fields for
#' module name, module type, label, fleet name, population name, age, length,
#' time, value, estimation type, distribution type, and distribution.
#' @param n_parameters An integer specifying the number of parameters in the template.
#' @return
#' A tibble template for a FIMS model.
#' @noRd
#' @examples
#' FIMS:::create_default_parameters_template(n_parameters = 3)
create_default_parameters_template <- function(n_parameters = 1) {
  template <- tibble::tibble(
    model_family = NA_character_,
    module_name = NA_character_,
    module_type = NA_character_,
    label = NA_character_,
    distribution_link = NA_character_,
    fleet_name = NA_character_,
    age = NA_real_,
    length = NA_real_,
    time = NA_integer_,
    value = NA_real_,
    estimation_type = NA_character_,
    distribution_type = NA_character_,
    distribution = NA_character_
  ) |>
    dplyr::slice(rep(1, each = n_parameters))
}

#' Create default population parameters
#'
#' @description
#' This function sets up default parameters for a population module.
#' @details
#' The natural log of the initial numbers at age (`log_init_naa.value`) is set based on
#' unexploited recruitment and natural mortality.
#' @param unnested_configurations A tibble of model configurations. Typically created
#'   by the `create_default_configurations()`.
#' @param data An S4 object. FIMS input data.
#' @param log_rzero A numeric value representing the natural log of unexploited
#'   recruitment.
#' @return
#' A tibble of default population parameters, including initial numbers at
#' age and natural mortality rate.
#' @noRd
create_default_Population <- function(
  unnested_configurations,
  data,
  log_rzero
) {
  # Input checks
  # Check if log_rzero is numeric
  if (!is.numeric(log_rzero) || length(log_rzero) != 1) {
    local_bullets <- c(
      "i" = "{.var log_rzero} argument must be a single numeric value.",
      "x" = "{.var log_rzero} has a length of {length(log_rzero)}.",
      "x" = "{.var log_rzero} is of the class {class(log_rzero)}."
    )
    names(local_bullets)[2] <- ifelse(length(log_rzero) > 1, "x", "i")
    names(local_bullets)[3] <- ifelse(inherits(log_rzero, "numeric"), "i", "x")
    cli::cli_abort(local_bullets)
  }

  # Extract necessary values from data
  n_years <- get_n_years(data)
  n_ages <- get_n_ages(data)

  # Define age and year vectors needed for the rep() calls ===
  ages <- get_ages(data)
  years <- get_start_year(data):get_end_year(data)

  # Set natural mortality rate
  M_value <- 0.2

  # Calculate initial numbers at age based on log_rzero and M_value
  init_naa <- exp(log_rzero) * exp(-(get_ages(data) - 1) * M_value)
  init_naa[n_ages] <- init_naa[n_ages] / M_value # sum of infinite series

  # Create a list of default parameters
  default <- create_default_parameters_template(
    n_parameters = n_years * n_ages
  ) |>
    # Add the module type, label, value, and estimation type
    dplyr::mutate(
      label = "log_M",
      value = log(M_value),
      age = rep(ages, n_years),
      time = rep(years, each = n_ages),
      estimation_type = "constant"
    ) |>
    dplyr::add_row(
      label = "log_init_naa",
      age = get_ages(data),
      value = log(init_naa),
      estimation_type = "fixed_effects"
    ) |>
    dplyr::mutate(
      module_name = "Population"
    )
}

#' Create default logistic parameters
#'
#' @description
#' This function sets up default parameters for a logistic function. There are
#' two specified parameters, the inflection point and slope.
#' @return
#' A tibble containing the default logistic parameters, with inflection_point and
#' slope values and their estimation status.
#' @noRd
create_default_Logistic <- function() {
  # Create a template for default parameters
  default <- create_default_parameters_template(n_parameters = 2) |>
    # Add the module type, label, value, and estimation type
    dplyr::mutate(
      module_type = "Logistic",
      label = c("inflection_point", "slope"),
      value = c(2, 1),
      estimation_type = "fixed_effects"
    )
}

#' Create default double logistic parameters
#'
#' @description
#' This function sets up default parameters for a double logistic function.
#' There four specified parameters, two for the ascending and two for the
#' descending inflection points and slopes.
#' @return
#' A tibble containing the default double logistic parameters,
#' inflection_point_asc, slope_asc, inflection_point_desc, and slope_desc
#' values and their estimation status.
#' @noRd
create_default_DoubleLogistic <- function(module_name = NA_character_) {
  default <- create_default_parameters_template(n_parameters = 4) |>
    dplyr::mutate(
      module_name = !!module_name,
      module_type = "DoubleLogistic",
      label = c("inflection_point_asc", "slope_asc", "inflection_point_desc", "slope_desc"),
      # TODO: Determine if inflection_point_desc should really be 4?
      value = c(2, 1, 4, 1),
      estimation_type = "fixed_effects"
    )
}

#' Create default selectivity parameters
#'
#' @description
#' This function sets up default parameters for a selectivity module.
#' @param form A string specifying the desired form of selectivity. Allowable
#'   forms include `r toString(formals(create_default_selectivity)[["form"]])`
#'   and the default is
#'   `r toString(formals(create_default_selectivity)[["form"]][1])`.
#' @return
#' A tibble is returned with the default parameter values for the specified form
#' of selectivity.
#' @noRd
create_default_selectivity <- function(
  form = c("Logistic", "DoubleLogistic")
) {
  # Input checks
  form <- rlang::arg_match(form)
  # NOTE: All new forms of selectivity must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  default <- switch(form,
    "Logistic" = create_default_Logistic(),
    "DoubleLogistic" = create_default_DoubleLogistic()
  ) |>
    dplyr::mutate(
      module_name = "Selectivity"
    )
}

#' Create default fleet parameters
#'
#' @description
#' This function sets up default parameters for a fleet module. It compiles
#' selectivity parameters along with distributions for each type of data that
#' are present for the given fleet.
#'
#' @param unnested_configurations A tibble of model configurations. Typically
#'   created by the `create_default_configurations()`.
#' @param fleet_name A character. Name of the fleet.
#' @param data An S4 object. FIMS input data.
#' @return
#' A tibble with default parameters for the fleet.
#' @noRd
create_default_fleet <- function(unnested_configurations,
                                 current_fleet_name,
                                 data) {
  # Input checks
  if (length(current_fleet_name) > 1) {
    cli::cli_abort(c(
      "i" = "{.var current_fleet_name} should have a length of 1.",
      "x" = "{.var current_fleet_name} has a length of {length(current_fleet_name)}."
    ))
  }
  if (!inherits(current_fleet_name, "character")) {
    cli::cli_abort(c(
      "i" = "{.var current_fleet_name} should be a string.",
      "x" = "{.var current_fleet_name} is a {class(current_fleet_name)}."
    ))
  }

  # Create default selectivity parameters
  selectivity_form <- unnested_configurations |>
    dplyr::filter(fleet_name == current_fleet_name & module_name == "Selectivity") |>
    dplyr::pull(module_type)

  selectivity_default <- create_default_selectivity(
    form = selectivity_form
  ) |>
    # Add fleet name
    dplyr::mutate(
      fleet_name = current_fleet_name
    )

  # Get types of data for this fleet from the data object
  data_types_present <- get_data(data) |>
    dplyr::filter(name == current_fleet_name) |>
    dplyr::pull(type) |>
    unique()

  # Get data likelihood distributions assigned for this fleet
  distribution_names_for_fleet <- unnested_configurations |>
    dplyr::filter(fleet_name == current_fleet_name & module_name == "Data") |>
    dplyr::pull(module_type)

  # Determine default fleet parameters based on types of data present
  if ("index" %in% data_types_present &&
    "Index" %in% distribution_names_for_fleet) {
    fleet_index <- get_data(data) |>
      dplyr::filter(type == "index" & name == current_fleet_name) |>
      dplyr::rename(time = timing)

    q_default <- create_default_parameters_template(n_parameters = 1) |>
      dplyr::mutate(
        module_name = "Fleet",
        label = "log_q",
        fleet_name = current_fleet_name,
        value = 0,
        estimation_type = "fixed_effects"
      )

    index_distribution <- unnested_configurations |>
      dplyr::filter(
        fleet_name == current_fleet_name & module_name == "Data" & module_type == "Index"
      ) |>
      dplyr::pull(distribution)

    index_uncertainty <- get_data(data) |>
      dplyr::filter(name == current_fleet_name, type %in% c("index")) |>
      dplyr::arrange(dplyr::desc(type)) |>
      dplyr::pull(uncertainty)

    index_distribution_default <- switch(index_distribution,
      "Dnorm" = create_default_DnormDistribution(
        value = index_uncertainty,
        input_type = "data",
        data = data
      ),
      "Dlnorm" = create_default_DlnormDistribution(
        value = index_uncertainty,
        input_type = "data",
        data = data
      )
    ) |>
      dplyr::mutate(
        module_name = "Data",
        module_type = "Index",
        distribution_link = "Index",
        fleet_name = current_fleet_name,
        time = fleet_index[["time"]]
      )
  } else {
    q_default <- create_default_parameters_template(n_parameters = 1) |>
      dplyr::mutate(
        module_name = "Fleet",
        label = "log_q",
        fleet_name = current_fleet_name,
        value = 0,
        estimation_type = "constant"
      )
    index_distribution_default <- NULL
  }

  if ("landings" %in% data_types_present &&
    "Landings" %in% distribution_names_for_fleet) {
    fleet_landings <- get_data(data) |>
      dplyr::filter(type == "landings" & name == current_fleet_name) |>
      dplyr::rename(time = timing)

    log_Fmort_default <- create_default_parameters_template(
      n_parameters = get_n_years(data)
    ) |>
      dplyr::mutate(
        module_name = "Fleet",
        label = "log_Fmort",
        fleet_name = current_fleet_name,
        time = get_start_year(data):get_end_year(data),
        value = -3,
        estimation_type = "fixed_effects"
      )

    landings_distribution <- unnested_configurations |>
      dplyr::filter(fleet_name == current_fleet_name & module_name == "Data" & module_type == "Landings") |>
      dplyr::pull(distribution)

    landings_uncertainty <- get_data(data) |>
      dplyr::filter(name == current_fleet_name, type %in% c("landings")) |>
      dplyr::arrange(dplyr::desc(type)) |>
      dplyr::pull(uncertainty)

    landings_distribution_default <- switch(landings_distribution,
      "Dnorm" = create_default_DnormDistribution(
        value = landings_uncertainty,
        input_type = "data",
        data = data
      ),
      "Dlnorm" = create_default_DlnormDistribution(
        value = landings_uncertainty,
        input_type = "data",
        data = data
      )
    ) |>
      dplyr::mutate(
        module_name = "Data",
        module_type = "Landings",
        distribution_link = "Landings",
        fleet_name = current_fleet_name,
        time = fleet_landings[["time"]]
      )
  } else {
    fleet_index <- get_data(data) |>
      dplyr::filter(type == "index" & name == current_fleet_name)

    log_Fmort_default <- create_default_parameters_template(
      n_parameters = get_n_years(data)
    ) |>
      dplyr::mutate(
        module_name = "Fleet",
        label = "log_Fmort",
        fleet_name = current_fleet_name,
        time = get_start_year(data):get_end_year(data),
        value = -200,
        estimation_type = "constant"
      )

    landings_distribution_default <- NULL
  }

  # Compile all default parameters into a single list
  default <- dplyr::bind_rows(
    selectivity_default,
    q_default,
    log_Fmort_default,
    index_distribution_default,
    landings_distribution_default
  )
}

#' Create default maturity parameters
#'
#' @description
#' This function sets up default parameters for a maturity module.
#' @param form A string specifying the form of maturity (e.g.,
#' `"Logistic"`).
#' @return
#' A tibble containing the default maturity parameters.
#' @noRd
create_default_maturity <- function(
  unnested_configurations,
  data
) {
  # Input checks
  available_forms <- c("Logistic")
  form <- unnested_configurations |>
    dplyr::filter(module_name == "Maturity") |>
    dplyr::pull(module_type)
  if (!form %in% available_forms) {
    cli::cli_abort(c(
      "Invalid `module_type`` for Maturity: {.var {form}}",
      "i" = "Valid options include: {.var {available_forms}}"
    ))
  }

  # NOTE: All new forms of maturity must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  default <- switch(form,
    "Logistic" = create_default_Logistic()
  ) |>
    # We don't have an option to input maturity data into FIMS, so the maturity
    # parameters aren't really estimable. The parameters should be constant for now.
    # See more details from https://github.com/orgs/NOAA-FIMS/discussions/944.
    dplyr::mutate(
      estimation_type = "constant",
      module_name = "Maturity"
    )
}

#' Create default Beverton--Holt recruitment parameters
#'
#' @description
#' This function sets up default parameters for a Beverton--Holt recruitment
#' relationship. Parameters include the natural log of unfished recruitment,
#' the logit transformation of the slope of the stock--recruitment curve to
#' keep it between zero and one, and the time series of stock--recruitment
#' deviations on the natural log scale.
#' @param data An S4 object. FIMS input data.
#' @return
#' A tibble containing default recruitment parameters.
#' @noRd
create_default_BevertonHoltRecruitment <- function(data) {
  # Create default parameters for Beverton--Holt recruitment
  log_rzero <- create_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      label = "log_rzero",
      value = log(1e+06),
      estimation_type = "fixed_effects"
    )
  logit_steep <- create_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      label = "logit_steep",
      value = -log(1.0 - 0.75) + log(0.75 - 0.2),
      estimation_type = "constant"
    )

  # TODO: Revisit the settings for log_r. Do we must set up log_r when
  # it is not random effect parameters?
  log_r <- create_default_parameters_template(
    n_parameters = get_n_years(data) - 1
  ) |>
    dplyr::mutate(
      # TODO: should this be LogRecDev to match output?
      label = "log_r",
      value = 0.0,
      time = (get_start_year(data) + 1):get_end_year(data),
      estimation_type = "constant"
    )

  log_devs <- create_default_parameters_template(
    n_parameters = get_n_years(data) - 1
  ) |>
    dplyr::mutate(
      # TODO: should this be LogRecDev to match output?
      label = "log_devs",
      value = 0.0,
      time = (get_start_year(data) + 1):get_end_year(data),
      estimation_type = "fixed_effects"
    )

  expected_recruitment <- create_default_parameters_template(
    n_parameters = get_n_years(data) + 1
  ) |>
    dplyr::mutate(
      label = "log_expected_recruitment",
      value = 0.0,
      estimation_type = "constant"
    )

  default <- dplyr::bind_rows(
    log_rzero,
    logit_steep,
    log_r,
    log_devs,
    expected_recruitment
  ) |>
    dplyr::mutate(
      module_name = "Recruitment",
      module_type = "BevertonHolt"
    )
}

#' Create default DnormDistribution parameters
#'
#' @description
#' This function sets up default parameters to calculate the density of a
#' normal distribution, i.e., `DnormDistribution`, module.
#' @param value A real number that is passed to `log_sd`. The default value is
#'   `0.1`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the input type. The available options
#'   are
#'   `r toString(formals(create_default_DnormDistribution)[["input_type"]])`.
#'   The default is
#'   `r toString(formals(create_default_DnormDistribution)[["input_type"]][1])`.
#' @return
#' A tibble of default parameters for Dnorm distribution.
#' @noRd
create_default_DnormDistribution <- function(
  value = 0.1,
  data,
  input_type = c("data", "process", "prior")
) {
  # Input checks
  input_type <- rlang::arg_match(input_type)

  # Create default parameters
  default <- create_default_parameters_template(
    n_parameters = length(value)
  ) |>
    # Add the module type and label
    dplyr::mutate(
      label = "log_sd",
      value = !!value,
      estimation_type = "constant",
      distribution_type = input_type,
      distribution = "Dnorm"
    )

  # If input_type is 'process', add additional parameters
  if (input_type == "process" | input_type == "prior") {
    new_params <- create_default_parameters_template(
      n_parameters = length(value)
    ) |>
      dplyr::mutate(label = "x", value = 0) |>
      dplyr::add_row(
        label = "expected_values",
        value = rep(0, length(value))
      ) |>
      dplyr::mutate(
        estimation_type = "constant",
        distribution_type = input_type,
        distribution = "Dnorm"
      )

    default <- dplyr::bind_rows(
      default,
      new_params
    )
  }
}

#' Create default DlnormDistribution parameters
#'
#' @description
#' This function sets up default parameters to calculate the density of a
#' log-normal distribution, i.e., `DlnormDistribution`, module.
#' @param value Default value for `log_sd`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the input type. The available options
#'   are
#'   `r toString(formals(create_default_DlnormDistribution)[["input_type"]])`.
#'   The default is
#'   `r toString(formals(create_default_DlnormDistribution)[["input_type"]][1])`.
#' @return
#' A tibble of default parameters for Dlnorm distribution.
#' @noRd
create_default_DlnormDistribution <- function(
  value = 0.1,
  data,
  input_type = c("data", "process")
) {
  # Input checks
  # TODO: Determine if value can be a vector?
  if (!is.numeric(value) || any(value <= 0, na.rm = TRUE)) {
    cli::cli_abort(c(
      "i" = "Inputs to {.var value} must be positive and numeric.",
      "x" = "{.var value} is {.var {value}}."
    ))
  }
  input_type <- rlang::arg_match(input_type)

  log_value <- log(value)
  # Create the default list with log standard deviation
  default <- create_default_parameters_template(
    n_parameters = get_n_years(data)
  ) |>
    # Add the module label and value
    dplyr::mutate(
      label = "log_sd",
      value = log_value
    )

  # Add additional parameters if input_type is "process"
  if (input_type == "process") {
    default <- default |>
      dplyr::add_row(
        label = "x",
        value = rep(0, get_n_years(data))
      )
  }

  default <- default |>
    dplyr::mutate(
      estimation_type = "constant",
      distribution_type = input_type,
      distribution = "Dlnorm"
    )
  return(default)
}

#' Create default recruitment parameters
#'
#' @description
#' This function sets up default parameters for a recruitment module.
#'
#' @param unnested_configurations A tibble of model configurations. Typically
#'   created by the `create_default_configurations()`.
#' @param data An S4 object. FIMS input data.
#' @return
#' A tibble with the default parameters for recruitment.
#' @noRd
create_default_recruitment <- function(
  unnested_configurations,
  data
) {
  # Input checks
  available_forms <- c("BevertonHolt")
  form <- unnested_configurations |>
    dplyr::filter(module_name == "Recruitment") |>
    dplyr::pull(module_type)
  if (!form %in% available_forms) {
    cli::cli_abort(c(
      "Invalid `module_type` for Recruitment: {.var {form}}",
      "i" = "Valid options include: {.var {available_forms}}"
    ))
  }
  # Create default parameters based on the recruitment form
  # NOTE: All new forms of recruitment must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  form_default <- switch(form,
    "BevertonHolt" = create_default_BevertonHoltRecruitment(data)
  )

  distribution_input <- unnested_configurations |>
    dplyr::filter(module_name == "Recruitment")

  if (!is.null(distribution_input[["distribution"]])) {
    distribution_default <- switch(distribution_input[["distribution"]],
      "Dnorm" = create_default_DnormDistribution(
        data = data,
        input_type = "process"
      )
    )

    distribution_link <- distribution_input[["distribution_link"]]
    if (distribution_link == "log_devs") {
      distribution_default <- distribution_default |>
        dplyr::mutate(
          distribution_link = !!distribution_link
        )

      expanded_rows <- distribution_default |>
        dplyr::filter(label %in% c("x", "expected_values")) |>
        # Create all combinations of the original rows and years
        tidyr::expand_grid(year = (get_start_year(data) + 1):get_end_year(data)) |>
        dplyr::mutate(
          time = year
        ) |>
        dplyr::select(-year)

      distribution_default <- distribution_default |>
        dplyr::filter(label == "log_sd") |>
        dplyr::bind_rows(expanded_rows)
    }
  }

  default <- dplyr::bind_rows(form_default, distribution_default) |>
    tidyr::fill(module_name, module_type)
}
