# To remove the WARNING
# no visible binding for global variable
utils::globalVariables(c(
  "type", "name", "value", "unit", "uncertainty",
  "datestart", "dateend", "age", "length", "year",
  # Used in initialize_comp dplyr code
  "valid_n"
))

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
  module <- methods::new(module_class)
  module_input <- parameters[["parameters"]][[module_name]]

  if (module_class_name == "Fleet") {
    module_fields <- setdiff(module_fields, c(
      "log_index_expected",
      "log_landings_expected",
      "index_expected",
      "landings_expected",
      "agecomp_expected",
      "agecomp_proportion",
      "observed_index_units",
      "observed_landings_units"
    ))

    fleet_types <- get_data(data) |>
      dplyr::filter(name == module_name) |>
      dplyr::pull(type) |>
      unique()

    data_distribution_names_for_fleet_i <- names(parameters[["modules"]][["fleets"]][[module_name]][["data_distribution"]])
    if ("age-to-length-conversion" %in% fleet_types &&
      "LengthComp" %in% data_distribution_names_for_fleet_i) {
      age_to_length_conversion_value <- FIMS::m_age_to_length_conversion(data, module_name)
      module[["age_to_length_conversion"]]$resize(length(age_to_length_conversion_value))
      # Assign each value to the corresponding position in the parameter vector
      purrr::walk(
        seq_along(age_to_length_conversion_value),
        \(x) module[["age_to_length_conversion"]][x][["value"]] <- age_to_length_conversion_value[x]
      )

      # Set the estimation information for the entire parameter vector
      module[["age_to_length_conversion"]]$set_all_estimable(FALSE)

      module[["age_to_length_conversion"]]$set_all_random(FALSE)
    } else {
      module_fields <- setdiff(module_fields, c(
        # Right now we can also remove nlengths because the default is 0
        "nlengths"
      ))
    }

    module_fields <- setdiff(module_fields, c(
      "age_to_length_conversion",
      "lengthcomp_expected",
      "lengthcomp_proportion"
    ))
  }

  # Populate fields based on common and specific settings
  # TODO:
  # - Population interface
  #   - Update the Population interface to consistently use n_ages and n_years,
  #     as done in the S4 data1 object.
  #   - Currently hard-coded `nseason` to 1 using the defaults from FIMS.
  #     Update as needed.
  #   - Add n_fleets to data1. Should n_fleets include both
  #     fishing and survey fleets? Currently, data1@fleets equals 1.
  # - Recruitment interface
  #   - Remove the field estimate_log_devs. It will be set up using the
  #   set_all_estimable() method instead.
  # - Fleet
  #   - Remove estimate_Fmort, estimate_q, and random_q from the Rcpp interface
  #   - Reconsider exposing `log_index_expected` and
  #     `agecomp_proportion` to users. Their IDs are linked with
  #     index and agecomp distributions. No input values are required.

  integer_fields <- c(
    "nages", "nfleets", "nlengths",
    "nseasons", "nyears"
  )

  boolean_fields <- c(
    "estimate_log_devs", "estimate_prop_female",
    "estimate_q", "random_q"
  )

  real_vector_fields <- c(
    "ages", "weights"
  )

  for (field in module_fields) {
    if (field %in% integer_fields) {
      module[[field]]$set(
        switch(field,
          "nages" = get_n_ages(data),
          "nfleets" = length(parameters[["modules"]][["fleets"]]),
          "nlengths" = get_n_lengths(data),
          "nseasons" = 1,
          "nyears" = get_n_years(data)
        )
      )
    } else if (field %in% boolean_fields) {
      module[[field]]$set(
        switch(field,
          "estimate_log_devs" = module_input[[
            paste0(module_class_name, ".estimate_log_devs")
          ]],
          "estimate_q" = module_input[[
            paste0(module_class_name, ".log_q.estimated")
          ]],
          "random_q" = FALSE,
          cli::cli_abort(c(
            "{.var {field}} is not a valid field in {.var {module_class_name}}
            module."
          ))
        )
      )
    } else if (field %in% c("ages", "weights")) {
      get_value_function <- switch(field,
        "ages" = get_ages,
        "weights" = m_weight_at_age
      )
      module[[field]]$resize(get_n_ages(data))
      purrr::walk(seq_len(get_n_ages(data)), function(x) {
        module[[field]]$set(x - 1, get_value_function(data)[x])
      })
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
    linked_ids) {
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
  distribution_module <- methods::new(distribution_value)
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

  switch(distribution_type,
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
#'  fleet, including IDs for "selectivity", "landings", "index", "age_comp", and "length_comp".
#' @return
#' The initialized fleet module as an object.
#' @noRd
initialize_fleet <- function(parameters, data, fleet_name, linked_ids) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = setNames(fleet_name, "Fleet")
  )

  module$SetSelectivityID(linked_ids["selectivity"])

  fleet_types <- get_data(data) |>
    dplyr::filter(name == fleet_name) |>
    dplyr::pull(type) |>
    unique()


  distribution_names_for_fleet <- names(parameters[["modules"]][["fleets"]][[fleet_name]][["data_distribution"]])

  # Link the observed landings data to the fleet module using its associated ID
  # if the data type includes "landings" and if "Landings" exists in the data distribution
  # specification
  if ("landings" %in% fleet_types &&
    "Landings" %in% distribution_names_for_fleet) {
    module$SetObservedLandingsDataID(linked_ids["landings"])
  }

  # Link the observed index data to the fleet module using its associated ID
  # if the data type includes "index" and if "Index" exists in the data distribution
  # specification
  if ("index" %in% fleet_types &&
    "Index" %in% distribution_names_for_fleet) {
    module$SetObservedIndexDataID(linked_ids["index"])
  }

  # Link the observed age composition data to the fleet module using its associated ID
  # if the data type includes "age" and if "AgeComp" exists in the data distribution
  # specification
  if ("age" %in% fleet_types &&
    "AgeComp" %in% distribution_names_for_fleet) {
    module$SetObservedAgeCompDataID(linked_ids["age_comp"])
  }

  # Link the observed length composition data to the fleet module using its associated ID
  # if the data type includes "length" and if "LengthComp" exists in the data
  # distribution specification
  if ("length" %in% fleet_types &&
    "LengthComp" %in% distribution_names_for_fleet) {
    module$SetObservedLengthCompDataID(linked_ids["length_comp"])
  }
  return(module)
}

#' Initialize a landings module
#'
#' @description
#' Initializes a landings module based on the provided data and fleet name.
#' @inheritParams initialize_module
#' @param fleet_name A character. Name of the fleet for which the landings module
#'   is initialized.
#' @return
#' The initialized landings module as an object.
#' @noRd
initialize_landings <- function(data, fleet_name) {
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

  if ("landings" %in% fleet_type) {
    module <- methods::new(Landings, get_n_years(data))
    purrr::walk(
      seq_along(m_landings(data, fleet_name)),
      \(x) module$landings_data$set(x - 1, m_landings(data, fleet_name)[x])
    )
    return(module)
  } else {
    return(NULL)
  }
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




  if ("index" %in% fleet_type) {
    module <- methods::new(Index, get_n_years(data))
    purrr::walk(
      seq_along(m_index(data, fleet_name)),
      \(x) module$index_data$set(x - 1, m_index(data, fleet_name)[x])
    )
    return(module)
  } else {
    return(NULL)
  }
}

#' Initialize a composition module
#'
#' Several types of composition modules exist and this function acts as a
#' generic interface to initialize any type, for example assigning
#' age-composition data to a given fleet would be an example of initializing
#' a composition module.
#'
#' @inheritParams initialize_module
#' @param fleet_name A character specifying the name of the fleet for which
#'   composition data is initialized.
#' @param type A character specifying the composition type, where the default
#'   is `"AgeComp"`. At the moment, one can initialize `"AgeComp"` or
#'   `"LengthComp"` modules.
#' @return
#' The initialized composition module as an object.
#' @noRd
initialize_comp <- function(data,
                            fleet_name,
                            type = c("AgeComp", "LengthComp")) {
  # Edit this list if a new type is added
  # Set up the specifics for the given type.
  comp_types <- list(
    "AgeComp" = list(
      "name" = "age",
      "comp_data_field" = "age_comp_data",
      "get_n_function" = get_n_ages,
      "comp_object" = AgeComp,
      "m_comp" = m_agecomp
    ),
    "LengthComp" = list(
      "name" = "length",
      "comp_data_field" = "length_comp_data",
      "get_n_function" = get_n_lengths,
      "comp_object" = LengthComp,
      "m_comp" = m_lengthcomp
    )
  )

  # Ensures the user input matches the options provided,
  #   if not, then match.arg() throws an error
  type <- match.arg(type)
  # Select the row in comp_types that matches the user's type selection
  comp <- comp_types[[type]]

  # Check if the specified fleet exists in the data
  fleet_exists <- any(get_data(data)["name"] == fleet_name)
  if (!fleet_exists) {
    cli::cli_abort("Fleet `{fleet_name}` not found in the data object.")
  }

  get_function <- comp[["get_n_function"]]
  module <- methods::new(
    comp[["comp_object"]],
    get_n_years(data),
    get_function(data)
  )

  # Validate that the fleet's composition data is available
  comp_data <- comp[["m_comp"]](data, fleet_name)
  if (is.null(comp_data) || length(comp_data) == 0) {
    cli::cli_abort(c(
      "`{comp[['name']]}`-composition data for fleet `{fleet_name}` is
      unavailable or empty."
    ))
  }

  model_data <- comp_data *
    get_data(data) |>
      dplyr::filter(
        name == fleet_name,
        type == comp[["name"]]
      ) |>
      dplyr::mutate(
        valid_n = ifelse(value == -999, 1, uncertainty)
      ) |>
      dplyr::pull(valid_n)

  purrr::walk(
    seq_along(model_data),
    \(x) module[[comp[["comp_data_field"]]]]$set(x - 1, model_data[x])
  )

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
    fleet_landings <- fleet_landings_distribution <-
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

    fleet_module_ids <- c(
      selectivity = fleet_selectivity[[i]]$get_id()
    )

    fleet_types <- get_data(data) |>
      dplyr::filter(name == fleet_names[i]) |>
      dplyr::pull(type) |>
      unique()

    data_distribution_names_for_fleet_i <- names(
      parameters[["modules"]][["fleets"]][[fleet_names[i]]][["data_distribution"]]
    )

    # Initialize landings module if the data type includes "landings" and
    # if "Landings" exists in the data distribution specification
    if ("landings" %in% fleet_types &&
      "Landings" %in% data_distribution_names_for_fleet_i) {
      # Initialize landings module for the current fleet
      fleet_landings[[i]] <- initialize_landings(
        data = data,
        fleet_name = fleet_names[i]
      )

      # Add the module ID for the initialized landings to the list of fleet module IDs
      fleet_module_ids <- c(
        fleet_module_ids,
        c(landings = fleet_landings[[i]]$get_id())
      )
    }

    # Initialize index module if the data type includes "index" and
    # if "Index" exists in the data distribution specification
    if ("index" %in% fleet_types &&
      "Index" %in% data_distribution_names_for_fleet_i) {
      # Initialize index module for the current fleet
      fleet_index[[i]] <- initialize_index(
        data = data,
        fleet_name = fleet_names[i]
      )

      # Add the module ID for the initialized index to the list of fleet module IDs
      fleet_module_ids <- c(
        fleet_module_ids,
        c(index = fleet_index[[i]]$get_id())
      )
    }

    # Initialize age composition module if the data type includes "age" and
    # if "AgeComp" exists in the data distribution specification
    if ("age" %in% fleet_types &&
      "AgeComp" %in% data_distribution_names_for_fleet_i) {
      # Initialize age composition module for the current fleet
      fleet_age_comp[[i]] <- initialize_comp(
        data = data,
        fleet_name = fleet_names[i],
        type = "AgeComp"
      )

      # Add the module ID for the initialized age composition to the list of fleet module IDs
      fleet_module_ids <- c(
        fleet_module_ids,
        c(age_comp = fleet_age_comp[[i]]$get_id())
      )
    }

    # Initialize length composition module if the data type includes "length" and
    # if "LengthComp" exists in the data distribution specification
    if ("length" %in% fleet_types &&
      "LengthComp" %in% data_distribution_names_for_fleet_i) {
      # Initialize length composition module for the current fleet
      fleet_length_comp[[i]] <- initialize_comp(
        data = data,
        fleet_name = fleet_names[i],
        type = "LengthComp"
      )

      # Add the module ID for the initialized length composition to the list of fleet module IDs
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

    if ("index" %in% fleet_types &&
      "Index" %in% data_distribution_names_for_fleet_i) {
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
    }

    if ("landings" %in% fleet_types &&
      "Landings" %in% data_distribution_names_for_fleet_i) {
      fleet_landings_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        family = lognormal(link = "log"),
        sd = list(
          value = exp(
            parameters[["parameters"]][[fleet_names[i]]][[parameter_value_name]]
          ),
          estimated = parameters[["parameters"]][[fleet_names[i]]][[parameter_estimated_name]]
        ),
        data_type = "landings"
      )
    }
    # TODO (Matthew): Determine if the "dims" field is required for DmultinomDistribution.
    # We need to decide whether to:
    # 1. Remove the "dims" field to maintain consistency with other distributions, or
    # 2. Update all relevant R functions (e.g., initialize_data_distribution())
    #    that call DmultinomDistribution to set the "dims" field.
    if ("age" %in% fleet_types &&
      "AgeComp" %in% data_distribution_names_for_fleet_i) {
      fleet_agecomp_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        family = multinomial(link = "logit"),
        data_type = "agecomp"
      )
    }

    if ("length" %in% fleet_types &&
      "LengthComp" %in% data_distribution_names_for_fleet_i) {
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
