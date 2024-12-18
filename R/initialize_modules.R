# To remove the WARNING
# no visible binding for global variable
utils::globalVariables(c("type", "name", "value", "unit", "uncertainty"))

#' Initialize a generic module
#'
#' @description
#' Initializes a generic module by setting up its fields based on the provided
#' `module_name`.
#' @param parameters A list. Contains parameters and modules required for
#'   initialization.
#' @param data An S4 object. FIMS input data.
#' @param module_name A character. Name of the module to initialize (e.g.,
#'   "population" or "fleet").
#' @return
#' The initialized module as an object.
#' @noRd
initialize_module <- function(parameters, data, module_name) {
  # TODO: how to return all modules between pipes and create links between
  # modules?
  # # Retrieve all objects in the environment
  # objs <- mget(ls())
  # modules <- get_rcpp_modules(objs)

  # Input checks
  # Check if parameters is a list and contains the necessary sub-elements
  if (!is.list(parameters)) {
    cli::cli_abort("The {.var parameters} argument should be a list.")
  } else if (!all(c("parameters", "modules") %in% names(parameters))) {
    cli::cli_abort(c(
      "The {.var parameters} argument must contain both parameters and modules
      lists."
    ))
  }
  # Validate module_name
  if (!is.character(module_name) || length(module_name) != 1) {
    cli::cli_abort("{.var module_name} must be a single character string.")
  }

  # Check if module_name exists in the parameters list
  if (!module_name %in% c(
    names(parameters[["parameters"]]),
    names(parameters[["modules"]])
  )) {
    cli::cli_abort("{.var module_name} is missing from the {.var parameters}.")
  }

  # Define module class and fields
  module_class_name <- if (module_name == "population") {
      "Population"
    } else if (!(module_name %in% names(parameters[["modules"]])) &&
               (names(module_name) == "selectivity")
      ) {
      parameters[["modules"]][["fleets"]][[
        module_name
      ]][[names(module_name)]][["form"]]
    } else if (!(module_name %in% names(parameters[["modules"]])) &&
               names(module_name) == "Fleet"
      ) {
      "Fleet"
    } else {
      parameters[["modules"]][[module_name]][["form"]]
    }

  module_class <- get(module_class_name)
  module_fields <- names(module_class@fields)
  module <- new(module_class)
  module_input <- parameters[["parameters"]][[module_name]]

  if (module_class_name == "Fleet") {
    module_fields <- setdiff(module_fields, c(
      "log_expected_index",
      "proportion_catch_numbers_at_age"
    ))

    fleet_types <- get_data(data) |>
      dplyr::filter(name == module_name) |>
      dplyr::pull(type) |>
      unique()

    if ("landings" %in% fleet_types) {
      module_fields <- setdiff(module_fields, c(
        "log_q",
        "random_q",
        "estimate_q"
      ))
    } else {
      module_fields <- setdiff(module_fields, c(
        "log_Fmort"
      ))
    }

    # TODO: refactor "age-to-length-conversion" in FIMSFrame data and
    # "age_length_conversion_matrix" in the Rcpp interface to
    # "age_to_legnth_conversion" for consistency
    if ("age-to-length-conversion" %in% fleet_types &
        "LengthComp" %in% names(parameters[["modules"]][["fleets"]][[module_name]][["data_distribution"]])) {
      age_length_conversion_matrix_value <- FIMS::m_age_to_length_conversion(data, module_name)
      module[["age_length_conversion_matrix"]]$resize(length(age_length_conversion_matrix_value))
      # Assign each value to the corresponding position in the parameter vector
      for (i in seq_along(age_length_conversion_matrix_value)) {
        module[["age_length_conversion_matrix"]][i][["value"]] <- age_length_conversion_matrix_value[i]
      }

      # Set the estimation information for the entire parameter vector
      module[["age_length_conversion_matrix"]]$set_all_estimable(FALSE)
    }

    module_fields <- setdiff(module_fields, c(
      "age_length_conversion_matrix",
      # Right now we can also remove nlengths because the default is 0
      "nlengths",
      "proportion_catch_numbers_at_length"
    ))
  }

  # Populate fields based on common and specific settings
  # TODO:
  # - Population interface
  #   - Update the Population interface to consistently use n_ages and n_years,
  #     as done in the S4 data1 object.
  #   - Currently hard-coded `estimate_prop_female` to TRUE, `proportion_female`
  #     to numeric(0), and `nseason` to 1 using the defaults from FIMS.
  #     Update as needed.
  #   - Add n_fleets to data1. Should n_fleets include both
  #     fishing and survey fleets? Currently, data1@fleets equals 1.
  # - Recruitment interface
  #   - Remove the field estimate_log_devs. It will be set up using the
  #   set_all_estimable() method instead.
  # - Fleet
  #   - Remove estimate_Fmort, estimate_q, and random_q from the Rcpp interface
  #   - Reconsider exposing `log_expected_index` and
  #     `proportion_catch_numbers_at_age` to users. Their IDs are linked with
  #     index and agecomp distributions. No input values are required.

  non_standard_field <- c(
    "ages", "nages", "nlengths",
    "proportion_female", "estimate_prop_female",
    "nyears", "nseasons", "nfleets", "estimate_log_devs", "weights",
    "is_survey", "estimate_q", "random_q"
  )
  for (field in module_fields) {
    if (field %in% non_standard_field) {
      # TODO: reorder the list alphabetically
      module[[field]] <- switch(
        field,
        "ages" = get_ages(data),
        "nages" = get_n_ages(data),
        "nlengths" = get_n_lengths(data),
        "proportion_female" = numeric(0),
        "estimate_prop_female" = TRUE,
        "nyears" = get_n_years(data),
        "nseasons" = 1,
        "nfleets" = length(parameters[["modules"]][["fleets"]]),
        "estimate_log_devs" = module_input[[
          paste0(module_class_name, ".estimate_log_devs")
        ]],
        "weights" = m_weight_at_age(data),
        "is_survey" = !("landings" %in% fleet_types),
        "estimate_q" = module_input[[
          paste0(module_class_name, ".log_q.estimated")
        ]],
        "random_q" = FALSE,
        cli::cli_abort(c(
          "{.var {field}} is not a valid field in {.var {module_class_name}}
          module."
        ))
      )
    } else {
      set_param_vector(
        field = field,
        module = module,
        module_input = module_input
      )
    }
  }

  return(module)
}

# TODO: Determine the relationship between distributions and the
# recruitment module, and implement the appropriate logic to retrieve
# distribution information.

#' Initialize a distribution module
#'
#' @description
#' Initializes a distribution module by setting up its fields based on the
#' distribution name and type. Supports both "data" and "process" types.
#' @param module_input A list. Contains parameters for initializing the
#'   distribution.
#' @param distribution_name A character. Name of the distribution to initialize.
#' @param distribution_type A character. Type of distribution, either "data" or
#'   "process".
#' @param linked_ids A vector. Named vector of linked IDs required for the
#'   distribution, such as data_link and fleet_link for setting up index
#'   distribution.
#' @rdname initialize_module
#' @return
#' The initialized distribution module as an object.
#' @noRd
initialize_distribution <- function(
  module_input,
  distribution_name,
  distribution_type = c("data", "process"),
  linked_ids
) {
  # Input checks
  # Check if distribution_name is provided
  if (is.null(distribution_name)) {
    return(NULL)
  }
  # Validate module_input
  if (!is.list(module_input)) {
    cli::cli_abort("{.var module_input} must be a list.")
  }
  # Validate distribution_type as "data" or "process"
  distribution_type <- rlang::arg_match(distribution_type)
  # Validate linked_ids as a named vector with required elements for "data" type
  if (!is.vector(linked_ids) ||
      !all(c("data_link", "fleet_link") %in% names(linked_ids))
  ) {
    cli::cli_abort(
      "{.var linked_ids} must be a named vector containing 'data_link' and
      'fleet_link' for 'data' distribution types."
    )
  }

  # Get distribution value and initialize the module
  distribution_value <- get(distribution_name)
  distribution_module <- new(distribution_value)
  distribution_fields <- names(distribution_value@fields)
  if (distribution_type == "data") {
    distribution_fields <- setdiff(
      distribution_fields,
      c("expected_values", "x", "dims")
    )
  }

  distribution_input_names <- grep(
    distribution_name,
    names(module_input),
    value = TRUE
  )
  for (field in distribution_fields) {
    set_param_vector(
      field = field, module = distribution_module,
      module_input = module_input[distribution_input_names]
    )
  }

  switch(
    distribution_type,
    "data" = {
      # Data distribution initialization
      distribution_module$set_observed_data(linked_ids["data_link"])
      distribution_module$set_distribution_links(
        distribution_type,
        linked_ids["fleet_link"]
      )
    },
    "process" = {
      # Process distribution initialization
      distribution_module$set_distribution_links("random_effects", linked_ids)
    }
  )

  # Final message to confirm success
  cli::cli_inform(c(
    "i" = "{distribution_name} initialized successfully for
          {names(distribution_name)}."
  ))

  return(distribution_module)
}

#' Initialize a recruitment module
#'
#' @description
#' Initializes a recruitment module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' recruitment initialization.
#' @inheritParams initialize_module
#' @return
#' The initialized recruitment module as an object.
#' @noRd
initialize_recruitment <- function(parameters, data) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames("recruitment", "population")
  )
  return(module)
}

#' Initialize a growth module
#'
#' @description
#' Initializes a growth module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' growth initialization.
#' @inheritParams initialize_module
#' @return
#' The initialized growth module as an object.
#' @noRd
initialize_growth <- function(parameters, data) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames("growth", "population")
  )
  return(module)
}

#' Initialize a maturity module
#'
#' @description
#' Initializes a maturity module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' maturity initialization.
#' @inheritParams initialize_module
#' @return
#' The initialized maturity module as an object.
#' @noRd
initialize_maturity <- function(parameters, data) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames("maturity", "population")
  )
  return(module)
}

#' Initialize a population module.
#'
#' @description
#' Initializes a population module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' population initialization.
#' @inheritParams initialize_module
#' @param linked_ids A vector. Named vector of linked IDs required for the
#'   population, including IDs for "growth", "maturity", and "recruitment".
#' @return
#' The initialized population module as an object.
#' @noRd
initialize_population <- function(parameters, data, linked_ids) {
  if (any(is.na(linked_ids[c("growth", "maturity", "recruitment")]))) {
    cli::cli_abort(c(
      "{.var linked_ids} for population must include `growth`, `maturity`, and
      `recruitment` IDs."
    ))
  }

  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames("population", "population")
  )

  # Link up the recruitment, growth, and maturity modules with
  # this population module
  module$SetGrowth(linked_ids["growth"])
  module$SetMaturity(linked_ids["maturity"])
  module$SetRecruitment(linked_ids["recruitment"])

  return(module)
}

#' Initialize a selectivity module.
#'
#' @description
#' Initializes a selectivity module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' population initialization.
#' @inheritParams initialize_module
#' @param fleet_name A character. Name of the fleet to initialize.
#' @return
#' The initialized selectivity module as an object.
#' @noRd
initialize_selectivity <- function(parameters, data, fleet_name) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames(fleet_name, "selectivity")
  )

  return(module)
}

# TODO: Do we want to put initialize_selectivity(), initialize_index(), and
# initial_age_comp() inside of initialize_fleet()?

#' Initialize a fleet module
#'
#' @description
#' Initializes a fleet module by setting up its fields. It links selectivity,
#' index, and age-composition modules.
#' @inheritParams initialize_module
#' @param fleet_name A character. Name of the fleet to initialize.
#' @param linked_ids A vector. Named vector of linked IDs required for the
#'  fleet, including IDs for "selectivity", "index", "age_comp", and "length_comp".
#' @return
#' The initialized fleet module as an object.
#' @noRd
initialize_fleet <- function(parameters, data, fleet_name, linked_ids) {
  if (any(is.na(linked_ids[c("selectivity", "index", "age_comp")]))) {
    cli::cli_abort(c(
      "{.var linked_ids} for {fleet_name} must include 'selectivity', 'index',
      and 'age_comp' IDs."
    ))
  }

  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames(fleet_name, "Fleet")
  )

  module$SetSelectivity(linked_ids["selectivity"])
  module$SetObservedIndexData(linked_ids["index"])
  module$SetObservedAgeCompData(linked_ids["age_comp"])

  fleet_types <- get_data(data) |>
      dplyr::filter(name == fleet_name) |>
      dplyr::pull(type) |>
      unique()
  if ("length" %in% fleet_types &
      "LengthComp" %in% names(parameters[["modules"]][["fleets"]][[fleet_name]][["data_distribution"]])) {
    module$SetObservedLengthCompData(linked_ids["length_comp"])
  }
  return(module)
}

#' Initialize an index module
#'
#' @description
#' Initializes an index module based on the provided data and fleet name.
#' @inheritParams initialize_module
#' @param fleet_name A character. Name of the fleet for which the index module
#'   is initialized.
#' @return
#' The initialized index module as an object.
#' @noRd
initialize_index <- function(data, fleet_name) {
  # Check if the specified fleet exists in the data
  fleet_exists <- any(get_data(data)["name"] == fleet_name)
  if (!fleet_exists) {
    cli::cli_abort("Fleet {fleet_name} not found in the data object.")
  }

  fleet_type <- dplyr::filter(
    .data = as.data.frame(data@data),
    name == fleet_name
  ) |>
    dplyr::distinct(type) |>
    dplyr::pull(type)


  module <- methods::new(Index, get_n_years(data))

  if ("landings" %in% fleet_type) {
    module[["index_data"]] <- m_landings(data, fleet_name)
  } else if ("index" %in% fleet_type) {
    module[["index_data"]] <- m_index(data, fleet_name)
  } else {
    cli::cli_abort(c(
      "Fleet type `{fleet_type}` is not valid for index module initialization.
      Only 'landings' or 'index' are supported."
    ))
  }

  return(module)
}

#' Initialize an age-composition module
#'
#' @description
#' Initializes an age-composition module for a specific fleet,
#' setting the age-composition data for the fleet over time.
#' @inheritParams initialize_module
#' @param fleet_name A character. Name of the fleet for which age-composition
#'   data is initialized.
#' @return
#' The initialized age-composition module as an object.
#' @noRd
initialize_age_comp <- function(data, fleet_name) {

  # Check if the specified fleet exists in the data
  fleet_exists <- any(get_data(data)["name"] == fleet_name)
  if (!fleet_exists) {
    cli::cli_abort("Fleet {fleet_name} not found in the data object.")
  }

  module <- methods::new(AgeComp, get_n_years(data), get_n_ages(data))

  # Validate that the fleet's age-composition data is available
  age_comp_data <- m_agecomp(data, fleet_name)
  if (is.null(age_comp_data) || length(age_comp_data) == 0) {
    cli::cli_abort(c(
      "Age-composition data for fleet `{fleet_name}` is unavailable or empty."
    ))
  }

  # Assign the age-composition data to the module
  # TODO: review the AgeComp interface, do we want to add
  # `age_comp_data` as an argument?

  module$age_comp_data <- age_comp_data * dplyr::filter(
    .data = as.data.frame(data@data),
    name == fleet_name,
    type == "age"
  ) |>
    dplyr::pull(uncertainty)

  return(module)
}

# TODO: combine initialize_length_comp and initialize_age_comp() into a single
# function, as they share similar code.
#' Initialize a length-composition module
#'
#' @description
#' Initializes a length-composition module for a specific fleet,
#' setting the length-composition data for the fleet over time.
#' @inheritParams initialize_module
#' @param fleet_name A character. Name of the fleet for which length-composition
#'   data is initialized.
#' @return
#' The initialized length-composition module as an object.
#' @noRd
initialize_length_comp <- function(data, fleet_name) {

  # Check if the specified fleet exists in the data
  fleet_exists <- any(get_data(data)["name"] == fleet_name)
  if (!fleet_exists) {
    cli::cli_abort("Fleet {fleet_name} not found in the data object.")
  }

  module <- methods::new(LengthComp, get_n_years(data), get_n_lengths(data))

  # Validate that the fleet's length-composition data is available
  length_comp_data <- m_lengthcomp(data, fleet_name)
  if (is.null(length_comp_data) || length(length_comp_data) == 0) {
    cli::cli_abort(c(
      "Length-composition data for fleet `{fleet_name}` is unavailable or empty."
    ))
  }

  # Assign the length-composition data to the module
  # TODO: review the LengthComp interface, do we want to add
  # `age_comp_data` as an argument?

  module$length_comp_data <- length_comp_data * dplyr::filter(
    .data = as.data.frame(data@data),
    name == fleet_name,
    type == "length"
  ) |>
    dplyr::pull(uncertainty)

  return(module)
}

#' Initialize FIMS modules
#'
#' @description
#' Initializes multiple modules within the Fisheries Integrated Modeling System
#' (FIMS), including fleet, recruitment, growth, maturity, and population
#' modules. This function iterates over the provided fleets, setting up
#' necessary sub-modules such as selectivity, index, and age composition. It
#' also sets up distribution models for fishery index and age-composition data.
#' @param parameters A list. Contains parameters and modules required for
#'   initialization.
#' @param data An S4 object. FIMS input data.
#' @return
#' A list containing parameters for the initialized FIMS modules, ready for use
#' in TMB modeling.
#' @export
initialize_fims <- function(parameters, data) {
  # Validate parameters input
  if (missing(parameters) || !is.list(parameters)) {
    cli::cli_abort("The {.var parameters} argument must be a non-missing list.")
  }
  # Clear any previous FIMS settings
  clear()

  module_name <- "fleets"
  fleet_names <- names(parameters[["modules"]][["fleets"]])
  if (length(fleet_names) == 0) {
    cli::cli_abort(c(
      "No fleets found in the provided {.var parameters[['modules']]}."
    ))
  }

  # Initialize lists to store fleet-related objects
  fleet <- fleet_selectivity <-
    fleet_index <- fleet_index_distribution <-
    fleet_age_comp <- fleet_agecomp_distribution <-
    fleet_length_comp <- fleet_lengthcomp_distribution <-
    vector("list", length(fleet_names))


  for (i in seq_along(fleet_names)) {
    fleet_selectivity[[i]] <- initialize_selectivity(
      parameters = parameters,
      data = data,
      fleet_name = fleet_names[i]
    )

    fleet_index[[i]] <- initialize_index(
      data = data,
      fleet_name = fleet_names[i]
    )

    fleet_age_comp[[i]] <- initialize_age_comp(
      data = data,
      fleet_name = fleet_names[i]
    )

    fleet_module_ids <- c(
      index = fleet_index[[i]]$get_id(),
      age_comp = fleet_age_comp[[i]]$get_id(),
      selectivity = fleet_selectivity[[i]]$get_id()
    )

    fleet_types <- get_data(data) |>
      dplyr::filter(name == fleet_names[i]) |>
      dplyr::pull(type) |>
      unique()

    if ("length" %in% fleet_types &
        "LengthComp" %in% names(parameters[["modules"]][["fleets"]][[fleet_names[i]]][["data_distribution"]])) {
      fleet_length_comp[[i]] <- initialize_length_comp(
        data = data,
        fleet_name = fleet_names[i]
      )
      fleet_module_ids <- c(
        fleet_module_ids,
        c(length_comp = fleet_length_comp[[i]]$get_id())
      )
    }

    fleet[[i]] <- initialize_fleet(
      parameters = parameters,
      data = data,
      fleet_name = fleet_names[i],
      linked_ids = fleet_module_ids
    )

    # TODO: update argument sd to log_sd to match the Rcpp interface
    parameter_value_name <- grep(
      paste0("log_sd", ".value"),
      names(parameters[["parameters"]][[fleet_names[i]]]),
      value = TRUE
    )
    parameter_estimated_name <- grep(
      paste0("log_sd", ".estimated"),
      names(parameters[["parameters"]][[fleet_names[i]]]),
      value = TRUE
    )

    if (length(parameter_value_name) == 0 ||
        length(parameter_estimated_name) == 0
    ) {
      cli::cli_abort(c(
        "Missing required inputs for `log_sd` in fleet `{fleet_name}`."
      ))
    }

    fleet_index_distribution[[i]] <- initialize_data_distribution(
      module = fleet[[i]],
      family = lognormal(link = "log"),
      sd = list(
        value = exp(
          parameters[["parameters"]][[fleet_names[i]]][[parameter_value_name]]
        ),
        estimated = parameters[["parameters"]][[fleet_names[i]]][[parameter_estimated_name]]
      ),
      data_type = "index"
    )

    fleet_agecomp_distribution[[i]] <- initialize_data_distribution(
      module = fleet[[i]],
      family = multinomial(link = "logit"),
      data_type = "agecomp"
    )

    if ("length" %in% fleet_types &
        "LengthComp" %in% names(parameters[["modules"]][["fleets"]][[fleet_names[i]]][["data_distribution"]])) {
      fleet_lengthcomp_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        family = multinomial(link = "logit"),
        data_type = "lengthcomp"
      )
    }

  }

  # Recruitment
  # create new module in the recruitment class (specifically Beverton--Holt,
  # when there are other options, this would be where the option would be
  # chosen)
  recruitment <- initialize_recruitment(
    parameters = parameters,
    data = data
  )

  parameter_name <- names(parameters$modules$recruitment$process_distribution)
  field_value_name <- grep(
    paste0("log_sd.value"),
    names(parameters[["parameters"]][["recruitment"]]),
    value = TRUE
  )
  field_estimated_name <- grep(
    paste0("log_sd.estimated"),
    names(parameters[["parameters"]][["recruitment"]]),
    value = TRUE
  )

  if (length(field_value_name) == 0 || length(field_estimated_name) == 0) {
    cli::cli_abort("Missing required inputs for recruitment distribution.")
  }

  recruitment_distribution <- initialize_process_distribution(
    module = recruitment,
    par = names(parameters$modules$recruitment$process_distribution),
    family = gaussian(),
    sd = list(
      value = parameters[["parameters"]][["recruitment"]][[field_value_name]],
      estimated = parameters[["parameters"]][[
        "recruitment"
      ]][[field_estimated_name]]
    ),
    is_random_effect = FALSE
  )

  # Growth
  growth <- initialize_growth(
    parameters = parameters,
    data = data
  )

  # Maturity
  maturity <- initialize_maturity(
    parameters = parameters,
    data = data
  )

  population_module_ids <- c(
    recruitment = recruitment$get_id(),
    growth = growth$get_id(),
    maturity = maturity$get_id()
  )

  # Population
  population <- initialize_population(
    parameters = parameters,
    data = data,
    linked_ids = population_module_ids
  )

  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameter_list <- list(
    parameters = list(p = get_fixed())
  )

  return(parameter_list)
}

#' Set parameter vector values based on module input
#'
#' @description
#' This function sets the parameter vector values in a module based on the
#' provided module input, including both initial values and estimation
#' information.
#' @param field A character string specifying the field name of the parameter
#'   vector to be updated.
#' @param module A module object in which the parameter vector is to be set.
#' @param module_input A list containing input parameters for the module,
#'   including value and estimation information for the parameter vector.
#' @return
#' Modified module object.
#' @noRd
set_param_vector <- function(field, module, module_input) {
  # Check if field_name is a non-empty character string
  if (missing(field) || !is.character(field) || nchar(field) == 0) {
    cli::cli_abort(c(
      "The {.var field} argument must be a non-empty character string."
    ))
  }

  # Check if module is a reference class
  if (!is(module, "refClass")) {
    cli::cli_abort(c(
      "The {.var module} argument must be a reference class created by
      {.fn methods::new}."
    ))
  }

  # Check if module_input is a list
  if (!is.list(module_input)) {
    cli::cli_abort("The {.var module_input} argument must be a list.")
  }

  # Identify the name for the parameter value and estimation fields in
  # module_input
  field_value_name <- grep(
    paste0(field, ".value"),
      names(module_input),
      value = TRUE
    )
  field_estimated_name <- grep(
    paste0(field, ".estimated"),
    names(module_input),
    value = TRUE
  )

  # Check if both value and estimation information are present
  if (length(field_value_name) == 0 || length(field_estimated_name) == 0) {
    cli::cli_abort(c(
      "Missing value or estimation information for {.var field}."
    ))
  }

  # Extract the value of the parameter vector
  field_value <- module_input[[field_value_name]]

  # Resize the field in the module if it has multiple values
  if (length(field_value) > 1) module[[field]]$resize(length(field_value))

  # Assign each value to the corresponding position in the parameter vector
  for (i in seq_along(field_value)) {
    module[[field]][i][["value"]] <- field_value[i]
  }

  # Set the estimation information for the entire parameter vector
  module[[field]]$set_all_estimable(module_input[[field_estimated_name]])
}
