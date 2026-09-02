# User-facing wrappers around the module functions that come from C++.
#
# Each C++ function takes an external pointer and, because Rcpp::XPtr does not
# check what it was handed, will accept a pointer to the wrong kind of module
# and behave unpredictably. These wrappers take a [fims_module], pick the right
# pointer from it, and check the module's name first, so a mistake produces
# an R error instead. Nothing here needs `$pointer` or `$base_pointer`.
#
# See R/fims_interface.R for the create_*() functions that produce modules.

#' Check that an argument is a module, and optionally of a given module name
#'
#' @param module The object to check.
#' @param module_name Character vector of acceptable module names, or NULL
#'   for any.
#' @param arg The argument name to use in the error message.
#' @return `module`, invisibly.
#' @noRd
check_module <- function(module, module_name = NULL, arg = "module") {
  if (!inherits(module, "fims_module")) {
    cli::cli_abort(c(
      "{.arg {arg}} must be a FIMS module.",
      "x" = "Got {.obj_type_friendly {module}}.",
      "i" = "Modules come from the {.code create_*()} functions."
    ))
  }
  if (!is.null(module_name) && !module[["module_name"]] %in% module_name) {
    cli::cli_abort(c(
      "{.arg {arg}} must be a {.val {module_name}} module.",
      "x" = "Got a {.val {module[['module_name']]}} module."
    ))
  }
  invisible(module)
}

#' Take an ID from either a module or a number
#'
#' @param x A [fims_module] or a module ID.
#' @param module_name Acceptable module names when `x` is a module.
#' @param arg The argument name to use in the error message.
#' @return An integer ID, or -999L for NULL, which FIMS reads as "no link".
#' @noRd
module_id <- function(x, module_name = NULL, arg = "x") {
  if (is.null(x)) {
    return(-999L)
  }
  if (inherits(x, "fims_module")) {
    check_module(x, module_name, arg)
    return(x[["id"]])
  }
  as.integer(x)
}

# ---- Fields shared by every module ------------------------------------------

#' Set a module's values by field name
#'
#' @description
#' `set_variable_vector()` writes a field that the model can estimate. Each
#' element carries an estimation status, and the field has an ID that a
#' distribution can refer to, so this is the setter for anything that might be
#' estimated or that a distribution applies to.
#'
#' `set_numeric_vector()` writes a field that is plain numbers with no
#' estimation status: observations, uncertainties, ages, empirical weights.
#'
#' Both replace the whole field, resizing it to match `values`.
#'
#' @param module A [fims_module].
#' @param name The field name, as it appears in the module's documentation.
#' @param values The values to write.
#' @param estimation_status How the model should treat each element: one of
#'   `"assumed_known"`, `"fixed_effects"`, `"random_effects"`, or
#'   `"derived_quantity"`. Give one status to apply to every element, or one
#'   per element. It is set here rather than fixed on the field, so the same
#'   quantity can be estimated in one model and assumed known in another.
#' @return
#' `module`, invisibly, so calls can be chained.
#' @seealso [set_data()], [get_variable_vector_id()]
#' @export
#' @rdname set_field
set_variable_vector <- function(module, name, values, estimation_status) {
  check_module(module)
  set_variable_vector_(
    module[["base_pointer"]], name, values, estimation_status
  )
  invisible(module)
}

#' @export
#' @rdname set_field
set_numeric_vector <- function(module, name, values) {
  check_module(module)
  set_numeric_vector_(module[["base_pointer"]], name, values)
  invisible(module)
}

#' Set a data module's observations
#'
#' @description
#' Writes the observed values, and optionally their uncertainty, on any of the
#' four data types. Every data module names these fields the same way, so this
#' one function covers age composition, length composition, index, and catch.
#'
#' @param module A data [fims_module], from [create_data()].
#' @param values The observations. For composition data pass the matrix
#'   transposed and flattened, so that it reads year by year.
#' @param uncertainty Uncertainty for each observation, or `NULL` to leave it
#'   as it is.
#' @return
#' `module`, invisibly.
#' @seealso [set_variable_vector()]
#' @export
set_data <- function(module, values, uncertainty = NULL) {
  check_module(module, "Data", "module")
  set_numeric_vector_(module[["base_pointer"]], "values", values)
  if (!is.null(uncertainty)) {
    set_numeric_vector_(module[["base_pointer"]], "uncertainty", uncertainty)
  }
  invisible(module)
}

#' Read a module's values by field name
#'
#' @description
#' The counterparts to [set_variable_vector()] and [set_numeric_vector()]: they
#' return what those functions wrote, so a value can be checked after setting
#' it.
#'
#' `get_variable_vector()` returns the estimation statuses alongside the values,
#' because they are set together.
#'
#' Both report the values given as *input*. After a model is fitted the
#' estimates live in the model rather than in the module, so these do not change
#' when a model is fitted --- use [get_output()] or the fitted object for
#' estimates.
#'
#' @param module A [fims_module].
#' @param name The field name.
#' @return
#' For `get_variable_vector()`, a list with `values` and `estimation_status`,
#' one status per element. For `get_numeric_vector()`, the values.
#' @seealso [set_variable_vector()], [set_numeric_vector()]
#' @export
#' @rdname get_field
get_variable_vector <- function(module, name) {
  check_module(module)
  get_variable_vector_(module[["base_pointer"]], name)
}

#' @export
#' @rdname get_field
get_numeric_vector <- function(module, name) {
  check_module(module)
  get_numeric_vector_(module[["base_pointer"]], name)
}

#' Ask whether a module has a field by this name
#'
#' @description
#' Useful when deciding what to set from a table of parameters: the table may
#' carry rows for a module and for the distribution attached to it, and only
#' the module's own rows can be set on it.
#'
#' @param module A [fims_module].
#' @param name The field name.
#' @return
#' `TRUE` if the module has a field by that name.
#' @seealso [set_variable_vector()]
#' @export
has_variable_vector <- function(module, name) {
  check_module(module)
  has_variable_vector_(module[["base_pointer"]], name)
}

#' Get the ID of one of a module's estimable fields
#'
#' @description
#' Distributions refer to a quantity by the ID of the field holding it, so this
#' is how you name the thing a distribution applies to. It works for fields the
#' model reads and for fields it computes: a likelihood on expected catch names
#' a computed quantity, a prior names an input.
#'
#' @param module A [fims_module].
#' @param name The field name.
#' @return
#' The field's ID, as an integer.
#' @seealso [set_distribution_links()]
#' @export
get_variable_vector_id <- function(module, name) {
  check_module(module)
  get_variable_vector_id_(module[["base_pointer"]], name)
}

#' Get a module's ID
#'
#' @param module A [fims_module].
#' @return
#' The module's ID, as an integer. The same value as `module[["id"]]`.
#' @export
get_module_id <- function(module) {
  check_module(module)
  get_module_id_(module[["base_pointer"]])
}

# ---- Fleet -------------------------------------------------------------------

#' Set a fleet's dimensions
#'
#' @param fleet A fleet [fims_module].
#' @param n_years,n_ages,n_lengths The number of years, age bins, and length
#'   bins the fleet's data cover.
#' @return
#' `fleet`, invisibly.
#' @export
set_fleet_constants <- function(fleet, n_years, n_ages, n_lengths) {
  check_module(fleet, "Fleet", "fleet")
  set_fleet_constants_(
    fleet[["pointer"]],
    as.integer(n_years), as.integer(n_ages), as.integer(n_lengths)
  )
  invisible(fleet)
}

#' Set or get a fleet's name
#'
#' @param fleet A fleet [fims_module].
#' @param name The name to record, used to label the fleet in the output.
#' @return
#' For `set_fleet_name()`, `fleet` invisibly. For `get_fleet_name()`, the name.
#' @export
#' @rdname fleet_name
set_fleet_name <- function(fleet, name) {
  check_module(fleet, "Fleet", "fleet")
  set_fleet_name_(fleet[["pointer"]], name)
  invisible(fleet)
}

#' @export
#' @rdname fleet_name
get_fleet_name <- function(fleet) {
  check_module(fleet, "Fleet", "fleet")
  get_fleet_name_(fleet[["pointer"]])
}

#' Set the units a fleet's observations are in
#'
#' @param fleet A fleet [fims_module].
#' @param catch_units,index_units The units of the observed catch and index.
#' @return
#' `fleet`, invisibly.
#' @export
set_fleet_units <- function(fleet, catch_units, index_units) {
  check_module(fleet, "Fleet", "fleet")
  set_fleet_units_(fleet[["pointer"]], catch_units, index_units)
  invisible(fleet)
}

#' Link a fleet to its selectivity curve
#'
#' @param fleet A fleet [fims_module].
#' @param selectivity A selectivity [fims_module], or its ID.
#' @return
#' For `set_fleet_selectivity()`, `fleet` invisibly. For
#' `get_fleet_selectivity_id()`, the linked selectivity's ID, or -999 if the
#' fleet has none.
#' @export
#' @rdname fleet_selectivity
set_fleet_selectivity <- function(fleet, selectivity) {
  check_module(fleet, "Fleet", "fleet")
  set_fleet_selectivity_id_(
    fleet[["pointer"]], module_id(selectivity, "Selectivity", "selectivity")
  )
  invisible(fleet)
}

#' @export
#' @rdname fleet_selectivity
get_fleet_selectivity_id <- function(fleet) {
  check_module(fleet, "Fleet", "fleet")
  get_fleet_selectivity_id_(fleet[["pointer"]])
}

#' Link a fleet to its observed data
#'
#' @description
#' Records which data modules hold this fleet's observations. Every slot is
#' replaced, so a slot left as `NULL` means the fleet has no data of that kind.
#'
#' @param fleet A fleet [fims_module].
#' @param age_comp,length_comp,index,catch The data [fims_module] holding each
#'   kind of observation, or its ID, or `NULL` for none.
#' @return
#' For `set_fleet_observed_data()`, `fleet` invisibly. For
#' `get_fleet_observed_data_ids()`, a named integer vector with elements
#' `agecomp`, `lengthcomp`, `index`, and `catch`, where -999 means none.
#' @export
#' @rdname fleet_observed_data
set_fleet_observed_data <- function(fleet,
                                    age_comp = NULL,
                                    length_comp = NULL,
                                    index = NULL,
                                    catch = NULL) {
  check_module(fleet, "Fleet", "fleet")
  set_fleet_observed_data_ids_(
    fleet[["pointer"]],
    module_id(age_comp, "Data", "age_comp"),
    module_id(length_comp, "Data", "length_comp"),
    module_id(index, "Data", "index"),
    module_id(catch, "Data", "catch")
  )
  invisible(fleet)
}

#' @export
#' @rdname fleet_observed_data
get_fleet_observed_data_ids <- function(fleet) {
  check_module(fleet, "Fleet", "fleet")
  get_fleet_observed_data_ids_(fleet[["pointer"]])
}

# ---- Population --------------------------------------------------------------

#' Set a population's dimensions
#'
#' @param population A population [fims_module].
#' @param n_years,n_ages The number of years and age bins the population is
#'   modeled over. Populations are age-structured; length structure belongs to
#'   the fleets, which convert from age with an age-to-length matrix.
#' @return
#' `population`, invisibly.
#' @export
set_population_constants <- function(population, n_years, n_ages) {
  check_module(population, "Population", "population")
  set_population_constants_(
    population[["pointer"]],
    as.integer(n_years), as.integer(n_ages)
  )
  invisible(population)
}

#' Set or get a population's name
#'
#' @param population A population [fims_module].
#' @param name The name to record, used to label the population in the output.
#' @return
#' For `set_population_name()`, `population` invisibly. For
#' `get_population_name()`, the name.
#' @export
#' @rdname population_name
set_population_name <- function(population, name) {
  check_module(population, "Population", "population")
  set_population_name_(population[["pointer"]], name)
  invisible(population)
}

#' @export
#' @rdname population_name
get_population_name <- function(population) {
  check_module(population, "Population", "population")
  get_population_name_(population[["pointer"]])
}

#' Link a population to its growth, maturity, and recruitment
#'
#' @description
#' Records which modules supply the population's processes. Every link is
#' replaced, so a link left as `NULL` is cleared.
#'
#' @param population A population [fims_module].
#' @param maturity,growth,recruitment The [fims_module] for each process, or
#'   its ID, or `NULL` for none.
#' @param recruitment_err The module supplying recruitment deviations, if the
#'   model uses a separate one.
#' @return
#' For `set_population_processes()`, `population` invisibly. For
#' `get_population_process_ids()`, a named integer vector with elements
#' `maturity`, `growth`, `recruitment`, and `recruitment_err`, where -999 means
#' the link is not set.
#' @export
#' @rdname population_processes
set_population_processes <- function(population,
                                     maturity = NULL,
                                     growth = NULL,
                                     recruitment = NULL,
                                     recruitment_err = NULL) {
  check_module(population, "Population", "population")
  set_population_process_ids_(
    population[["pointer"]],
    module_id(maturity, "Maturity", "maturity"),
    module_id(growth, "Growth", "growth"),
    module_id(recruitment, "Recruitment", "recruitment"),
    module_id(recruitment_err, "Recruitment", "recruitment_err")
  )
  invisible(population)
}

#' @export
#' @rdname population_processes
get_population_process_ids <- function(population) {
  check_module(population, "Population", "population")
  get_population_process_ids_(population[["pointer"]])
}

#' Link a population to its fleets
#'
#' @description
#' Replaces the whole set of fleets, which is what allows a fleet to be removed
#' as well as added. The population's fleet count follows from this list, so it
#' cannot disagree with it.
#'
#' @param population A population [fims_module].
#' @param fleets A list of fleet [fims_module]s.
#' @return
#' `population`, invisibly.
#' @export
set_population_fleets <- function(population, fleets) {
  check_module(population, "Population", "population")
  if (!is.list(fleets)) {
    fleets <- list(fleets)
  }
  for (i in seq_along(fleets)) {
    check_module(fleets[[i]], "Fleet", paste0("fleets[[", i, "]]"))
  }
  set_population_fleets_(
    population[["pointer"]],
    lapply(fleets, `[[`, "pointer")
  )
  invisible(population)
}

# ---- Fishery model -----------------------------------------------------------

#' Link a fishery model to its populations
#'
#' @description
#' Replaces the whole set of populations, so a population can be removed as
#' well as added.
#'
#' @param model A fishery model [fims_module], from [create_fishery_model()].
#' @param populations A list of population [fims_module]s.
#' @return
#' `model`, invisibly.
#' @export
set_model_populations <- function(model, populations) {
  check_module(model, "Model", "model")
  if (!is.list(populations)) {
    populations <- list(populations)
  }
  for (i in seq_along(populations)) {
    check_module(
      populations[[i]], "Population", paste0("populations[[", i, "]]")
    )
  }
  set_model_populations_(
    model[["pointer"]],
    lapply(populations, `[[`, "pointer")
  )
  invisible(model)
}

#' Turn a fishery model's reporting on or off
#'
#' @param model A fishery model [fims_module].
#' @param report `TRUE` to report, `FALSE` to skip it.
#' @return
#' For `do_model_reporting()`, `model` invisibly. For `is_model_reporting()`,
#' `TRUE` or `FALSE`.
#' @export
#' @rdname model_reporting
do_model_reporting <- function(model, report) {
  check_module(model, "Model", "model")
  do_model_reporting_(model[["pointer"]], report)
  invisible(model)
}

#' @export
#' @rdname model_reporting
is_model_reporting <- function(model) {
  check_module(model, "Model", "model")
  is_model_reporting_(model[["pointer"]])
}

#' Get a fishery model's output as JSON
#'
#' @description
#' Returns the model's results as a JSON string, which is what
#' `reshape_json_estimates()` and the other reshaping functions read.
#'
#' The model holds only the IDs of the modules it describes, so those modules
#' have to be supplied. By default every module created since the last
#' [clear()] is used, which is what you want unless you are deliberately
#' describing a subset.
#'
#' This reports the model as it currently stands. After fitting, use
#' [get_model_output()] on the returned `FIMSFit` to read the output recorded
#' at fit time.
#'
#' @param model A fishery model [fims_module].
#' @param modules A list of [fims_module]s to describe, or `NULL` to use every
#'   registered module.
#' @return
#' A JSON string.
#' @export
get_output <- function(model, modules = NULL) {
  check_module(model, "Model", "model")
  pointers <- if (is.null(modules)) {
    lapply(.fims_registry[["objects"]], `[[`, "base_pointer")
  } else {
    if (!is.list(modules)) {
      modules <- list(modules)
    }
    for (i in seq_along(modules)) {
      check_module(modules[[i]], arg = paste0("modules[[", i, "]]"))
    }
    lapply(modules, `[[`, "base_pointer")
  }
  get_model_output_(model[["pointer"]], pointers)
}

# ---- Distribution ------------------------------------------------------------

#' Link a distribution to the data it applies to
#'
#' @param distribution A distribution [fims_module].
#' @param data A data [fims_module], or its ID.
#' @return
#' `distribution`, invisibly.
#' @export
set_distribution_observed_data <- function(distribution, data) {
  check_module(distribution, "Distribution", "distribution")
  set_distribution_observed_data_(
    distribution[["pointer"]], module_id(data, "Data", "data")
  )
  invisible(distribution)
}

#' Link a distribution to the quantities it applies to
#'
#' @description
#' Names the model quantities this distribution is evaluated against, by the
#' IDs from [get_variable_vector_id()].
#'
#' @param distribution A distribution [fims_module].
#' @param input_type What the link is for, such as `"data"`,
#'   `"random_effects"`, or `"prior"`.
#' @param ids One or more field IDs, from [get_variable_vector_id()].
#' @return
#' `distribution`, invisibly.
#' @seealso [get_variable_vector_id()]
#' @export
set_distribution_links <- function(distribution, input_type, ids) {
  check_module(distribution, "Distribution", "distribution")
  set_distribution_links_(
    distribution[["pointer"]], input_type, as.integer(ids)
  )
  invisible(distribution)
}

#' Record a note on a distribution
#'
#' @param distribution A distribution [fims_module].
#' @param note Text to label the distribution with in the output.
#' @return
#' `distribution`, invisibly.
#' @export
set_distribution_note <- function(distribution, note) {
  check_module(distribution, "Distribution", "distribution")
  set_distribution_note_(distribution[["pointer"]], note)
  invisible(distribution)
}

#' Set a distribution's mean to a fixed value
#'
#' @param distribution A distribution [fims_module].
#' @param value The mean to use.
#' @return
#' `distribution`, invisibly.
#' @export
set_distribution_fixed_mean <- function(distribution, value) {
  check_module(distribution, "Distribution", "distribution")
  set_distribution_fixed_mean_(distribution[["pointer"]], value)
  invisible(distribution)
}

#' Evaluate a distribution
#'
#' @param distribution A distribution [fims_module].
#' @return
#' The log density, as a numeric value.
#' @export
evaluate_distribution <- function(distribution) {
  check_module(distribution, "Distribution", "distribution")
  evaluate_distribution_(distribution[["pointer"]])
}

# ---- Recruitment, growth, maturity, selectivity ------------------------------

#' Link a recruitment module to its process
#'
#' @param recruitment A recruitment [fims_module].
#' @param process The recruitment process [fims_module], or its ID.
#' @return
#' For `set_recruitment_process()`, `recruitment` invisibly. For
#' `get_recruitment_process_id()`, the linked process's ID.
#' @export
#' @rdname recruitment_process
set_recruitment_process <- function(recruitment, process) {
  check_module(recruitment, "Recruitment", "recruitment")
  set_recruitment_process_id_(
    recruitment[["pointer"]], module_id(process, "Recruitment", "process")
  )
  invisible(recruitment)
}

#' @export
#' @rdname recruitment_process
get_recruitment_process_id <- function(recruitment) {
  check_module(recruitment, "Recruitment", "recruitment")
  get_recruitment_process_id_(recruitment[["pointer"]])
}

#' Set the number of years a module covers
#'
#' @param recruitment A recruitment [fims_module].
#' @param growth A growth [fims_module].
#' @param n_years The number of years.
#' @return
#' The module, invisibly.
#' @export
#' @rdname set_n_years
set_recruitment_n_years <- function(recruitment, n_years) {
  check_module(recruitment, "Recruitment", "recruitment")
  set_recruitment_n_years_(recruitment[["pointer"]], as.integer(n_years))
  invisible(recruitment)
}

#' @export
#' @rdname set_n_years
set_growth_n_years <- function(growth, n_years) {
  check_module(growth, "Growth", "growth")
  set_growth_n_years_(growth[["pointer"]], as.integer(n_years))
  invisible(growth)
}

#' Evaluate a process module at a point
#'
#' @description
#' Computes what the module returns for one input, without building a model.
#' Useful for checking that a curve has the shape you expect before using it.
#'
#' @param growth A growth [fims_module].
#' @param maturity A maturity [fims_module].
#' @param selectivity A selectivity [fims_module].
#' @param recruitment A recruitment [fims_module].
#' @param age The age to evaluate growth at.
#' @param x The age or length to evaluate maturity or selectivity at.
#' @param spawners The spawning biomass.
#' @param ssbzero The unfished spawning biomass.
#' @param value The recruitment deviation to evaluate the process at.
#' @return
#' A numeric value.
#' @export
#' @rdname evaluate_module
evaluate_growth <- function(growth, age) {
  check_module(growth, "Growth", "growth")
  evaluate_growth_(growth[["pointer"]], age)
}

#' @export
#' @rdname evaluate_module
evaluate_maturity <- function(maturity, x) {
  check_module(maturity, "Maturity", "maturity")
  evaluate_maturity_(maturity[["pointer"]], x)
}

#' @export
#' @rdname evaluate_module
evaluate_selectivity <- function(selectivity, x) {
  check_module(selectivity, "Selectivity", "selectivity")
  evaluate_selectivity_(selectivity[["pointer"]], x)
}

#' @export
#' @rdname evaluate_module
evaluate_recruitment_mean <- function(recruitment, spawners, ssbzero) {
  check_module(recruitment, "Recruitment", "recruitment")
  evaluate_recruitment_mean_(recruitment[["pointer"]], spawners, ssbzero)
}

#' @export
#' @rdname evaluate_module
evaluate_recruitment_process <- function(recruitment, value) {
  check_module(recruitment, "Recruitment", "recruitment")
  evaluate_recruitment_process_(recruitment[["pointer"]], value)
}
