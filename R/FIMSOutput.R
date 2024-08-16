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

get_estimates <- function(json_list, nyears = 30, ages = 1:12) {
  # Need to define tibble with zero rows so NA will be used to fill in
  # missing sections when combining estimates and derived quantities later
  estimates_outline <- dplyr::tibble(
    label = character(),
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

  # Format the JSON to get information out easier
  # Delete anything you want here to the return function

  # # Doing this renaming is dumb
  # # but helped me understand some stuff in the beginning
  # names(json_list) <- ifelse(
  #   names(json_list) == "module",
  #   purrr::map(json_list, "name"),
  #   names(json_list)
  # )

  # # Does not get what we need but helped me understand the structure
  # # it could be better to use tidyjson in the beginning then reverse
  # # engineer their functions using purrr and tidyr later.
  # # Might want to remove the data objects and just work with modules
  # # for this function
  # z <- json_list |>
  #   unlist() |>
  #   tibble::enframe() |>
  #   tidyr::separate_wider_delim(
  #     name,
  #     delim = ".",
  #     too_few = "align_start",
  #     names = c("module", "type", "x")
  #   ) |>
  #   dplyr::filter(type == "name")

  # loop over highest level elements in json file which have name "module"
  for (i in which(names(json_list) == "module")) {
    # module names in fims-demo currently include
    # "data", "selectivity", "Fleet", "recruitment", "growth", "maturity", "Population"
    # code below doesn't do anything with "data" as it doesn't have elements called
    # "parameter" or "derived_quantity"
    message(json_list[[i]]$name)

    # loop over parameters (if there are none then won't loop)
    # still need to add loop over derived quantities
    for (parameter_index in which(names(json_list[[i]]) == "parameter")) {
      # get info from parameter element of module
      # "is_random_effect" is the one element not yet utilized in the code below
      parameter_info <- json_list[[i]][[parameter_index]]
      # check for zero length output (currently present for parameter with name = "proportion_female")
      if (
        "value" %in% names(parameters) && length(parameter_info$value) != 0 |
          "values" %in% names(parameters) && length(parameter_info$values) != 0
      ) {
        # create parameter label
        # parameter labeling will change in the future, currently just concatenating info
        label <- paste(
          json_list[[i]]$name, # module name (e.g. "selectivity", or "Fleet")
          json_list[[i]]$type, # module type (e.g. "Logistic", "fleet")
          parameter_info$name, # parameter name (e.g. "inflection_point")
          sep = "_"
        )
        # Hack to make unique labels by appending sequence of numbers to
        # parameter label for vectors (only other type seems to be "scalar")
        if (parameter_info$type == "vector") {
          label <- paste(label,
            1:length(parameter_info$value), # hack to make unique names, need to figure out assigment of age and time
            sep = "_"
          )
        }

        # create tibble with new rows for these parameters
        estimates_newrows <- dplyr::tibble(
          label = label,
          # fleet number is in "id"
          fleet = ifelse(
            test = json_list[[i]]$name == "Fleet",
            yes = json_list[[i]]$id,
            no = NA
          ),
          age = NA,
          time = NA,
          # "value" vs "values" depending on whether parameter is a scalar or vector
          initial = ifelse(
            test = parameter_info$type == "scalar",
            yes = parameter_info$value,
            no = parameter_info$values
          ),
          estimate = ifelse(
            test = parameter_info$type == "scalar",
            yes = parameter_info$estimated_value,
            no = parameter_info$estimated_values
          ),
          uncertainty = NA,
          likelihood = NA,
          gradient = ifelse(
            test = parameter_info$id >= 0,
            yes = json_list$final_gradient[parameter_info$id + 1], # id starts at 0
            no = NA
          ),
          estimated = parameter_info$is_estimated
        )
        # if new rows were created above, bind to end of tibble
        if (nrow(estimates_newrows) > 0) {
          estimates_outline <- dplyr::bind_rows(estimates_outline, estimates_newrows)
        }
      } # end check for non-zero length of values vector
    } # end loop over parameters

    # loop over derived quantities
    # in the future this could be merged with processing of parameters, but easier to keep separate for now
    for (derived_quantity_index in which(names(json_list[[i]]) == "derived_quantity")) {
      info <- json_list[[i]][[derived_quantity_index]]
      label <- paste(
        json_list[[i]]$name, # module name (e.g. "Population")
        json_list[[i]]$type, # module type for things where there's no tag (e.g. "Logistic")
        info$name, # parameter name (e.g. "inflection_point")
        sep = "_"
      )

      # create tibble with new rows for these parameters
      estimates_newrows <- dplyr::tibble(
        label = label,
        # fleet number is in "id"
        fleet = ifelse(
          test = json_list[[i]]$name == "Fleet",
          yes = json_list[[i]]$id,
          no = NA
        ),
        age = NA,
        time = NA,
        initial = NA,
        estimate = info$values,
        uncertainty = NA,
        likelihood = NA,
        gradient = NA,
        estimated = NA
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
