#' Create a module and set the fields the parameters tibble gives for it
#'
#' @description
#' Creates the module named by `module_name`, then sets every field the tibble
#' has a row for. Values and estimation statuses come from the tibble; a field
#' takes as many elements as it has rows.
#'
#' Which module to create comes from the tibble as well: its `module_name` and
#' `module_type` columns name the module and its functional form, and the
#' creators take those values as they are.
#'
#' A label that is not a field of the module is an error, unless the row has a
#' distribution attached. The tibble carries rows for a module and for the
#' distribution attached to it --- a recruitment module has a `log_sd` row
#' belonging to its process distribution --- and those are consumed by the
#' distribution setup instead.
#'
#' Dimensions, links, and observed data are set by the `initialize_*()`
#' function for the module type, not here.
#'
#' @param parameters A tibble of parameter values and estimation statuses.
#' @param data A `FIMSFrame`.
#' @param module_name The module to build, matching the tibble's `module_name`.
#' @param fleet The fleet name, when the module belongs to one fleet.
#' @return
#' A [fims_module] with its tibble-supplied fields set.
#' @noRd
initialize_module <- function(parameters, data, module_name, fleet = NA_character_) {
  module_input <- parameters |>
    # Using !! to unquote the variables
    dplyr::filter(.data$module_name == !!module_name)

  if (!is.na(fleet)) {
    module_input <- module_input |>
      dplyr::filter(.data$fleet == !!fleet)
  }

  module_type <- module_input |>
    dplyr::pull(.data$module_type) |>
    unique()
  module_type <- module_type[!is.na(module_type)]

  if (length(module_type) > 1) {
    cli::cli_abort(c(
      "Expected one {.var module_type} for {.val {module_name}}.",
      "x" = "Found {.val {module_type}}."
    ))
  }

  maker <- .fims_module_names[[module_name]]
  if (is.null(maker)) {
    cli::cli_abort(c(
      "{.val {module_name}} is not a module FIMS knows how to create.",
      "i" = "Add it to {.var .fims_module_names} in {.file R/fims_interface.R}."
    ))
  }
  module <- maker[["create"]](
    if (length(module_type) == 1) module_type else NA_character_
  )

  # One call per label, so a field takes all of its rows at once.
  labels <- module_input |>
    dplyr::pull(.data$label) |>
    unique()
  labels <- labels[!is.na(labels)]

  for (label in labels) {
    rows <- dplyr::filter(module_input, .data$label == !!label)

    if (!has_variable_vector(module, label)) {
      # A label the module does not have is only allowed when a distribution is
      # attached to the row, because it belongs to that distribution rather
      # than to the module: a recruitment module carries a `log_sd` row for its
      # process distribution. Anything else is a label that nothing will read.
      if (all(is.na(rows[["distribution"]]))) {
        cli::cli_abort(c(
          "{.val {module_name}} has no field named {.val {label}}.",
          "x" = "No distribution is attached to that row, so nothing will
                 read it.",
          "i" = "Check the {.var label} column of the parameters tibble."
        ))
      }
      next
    }

    set_variable_vector(
      module, label, rows[["value"]], rows[["estimation_status"]]
    )
  }

  module
}

#' Initialize a recruitment module
#'
#' @description
#' Initializes a recruitment module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' recruitment initialization.
#' @inheritParams initialize_module
#' @return
#' The initialized recruitment module, as a [fims_module].
#' @noRd
initialize_recruitment <- function(parameters, data) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = "Recruitment"
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
#' The initialized growth module, as a [fims_module].
#' @noRd
initialize_growth <- function(parameters, data) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = "Growth"
  )

  # Growth has no rows in the parameters tibble: its inputs are data, not
  # estimated quantities, so they come from the FIMSFrame. Weights are stored
  # for each age in each year, plus one more year, hence n_ages * (n_years + 1).
  set_growth_n_years(module, get_n_years(data))
  set_numeric_vector(module, "ages", get_ages(data))
  set_numeric_vector(module, "weights", model_weight_at_age(data))

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
#' The initialized maturity module, as a [fims_module].
#' @noRd
initialize_maturity <- function(parameters, data) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = "Maturity"
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
#' @param growth,maturity,recruitment The process modules the population uses.
#' @param fleets A list of the fleet modules that operate on the population.
#' @return
#' The initialized population module, as a [fims_module].
#' @noRd
initialize_population <- function(parameters, data,
                                  growth, maturity, recruitment, fleets) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = "Population"
  )

  set_population_constants(module, get_n_years(data), get_n_ages(data))
  set_numeric_vector(module, "ages", get_ages(data))

  # Link up the recruitment, growth, and maturity modules with
  # this population module, then its fleets. The fleet count follows from the
  # fleets given here.
  set_population_processes(
    module,
    growth = growth,
    maturity = maturity,
    recruitment = recruitment
  )
  set_population_fleets(module, fleets)

  return(module)
}

#' Initialize a selectivity module.
#'
#' @description
#' Initializes a selectivity module by setting up fields. This function uses
#' the `initialize_module` function to handle specific requirements for
#' population initialization.
#'
#' For logistic selectivity, the curve can be either ascending or descending
#' based on the sign of the slope parameter. A positive slope creates an
#' ascending logistic curve where selectivity increases from 0 to 1 with
#' increasing values of the independent variable (e.g., age or size). A
#' negative slope creates a descending logistic curve where selectivity
#' decreases from 1 to 0.
#'
#' @inheritParams initialize_module
#' @param fleet A character. Name of the fleet to initialize.
#' @return
#' The initialized selectivity module, as a [fims_module].
#' @noRd
initialize_selectivity <- function(parameters, data, fleet) {
  module_name <- "Selectivity"
  module <- initialize_module(
    parameters = parameters,
    data = data,
    module_name = module_name,
    fleet = fleet
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
#' @param fleet A character. Name of the fleet to initialize.
#' @param selectivity The selectivity module the fleet uses.
#' @param catch,index,age_comp,length_comp The data modules holding this
#'   fleet's observations, or `NULL` where the fleet has no data of that kind.
#' @return
#' The initialized fleet module, as a [fims_module].
#' @noRd
initialize_fleet <- function(parameters, data, fleet, selectivity,
                             catch = NULL, index = NULL,
                             age_comp = NULL, length_comp = NULL) {
  module <- initialize_module(
    parameters = parameters,
    data = data,
    fleet = fleet,
    module_name = "Fleet"
  )

  set_fleet_constants(
    module, get_n_years(data), get_n_ages(data), get_n_lengths(data)
  )
  set_fleet_selectivity(module, selectivity)

  # A slot left NULL means the fleet has no data of that kind. Every slot is
  # replaced, so the links match the modules given here.
  set_fleet_observed_data(
    module,
    age_comp = age_comp,
    length_comp = length_comp,
    index = index,
    catch = catch
  )

  return(module)
}

#' Initialize a catch module
#'
#' @description
#' Initializes a catch module based on the provided data and fleet name.
#' @inheritParams initialize_module
#' @param fleet A character. Name of the fleet for which the catch
#'   module is initialized.
#' @return
#' The initialized catch module, as a [fims_module], or `NULL` if the fleet
#' has no catch data.
#' @noRd
initialize_catch <- function(data, fleet) {
  # Check if the specified fleet exists in the data
  fleet_exists <- fleet %in% get_fleets(data)
  if (!fleet_exists) {
    cli::cli_abort("Fleet {.var {fleet}} not found in the data object.")
  }

  fleet_type <- dplyr::filter(
    .data = as.data.frame(get_data(data)),
    .data$fleet == .env$fleet
  ) |>
    dplyr::distinct(.data$type) |>
    dplyr::pull(.data$type)

  if ("catch" %in% fleet_type) {
    module <- create_data("catch", get_n_years(data))
    set_data(module, model_catch(data, fleet))

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
#' @param fleet A character. Name of the fleet for which the index module
#'   is initialized.
#' @return
#' The initialized index module, as a [fims_module], or `NULL` if the fleet
#' has no index data.
#' @noRd
initialize_index <- function(data, fleet) {
  # Check if the specified fleet exists in the data
  fleet_exists <- fleet %in% get_fleets(data)
  if (!fleet_exists) {
    cli::cli_abort("Fleet {.var {fleet}} not found in the data object.")
  }

  fleet_type <- dplyr::filter(
    .data = as.data.frame(get_data(data)),
    .data$fleet == .env$fleet
  ) |>
    dplyr::distinct(.data$type) |>
    dplyr::pull(.data$type)

  if ("index" %in% fleet_type) {
    module <- create_data("index", get_n_years(data))
    set_data(module, model_index(data, fleet))

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
#' @param fleet A character specifying the name of the fleet for which
#'   composition data is initialized.
#' @param type A character specifying the composition type, where the default
#'   is `"AgeComp"`. At the moment, one can initialize `"AgeComp"` or
#'   `"LengthComp"` modules.
#' @return
#' The initialized composition module, as a [fims_module].
#' @noRd
initialize_comp <- function(data,
                            fleet,
                            type = c("AgeComp", "LengthComp")) {
  # Edit this list if a new type is added
  # Set up the specifics for the given type.
  comp_types <- list(
    "AgeComp" = list(
      "name" = "age_comp",
      "get_n_function" = get_n_ages,
      "m_comp" = model_age_comp
    ),
    "LengthComp" = list(
      "name" = "length_comp",
      "get_n_function" = get_n_lengths,
      "m_comp" = model_length_comp
    )
  )

  # Ensures the user input matches the options provided,
  #   if not, then match.arg() throws an error
  type <- match.arg(type)
  # Select the row in comp_types that matches the user's type selection
  comp <- comp_types[[type]]

  # Check if the specified fleet exists in the data
  fleet_exists <- fleet %in% get_fleets(data)
  if (!fleet_exists) {
    cli::cli_abort("Fleet {.var {fleet}} not found in the data object.")
  }

  get_function <- comp[["get_n_function"]]
  module <- create_data(
    comp[["name"]],
    get_n_years(data),
    get_function(data)
  )

  # Validate that the fleet's composition data is available
  comp_data <- comp[["m_comp"]](data, fleet)
  pretty_comp_name <- gsub("_comp", "-composition", comp[["name"]])
  if (is.null(comp_data) || length(comp_data) == 0) {
    cli::cli_abort(c(
      "The {pretty_comp_name} data for fleet {.var {fleet}} is
      unavailable or empty."
    ))
  }

  if (length(comp_data) != get_n_years(data) * get_function(data)) {
    bad_data_years <- get_data(data) |>
      dplyr::filter(
        .data$fleet == .env$fleet,
        .data$type == comp[["name"]]
      ) |>
      dplyr::count(.data$timing) |>
      dplyr::filter(.data$n != get_function(data)) |>
      dplyr::pull(.data$timing)

    cli::cli_abort(c(
      "The length of the `{comp[['name']]}`-composition data for fleet
      `{fleet}` does not match the expected dimensions.",
      i = "Expected length: {get_n_years(data) * get_function(data)}",
      i = "Actual length: {length(comp_data)}",
      i = "Number of -999 values: {sum(comp_data == -999)}",
      i = "Dates with invalid data: {bad_data_years}"
    ))
  }
  set_data(module, comp_data)

  return(module)
}

#' Initialize C++ modules via Rcpp for a FIMS model
#'
#' @description
#' This function uses information from a parameter data frame that stores the
#' model specifications and a`FIMSFrame` object that stores the data to
#' instantiate, i.e., create an instance of a class, the required C++ modules.
#' Several C++ modules are needed to run a FIMS model and the required modules
#' will be different for each model type. For example, for a catch-at-age
#' model one needs to instantiate recruitment, growth, and maturity modules and
#' at least one fleet and population module.
#'
#' @param parameters A tibble returned from [setup_default_parameters()]. It
#'   is the primary source of information for what is initialized. That is, if a
#'   fleet exists in the data but parameter information for how to specify
#'   selectivity for that fleet is not provided, then selectivity will not be
#'   initialized for that fleet.
#' @param data An S4 object with the `FIMSFrame` class, which is returned from
#'   [FIMSFrame()]. Passing the data is required because initialization of the
#'   modules requires passing the data and information regarding the uncertainty
#'   of that data, i.e., input sample sizes for the multinomial distribution.
#' @return
#' A list is returned with two elements, `parameters` and `model`. The list can
#' be passed to the `input` argument of [fit_fims()] to fit the model. The first
#' element of the list can also be passed to the `parameters` argument of
#' [TMB::MakeADFun()] if you wish to have more control over the model-fitting
#' process.
#' The model element of the returned list stores the instantiated C++ model
#' module, e.g., the results of `create_fishery_model("CatchAtAge")` for a
#' catch-at-age model.
#' It is important that you only have one FIMS model initialized in your R
#' workspace at a time. Thus, after you initialize and fit the model, you should
#' run [clear()].
#' @export
#' @seealso
#' * [setup_default_parameters()]
#' * [FIMSFrame()]
#' * [fit_fims()]
#' * [clear()]
#' @examples
#' \dontrun{
#' # Prepare data for FIMS model
#' data("data_big", package = "FIMS")
#' data_4_model <- FIMSFrame(data_big)
#' # Instantiate modules
#' parameters_list <- setup_default_parameters(data = data_4_model) |>
#'   initialize_fims(data = data_4_model)
#' clear()
#' }
initialize_fims <- function(parameters, data) {
  # Validate parameters input
  if (missing(parameters) || !tibble::is_tibble(parameters)) {
    cli::cli_abort("The {.var parameters} argument must be a tibble.")
  }

  # Check if estimation_status is within "assumed_known", "fixed_effect", "random_effect"
  # Validates supported estimation status to avoid errors later when
  #
  valid_estimation_status <- c("assumed_known", "fixed_effects", "random_effects")
  invalid_estimation_status <- parameters |>
    dplyr::filter(!.data$estimation_status %in% valid_estimation_status) |>
    dplyr::pull(.data$estimation_status) |>
    unique() |>
    na.omit()

  if (length(invalid_estimation_status) > 0) {
    cli::cli_abort(c(
      "The `estimation_status` must be one of: {valid_estimation_status}.",
      i = "Invalid values found: {invalid_estimation_status}."
    ))
  }

  # Clear any previous FIMS settings
  clear()

  fleets <- parameters |>
    dplyr::pull(.data$fleet) |>
    unique() |>
    na.omit()

  if (length(fleets) == 0) {
    cli::cli_abort(c(
      "No fleets found in the provided {.var parameters}."
    ))
  }

  # Initialize lists to store fleet-related objects
  fleet <- fleet_selectivity <-
    fleet_catch <- fleet_catch_distribution <-
    fleet_index <- fleet_index_distribution <-
    fleet_age_comp <- fleet_agecomp_distribution <-
    fleet_length_comp <- fleet_lengthcomp_distribution <-
    vector("list", length(fleets))

  for (i in seq_along(fleets)) {
    fleet_selectivity[[i]] <- initialize_selectivity(
      parameters = parameters,
      data = data,
      fleet = fleets[i]
    )

    fleet_types <- get_data(data) |>
      dplyr::filter(.data$fleet == .env$fleets[i]) |>
      dplyr::pull(.data$type) |>
      unique()

    # Initialize catch module if the data type includes "catch" and
    # if "Catch" exists in the data distribution specification
    if ("catch" %in% fleet_types) {
      # Initialize catch module for the current fleet
      fleet_catch[[i]] <- initialize_catch(
        data = data,
        fleet = fleets[i]
      )

    }

    # Initialize index module if the data type includes "index" and
    # if "Index" exists in the data distribution specification
    if ("index" %in% fleet_types) {
      # Initialize index module for the current fleet
      fleet_index[[i]] <- initialize_index(
        data = data,
        fleet = fleets[i]
      )

    }

    # Initialize age composition module if the data type includes "age_comp" and
    # if "AgeComp" exists in the data distribution specification
    if ("age_comp" %in% fleet_types) {
      # Initialize age composition module for the current fleet
      fleet_age_comp[[i]] <- initialize_comp(
        data = data,
        fleet = fleets[i],
        type = "AgeComp"
      )

    }

    # Initialize length composition module if the data type includes "length_comp" and
    # if "LengthComp" exists in the data distribution specification
    if ("length_comp" %in% fleet_types) {
      # Initialize length composition module for the current fleet
      fleet_length_comp[[i]] <- initialize_comp(
        data = data,
        fleet = fleets[i],
        type = "LengthComp"
      )

    }

    fleet[[i]] <- initialize_fleet(
      parameters = parameters,
      data = data,
      fleet = fleets[i],
      selectivity = fleet_selectivity[[i]],
      catch = fleet_catch[[i]],
      index = fleet_index[[i]],
      age_comp = fleet_age_comp[[i]],
      length_comp = fleet_length_comp[[i]]
    )

    if ("index" %in% fleet_types) {
      fleet_index_distribution[[i]] <- initialize_data_distribution(
        data_type = "index",
        module = fleet[[i]],
        # TODO: need to update family and match options from the distribution
        # column from the parameters tibble
        uncertainty = get_data(data) |>
          dplyr::filter(
            .data$fleet == .env$fleets[i] &
              .data$type == "index"
          ) |>
          dplyr::pull(dplyr::all_of("uncertainty"))
      )
    }

    if ("catch" %in% fleet_types) {
      fleet_catch_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        data_type = "catch",
        uncertainty = get_data(data) |>
          dplyr::filter(
            .data$fleet == .env$fleets[i] &
              .data$type == "catch"
          ) |>
          dplyr::pull(dplyr::all_of("uncertainty"))
      )
    }

    if ("age_comp" %in% fleet_types) {
      fleet_agecomp_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        data_type = "age_comp",
        uncertainty = get_data(data) |>
          dplyr::filter(
            .data$fleet == .env$fleets[i] &
              .data$type == "age_comp"
          ) |>
          dplyr::pull(dplyr::all_of("uncertainty"))
      )
    }

    if ("length_comp" %in% fleet_types) {
      fleet_lengthcomp_distribution[[i]] <- initialize_data_distribution(
        module = fleet[[i]],
        data_type = "length_comp",
        uncertainty = get_data(data) |>
          dplyr::filter(
            .data$fleet == .env$fleets[i] &
              .data$type == "length_comp"
          ) |>
          dplyr::pull(dplyr::all_of("uncertainty"))
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
    dplyr::filter(.data$module_name == "Recruitment" & .data$distribution_type == "process" & !is.na(.data$distribution))
  if (recruitment_process_input |> nrow() == 0) {
    process_par <- parameters |>
      dplyr::filter(.data$module_name == "Recruitment" & (.data$label == "log_devs" | .data$label == "log_r"))
    process_par_name <- process_par |>
      dplyr::pull(.data$label) |>
      unique()
    if (any(process_par[["estimation_status"]] != "assumed_known")) {
      cli::cli_abort(c(
        x = "Missing required inputs for recruitment process random or
        fixed effects.",
        i = "There is no distribution process specified for the
        {.var {process_par_name}} variable in the recruitment module.",
        i = "Implement either one of the following options to resolve this
        error:",
        i = "1. Set a distribution and distribution_type for the Recruitment
        {.var module_name} in configurations tibble.",
        i = "2. Set the estimation_status for the recruitment
        {.var {process_par_name}} variable in the parameter tibble to
        {.var assumed_known}."
      ))
    }
    # TODO: need to revisit initialize_process_structure and add R tests
    recruitment_process <- initialize_process_structure(
      module = recruitment,
      par = "log_devs"
    )
  } else {
    par <- recruitment_process_input |>
      dplyr::filter(.data$label != "log_sd") |>
      dplyr::pull(.data$label) |>
      unique()

    if (length(par) == 0) {
      cli::cli_abort(c(
        x = "Missing required inputs for recruitment process random or
        fixed effects.",
        i = "There is a distribution specified for the Recruitment
        {.var module_name} in the configurations tibble, but no parameters are
        specified for the recruitment process in the parameters tibble.",
        i = "Implement either one of the following options to resolve this
        error:",
        i = "1. Add parameter, {.var log_devs} or {.var log_r}, for the
        recruitment process in the parameters tibble with an estimation_status of
        random_effects or fixed_effects.",
        i = "2. Set the distribution for the Recruitment distribution and
        distribution_type to {.var NA} in the configurations tibble."
      ))
    }

    if (any(recruitment_process_input |> dplyr::filter(.data$label != "log_sd") |>
      dplyr::pull(.data$estimation_status) == "assumed_known")) {
      cli::cli_abort(c(
        x = "Missing required inputs for recruitment process random or
        fixed effects.",
        i = "The estimation status for {.var {par}} is assumed_known, but there is a
        distribution specified for the Recruitment {.var module_name} in the
        configurations tibble.",
        i = "Implement either one of the following options to resolve this
        error:",
        i = "1. Set the distribution for the Recruitment distribution and
        distribution_type to {.var NA} in the configurations tibble.",
        i = "2. Set the estimation_status for the recruitment {.var {par}} in the
        parameter tibble to {.var random_effects} or {.var fixed_effects}."
      ))
    }


    # Initialize_process_distribution
    sd_input <- recruitment_process_input |>
      dplyr::filter(.data$label == "log_sd") |>
      dplyr::mutate(
        label = "sd",
        value = exp(.data$value)
      )
    recruitment_distribution <- initialize_process_distribution(
      module = recruitment,
      par = par,
      # TODO: need to update family and match options from the distribution
      # column from the parameters tibble
      family = gaussian(),
      sd = sd_input
    )

    recruitment_process <- initialize_process_structure(
      module = recruitment,
      par = par
    )
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

  # Population
  population <- initialize_population(
    parameters = parameters,
    data = data,
    growth = growth,
    maturity = maturity,
    recruitment = recruitment,
    fleets = fleet
  )

  # Set-up TMB
  # Hard code to be a catch-at-age model
  fims_model <- create_fishery_model("CatchAtAge")
  set_model_populations(fims_model, list(population))

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
