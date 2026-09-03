# R-side module registry for the Rcpp::XPtr interface.
#
# Modules are reached only through the pointers R holds, so the list of modules
# to build a model from is assembled here. 
#
# Naming: a trailing underscore marks a function that comes from the C++ side.
# create_age_comp() is the R function a user calls, and the one underneath it
# is create_data_interface_(). The R functions own the clean names.

# Private package-level registry. Not exported and not user-visible: an
# environment rather than a list so the create_*() functions can add to it
# without having to return it.
.fims_registry <- new.env(parent = emptyenv())
.fims_registry[["objects"]] <- list()
.fims_registry[["metadata"]] <- list()
# The handle CreateTMBModel() last returned, or NULL if no model is built.
.fims_registry[["model_handle"]] <- NULL

#' A FIMS module
#'
#' @description
#' Every `create_*()` function returns a `fims_module` --- a fleet, 
#' a population, a selectivity curve --- that lives on the C++ side. 
#' The return from `create_*()` points to the C++ module directly (not a copy). 
#'
#' That reference behaviour is worth knowing, because it differs from most R
#' objects. If you write `b <- a`, you have not made a second module. Both
#' names refer to the same one, and a value set through `b` is visible through
#' `a`. Modules are changed by passing them to a setter, never by assigning to
#' their parts, and you should not edit the fields of a `fims_module`
#' yourself.
#'
#' @section Fields:
#' `id` is the number FIMS gave this module. Modules refer to each other by
#' number rather than by name, so this is the value to pass when linking one
#' module to another --- telling a fleet which selectivity curve to use, for
#' instance. `type` is a label for what kind of module it is, used when
#' `describe_model()` prints a summary.
#'
#' `pointer` and `base_pointer` are two ways of referring to the same module,
#' and functions differ in which one they expect. The rule of thumb is that a
#' function named for a particular kind of module wants `pointer`, and a
#' function that works on any module wants `base_pointer`:
#'
#' ```r
#' set_population_constants_(population$pointer, n_years = 30, ...)
#' set_variable_vector_(population$base_pointer, "log_M", values, status)
#' ```
#'
#' `module_name` and `module_type` say which kind of module this is, matching
#' the `module_name` and `module_type` columns of the parameters tibble.
#' `module_type` is `NA` for a fleet or a population, which have one kind each.
#'
#' @section After clear():
#' [clear()] invalidates every module. The R variable holding one still exists,
#' but the module behind it is gone, so using it afterwards raises an error
#' rather than quietly building a model that is missing a piece. Create modules
#' again after a `clear()` rather than reusing the old ones.
#'
#' @name fims_module
#' @seealso [CreateTMBModel()], [describe_model()], [clear()]
NULL

# The two C++ functions that differ per module name. Both are wrapped in
# closures rather than referenced directly, because the Rcpp module is not
# loaded until .onLoad() runs, well after this file is sourced.
#
# create   makes a module of this name, taking the module_type where there is
#          more than one. Used by initialize_module(), which reads both from
#          the parameters tibble.
# to_base  produces the base-class pointer used by everything that works on any
#          module, and by CreateTMBModel().
# release  invalidates the module's own pointer. Releasing deletes through the
#          pointer's own type, so this cannot be shared between module names.
.fims_module_names <- list(
  Data = list(
    create = function(type, ...) create_data(type, ...),
    to_base = function(pointer) data_to_fims_xptr_(pointer),
    release = function(pointer) release_data_(pointer)
  ),
  Fleet = list(
    create = function(type, ...) create_fleet(),
    to_base = function(pointer) fleet_to_fims_xptr_(pointer),
    release = function(pointer) release_fleet_(pointer)
  ),
  Growth = list(
    create = function(type, ...) create_growth(type),
    to_base = function(pointer) growth_to_fims_xptr_(pointer),
    release = function(pointer) release_growth_(pointer)
  ),
  Maturity = list(
    create = function(type, ...) create_maturity(type),
    to_base = function(pointer) maturity_to_fims_xptr_(pointer),
    release = function(pointer) release_maturity_(pointer)
  ),
  Population = list(
    create = function(type, ...) create_population(),
    to_base = function(pointer) population_to_fims_xptr_(pointer),
    release = function(pointer) release_population_(pointer)
  ),
  Recruitment = list(
    create = function(type, ...) create_recruitment(type),
    to_base = function(pointer) recruitment_to_fims_xptr_(pointer),
    release = function(pointer) release_recruitment_(pointer)
  ),
  Selectivity = list(
    create = function(type, ...) create_selectivity(type),
    to_base = function(pointer) selectivity_to_fims_xptr_(pointer),
    release = function(pointer) release_selectivity_(pointer)
  ),
  Distribution = list(
    create = function(type, ...) create_distribution(type),
    to_base = function(pointer) distribution_to_fims_xptr_(pointer),
    release = function(pointer) release_distribution_(pointer)
  ),
  Model = list(
    create = function(type, ...) create_fishery_model(type),
    to_base = function(pointer) model_to_fims_xptr_(pointer),
    release = function(pointer) release_models_(pointer)
  )
)

#' Record a newly created module and wrap it up for the user
#'
#' @description
#' The last thing every `create_*()` function does. It takes what
#' `create_*_()` returned and does three things with it: asks FIMS what number
#' the module was given, adds the module to the registry (`.fims_registry`,
#' declared at the top of this file), and returns it as a [fims_module].
#'
#' The registry is what `CreateTMBModel()` builds the model from. Because every
#' `create_*()` function ends here, a module cannot be created and then
#' accidentally left out of the model --- there is no separate registration
#' step for anyone to forget.
#'
#' Adding a new kind of module means writing a `create_*()` function that ends
#' with a call to this one. A new functional form needs its type added to
#' `.fims_module_types`; a genuinely new kind of module also needs an entry in
#' `.fims_module_names` above.
#'
#' @param pointer What `create_*_()` returned, which refers to one particular
#'   kind of module.
#' @param module_name Which kind of module this is, matching a name in
#'   `.fims_module_names` and the parameters tibble's `module_name` column.
#' @param module_type The functional form, matching the tibble's `module_type`
#'   column. `NA` for modules that have only one kind.
#' @param ... Anything else worth showing in `describe_model()`, such as the
#'   number of years and ages a data module was created with. Named values
#'   only, and each is printed as given.
#' @return
#' A [fims_module]. The same module is also added to the registry, so
#' `CreateTMBModel()` will include it.
#' @noRd
register_module <- function(pointer, module_name, module_type, ...) {
  base_pointer <- .fims_module_names[[module_name]][["to_base"]](pointer)
  # as.integer() because the C++ id is an unsigned int, which can arrive as a
  # double, and describe_model() formats it with %d.
  id <- as.integer(get_module_id_(base_pointer))

  module <- structure(
    list(
      pointer = pointer,
      base_pointer = base_pointer,
      module_name = module_name,
      module_type = module_type,
      id = id
    ),
    class = "fims_module"
  )

  # The whole module is stored, not just its base pointer, so that clear() can
  # invalidate both pointers.
  .fims_registry[["objects"]] <- c(.fims_registry[["objects"]], list(module))
  .fims_registry[["metadata"]] <- c(
    .fims_registry[["metadata"]],
    list(c(list(module_type = module_type, id = id), list(...)))
  )

  module
}

#' Print a FIMS module
#'
#' @param x A `fims_module`, as returned by any `create_*()` function.
#' @param ... Ignored, present for consistency with the generic.
#' @return
#' `x`, invisibly. Called for the description printed to the console.
#' @export
print.fims_module <- function(x, ...) {
  label <- if (is.na(x[["module_type"]])) {
    x[["module_name"]]
  } else {
    paste(x[["module_type"]], x[["module_name"]])
  }
  cat(sprintf("<fims_module> %s (id: %d)\n", label, x[["id"]]))
  invisible(x)
}

# ---- Creating modules --------------------------------------------------------

# The module_types each module_name accepts, matching both the C++ type strings
# the creators take and the parameters tibble's module_type column. Data types
# are snake_case because they come from the data's own type column.
.fims_module_types <- list(
  Data = c("age_comp", "length_comp", "index", "catch"),
  Selectivity = c("Logistic", "DoubleLogistic"),
  Recruitment = c("BevertonHolt", "log_devs", "log_r"),
  Distribution = c("dnorm", "dlnorm", "dmultinom"),
  Growth = "EWAA",
  Maturity = "Logistic",
  Model = "CatchAtAge",
  # Fleets and populations have one kind each and take no module_type.
  Fleet = NA_character_,
  Population = NA_character_
)

#' Check a module type, or stop listing what is available
#'
#' @param module_name The kind of module, naming an entry of
#'   `.fims_module_types`.
#' @param module_type The type string given by the caller.
#' @return `module_type`, invisibly.
#' @noRd
check_module_type <- function(module_name, module_type) {
  valid <- .fims_module_types[[module_name]]
  if (!is.character(module_type) || length(module_type) != 1 ||
    !module_type %in% valid) {
    cli::cli_abort(c(
      "{.arg module_type} is not a {module_name} type FIMS knows about.",
      "x" = "Got {.val {module_type}}.",
      "i" = "Available: {.val {valid}}."
    ))
  }
  invisible(module_type)
}

#' Create a data module
#'
#' @description
#' Creates one observed-data module and registers it, so that
#' [CreateTMBModel()] picks it up without the module having to be passed in by
#' hand. Set the observations with [set_data()].
#'
#' @param module_type The kind of data: `"age_comp"`, `"length_comp"`,
#'   `"index"`, or `"catch"`. These are snake_case because they come from the
#'   data's own `type` column.
#' @param n_years The number of years of observations.
#' @param n_bins The number of age bins for `"age_comp"` or length bins for
#'   `"length_comp"`. Index and catch data are one value per year, so leave
#'   this at 0 for them.
#' @return
#' A [fims_module] of the requested data type.
#' @seealso [set_data()]
#' @export
create_data <- function(module_type, n_years, n_bins = 0) {
  check_module_type("Data", module_type)
  pointer <- create_data_interface_(
    module_type, as.integer(n_years), as.integer(n_bins)
  )
  # The bin count is named for what it counts, so describe_model() reads well.
  extras <- list(n_years = n_years)
  if (module_type == "age_comp") extras[["n_ages"]] <- n_bins
  if (module_type == "length_comp") extras[["n_lengths"]] <- n_bins
  do.call(
    register_module,
    c(
      list(pointer = pointer, module_name = "Data", module_type = module_type),
      extras
    )
  )
}

#' Create a selectivity module
#'
#' @param module_type The functional form: `"Logistic"` or `"DoubleLogistic"`.
#' @return
#' A [fims_module] of the requested selectivity type.
#' @export
create_selectivity <- function(module_type) {
  check_module_type("Selectivity", module_type)
  register_module(create_selectivity_(module_type), "Selectivity", module_type)
}

#' Create a maturity module
#'
#' @param module_type The functional form: `"Logistic"`.
#' @return
#' A [fims_module] of the requested maturity type.
#' @export
create_maturity <- function(module_type) {
  check_module_type("Maturity", module_type)
  register_module(create_maturity_(module_type), "Maturity", module_type)
}

#' Create a growth module
#'
#' @param module_type The functional form: `"EWAA"`, empirical weight at age.
#' @return
#' A [fims_module] of the requested growth type.
#' @export
create_growth <- function(module_type) {
  check_module_type("Growth", module_type)
  register_module(create_growth_(module_type), "Growth", module_type)
}

#' Create a recruitment module
#'
#' @description
#' Covers both the stock--recruit relationship and the process that supplies
#' its deviations, since both are recruitment modules on the C++ side.
#'
#' @param module_type `"BevertonHolt"` for the stock--recruit relationship,
#'   or `"log_devs"` or `"log_r"` for the random effects parameterization.
#' @return
#' A [fims_module] of the requested recruitment type.
#' @export
create_recruitment <- function(module_type) {
  check_module_type("Recruitment", module_type)
  register_module(create_recruitment_(module_type), "Recruitment", module_type)
}

#' Create a distribution module
#'
#' @description
#' Distributions are not linked to the fishery model directly; they reach the
#' model by being registered, and name the quantities they apply to with
#' [set_distribution_links()].
#'
#' @param module_type The distribution: `"dnorm"`, `"dlnorm"`, or
#'   `"dmultinom"`.
#' @return
#' A [fims_module] of the requested distribution type.
#' @export
create_distribution <- function(module_type) {
  check_module_type("Distribution", module_type)
  register_module(
    create_distribution_(module_type), "Distribution", module_type
  )
}

#' Create a fishery model module
#'
#' @param module_type The model structure: `"CatchAtAge"`.
#' @return
#' A [fims_module] of the requested model type.
#' @export
create_fishery_model <- function(module_type) {
  check_module_type("Model", module_type)
  register_module(create_fishery_model_(module_type), "Model", module_type)
}

#' Create a fleet or a population
#'
#' @description
#' These take no type because there is only one kind of each. Both hold links
#' to other modules: a fleet names its selectivity and observed data, a
#' population names its growth, maturity, recruitment, and fleets.
#'
#' @return
#' A [fims_module].
#' @export
#' @rdname create_structure
create_fleet <- function() {
  register_module(create_fleet_(), "Fleet", NA_character_)
}

#' @export
#' @rdname create_structure
create_population <- function() {
  register_module(create_population_(), "Population", NA_character_)
}

# ---- Model lifecycle -------------------------------------------------------

#' Build the TMB model from the registered modules
#'
#' @description
#' Passes every module created since the last `clear()` to the C++ model
#' builder. Because modules register themselves as they are created, there is
#' no list to assemble and no way to silently leave a module out.
#'
#' Calling this a second time rebuilds the model from the same modules, picking
#' up any values changed in between. The C++ side clears its own state first,
#' so a rebuild does not accumulate on top of the previous run. The registry is
#' deliberately *not* cleared, which is what allows the same modules to be
#' reused across scenarios.
#'
#' @return
#' A model handle: a number identifying this build. It changes every time the
#' model is rebuilt, so two handles are equal only if they came from the same
#' build. Keep it if you need to tell later whether a fitted object still
#' matches the model in memory; ignore it otherwise. A failed build raises an
#' error rather than returning anything.
#' @seealso [clear()], [describe_model()]
#' @export
CreateTMBModel <- function() {
  if (length(.fims_registry[["objects"]]) == 0) {
    cli::cli_abort(c(
      "No model components have been registered.",
      "i" = "Create modules with the {.code create_*()} functions before
             calling {.fn CreateTMBModel}."
    ))
  }
  # The list holds whole modules; CreateTMBModel_() wants their base pointers,
  # which is what it calls add_to_fims_tmb() through.
  handle <- CreateTMBModel_(
    lapply(.fims_registry[["objects"]], `[[`, "base_pointer")
  )

  # Kept so that a later call can report which build the current model is from.
  .fims_registry[["model_handle"]] <- handle
  handle
}

#' Reset FIMS
#'
#' @description
#' Clears the C++ model state and empties the R-side module registry, so the
#' next model starts from an empty memory state.
#'
#' Every module created since the last `clear()` call is invalidated. An R 
#' variable still holding a module keeps working as a variable, but the module 
#' behind it is gone, and using it raises an error. The memory is returned 
#' when `clear()` returns, rather than whenever R next garbage collects.
#'
#' @return
#' Nothing, invisibly. Called for its effect on the model state.
#' @seealso [CreateTMBModel()]
#' @export
clear <- function() {
  # Both of a module's pointers are released, so nothing owns it afterwards.
  for (module in .fims_registry[["objects"]]) {
    name <- module[["module_name"]]
    .fims_module_names[[name]][["release"]](module[["pointer"]])
    release_base_(module[["base_pointer"]])
  }

  clear_()
  .fims_registry[["objects"]] <- list()
  .fims_registry[["metadata"]] <- list()
  .fims_registry[["model_handle"]] <- NULL
  invisible(NULL)
}

#' Describe the model assembled so far
#'
#' @description
#' Prints the modules registered since the last [clear()], with the ID each was
#' given and the dimensions it was created with. Intended as an interactive
#' check before [CreateTMBModel()], to confirm that every component is present.
#'
#' This reads only the R-side registry, so it works before the model is built
#' and does not touch the C++ model state.
#'
#' @return
#' The registry metadata, invisibly, as a list with one entry per module.
#' @seealso [CreateTMBModel()], [clear()]
#' @export
describe_model <- function() {
  metadata <- .fims_registry[["metadata"]]

  if (length(metadata) == 0) {
    message("No model components have been registered.")
    return(invisible(NULL))
  }

  cat("Model components registered:\n")
  cat(strrep("-", 40), "\n")
  for (module in metadata) {
    # Anything beyond type and id was recorded by the create_*() function as
    # configuration worth showing, and differs from one module type to the next.
    extras <- module[!names(module) %in% c("module_name", "module_type", "id")]
    extra_string <- if (length(extras) > 0) {
      paste(names(extras), unlist(extras), sep = " = ", collapse = ", ")
    } else {
      ""
    }
    # A module with one kind has no module_type, so only its name is shown.
    label <- if (is.na(module[["module_type"]])) {
      module[["module_name"]]
    } else {
      paste(module[["module_type"]], module[["module_name"]])
    }
    cat(sprintf("  %-28s id: %d  %s\n", label, module[["id"]], extra_string))
  }
  cat(strrep("-", 40), "\n")
  cat(sprintf("Total: %d component(s)\n", length(metadata)))

  invisible(metadata)
}
