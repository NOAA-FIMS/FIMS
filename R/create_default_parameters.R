# TODO: Document the names/items in each list that are returned

#' Create default parameters for a FIMS model
#'
#' @description
#' This function generates a tibble with all of the parameters necessary to run
#' a FIMS model given the desired high-level configuration that is specified in
#' `configurations`. The tibble contains default initial values and estimation
#' settings required to build and run the model. You can edit the returned
#' tibble if you want to changes things such as initial values to values more
#' specific to your population before running your model. For example, the
#' default maturity parameters will need modified.
#'
#' @details
#' The function processes the `configurations` tibble, which only contains
#' high-level information for running your model by calling internal helper
#' functions on each row and returning a multi-row parameter set for each input
#' row. For example, if a selectivity for the first fleet is configured as
#' `"Logistic"`, it takes that single row of input information and returns a
#' parameter set with two rows, one for each parameter, `"inflection_point"`
#' and `"slope"`.
#'
#' @param configurations A tibble of model configurations. Typically created
#'   by [create_default_configurations()]. Users can modify this tibble
#'   to customize the model structure before using it as input to this function.
#' @param data A `FIMSFrame` object returned from running [FIMSFrame()] on
#'   your long input data.
#' @return
#' A nested `tibble` containing information on parameters for your model with
#' the same top-level columns as the input tibble passed to `configurations`
#' but with additional information in the nested `data` column. See below for
#' more details:
#' \describe{
#'   \item{\code{model_family}:}{The specified model family (e.g.,
#'     "catch_at_age").}
#'   \item{\code{module_name}:}{The name of the FIMS module (e.g.,
#'     "Data", "Selectivity", "Recruitment", "Growth", "Maturity"). These
#'     entries are always written in PascalCase to match the names used in the
#'     C++ code.}
#'   \item{\code{fleet_name}:}{The name of the fleet the module applies to. This
#'     will be `NA` for non-fleet-specific modules like "Recruitment".}
#'   \item{\code{data}:}{A list-column containing a `tibble` with detailed
#'     parameters. Unnesting this column reveals:
#'     \describe{
#'       \item{\code{module_type}:}{The specific type of the module (e.g.,
#'         "Logistic" for a "Selectivity" module). This column will always be
#'         written in PascalCase to match the names used in the C++ code.}
#'       \item{\code{label}:}{The name of the parameter (e.g.,
#'         "inflection_point").}
#'       \item{\code{age}:}{The age the parameter applies to.}
#'       \item{\code{length}:}{The length bin the parameter applies to.}
#'       \item{\code{time}:}{The time step (i.e., year) the parameter applies
#'         to.}
#'       \item{\code{value}:}{The initial value of the parameter.}
#'       \item{\code{estimation_type}:}{The type of estimation (e.g.,
#'         "constant", "fixed_effects", "random_effects").}
#'       \item{\code{distribution_type}:}{The type of distribution (e.g.,
#'         "Data", "process"), where a process distribution can refer to a
#'         fixed effect or a random effect but it does not fit to data, e.g.,
#'         recruitment deviations.}
#'       \item{\code{distribution}:}{The name of distribution (e.g.,
#'         "Dlnorm", `Dmultinom`). The column will always be written in
#'         PascalCase to match the names used in the C++ code.}
#'     }
#'   }
#' }
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
#' @seealso
#' * [FIMSFrame()]
#' * [create_default_configurations()]
#' @examples
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data_big")
#' fims_frame <- FIMSFrame(data_big)
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
#' # show(DoubleLogisticSelectivity) and look at the Fields list
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
    dplyr::pull(.data$fleet_name) |>
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
    data = data
  )

  # Create maturity parameters
  maturity_temp <- create_default_maturity(data = data)

  # Create population parameters
  # Handle population parameters based on recruitment form
  log_rzero <- recruitment_temp |>
    dplyr::filter(.data$label == "log_rzero") |>
    dplyr::pull(.data$value)

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
    by = c("module_name", "fleet_name", "module_type")
  ) |>
    dplyr::mutate(
      model_family = dplyr::coalesce(.data$model_family.y, .data$model_family.x),
      distribution_type = dplyr::coalesce(
        .data$distribution_type.y,
        .data$distribution_type.x
      ),
      distribution = dplyr::coalesce(.data$distribution.y, .data$distribution.x),
      distribution_type = dplyr::if_else(
        .data$module_name == "Recruitment" & !.data$label %in% c("log_devs", "log_r", "log_sd"),
        NA_character_,
        .data$distribution_type
      ),
      distribution = dplyr::if_else(
        .data$module_name == "Recruitment" & !.data$label %in% c("log_devs", "log_r", "log_sd"),
        NA_character_,
        .data$distribution
      )
    ) |>
    dplyr::select(-dplyr::ends_with(c(".x", ".y"))) |>
    tidyr::fill(dplyr::all_of("model_family"), .direction = "downup") |>
    dplyr::select(
      dplyr::all_of(c("model_family", "module_name", "module_type")),
      dplyr::everything()
    ) |>
    dplyr::filter(
      !(is.na(.data$label) & is.na(.data$distribution_type) & is.na(.data$distribution) & .data$module_name != "Growth")
    ) |>
    tidyr::nest(.by = c("model_family", "module_name", "fleet_name"))
}

#' Create default parameters for a FIMS model
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
#' FIMS:::create_default_parameters_template(n_parameters = 3)
create_default_parameters_template <- function(n_parameters = 1) {
  template <- tibble::tibble(
    model_family = NA_character_,
    module_name = NA_character_,
    module_type = NA_character_,
    label = NA_character_,
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

#' Create default data (index or landings) for a FIMS model
#' 
#' @description
#' This function creates default data for a Fisheries Integrated Modeling 
#' System (FIMS) model, specifically for index or landings data. It generates 
#' a tibble with multiple fields.
#' @param data An S4 object representing the FIMS input data.
#' @param fleet_name A character string specifying the name of the fleet.
#' @param module_type A character string specifying the type of module, either "Index" or "Landings".
#' @param distribution A character string specifying the distribution type,
#' either "Dlnorm" or "Dnorm".
#' @param model_family A character string specifying the model family,
#' currently only "catch_at_age" is supported.
#' @return
#' A tibble containing default data for the specified fleet, including module
#' family, module name, fleet name, module type, label, age, length, time, value,
#' estimation type, distribution type, and distribution.
#' @export 
#' @rdname create_default_parameters
#' @keywords create_default_parameters
#' @examples 
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data_big")
#' fims_frame <- FIMSFrame(data_big)
#' # Create default data for a specific fleet
#' default_data <- create_default_data_index_landings(
#'   data = fims_frame,
#'   fleet_name = "fleet1",
#'   distribution = "Dlnorm",
#'   model_family = "catch_at_age"
#' ) 
#' }   
create_default_data_index_landings <- function(
  data, 
  fleet_name,
  module_type = c("Index", "Landings"),
  distribution = c("Dlnorm", "Dnorm"),
  model_family = c("catch_at_age")
) {
  # check fleet_name is a single character string
  if (!is.character(fleet_name) || length(fleet_name) != 1) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} must be a single character string.",
      "x" = "{.var fleet_name} is of class {.cls {class(fleet_name)}} and has length {length(fleet_name)}."
    ))
  }

  # check module_type is a single character string and one of the allowed values
  module_type <- match.arg(module_type)

  # check if the specified fleet has index or landings data in the input data
  available_types <- get_data(data) |>
    dplyr::filter(name == fleet_name) |>
    dplyr::pull(type) |>
    unique() |>
    snake_to_pascal()

  if (!module_type %in% available_types) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} does not have {.var module_type} data in the input data.",
      "x" = "Please check the input data or specify a different {.var fleet_name} or {.var module_type}."
    ))
  }

  # check distribution is a single character string and one of the allowed values
  distribution <- match.arg(distribution)

  # check model_family is a single character string and one of the allowed values
  model_family <- match.arg(model_family)

  # extract index or landings for the specified fleet
  fleet_data <- get_data(data) |>
    dplyr::filter(name == fleet_name, type == module_type)

  # extract uncertainty for the specified fleet
  data_uncertainty <- fleet_data |>
    # TODO-Bai: check the reason for the code below
    dplyr::arrange(dplyr::desc(type)) |>
    dplyr::pull(uncertainty)
  
  distribution_default <- switch(distribution,
      "Dnorm" = create_default_DnormDistribution(
        value = data_uncertainty,
        input_type = "data",
        data = data
      ),
      "Dlnorm" = create_default_DlnormDistribution(
        value = data_uncertainty,
        input_type = "data",
        data = data
      )
    ) |>
      dplyr::mutate(
        module_name = "Data",
        module_type = module_type,
        fleet_name = fleet_name,
        time = fleet_data[["timing"]],
        model_family = model_family
      )
}

#' Create default data (composition) for a FIMS model
#' 
#' @description
#' This function creates default data for a Fisheries Integrated Modeling
#' System (FIMS) model, specifically for age or length composition data. It generates a
#' tibble with multiple fields.
#' @param data An S4 object representing the FIMS input data.
#' @param fleet_name A character string specifying the name of the fleet.
#' @param module_type A character string specifying the type of module, either 
#' "AgeComp" or "LengthComp".
#' @param distribution A character string specifying the distribution type,
#' either "Dmultinom" or "Ddirichlet".
#' @param model_family A character string specifying the model family,
#' currently only "catch_at_age" is supported.
#' @return
#' A tibble containing default data for the specified fleet, including module
#' family, module name, fleet name, module type, label, age, length, time, value,
#' estimation type, distribution type, and distribution.
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
#' @examples
#' \dontrun{
#' # Load the example dataset and create a FIMS data frame
#' data("data_big")
#' fims_frame <- FIMSFrame(data_big)
#' # Create default data for a specific fleet
#' default_data_agecomp <- create_default_data_agecomp(
#'   data = fims_frame,
#'   fleet_name = "fleet1",
#'   module_type = "AgeComp",
#'   distribution = "Dmultinom",
#'   model_family = "catch_at_age"
#' )
create_default_data_composition <- function(
  data, 
  fleet_name,
  module_type = c("AgeComp", "LengthComp"),
  distribution = c("Dmultinom", "Ddirichlet"),
  model_family = c("catch_at_age")
) {
  # check fleet_name is a single character string
  if (!is.character(fleet_name) || length(fleet_name) != 1) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} must be a single character string.",
      "x" = "{.var fleet_name} is of class {.cls {class(fleet_name)}} and has length {length(fleet_name)}."
    ))
  }
  
  # check module_type is a single character string and one of the allowed values
  module_type <- match.arg(module_type)

  # check if the specified fleet has specified composition data in the input data
  available_types <- get_data(data) |>
    dplyr::filter(name == fleet_name) |>
    dplyr::pull(type) |>
    unique() |>
    snake_to_pascal()

  if (!module_type %in% available_types) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} does not have {.var module_type} data in the input data.",
      "x" = "Please check the input data or specify a different {.var fleet_name} or {.var module_type}."
    ))
  }
  
  # check distribution is a single character string and one of the allowed values
  distribution <- match.arg(distribution)

  # check model_family is a single character string and one of the allowed values
  model_family <- match.arg(model_family)

  default <- create_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      model_family = model_family,
      module_name = "Data",
      fleet_name = fleet_name,
      module_type = module_type,
      value = distribution,
      distribution_type = "Data",
      distribution = distribution
    )
}

#' Create default growth parameters
#' 
#' @description
#' This function creates default growth parameters for a Fisheries Integrated 
#' Modeling System (FIMS) model. It generates a tibble with fields for 
#' model family, module name, module type, label, value, and estimation type.
#' @return
#' A tibble containing default growth parameters, including model family, module 
#' name, module type, label, value, and estimation type.
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
#' @examples
#' \dontrun{
#' # Create default growth parameters
#' default_growth_parameters <- create_default_growth()
#' }
create_default_growth <- function() {
  default <- create_default_parameters_template(
    n_parameters = 1
  ) |>
    dplyr::mutate(
      model_name = "catch_at_age",
      module_name = "Growth",
      module_type = "EWAA"
    )
  class(default) <- c("fims_parameters", class(default))
  return(default)
}

#' Create default population parameters
#'
#' @description
#' This function sets up default parameters for a population module.
#' @details
#' The natural log of the initial numbers at age (`log_init_naa.value`) is set
#' based on unexploited recruitment and natural mortality.
#' @param unnested_configurations A tibble of model configurations. Typically
#'   created by the `create_default_configurations()`.
#' @param data An S4 object. FIMS input data.
#' @param log_rzero A numeric value representing the natural log of unexploited
#'   recruitment.
#' @inheritParams create_default_parameters
#' @return
#' A tibble of default population parameters, including initial numbers at
#' age and natural mortality rate.
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
create_default_Population <- function(
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
    dplyr::add_row(
      label = "proportion_female",
      value = 0.5,
      estimation_type = "constant"
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
#' A tibble containing the default logistic parameters, with inflection_point
#' and slope values and their estimation status.
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

#' Create default selectivity parameters
#'
#' @description
#' This function sets up default parameters for a selectivity module.
#' @param fleet_name A string specifying the name of the fleet for which selectivity
#'   parameters are being created.
#' @param module_type A string specifying the desired form of selectivity. Allowable
#'   forms include `r toString(eval(formals(create_default_selectivity)[["module_type"]]))`
#'   and the default is
#'   `r toString(eval(formals(create_default_selectivity)[["module_type"]])[1])`.
#' @inherit create_default_paramters return
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
create_default_selectivity <- function(
  data,
  fleet_name,
  module_type = c("Logistic", "DoubleLogistic"),
  model_family = c("catch_at_age")
) {
  # Input checks
  if (!inherits(data, "FIMSFrame")) {
    cli::cli_abort(c(
      "i" = "{.var data} should be a {.cls FIMSFrame} object.",
      "x" = "{.var data} is a {.cls {class(data)}} object."
    ))
  }

  if (!is.character(fleet_name) || length(fleet_name) != 1) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} must be a single character string.",
      "x" = "{.var fleet_name} is of class {.cls {class(fleet_name)}} and has length {length(fleet_name)}."
    ))
  }
  
  fleet_names <- data@data |> 
    dplyr::pull(name) |>
    unique() |>
    na.omit()
  if (!fleet_name %in% fleet_names) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} is not present in the {.var data}.",
      "x" = "Available fleet names are: {.val {fleet_names}}."
    ))
  }

  module_type <- rlang::arg_match(module_type)
  model_family <- rlang::arg_match(model_family)
  # NOTE: All new forms of selectivity must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  default <- switch(module_type,
    "Logistic" = create_default_Logistic(),
    "DoubleLogistic" = create_default_DoubleLogistic()
  ) |>
    dplyr::mutate(
      module_name = "Selectivity",
      fleet_name = .env$fleet_name,
      model_family = .env$model_family
    )
  class(default) <- c("fims_parameters", class(default))
  return(default)
}

#' Create default fleet parameters
#'
#' @description
#' This function sets up default parameters for a fleet module. It compiles
#' selectivity parameters along with distributions for each type of data that
#' are present for the given fleet.
#'
#' @param data An S4 object. FIMS input data.
#' @param fleet_name A character. Name of the fleet.
#' @param model_family A string specifying the model family. The available 
#' options are `r toString(eval(formals(create_default_fleet)[["model_family"]]))`.
#' @inherit create_default_paramters return
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
create_default_fleet <- function(
  data,
  fleet_name,
  model_family = c("catch_at_age")
) {
  # Input checks
  if (!inherits(data, "FIMSFrame")) {
    cli::cli_abort(c(
      "i" = "{.var data} should be a {.cls FIMSFrame} object.",
      "x" = "{.var data} is a {.cls {class(data)}} object."
    ))
  }

  if (length(fleet_name) > 1) {
    cli::cli_abort(c(
      "i" = "{.var fleet_name} should have a length of 1.",
      "x" = "{.var fleet_name} has a length of {length(fleet_name)}."
    ))
  }

  if (!inherits(fleet_name, "character")) {
    cli::cli_abort(c(
      "i" = "{.var current_fleet_name} should be a string.",
      "x" = "{.var current_fleet_name} is a {class(current_fleet_name)}."
    ))
  }

  model_family <- rlang::arg_match(model_family)

  # Extract fleet's type
  fleet_types <- get_data(data) |>
    dplyr::filter(.data$name == .env$fleet_name) |>
    dplyr::pull(.data$type) |>
    unique()
  
  # Evaluate conditions upfront
  has_landings <- "landings" %in% fleet_types
  has_index <- "index" %in% fleet_types

  # Create default paramters for catchability
  q_default <- create_default_parameters_template(n_parameters = 1) |>
    dplyr::mutate(
      model_family = .env$model_family,
      module_name = "Fleet",
      fleet_name = .env$fleet_name,
      label = "log_q",
      value = 0,
      estimation_type = if (has_landings) "constant" else "fixed_effects"
    )
  
  # Create default parameters for fishing mortality
  log_Fmort_default <- create_default_parameters_template(
      n_parameters = get_n_years(data)
    ) |>
      dplyr::mutate(
        model_family = .env$model_family,
        module_name = "Fleet",
        fleet_name = .env$fleet_name,
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
  class(default) <- c("fims_parameters", class(default))
  return(default)

}

#' Create default maturity parameters
#'
#' @description
#' This function sets up default parameters for a maturity module.
#' @param data An S4 object. FIMS input data.
#' @param module_type A string specifying the type of maturity module. The
#'   available options are `r toString(eval(formals(create_default_maturity)[["module_type"]]))`.
#' @param model_family A string specifying the model family. The default is
#'   "catch_at_age".
#' @inherit create_default_paramters return
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
create_default_maturity <- function(
  data,
  module_type = c("Logistic"),
  model_family = c("catch_at_age")
) {
  # Input checks
  if (!inherits(data, "FIMSFrame")) {
    cli::cli_abort(c(
      "i" = "{.var data} must be a FIMSFrame object.",
      "x" = "{.var data} is of class {.cls {class(data)}}."
    ))
  }
  module_type <- rlang::arg_match(module_type)
  model_family <- rlang::arg_match(model_family)

  # NOTE: All new module_type of maturity must be placed in the vector of default
  # arguments for `module_type` and their methods but be placed below in the call to
  # `switch`
  default <- switch(module_type,
    "Logistic" = create_default_Logistic()
  ) |>
    # We don't have an option to input maturity data into FIMS, so the maturity
    # parameters aren't really estimable. The parameters should be constant for
    # now. See more details from
    # https://github.com/orgs/NOAA-FIMS/discussions/944.
    dplyr::mutate(
      estimation_type = "constant",
      module_name = "Maturity",
      model_family = .env$model_family
    )
  class(default) <- c("fims_parameters", class(default))
  return(default)
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
#' @param distribution A string specifying the distribution type for the stock--recruitment
#'   deviations. The default is `NA_character_`, which means that the stock--recruitment 
#'   deviations are not estimated.
#' @inherit create_default_paramters return.
#' @noRd
create_default_BevertonHoltRecruitment <- function(
  data,
  distribution = NA_character_
) {
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

  log_devs <- create_default_parameters_template(
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
          distribution_type = NA_character_,
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
#' @inherit create_default_paramters return
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
#' @inherit create_default_paramters return
#' @noRd
create_default_DlnormDistribution <- function(
  value = 0.1,
  data,
  input_type = c("data", "process", "prior")
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

  default <- default |>
    dplyr::mutate(
      estimation_type = "constant",
      distribution_type = input_type,
      distribution = "Dlnorm"
    )
}

#' Create default recruitment parameters
#'
#' @description
#' This function sets up default parameters for a recruitment module.
#'
#' @param data An S4 object. FIMS input data.
#' @param module_type A string specifying the type of recruitment model. The
#'   available options are
#'   `r toString(eval(formals(create_default_recruitment)[["module_type"]]))`. 
#' @param distribution A string specifying the distribution for the recruitment process.
#'   The available options are
#'   `r toString(eval(formals(create_default_recruitment)[["distribution"]]))`. 
#' @param model_family A string specifying the model family. The available
#'   options are
#'   `r toString(eval(formals(create_default_recruitment)[["model_family"]]))`.
#' @inherit create_default_paramters return
#' @export
#' @rdname create_default_parameters
#' @keywords create_default_parameters
create_default_recruitment <- function(
  data,
  module_type = c("BevertonHolt"),
  distribution = c("Dnorm", NA_character_),
  model_family = c("catch_at_age")
) {
  # Input checks
  # TODO: extend this code when there is more than one form of recruitment
  #       (i.e., multiple populations)
  if (!inherits(data, "FIMSFrame")) { # Replace "FIMS_data" with your actual class name
    cli::cli_abort(c(
      "The {.arg data} argument must be a valid {.cls FIMSFrame} object.",
      "x" = "You provided an object of class {.cls {class(data)[1]}}."
    ))
  }
  module_type <- rlang::arg_match(module_type)
  distribution <- rlang::arg_match(distribution)
  model_family <- rlang::arg_match(model_family)
  
  # Create default parameters based on the recruitment form
  # NOTE: All new forms of recruitment must be placed in the vector of default
  # arguments for `form` and their methods but be placed below in the call to
  # `switch`
  module_type_default <- switch(module_type,
    "BevertonHolt" = create_default_BevertonHoltRecruitment(data, distribution)
  ) |>
    dplyr::mutate(
      model_family = .env$model_family
    )

  # Keep an empty tibble with the same columns so bind_rows() always works.
  distribution_default <- module_type_default |>
    dplyr::slice(0)

  if (!is.na(distribution)) {
    distribution_default <- switch(distribution,
      "Dnorm" = create_default_DnormDistribution(
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
    tidyr::fill(dplyr::all_of(c("model_family", "module_name", "module_type")))
  
  # set the class of the default parameters to "fims_parameters" so users can
  # use `+.fims_parameters` method 
  class(default) <- c("fims_parameters", class(default))
  return(default)
}

#' @export
`+.fims_parameters` <- function(e1, e2) {
  dplyr::bind_rows(e1, e2)
}