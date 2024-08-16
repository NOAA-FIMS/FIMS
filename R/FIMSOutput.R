# This file defines a parent class and its children by
# (1) setting the class;
# (2) defining methods, using setMethod();
# (3) setting the validators; and
# (4) establishing the constructors (i.e., functions called by users)
# where only the constructors are documented using roxygen.

# setClass: ----
# Classes are not currently exported, and therefore, do not need documentation.
# See the following link if we do want to document them in the future:
# https://stackoverflow.com/questions/7368262/how-to-properly-document-s4-class-slots-using-roxygen2

# The following setClassUnion is necessary because the default value for the
# call is NULL which is not a member of the call class. The call will only be
# available if the model is ran using the wrapper function within FIMS.
methods::setClassUnion("nullOrCall", c("NULL", "call"))
# Need to use an S3 class for the following S4 class
methods::setOldClass(Classes = "package_version")
setClass(
  Class = "FIMSOutput",
  slots = c(
    estimates = "tbl_df",
    fits = "tbl_df",
    tmb = "list",
    call = "nullOrCall",
    timestamp = "POSIXlt",
    version = "package_version"
  ),
  contains = class(dplyr::tibble())
)

# Getters

setGeneric("estimates", function(x) standardGeneric("estimates"))
setMethod("estimates", "FIMSOutput", function(x) x@estimates)

setGeneric("fits", function(x) standardGeneric("fits"))
setMethod("fits", "FIMSOutput", function(x) x@fits)

# Constructors ----
# All constructors in this file are documented in 1 roxygen file via @rdname.

#' Class constructors for `FIMSOutput` and associated child classes
#'
#' All constructor functions take a single input and build an object specific to
#' the needs of each model type within \pkg{FIMS}. `FIMSOutput` is the
#' parent class and the associated child classes have additional slots needed
#' for each model type.
#' @export
#' @rdname FIMSOutput
#' @param json_list A list of json information output from `Finalize()`.
#' @param call The call the users made to run FIMS. The default is `NULL`.
#'   This is not available for FIMS yet.
#' @param data The input data used to fit the model. This needs to be a
#'   FIMSFrame object.
#' @return
#' An object of the S4 class `FIMSOutput` or one of its child classes is
#' validated and then returned. All objects will at a minimum have a slot
#' called `data` to store the input data frame. Additional slots are dependent
#' on the child class. Use [showClass()] to see all available slots.
create_fims_output <- function(json_list, call = NULL, data) {
  # The code will break if
  # * The age bins are not the same as the ages in the data, e.g., if age 3 is
  #   not in the data or if there is a plus group that is created internally
  # * The model reports values for age zero fish but age zero is not in the data
  # Questions:
  # * How should the initial year should be labeled, e.g., should it always be
  #   zero or one minus the initial year?
  # * Should we be reporting the covariance from sdr?
  # * Should time be year because that is what it actually is at the moment?

  # Need to run the following timestamp code before running FIMS and after.
  timestamp <- as.POSIXlt(Sys.time(), tz = "UTC")
  # The version should be taken from the FIMS wrapper to run the model rather
  # than the version used to read in the results because they in theory
  # could be different?
  version <- utils::packageVersion("FIMS")
  # match.call has to be used to create the call when the user run FIMS
  # call <- match.call()

  # Fill the empty data frames with data extracted from the data file
  out <- new(
    "FIMSOutput",
    estimates = get_estimates(json_list),
    fits = get_fits(json_list, data),
    tmb = obj,
    call = call,
    timestamp = timestamp,
    version = version
  )
  return(out)
}

#' Process parameter info in json output and return rows for the estimates tibble
#'
#' @param json A list extracted from an element of the `json_list`` provided to [get_estimates()].
#' @param fleet The fleet number associated with this parameter (if applicable)
#' @param module_name The name of the module which contains this parameter (e.g. "selectivity" or "Fleet")
#' @param module_type The type of the module which contains this parameter (e.g. "Logistic" or "survey")
#' @return
#' Rows that can be added to the `estimates` tibble.
get_parameter <- function(json, fleet = NA, module_name, module_type) {
  # placeholder for output
  estimates_newrows <- NULL

  # rename element to remove inconsistency between "value" and "values".
  # TODO: delete this step when json becomes standardized
  #  if ("value" in )
  names(json)[names(json) == "value"] <- "values"
  names(json)[names(json) == "estimated_value"] <- "estimated_values"

  # check for zero length output
  # (currently only present for parameter with name = "proportion_female")
  if ("values" %in% names(json) && length(json$values) > 0) {
    # create parameter label
    # TODO: revise this once parameter labeling gets refined in the future
    label <- paste(
      module_name, # (e.g. "selectivity" or "Fleet")
      module_type, # (e.g. "Logistic" or "survey")
      json$name, # parameter name (e.g. "inflection_point")
      sep = "_"
    )

    # create tibble with new rows for these parameters
    estimates_newrows <- dplyr::tibble(
      label = label,
      parameter_id = json$id,
      fleet = fleet,
      age = NA, # TODO: not yet available in the json output
      time = NA, # TODO: not yet available in the json output
      initial = json$values,
      estimate = json$estimated_values,
      uncertainty = NA, # TODO: not yet available in the json output
      likelihood = NA, # TODO: not yet available in the json output
      gradient = NA, # to be filled in based on parameter_id
      estimated = json$is_estimated

      # TODO: do something with "is_random_effect" in the future?
    )
  } # end check for non-zero length of values vector

  # return result (could be NULL if "values" is empty)
  return(estimates_newrows)
}

# TODO: consider merging get_parameter() and get_derived_quantity()

#' Process derived quantitiy info in json output and return rows for the estimates tibble
#'
#' @param json A list extracted from an element of the `json_list`` provided to [get_estimates()].
#' @param fleet The fleet number associated with this derived quantity (if applicable)
#' @param module_name The name of the module which contains this derived quantity
#' @param module_type The type of the module which contains this derived quantity
#' @return
#' Rows that can be added to the `estimates` tibble.
get_derived_quantity <- function(json, fleet = NA, module_name, module_type) {
  # placeholder for output
  estimates_newrows <- NULL

  # create parameter label
  # TODO: revise this once parameter labeling gets refined in the future
  label <- paste(
    module_name, # (e.g. "selectivity" or "Fleet")
    module_type, # (e.g. "Logistic" or "survey")
    json$name, # parameter name (e.g. "inflection_point")
    sep = "_"
  )
  # create tibble with new rows for these parameters
  estimates_newrows <- dplyr::tibble(
    label = label,
    parameter_id = NA,
    fleet = fleet,
    age = NA, # TODO: not yet available in the json output
    time = NA, # TODO: not yet available in the json output
    initial = NA,
    estimate = json$values,
    uncertainty = NA, # TODO: not yet available in the json output
    likelihood = NA, # TODO: not yet available in the json output
    gradient = NA, # to be filled in based on parameter_id
    estimated = NA
  )

  # return result (could be NULL if "values" is empty)
  return(estimates_newrows)
}

get_estimates <- function(json_list, nyears = 30, ages = 1:12) {
  # Need to define tibble with zero rows so NA will be used to fill in
  # missing sections when combining estimates and derived quantities later
  estimates_outline <- dplyr::tibble(
    label = character(),
    parameter_id = integer(), # not included in design doc but will be useful for processing
    fleet = integer(), # was initially character()
    age = numeric(),
    time = numeric(),
    initial = numeric(),
    estimate = numeric(),
    uncertainty = numeric(),
    likelihood = numeric(),
    gradient = numeric(),
    estimated = logical()
  )

  # loop over highest level elements in json file which have name "module"
  for (i in which(names(json_list) == "module")) {
    # module names in fims-demo currently include
    # "data", "selectivity", "Fleet", "recruitment", "growth", "maturity", "Population"
    # code below doesn't do anything with "data" as it doesn't have elements called
    # "parameter" or "derived_quantity"
    module_name <- json_list[[i]]$name
    module_type <- json_list[[i]]$type
    fleet <- NA
    if (module_name == "Fleet") {
      fleet <- json_list[[i]]$id
    }
    message("processing element ", i, ": ", module_name)

    # loop over parameters (if there are none then won't loop)
    # still need to add loop over derived quantities
    for (index in which(names(json_list[[i]]) == "parameter")) {
      # get info from parameter element of module
      estimates_newrows <- get_parameter(
        json = json_list[[i]][[index]],
        fleet = fleet,
        module_name = module_name,
        module_type = module_type
      )
      # if new rows were returned by get_parameter(), then bind to end of tibble
      if (!is.null(estimates_newrows) > 0) {
        estimates_outline <- dplyr::bind_rows(estimates_outline, estimates_newrows)
      }
    } # end loop over parameters

    # add gradient based on parameter_id
    # for all parameters with id avialable (not -999),
    # TODO: check why length of final_gradient doesn't match number of parameters
    good_parameter_id <- !is.na(estimates_outline$parameter_id) & estimates_outline$parameter_id >= 0
    estimates_outline$gradient[good_parameter_id] <-
      json_list$final_gradient[estimates_outline$parameter_id[good_parameter_id] + 1]

    # loop over derived quantities
    # in the future this could be merged with processing of parameters, 
    # but easier to keep separate for now
    for (index in which(names(json_list[[i]]) == "derived_quantity")) {
      estimates_newrows <- get_derived_quantity(
        json = json_list[[i]][[index]],
        fleet = fleet,
        module_name = module_name,
        module_type = module_type
      )
      # if new rows were created above, bind to end of tibble
      if (nrow(estimates_newrows) > 0) {
        estimates_outline <- dplyr::bind_rows(estimates_outline, estimates_newrows)
      }
    } # end loop over derived quantities
  } # end loop over elements of json_list

  estimates <- estimates_outline # maybe no need for separate objects
  return(estimates)
}

get_fits <- function(obj, sdr) {
  fits <- dplyr::tibble()
  return(fits)
}
