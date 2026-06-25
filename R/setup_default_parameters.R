# TODO: Document the names/items in each list that are returned

#' Set up default parameters for a FIMS model
#'
#' @description
#' This function generates a tibble with default parameters needed to run a
#' FIMS model from a [FIMSFrame()] input object. The returned tibble includes
#' default initial values and estimation settings for each configured module.
#' You can modify the returned tibble before fitting a model (for example,
#' updating maturity and selectivity parameter values).
#'
#' @details
#' The function builds module-specific defaults by calling helper
#' functions for data, fleet, selectivity, recruitment, maturity, growth, and
#' population components, then combines those defaults into one tibble.
#'
#' @param data A `FIMSFrame` object returned from running [FIMSFrame()] on
#'   your long input data.
#' @return
#' A `tibble` containing default parameter values and metadata for your model.
#' Key columns are listed below:
#' \describe{
#'   \item{\code{module_name}:}{The name of the FIMS module (e.g.,
#'     "Data", "Selectivity", "Recruitment", "Growth", "Maturity"). These
#'     entries are always written in PascalCase to match the names used in the
#'     C++ code.}
#'   \item{\code{fleet}:}{The name of the fleet the module applies to. This
#'     will be `NA` for non-fleet-specific modules like "Recruitment".}
#'   \item{\code{module_type}:}{The specific type of the module (e.g.,
#'     "Logistic" for a "Selectivity" module). This column will always be written
#'     in PascalCase to match the names used in the C++ code.}
#'   \item{\code{label}:}{The parameter name (e.g., "inflection_point").}
#'   \item{\code{age}:}{The age the parameter applies to.}
#'   \item{\code{length}:}{The length bin the parameter applies to.}
#'   \item{\code{time}:}{The time step (year) the parameter applies to.}
#'   \item{\code{value}:}{The initial value of the parameter.}
#'   \item{\code{estimation_type}:}{The estimation type (e.g., "constant",
#'     "fixed_effects", "random_effects").}
#'   \item{\code{distribution_type}:}{The type of distribution (e.g., "data",
#'     "process"), where a process distribution can refer to a fixed effect or a 
#'     random effect but it does not fit to data, e.g., recruitment deviation.}
#'   \item{\code{distribution}:}{The distribution name (e.g., "Dlnorm",
#'     "Dmultinom"). This column will always be written
#'     in PascalCase to match the names used in the C++ code.}
#' }
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
#' @seealso
#' * [FIMSFrame()]
#' @examples
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data_big")
#' fims_frame <- FIMSFrame(data_big)
#' 
#' # Set up default parameters
#' default_parameters <- setup_default_parameters(data = fims_frame)
#'
#' # Update selectivity parameters for survey1
#' updated_parameters <- default_parameters |>
#'   dplyr::rows_update(
#'     tibble::tibble(
#'       fleet = "survey1",
#'       label = c("inflection_point", "slope"),
#'       value = c(1.5, 2)
#'     ),
#'     by = c("fleet", "label")
#'   )
#'
#' # Do the same as above except, model fleet1 with double logistic selectivity
#' # To see required parameters for double logistic selectivity, run
#' # show(DoubleLogisticSelectivity) and look at the Fields list
#' parameters_with_double_logistic <- updated_parameters |>
#'   dplyr::filter(!(fleet == "fleet1" & module_name == "Selectivity")) |>
#'   dplyr::bind_rows(
#'     setup_default_selectivity(
#'       data = fims_frame,
#'       fleet = "fleet1",
#'       module_type = "DoubleLogistic"
#'     )
#'   )
#' }
setup_default_parameters <- function(
  data
) {
  assert_is_fims_frame(data)

  # Cache input data once to avoid repeated extraction in map() loops.
  model_data <- get_data(data)

  fleets <- model_data |>
    dplyr::pull(.data$fleet) |>
    na.omit() |>
    unique()

  data_index_landings_defaults <- purrr::map(
    fleets,
    function(fleet_i) {
      # Available module types (Index or Landings) for the current fleet
      module_types <- model_data |>
        dplyr::filter(
          .data$fleet == fleet_i, 
          .data$type %in% c("index", "landings")
        ) |>
        dplyr::pull(.data$type) |>
        unique() |>
        snake_to_pascal()
      
      # Set up defaults for each available module type in the current fleet.
      purrr::map(
        module_types,
        function(module_type_i) {
          setup_default_data_index_landings(
            data = data,
            fleet = fleet_i,
            module_type = module_type_i
          )
        }
      ) |>
        dplyr::bind_rows()
    }
  ) |>
    dplyr::bind_rows()

  data_composition_defaults <- purrr::map(
    fleets,
    function(fleet_i) {
      # Available module types (AgeComp) for the current fleet
      module_types <- model_data |>
        dplyr::filter(
          .data$fleet == fleet_i, 
          .data$type %in% c("age_comp", "length_comp")
        ) |>
        dplyr::pull(.data$type) |>
        unique() |>
        snake_to_pascal()
      
      # Set up defaults for each available module type in the current fleet.
      purrr::map(
        module_types,
        function(module_type_i) {
          setup_default_data_composition(
            data = data,
            fleet = fleet_i,
            module_type = module_type_i
          )
        }
      ) |>
        dplyr::bind_rows()
    }
  ) |>
    dplyr::bind_rows()

  # Create fleet parameters
  fleet_defaults <- purrr::map(
    fleets,
    function(fleet_i) {
      setup_default_fleet(
        data = data,
        fleet = fleet_i
      )
    }
  ) |>
    # bind_rows now directly takes the list of tibbles from map()
    dplyr::bind_rows()

  # Create selectivity parameters
  selectivity_defaults <- purrr::map(
    fleets,
    function(fleet_i) {
      setup_default_selectivity(
        data = data,
        fleet = fleet_i
      )
    }
  ) |>
    dplyr::bind_rows()

  # Create recruitment parameters
  recruitment_defaults <- setup_default_recruitment(data = data)

  # Create maturity parameters
  maturity_defaults <- setup_default_maturity(data = data)

  # Create growth parameters
  growth_defaults <- setup_default_growth()

  # Calculate initial numbers at age based on log_rzero and M_value
  # Set natural mortality rate
  log_M <- log(0.2)
  log_init_naa <- setup_default_log_init_naa(
    data = data,
    recruitment_defaults = recruitment_defaults,
    log_m = log_M
  )
  
  # Create population parameters
  population_defaults <- setup_default_Population(
    data = data,
    log_M = log_M,
    log_init_naa = log_init_naa
  )

  # Compile defaults
  default_parameters <- dplyr::bind_rows(
    data_index_landings_defaults,
    data_composition_defaults,
    selectivity_defaults,
    fleet_defaults,
    recruitment_defaults,
    maturity_defaults,
    growth_defaults,
    population_defaults
  )
}

#' Set up default initial numbers-at-age on the log scale
#'
#' @description
#' Helper used by [setup_default_parameters()] to compute `log_init_naa` from
#' recruitment defaults and a natural mortality value.
#' @param data A `FIMSFrame` object.
#' @param recruitment_defaults A tibble returned by [setup_default_recruitment()].
#' @param log_m A scalar natural-log mortality value.
#' @return A numeric vector of log initial numbers-at-age.
#' @noRd
setup_default_log_init_naa <- function(
  data,
  recruitment_defaults,
  log_m = log(0.2)
) {
  log_rzero <- recruitment_defaults |>
    dplyr::filter(.data[["label"]] == "log_rzero") |>
    dplyr::pull(.data[["value"]])

  init_naa <- exp(log_rzero) * exp(-(get_ages(data) - 1) * exp(log_m))
  n_ages <- get_n_ages(data)
  init_naa[n_ages] <- init_naa[n_ages] / exp(log_m)

  log(init_naa)
}

#' Set up default parameters for a FIMS model
#' @description
#' This function creates a template for default parameters used in a Fisheries
#' Integrated Modeling System (FIMS) model. The template includes fields for
#' module name, module type, label, fleet name, population name, age, length,
#' time, value, estimation type, distribution type, and distribution.
#' @param n_parameters An integer specifying the number of parameters in the
#' template.
#' @return
#' A tibble template for a FIMS model.
#' @noRd
#' @examples
#' FIMS:::setup_default_parameters_template(n_parameters = 3)
setup_default_parameters_template <- function(n_parameters = 1) {
  template <- tibble::tibble(
    module_name = NA_character_,
    fleet = NA_character_,
    module_type = NA_character_,
    label = NA_character_,
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

#' Set up default data (index or landings) for a FIMS model
#' 
#' @description
#' This function creates default data for a Fisheries Integrated Modeling 
#' System (FIMS) model, specifically for index or landings data. It generates 
#' a tibble with multiple fields.
#' @param data An S4 object representing the FIMS input data.
#' @param fleet A character string specifying the name of the fleet.
#' @param module_type A character string specifying the type of module, either "Index" or "Landings".
#' @param distribution A character string specifying the distribution type,
#' either "Dlnorm" or "Dnorm".
#' @return
#' A tibble containing default data for the specified fleet, including module
#' name, fleet name, module type, label, age, length, time, value,
#' estimation type, distribution type, and distribution.
#' @export 
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
#' @examples 
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data_big")
#' fims_frame <- FIMSFrame(data_big)
#' # Set up default data for a specific fleet
#' default_data <- setup_default_data_index_landings(
#'   data = fims_frame,
#'   fleet = "fleet1",
#'   distribution = "Dlnorm"
#' )
#' }   
setup_default_data_index_landings <- function(
  data, 
  fleet,
  module_type = c("Index", "Landings"),
  distribution = c("Dlnorm", "Dnorm")
) {
  assert_is_fims_frame(data)
  assert_single_fleet(fleet)

  # check module_type is a single character string and one of the allowed values
  module_type <- match.arg(module_type)

  # Ensure the selected module type is available for this fleet.
  assert_module_type_available(data, fleet, module_type)

  # check distribution is a single character string and one of the allowed values
  distribution <- match.arg(distribution)

  # extract index or landings for the specified fleet
  fleet_data <- get_data(data) |>
    dplyr::filter(
      .data$fleet == .env$fleet, 
      .data$type == tolower(.env$module_type)
    )

  # Extract uncertainty values for the specified fleet and module type.
  data_uncertainty <- fleet_data |>
    # TODO-Bai: check the reason for the code below
    dplyr::arrange(dplyr::desc(.data$type)) |>
    dplyr::pull(.data$uncertainty)
  
  distribution_default <- switch(distribution,
      "Dnorm" = setup_default_DnormDistribution(
        value = data_uncertainty,
        input_type = "data",
        data = data
      ),
      "Dlnorm" = setup_default_DlnormDistribution(
        value = data_uncertainty,
        input_type = "data",
        data = data
      )
    ) |>
      dplyr::mutate(
        module_name = "Data",
        module_type = .env$module_type,
        fleet = .env$fleet,
        time = fleet_data[["timing"]]
      )
}

#' Set up default data (composition) for a FIMS model
#' 
#' @description
#' This function creates default data for a Fisheries Integrated Modeling
#' System (FIMS) model, specifically for age or length composition data. It generates a
#' tibble with multiple fields.
#' @param data An S4 object representing the FIMS input data.
#' @param fleet A character string specifying the name of the fleet.
#' @param module_type A character string specifying the type of module, either 
#' "AgeComp" or "LengthComp".
#' @param distribution A character string specifying the distribution type,
#' either "Dmultinom" or "Ddirichlet".
#' @return
#' A tibble containing default data for the specified fleet, including module
#' name, fleet name, module type, label, age, length, time, value,
#' estimation type, distribution type, and distribution.
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
#' @examples
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data_big")
#' fims_frame <- FIMSFrame(data_big)
#' # Set up default data for a specific fleet
#' default_data_agecomp <- setup_default_data_composition(
#'   data = fims_frame,
#'   fleet = "fleet1",
#'   module_type = "AgeComp",
#'   distribution = "Dmultinom"
#' }
setup_default_data_composition <- function(
  data, 
  fleet,
  module_type = c("AgeComp", "LengthComp"),
  distribution = c("Dmultinom", "Ddirichlet")
) {
  assert_is_fims_frame(data)
  assert_single_fleet(fleet)
  
  # check module_type is a single character string and one of the allowed values
  module_type <- rlang::arg_match(module_type)

  # Ensure the selected module type is available for this fleet.
  assert_module_type_available(data, fleet, module_type)
  
  # check distribution is a single character string and one of the allowed values
  distribution <- rlang::arg_match(distribution)

  default <- setup_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      module_name = "Data",
      fleet = .env$fleet,
      module_type = .env$module_type,
      distribution_type = "data",
      distribution = .env$distribution
    )
}

#' Set up default growth parameters
#' 
#' @description
#' This function creates default growth parameters for a Fisheries Integrated 
#' Modeling System (FIMS) model. It generates a tibble with fields for 
#' module name, module type, label, value, and estimation type.
#' @param module_type A character string specifying the type of growth module. The
#' default is `"EWAA"`.
#' @return
#' A tibble containing default growth parameters, including module 
#' name, module type, label, value, and estimation type.
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
#' @examples
#' \dontrun{
#' # Set up default growth parameters
#' default_growth_parameters <- setup_default_growth()
#' }
setup_default_growth <- function(
  module_type = c("EWAA")
) {

  # Input check
  module_type <- rlang::arg_match(module_type)

  default <- setup_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      module_name = "Growth",
      module_type = .env$module_type
    )
}

#' Set up default population parameters
#'
#' @description
#' This function sets up default parameters for a population module.
#' @details
#' The function creates a tibble of default population configurations, including 
#' log natural mortality rate, log initial numbers at age, and proportion of
#' females. The function performs input checks to ensure that the provided 
#' arguments are valid.
#' @param data An S4 object. FIMS input data.
#' @param log_M A numeric value or vector (length equal to the number of ages *
#' number of years) specifying the log natural mortality rate. Default is `log(0.2)`.
#' @param log_init_naa A numeric value or vector (length equal to the number of 
#' ages) specifying the log initial numbers at age. Default is `NA_real_`.
#' @param proportion_female A numeric value between 0 and 1 specifying the
#' proportion of females. Default is `0.5`.
#' @inheritParams setup_default_parameters
#' @return
#' A tibble of default population configurations, including initial numbers at
#' age, natural mortality rate, and proportion of females.
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
setup_default_Population <- function(
  data,
  log_M = log(0.2),
  log_init_naa = NA_real_,
  proportion_female = 0.5
) {
  # Input checks
  assert_is_fims_frame(data)
  
  # Extract necessary values from data
  n_years <- get_n_years(data)
  n_ages <- get_n_ages(data)

  # Define age and year vectors needed for the rep() calls ===
  ages <- get_ages(data)
  years <- get_start_year(data):get_end_year(data)

  # Check log_M is a scalar or a full age-year vector.
  assert_numeric_length(
    x = log_M,
    x_name = "log_M",
    valid_lengths = c(1, n_ages * n_years),
    requirement = "a single numeric value or a vector of length equal to the number of ages * number of years"
  )

  # Check log_init_naa is a scalar or a full age vector.
  assert_numeric_length(
    x = log_init_naa,
    x_name = "log_init_naa",
    valid_lengths = c(1, n_ages),
    requirement = "a single numeric value or a vector of length equal to the number of ages"
  )

  # Check proportion_female is a single numeric value between 0 and 1
  if (!is.numeric(proportion_female) || length(proportion_female) != 1 || proportion_female < 0 || proportion_female > 1) {
    local_bullets <- c(
      "i" = "{.var proportion_female} argument must be a single numeric value between 0 and 1.",
      "x" = "{.var proportion_female} has a length of {length(proportion_female)}.",
      "x" = "{.var proportion_female} is of the class {class(proportion_female)}."
    )
    names(local_bullets)[2] <- ifelse(length(proportion_female) > 1, "x", "i")
    names(local_bullets)[3] <- ifelse(inherits(proportion_female, "numeric"), "i", "x")
    cli::cli_abort(local_bullets)
  }
  
  # Create a list of default parameters
  default <- setup_default_parameters_template(
    n_parameters = n_years * n_ages
  ) |>
    # Add the module type, label, value, and estimation type
    dplyr::mutate(
      label = "log_M",
      value = .env$log_M,
      age = rep(ages, n_years),
      time = rep(years, each = n_ages),
      estimation_type = "constant"
    ) |>
    dplyr::add_row(
      label = "log_init_naa",
      age = get_ages(data),
      value = .env$log_init_naa,
      estimation_type = "fixed_effects"
    ) |>
    dplyr::add_row(
      label = "proportion_female",
      value = .env$proportion_female,
      estimation_type = "constant"
    ) |>
    dplyr::mutate(
      module_name = "Population"
    )
  
  # If log_init_naa is NA, print a message to the user and advise them to set it
  # using dplyr::rows_update() or dplyr::mutate() before running the model
  if (any(is.na(log_init_naa))) {
    cli::cli_alert_info(c(
      "i" = "The {.var log_init_naa} parameter is set to {.val NA}.",
      "i" = "Please set the initial numbers at age using {.code dplyr::rows_update()} or {.code dplyr::mutate()} before running the model."
    ))
  }

  return(default)
}

#' Set up default logistic parameters
#'
#' @description
#' This function sets up default parameters for a logistic function. There are
#' two specified parameters, the inflection point and slope.
#' @return
#' A tibble containing the default logistic parameters, with inflection_point
#' and slope values and their estimation status.
#' @noRd
setup_default_Logistic <- function() {
  # Create a template for default parameters
  default <- setup_default_parameters_template(n_parameters = 2) |>
    # Add the module type, label, value, and estimation type
    dplyr::mutate(
      module_type = "Logistic",
      label = c("inflection_point", "slope"),
      value = c(2, 1),
      estimation_type = "fixed_effects"
    )
}

#' Set up default double logistic parameters
#'
#' @description
#' This function sets up default parameters for a double logistic function.
#' There are four specified parameters, two for the ascending and two for the
#' descending inflection points and slopes.
#' @return
#' A tibble containing the default double logistic parameters,
#' inflection_point_asc, slope_asc, inflection_point_desc, and slope_desc
#' values and their estimation status.
#' @noRd
setup_default_DoubleLogistic <- function() {
  default <- setup_default_parameters_template(n_parameters = 4) |>
    dplyr::mutate(
      module_type = "DoubleLogistic",
      label = c(
        "inflection_point_asc",
        "slope_asc",
        "inflection_point_desc",
        "slope_desc"
      ),
      # TODO: Determine if inflection_point_desc should really be 4?
      value = c(2, 1, 4, 1),
      estimation_type = "fixed_effects"
    )
}

#' Set up default selectivity parameters
#'
#' @description
#' This function sets up default parameters for a selectivity module.
#' @param fleet A string specifying the name of the fleet for which selectivity
#'   parameters are being created.
#' @param module_type A string specifying the desired form of selectivity. Allowable
#'   forms include `r toString(eval(formals(setup_default_selectivity)[["module_type"]]))`
#'   and the default is
#'   `r toString(eval(formals(setup_default_selectivity)[["module_type"]])[1])`.
#' @inherit setup_default_parameters 
#' @return A tibble containing the default selectivity parameters.
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
setup_default_selectivity <- function(
  data,
  fleet,
  module_type = c("Logistic", "DoubleLogistic")
) {
  # Input checks
  assert_is_fims_frame(data)
  assert_single_fleet(fleet)
  assert_fleet_in_data(data, fleet)

  module_type <- rlang::arg_match(module_type)
  # NOTE: All new forms of selectivity must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  default <- switch(module_type,
    "Logistic" = setup_default_Logistic(),
    "DoubleLogistic" = setup_default_DoubleLogistic()
  ) |>
    dplyr::mutate(
      module_name = "Selectivity",
      fleet = .env$fleet
    )
}

#' Set up default fleet parameters
#'
#' @description
#' This function sets up default parameters for a fleet module. It compiles
#' selectivity parameters along with distributions for each type of data that
#' are present for the given fleet.
#'
#' @param data An S4 object. FIMS input data.
#' @param fleet A character. Name of the fleet.
#' @inherit setup_default_parameters 
#' @return A tibble containing the default fleet parameters.
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
setup_default_fleet <- function(
  data,
  fleet
) {
  # Input checks
  assert_is_fims_frame(data)
  assert_single_fleet(fleet)
  assert_fleet_in_data(data, fleet)

  # Extract fleet's type
  fleet_types <- get_data(data) |>
    dplyr::filter(.data$fleet == .env$fleet) |>
    dplyr::pull(.data$type) |>
    unique()
  
  # Evaluate conditions upfront
  has_landings <- "landings" %in% fleet_types
  has_index <- "index" %in% fleet_types

  # Set up default parameters for catchability
  q_default <- setup_default_parameters_template(n_parameters = 1) |>
    dplyr::mutate(
      module_name = "Fleet",
      fleet = .env$fleet,
      label = "log_q",
      value = 0,
      estimation_type = if (has_landings) "constant" else "fixed_effects"
    )
  
  # Set up default parameters for fishing mortality
  log_Fmort_default <- setup_default_parameters_template(
      n_parameters = get_n_years(data)
    ) |>
      dplyr::mutate(
        module_name = "Fleet",
        fleet = .env$fleet,
        label = "log_Fmort",
        time = get_start_year(data):get_end_year(data),
        value = if (has_index) -200 else -3,
        estimation_type = if (has_index) "constant" else "fixed_effects"
      )
  # Compile all default parameters into a single list
  default <- dplyr::bind_rows(
    q_default,
    log_Fmort_default
  )
}

#' Set up default maturity parameters
#'
#' @description
#' This function sets up default parameters for a maturity module.
#' @param data An S4 object. FIMS input data.
#' @param module_type A string specifying the type of maturity module. The
#'   available options are `r toString(eval(formals(setup_default_maturity)[["module_type"]]))`.
#' @inherit setup_default_parameters return
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
setup_default_maturity <- function(
  data,
  module_type = c("Logistic")
) {
  # Input checks
  assert_is_fims_frame(data)
  module_type <- rlang::arg_match(module_type)

  # NOTE: All new module_type of maturity must be placed in the vector of default
  # arguments for `module_type` and their methods but be placed below in the call to
  # `switch`
  default <- switch(module_type,
    "Logistic" = setup_default_Logistic()
  ) |>
    # We don't have an option to input maturity data into FIMS, so the maturity
    # parameters aren't really estimable. The parameters should be constant for
    # now. See more details from
    # https://github.com/orgs/NOAA-FIMS/discussions/944.
    dplyr::mutate(
      estimation_type = "constant",
      module_name = "Maturity"
    )
}

#' Set up default Beverton--Holt recruitment parameters
#'
#' @description
#' This function sets up default parameters for a Beverton--Holt recruitment
#' relationship. Parameters include the natural log of unfished recruitment,
#' the logit transformation of the slope of the stock--recruitment curve to
#' keep it between zero and one, and the time series of stock--recruitment
#' deviations on the natural log scale.
#' @param data An S4 object. FIMS input data.
#' @param distribution A string specifying the distribution type for the stock--recruitment
#'   deviations. The default is `NA_character_`, which means that the stock--recruitment 
#'   deviations are not estimated.
#' @inherit setup_default_parameters
#' @return
#' A tibble containing default recruitment parameters.
#' @noRd
setup_default_BevertonHoltRecruitment <- function(
  data,
  distribution = NA_character_
) {
  assert_is_fims_frame(data)

  # Set up default parameters for Beverton--Holt recruitment
  log_rzero <- setup_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      label = "log_rzero",
      value = log(1e+06),
      estimation_type = "fixed_effects"
    )
  logit_steep <- setup_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      label = "logit_steep",
      value = -log(1.0 - 0.75) + log(0.75 - 0.2),
      estimation_type = "constant"
    )

  log_devs <- setup_default_parameters_template(
    n_parameters = get_n_years(data) - 1
  ) |>
    dplyr::mutate(
      # TODO: should this be log_recruit_dev to match output?
      label = "log_devs",
      value = 0.0,
      time = (get_start_year(data) + 1):get_end_year(data),
      estimation_type = "random_effects",
      distribution_type = "process",
      distribution = .env$distribution
    )
  if (is.na(distribution)) {
    log_devs <- log_devs |>
      dplyr::rows_update(
        tibble::tibble(
          label = "log_devs",
          time = (get_start_year(data) + 1):get_end_year(data),
          estimation_type = "constant",
          distribution_type = NA_character_
        ),
        by = c("label", "time")
      )
  }

  default <- dplyr::bind_rows(
    log_rzero,
    logit_steep,
    log_devs
  ) |>
    dplyr::mutate(
      module_name = "Recruitment",
      module_type = "BevertonHolt"
    )
}

#' Set up default DnormDistribution parameters
#'
#' @description
#' This function sets up default parameters to calculate the density of a
#' normal distribution, i.e., `DnormDistribution`, module.
#' @param value A real number that is passed to `log_sd`. The default value is
#'   `0.1`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the input type. The available options
#'   are
#'   `r toString(formals(setup_default_DnormDistribution)[["input_type"]])`.
#'   The default is
#'   `r toString(formals(setup_default_DnormDistribution)[["input_type"]][1])`.
#' @inherit setup_default_parameters 
#' @return A tibble containing the default DnormDistribution parameters.
#' @noRd
setup_default_DnormDistribution <- function(
  value = 0.1,
  data,
  input_type = c("data", "process", "prior")
) {
  # Input checks
  assert_is_fims_frame(data)
  input_type <- rlang::arg_match(input_type)

  # Set up default parameters
  default <- setup_default_parameters_template(
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
}

#' Set up default DlnormDistribution parameters
#'
#' @description
#' This function sets up default parameters to calculate the density of a
#' log-normal distribution, i.e., `DlnormDistribution`, module.
#' @param value Default value for `log_sd`.
#' @param data An S4 object. FIMS input data.
#' @param input_type A string specifying the input type. The available options
#'   are
#'   `r toString(formals(setup_default_DlnormDistribution)[["input_type"]])`.
#'   The default is
#'   `r toString(formals(setup_default_DlnormDistribution)[["input_type"]][1])`.
#' @inherit setup_default_parameters
#' @return A tibble containing the default DlnormDistribution parameters.
#' @noRd
setup_default_DlnormDistribution <- function(
  value = 0.1,
  data,
  input_type = c("data", "process", "prior")
) {
  # Input checks
  assert_is_fims_frame(data)
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
  default <- setup_default_parameters_template(
    n_parameters = get_n_years(data)
  ) |>
    # Add the module label and value
    dplyr::mutate(
      label = "log_sd",
      value = log_value
    )

  default <- default |>
    dplyr::mutate(
      estimation_type = "constant",
      distribution_type = input_type,
      distribution = "Dlnorm"
    )
}

#' Set up default recruitment parameters
#'
#' @description
#' This function sets up default parameters for a recruitment module.
#'
#' @param data An S4 object. FIMS input data.
#' @param module_type A string specifying the type of recruitment model. The
#'   available options are
#'   `r toString(eval(formals(setup_default_recruitment)[["module_type"]]))`. 
#' @param distribution A string specifying the distribution for the recruitment process.
#'   The available options are
#'   `r toString(eval(formals(setup_default_recruitment)[["distribution"]]))`. 
#' @inherit setup_default_parameters return
#' @export
#' @rdname setup_default_parameters
#' @keywords setup_default_parameters
setup_default_recruitment <- function(
  data,
  module_type = c("BevertonHolt"),
  distribution = c("Dnorm", NA_character_)
) {
  # Input checks
  # TODO: extend this code when there is more than one form of recruitment
  #       (i.e., multiple populations)
  assert_is_fims_frame(data)
  module_type <- rlang::arg_match(module_type)
  distribution <- rlang::arg_match(distribution)
  
  # Set up default parameters based on the recruitment form
  # NOTE: All new forms of recruitment must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  module_type_default <- switch(module_type,
    "BevertonHolt" = setup_default_BevertonHoltRecruitment(data, distribution)
  )

  # Keep an empty tibble with the same columns so bind_rows() always works.
  distribution_default <- module_type_default |>
    dplyr::slice(0)

  if (!is.na(distribution)) {
    distribution_default <- switch(distribution,
      "Dnorm" = setup_default_DnormDistribution(
        data = data,
        input_type = "process"
      )
    )
    distribution_default <- distribution_default |> 
      dplyr::rows_update(
        tibble::tibble(
          label = "log_sd",
          estimation_type = "fixed_effects"
        ),
        by = "label"
      )
  }

  default <- dplyr::bind_rows(module_type_default, distribution_default) |>
    tidyr::fill(dplyr::all_of(c("module_name", "module_type")))
}
