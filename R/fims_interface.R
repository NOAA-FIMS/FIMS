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
#' set_parameter_(population$base_pointer, "log_M", values, status)
#' ```
#'
#' `family` records which kind of module this is, for FIMS's own use.
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

# The two C++ functions that differ per module family. Both are wrapped in
# closures rather than referenced directly, because the Rcpp module is not
# loaded until .onLoad() runs, well after this file is sourced.
#
# to_base  produces the base-class pointer used by everything that works on any
#          module, and by CreateTMBModel().
# release  invalidates the family-typed pointer. Releasing deletes through the
#          pointer's own type, so this cannot be shared across families.
.fims_module_family <- list(
  data = list(
    to_base = function(pointer) data_to_fims_xptr_(pointer),
    release = function(pointer) release_data_(pointer)
  ),
  fleet = list(
    to_base = function(pointer) fleet_to_fims_xptr_(pointer),
    release = function(pointer) release_fleet_(pointer)
  ),
  growth = list(
    to_base = function(pointer) growth_to_fims_xptr_(pointer),
    release = function(pointer) release_growth_(pointer)
  ),
  maturity = list(
    to_base = function(pointer) maturity_to_fims_xptr_(pointer),
    release = function(pointer) release_maturity_(pointer)
  ),
  population = list(
    to_base = function(pointer) population_to_fims_xptr_(pointer),
    release = function(pointer) release_population_(pointer)
  ),
  recruitment = list(
    to_base = function(pointer) recruitment_to_fims_xptr_(pointer),
    release = function(pointer) release_recruitment_(pointer)
  ),
  selectivity = list(
    to_base = function(pointer) selectivity_to_fims_xptr_(pointer),
    release = function(pointer) release_selectivity_(pointer)
  ),
  distribution = list(
    to_base = function(pointer) distribution_to_fims_xptr_(pointer),
    release = function(pointer) release_distribution_(pointer)
  ),
  model = list(
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
#' with a call to this one. If it belongs to a family FIMS already knows about,
#' nothing else is needed; a genuinely new family also needs an entry in
#' `.fims_module_family` above.
#'
#' @param pointer What `create_*_()` returned, which refers to one particular
#'   kind of module.
#' @param family Which family the module belongs to, matching a name in
#'   `.fims_module_family`. FIMS uses this to find the two functions that
#'   differ from one family to the next.
#' @param type A label for the kind of module, shown by `describe_model()`.
#' @param ... Anything else worth showing in `describe_model()`, such as the
#'   number of years and ages a data module was created with. Named values
#'   only, and each is printed as given.
#' @return
#' A [fims_module]. The same module is also added to the registry, so
#' `CreateTMBModel()` will include it.
#' @noRd
register_module <- function(pointer, family, type, ...) {
  base_pointer <- .fims_module_family[[family]][["to_base"]](pointer)
  # as.integer() because the C++ id is an unsigned int, which can arrive as a
  # double, and describe_model() formats it with %d.
  id <- as.integer(get_module_id_(base_pointer))

  module <- structure(
    list(
      pointer = pointer,
      base_pointer = base_pointer,
      family = family,
      type = type,
      id = id
    ),
    class = "fims_module"
  )

  # The whole module is stored, not just its base pointer, so that clear() can
  # invalidate both pointers.
  .fims_registry[["objects"]] <- c(.fims_registry[["objects"]], list(module))
  .fims_registry[["metadata"]] <- c(
    .fims_registry[["metadata"]],
    list(c(list(type = type, id = id), list(...)))
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
  cat(sprintf("<fims_module> %s (id: %d)\n", x[["type"]], x[["id"]]))
  invisible(x)
}

# ---- Data ------------------------------------------------------------------

#' Create a data module
#'
#' @description
#' Each function creates one observed-data module and registers it, so that
#' `CreateTMBModel()` picks it up without the module having to be passed in by
#' hand. Set the observations and their uncertainty with `set_vector_()`, using
#' the field names `"values"` and `"uncertainty"`.
#'
#' @param n_years The number of years of observations.
#' @param n_ages The number of age bins.
#' @param n_lengths The number of length bins.
#' @return
#' A [fims_module] of the requested data type.
#' @export
#' @rdname create_data
create_age_comp <- function(n_years, n_ages) {
  pointer <- create_data_interface_(
    "age_comp",
    as.integer(n_years),
    as.integer(n_ages)
  )
  register_module(
    pointer,
    "data",
    "AgeComp",
    n_years = n_years,
    n_ages = n_ages
  )
}

#' @export
#' @rdname create_data
create_length_comp <- function(n_years, n_lengths) {
  pointer <- create_data_interface_(
    "length_comp",
    as.integer(n_years),
    as.integer(n_lengths)
  )
  register_module(
    pointer,
    "data",
    "LengthComp",
    n_years = n_years,
    n_lengths = n_lengths
  )
}

#' @export
#' @rdname create_data
create_index <- function(n_years) {
  # create_data_interface_() takes n_bins as a third argument. Rcpp does not
  # carry C++ default arguments across to R, so the 0 is passed explicitly
  # here; it rejects a non-zero value for one-dimensional data types.
  pointer <- create_data_interface_("index", as.integer(n_years), 0L)
  register_module(
    pointer,
    "data",
    "Index",
    n_years = n_years
  )
}

#' @export
#' @rdname create_data
create_catch <- function(n_years) {
  pointer <- create_data_interface_("catch", as.integer(n_years), 0L)
  register_module(
    pointer,
    "data",
    "Catch",
    n_years = n_years
  )
}

# ---- Population processes --------------------------------------------------

#' Create a population process module
#'
#' @description
#' Each function creates one population process module and registers it, so
#' that `CreateTMBModel()` picks it up without the module having to be passed
#' in by hand. Set the module's parameters with `set_parameter_()`.
#'
#' @return
#' A [fims_module] of the requested process type.
#' @export
#' @rdname create_process
create_ewaa_growth <- function() {
  pointer <- create_growth_("ewaa")
  register_module(pointer, "growth", "EWAAGrowth")
}

#' @export
#' @rdname create_process
create_logistic_maturity <- function() {
  pointer <- create_maturity_("logistic")
  register_module(pointer, "maturity", "LogisticMaturity")
}

#' @export
#' @rdname create_process
create_logistic_selectivity <- function() {
  pointer <- create_selectivity_("logistic")
  register_module(
    pointer,
    "selectivity",
    "LogisticSelectivity"
  )
}

#' @export
#' @rdname create_process
create_double_logistic_selectivity <- function() {
  pointer <- create_selectivity_("double_logistic")
  register_module(
    pointer,
    "selectivity",
    "DoubleLogisticSelectivity"
  )
}

#' @export
#' @rdname create_process
create_beverton_holt_recruitment <- function() {
  pointer <- create_recruitment_("beverton_holt")
  register_module(
    pointer,
    "recruitment",
    "BevertonHoltRecruitment"
  )
}

#' @export
#' @rdname create_process
create_log_devs_recruitment_process <- function() {
  pointer <- create_recruitment_("log_devs_process")
  register_module(
    pointer,
    "recruitment",
    "LogDevsRecruitmentProcess"
  )
}

#' @export
#' @rdname create_process
create_log_r_recruitment_process <- function() {
  pointer <- create_recruitment_("log_r_process")
  register_module(
    pointer,
    "recruitment",
    "LogRRecruitmentProcess"
  )
}

# ---- Distributions ---------------------------------------------------------

#' Create a distribution module
#'
#' @description
#' Each function creates one distribution module and registers it, so that
#' `CreateTMBModel()` picks it up without the module having to be passed in by
#' hand.
#'
#' @return
#' A [fims_module] of the requested distribution type.
#' @export
#' @rdname create_distribution
create_dnorm_distribution <- function() {
  pointer <- create_distribution_("dnorm")
  register_module(
    pointer,
    "distribution",
    "DnormDistribution"
  )
}

#' @export
#' @rdname create_distribution
create_dlnorm_distribution <- function() {
  pointer <- create_distribution_("dlnorm")
  register_module(
    pointer,
    "distribution",
    "DlnormDistribution"
  )
}

#' @export
#' @rdname create_distribution
create_dmultinom_distribution <- function() {
  pointer <- create_distribution_("dmultinom")
  register_module(
    pointer,
    "distribution",
    "DmultinomDistribution"
  )
}

# ---- Structural modules ----------------------------------------------------

#' Create a fleet, population, or fishery model module
#'
#' @description
#' Each function creates one structural module and registers it, so that
#' `CreateTMBModel()` picks it up without the module having to be passed in by
#' hand. These modules hold links to other modules: a fleet names its
#' selectivity and its observed data, a population names its growth, maturity,
#' recruitment, and fleets, and the fishery model names its populations.
#'
#' @return
#' A [fims_module] of the requested type.
#' @export
#' @rdname create_structure
create_fleet <- function() {
  pointer <- create_fleet_()
  register_module(pointer, "fleet", "Fleet")
}

#' @export
#' @rdname create_structure
create_population <- function() {
  pointer <- create_population_()
  register_module(pointer, "population", "Population")
}

#' @export
#' @rdname create_structure
create_catch_at_age_model <- function() {
  pointer <- create_fishery_model_("catch_at_age")
  register_module(pointer, "model", "CatchAtAge")
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
    .fims_module_family[[module[["family"]]]][["release"]](module[["pointer"]])
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
    extras <- module[!names(module) %in% c("type", "id")]
    extra_string <- if (length(extras) > 0) {
      paste(names(extras), unlist(extras), sep = " = ", collapse = ", ")
    } else {
      ""
    }
    cat(sprintf(
      "  %-25s id: %d  %s\n",
      module[["type"]],
      module[["id"]],
      extra_string
    ))
  }
  cat(strrep("-", 40), "\n")
  cat(sprintf("Total: %d component(s)\n", length(metadata)))

  invisible(metadata)
}
