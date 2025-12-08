# To remove the WARNING
# no visible binding for global variable
utils::globalVariables(c(
  "type", "name", "value", "unit", "uncertainty",
  "timing", "age", "length", "year",
  # Used in initialize_comp dplyr code
  "valid_n"
))

#' Initialize a generic module
#'
#' @description
#' Initializes a generic module by setting up its fields based on the provided
#' `module_name`.
#' @param parameters A tibble. Contains parameters and modules required for
#'   initialization.
#' @param data An S4 object. FIMS input data.
#' @param module_name A character. Name of the module to initialize (e.g.,
#'   "Population" or "Fleet").
#' @param fleet_name A character. Name of the fleet to initialize. If not
#'   specified, the module will be initialized without fleet-specific data.
#' @return
#' The initialized module as an object.
#' @noRd
initialize_module <- function(parameters, data, module_name, fleet_name = NA_character_) {
  module_input <- parameters |>
    # Using !! to unquote the variables
    dplyr::filter(module_name == !!module_name)

  if (!is.na(fleet_name)) {
    module_input <- module_input |>
      dplyr::filter(fleet_name == !!fleet_name)
  }

  module_class_name <- module_input |>
    # Combine module_type and module_name into a single string
    dplyr::mutate(
      temp_name = paste0(
        # Replace NAs with ""
        dplyr::coalesce(module_type, ""),
        dplyr::coalesce(module_name, "")
      )
    ) |>
    dplyr::pull(temp_name) |>
    unique()

  module_class <- get(module_class_name)
  module_fields <- names(module_class@fields)
  module <- methods::new(module_class)

  if (module_class_name == "Fleet") {
    # Remove certain fields for the Fleet module
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
      dplyr::filter(name == fleet_name) |>
      dplyr::pull(type) |>
      unique()

    data_distribution_names_for_fleet_i <- parameters |>
      dplyr::filter(fleet_name == !!fleet_name & distribution_type == "Data") |>
      dplyr::pull(module_type)
    if ("age-to-length-conversion" %in% fleet_types &&
      "LengthComp" %in% data_distribution_names_for_fleet_i) {
      age_to_length_conversion_value <- FIMS::m_age_to_length_conversion(data, fleet_name)
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
        # Right now we can also remove n_lengths because the default is 0
        "n_lengths"
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
  #     Update as needed.
  #   - Add n_fleets to data1. Should n_fleets include both
  #     fishing and survey fleets? Currently, data1@fleets equals 1.
  # - Fleet
  #   - Reconsider exposing `log_expected_index` and
  #     `agecomp_proportion` to users. Their IDs are linked with
  #     index and agecomp distributions. No input values are required.

  integer_fields <- c(
    "n_ages", "n_fleets", "n_lengths",
    "n_years"
  )

  boolean_fields <- c(
    "estimate_prop_female"
  )

  real_vector_fields <- c(
    "ages", "weights"
  )

  for (field in module_fields) {
    if (field %in% integer_fields) {
      module[[field]]$set(
        switch(field,
          "n_ages" = get_n_ages(data),
          "n_fleets" = parameters |>
            dplyr::filter(module_name == "Fleet") |>
            dplyr::pull(fleet_name) |>
            unique() |>
            length(),
          # Or we can use get_n_fleets(data),
          "n_lengths" = get_n_lengths(data),
          "n_years" = get_n_years(data)
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
    module_name = "Recruitment"
  )
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
    module_name = "Growth"
  )
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
    module_name = "Maturity"
  )
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
  if (anyNA(linked_ids[c("growth", "maturity", "recruitment")])) {
    cli::cli_abort(c(
      "{.var linked_ids} for population must include `growth`, `maturity`, and
      `recruitment` IDs."
    ))
  }

  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = "Population"
  )

  # Link up the recruitment, growth, and maturity modules with
  # this population module
  module$SetGrowthID(linked_ids[["growth"]])
  module$SetMaturityID(linked_ids[["maturity"]])
  module$SetRecruitmentID(linked_ids[["recruitment"]])
  # Link fleets to module
  for (i in which(grepl("fleet", names(linked_ids)))) {
    module$AddFleet(linked_ids[[i]])
  }

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
  module_name <- "Selectivity"
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = module_name,
    fleet_name = fleet_name
  )
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
    fleet_name = fleet_name,
    module_name = "Fleet"
  )

  module$SetSelectivityID(linked_ids["selectivity"])

  fleet_types <- get_data(data) |>
    dplyr::filter(name == fleet_name) |>
    dplyr::pull(type) |>
    unique()


  distribution_names_for_fleet <- parameters |>
    dplyr::filter(fleet_name == !!fleet_name & distribution_type == "Data") |>
    dplyr::pull(module_type)

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
  # if the data type includes "age_comp" and if "AgeComp" exists in the data distribution
  # specification
  if ("age_comp" %in% fleet_types &&
    "AgeComp" %in% distribution_names_for_fleet) {
    module$SetObservedAgeCompDataID(linked_ids["age_comp"])
  }

  # Link the observed length composition data to the fleet module using its associated ID
  # if the data type includes "length_comp" and if "LengthComp" exists in the data
  # distribution specification
  if ("length_comp" %in% fleet_types &&
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
    .data = as.data.frame(get_data(data)),
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
    .data = as.data.frame(get_data(data)),
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
      "name" = "age_comp",
      "comp_data_field" = "age_comp_data",
      "get_n_function" = get_n_ages,
      "comp_object" = AgeComp,
      "m_comp" = m_agecomp
    ),
    "LengthComp" = list(
      "name" = "length_comp",
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

  if (length(model_data) != get_n_years(data) * get_function(data)) {
    bad_data_years <- get_data(data) |>
      dplyr::filter(
        name == fleet_name,
        type == comp[["name"]]
      ) |>
      dplyr::count(timing) |>
      dplyr::filter(n != get_function(data)) |>
      dplyr::pull(timing)

    cli::cli_abort(c(
      "The length of the `{comp[['name']]}`-composition data for fleet
      `{fleet_name}` does not match the expected dimensions.",
      i = "Expected length: {get_n_years(data) * get_function(data)}",
      i = "Actual length: {length(model_data)}",
      i = "Number of -999 values: {sum(model_data == -999)}",
      i = "Dates with invalid data: {bad_data_years}"
    ))
  }

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
#' @param parameters A tibble. Contains parameters and modules required for
#'   initialization.
#' @param data An S4 object. FIMS input data.
#' @return
#' A list containing parameters for the initialized FIMS modules, ready for use
#' in TMB modeling.
#' @export
initialize_fims <- function(parameters, data) {
  # Validate parameters input
  if (missing(parameters) || !tibble::is_tibble(parameters)) {
    cli::cli_abort("The {.var parameters} argument must be a tibble.")
  }

  # Check if parameters is a nested tibble. If so, unnest parameters
  if ("data" %in% names(parameters)) {
    parameters <- parameters |>
      tidyr::unnest(cols = c(data))
  }

  # Check if estimation_type is within "constant", "fixed_effect", "random_effect"
  valid_estimation_types <- c("constant", "fixed_effects", "random_effects")
  invalid_estimation_types <- parameters |>
    dplyr::filter(!estimation_type %in% valid_estimation_types) |>
    dplyr::pull(estimation_type) |>
    unique() |>
    na.omit()

  if (length(invalid_estimation_types) > 0) {
    cli::cli_abort(c(
      "The `estimation_type` must be one of: {valid_estimation_types}.",
      i = "Invalid values found: {invalid_estimation_types}."
    ))
  }

  # Clear any previous FIMS settings
  clear()

  fleet_names <- parameters |>
    dplyr::pull(fleet_name) |>
    unique() |>
    na.omit()

  if (length(fleet_names) == 0) {
    cli::cli_abort(c(
      "No fleets found in the provided {.var parameters}."
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

    data_distribution_names_for_fleet_i <- parameters |>
      dplyr::filter(fleet_name == fleet_names[i] & distribution_type == "Data") |>
      dplyr::pull(module_type)

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

    # Initialize age composition module if the data type includes "age_comp" and
    # if "AgeComp" exists in the data distribution specification
    if ("age_comp" %in% fleet_types &&
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

    # Initialize length composition module if the data type includes "length_comp" and
    # if "LengthComp" exists in the data distribution specification
    if ("length_comp" %in% fleet_types &&
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
      # TODO: need to remove linked_ids from the function and add module_id to the
      # parameters tibble
      linked_ids = fleet_module_ids
    )

    fleet_sd_input <- parameters |>
      dplyr::filter(fleet_name == fleet_names[i] & label == "log_sd") |>
      dplyr::mutate(
        label = "sd",
        value = exp(value)
      )

    if (length(fleet_sd_input) == 0) {
      cli::cli_abort(c(
        "Missing required inputs for `log_sd` in fleet `{fleet_name}`."
      ))
    }

    if ("index" %in% fleet_types &&
      "Index" %in% data_distribution_names_for_fleet_i) {
      fleet_index_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        # TODO: need to update family and match options from the distribution
        # column from the parameters tibble
        family = lognormal(link = "log"),
        sd = fleet_sd_input,
        data_type = "index"
      )
    }

    if ("landings" %in% fleet_types &&
      "Landings" %in% data_distribution_names_for_fleet_i) {
      fleet_landings_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        # TODO: need to update family and match options from the distribution
        # column from the parameters tibble
        family = lognormal(link = "log"),
        sd = fleet_sd_input,
        data_type = "landings"
      )
    }

    if ("age_comp" %in% fleet_types &&
      "AgeComp" %in% data_distribution_names_for_fleet_i) {
      fleet_agecomp_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        # TODO: need to update family and match options from the distribution
        # column from the parameters tibble
        family = multinomial(link = "logit"),
        data_type = "agecomp"
      )
    }

    if ("length_comp" %in% fleet_types &&
      "LengthComp" %in% data_distribution_names_for_fleet_i) {
      fleet_lengthcomp_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        # TODO: need to update family and match options from the distribution
        # column from the parameters tibble
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

  recruitment_process_input <- parameters |>
    dplyr::filter(module_name == "Recruitment" & distribution_type == "process")

  if (length(recruitment_process_input) == 0) {
    # TODO: need to revisit initialize_process_structure and add R tests
    recruitment_process <- initialize_process_structure(
      module = recruitment,
      par = "log_devs"
    )
  } else {
    pars <- recruitment_process_input |>
      dplyr::pull(distribution_link) |>
      unique()

    # Initialize_process_distribution for each par
    recruitment_distribution <- purrr::map(pars, function(par) {
      sd_input <- recruitment_process_input |>
        dplyr::filter(distribution_link == par & label == "log_sd")
      initialize_process_distribution(
        module = recruitment,
        par = par,
        # TODO: need to update family and match options from the distribution
        # column from the parameters tibble
        family = gaussian(),
        sd = sd_input,
        # TODO: need to remove is_random_effect and match options from the
        # estimation_type from the parameters tibble
        is_random_effect = FALSE
      )

      recruitment_process <- initialize_process_structure(
        module = recruitment,
        par = par
      )
    })
  }

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
    maturity = maturity$get_id(),
    fleets = purrr::map(fleet, \(x) x$get_id())
  )

  # Population
  population <- initialize_population(
    parameters = parameters,
    data = data,
    # TODO: need to remove linked_ids from the function and add module_id to the
    # parameters tibble
    linked_ids = population_module_ids
  )

  # Set-up TMB
  # Hard code to be a catch-at-age model
  fims_model <- methods::new(CatchAtAge)
  fims_model$AddPopulation(population$get_id())

  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameter_list <- list(
    parameters = list(
      p = get_fixed(),
      re = get_random()
    ),
    model = fims_model
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
  if (!tibble::is_tibble(module_input)) {
    cli::cli_abort("The {.var module_input} argument must be tibble.")
  }

  # Extract the value of the parameter vector
  field_value <- module_input |>
    dplyr::filter(label == field) |>
    dplyr::pull(value)

  field_estimation_type <- module_input |>
    dplyr::filter(label == field) |>
    dplyr::pull(estimation_type)

  # Check if both value and estimation information are present
  if (length(field_value) == 0 || length(field_estimation_type) == 0) {
    cli::cli_abort(c(
      "Missing value or estimation_type information for {.var field}."
    ))
  }
  # Resize the field in the module
  module[[field]]$resize(length(field_value))

  # Assign each value to the corresponding position in the parameter vector
  for (i in seq_along(field_value)) {
    module[[field]][i][["value"]] <- field_value[i]
    module[[field]][i][["estimation_type"]]$set(field_estimation_type[i])
  }
}
