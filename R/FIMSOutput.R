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
# TODO from Ian 2024-08-23: I'm not sure what functions should go to
# FIMSOutput.Rd and the .Rd file was getting confusing so currently
# `create_fims_output()`, `get_estimates()`, and `get_fits()` go there.

#' Class constructors for `FIMSOutput` and associated child classes
#'
#' All constructor functions take a single input and build an object specific to
#' the needs of each model type within \pkg{FIMS}. `FIMSOutput` is the
#' parent class and the associated child classes have additional slots needed
#' for each model type.
#' @export
#' @rdname FIMSOutput
#' @param json_list A list of JSON information output from `Finalize()`.
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

#' Get information on the name and type of each element in the JSON output
#'
#' TODO: this function could surely be replaced by some kind of lapply
#' function or other more elegant solution.
#'
#' @param json_list A list of JSON information output from `Finalize()`.
#' @return A tibble with columns "thing", "name", "type" and "id" like
#' ```
#' # A tibble: 20 x 4
#'    thing                    name        type             id
#'    <chr>                    <chr>       <chr>         <int>
#'  1 timestamp                NA          NA               NA
#'  2 nyears                   NA          NA               NA
#'  3 nseasons                 NA          NA               NA
#'  4 nages                    NA          NA               NA
#'  5 finalized                NA          NA               NA
#'  6 objective_function_value NA          NA               NA
#'  7 max_gradient_component   NA          NA               NA
#'  8 final_gradient           NA          NA               NA
#'  9 module                   data        Index             1
#' 10 module                   data        AgeComp           2
#' 11 module                   selectivity Logistic          1
#' 12 module                   Fleet       fleet             1
#' 13 module                   data        Index             3
#' 14 module                   data        AgeComp           4
#' 15 module                   selectivity Logistic          2
#' 16 module                   Fleet       survey            2
#' 17 module                   recruitment Beverton-Holt     1
#' 18 module                   growth      EWAA              1
#' 19 module                   maturity    Logistic          1
#' 20 module                   Population  population        1
#' ```
get_json_summary <- function(json_list) {
  json_summary <- NULL
  for (i in 1:length(json_list)) {
    json_summary <- dplyr::bind_rows(
      json_summary,
      dplyr::tibble(
        element = i,
        thing = names(json_list)[i], # e.g. "module"
        name = ifelse(
          test = "name" %in% names(json_list[[i]]),
          yes = json_list[[i]][["name"]],
          no = NA
        ),
        type = ifelse(
          test = "type" %in% names(json_list[[i]]),
          yes = json_list[[i]][["type"]],
          no = NA
        ),
        id = ifelse(
          test = "id" %in% names(json_list[[i]]),
          yes = json_list[[i]][["id"]],
          no = NA
        )
      )
    )
  }
  return(json_summary)
}

#' Process parameter info in JSON output and return rows for the 'estimates' tibble
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
  # TODO: delete this step when JSON becomes standardized
  #  if ("value" in )
  names(json)[names(json) == "value"] <- "values"
  names(json)[names(json) == "estimated_value"] <- "estimated_values"

  # check for zero length output
  # (currently only present for parameter with name = "proportion_female")
  if ("values" %in% names(json) && length(json[["values"]]) > 0) {
    # create parameter label
    # TODO: revise this once parameter labeling gets refined in the future
    label <- paste(
      module_name, # (e.g. "selectivity" or "Fleet")
      module_type, # (e.g. "Logistic" or "survey")
      json[["name"]], # parameter name (e.g. "inflection_point")
      sep = "_"
    )

    # create tibble with new rows for these parameters
    estimates_newrows <- dplyr::tibble(
      label = label,
      parameter_id = json[["id"]],
      fleet = fleet,
      age = NA, # TODO: not yet available in the JSON output
      time = NA, # TODO: not yet available in the JSON output
      initial = json[["values"]],
      estimate = json[["estimated_values"]],
      uncertainty = NA, # TODO: not yet available in the JSON output
      likelihood = NA, # TODO: not yet available in the JSON output
      gradient = NA, # to be filled in based on parameter_id
      estimated = json[["is_estimated"]]

      # TODO: do something with "is_random_effect" in the future?
    )
  } # end check for non-zero length of values vector

  # return result (could be NULL if "values" is empty)
  return(estimates_newrows)
}

# TODO: consider merging get_parameter() and get_derived_quantity()

#' Process derived quantitiy info in JSON output and return rows for the 'estimates' tibble
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
    json[["name"]], # parameter name (e.g. "inflection_point")
    sep = "_"
  )
  # create tibble with new rows for these parameters
  estimates_newrows <- dplyr::tibble(
    label = label,
    parameter_id = NA,
    fleet = fleet,
    age = NA, # TODO: not yet available in the JSON output
    time = NA, # TODO: not yet available in the JSON output
    initial = NA,
    estimate = json[["values"]],
    uncertainty = NA, # TODO: not yet available in the JSON output
    likelihood = NA, # TODO: not yet available in the JSON output
    gradient = NA, # to be filled in based on parameter_id
    estimated = NA
  )

  # return result (could be NULL if "values" is empty)
  return(estimates_newrows)
}

#' Convert JSON output into the 'estimates' tibble
#'
#' @param json_list A list of JSON information output from `Finalize()`.
#' @rdname FIMSOutput
#' @return
#' A tibble `estimates` to be included with other output in a list
get_estimates <- function(json_list) {
  # Need to define tibble with zero rows so NA will be used to fill in
  # missing sections when combining estimates and derived quantities later
  estimates <- dplyr::tibble(
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

  # loop over highest level elements in JSON file which have name "module"
  for (i in which(names(json_list) == "module")) {
    # module names in fims-demo currently include
    # "data", "selectivity", "Fleet", "recruitment", "growth", "maturity", "Population"
    # code below doesn't do anything with "data" as it doesn't have elements called
    # "parameter" or "derived_quantity"
    module_name <- json_list[[i]][["name"]]
    module_type <- json_list[[i]][["type"]]
    fleet <- NA
    if (module_name == "Fleet") {
      fleet <- json_list[[i]][["id"]]
    }
    message("processing element ", i, ": ", module_name)

    # loop over parameters (if there are none then won't loop)
    # j is the second level in the nested lists, with i as the first
    for (j in which(names(json_list[[i]]) == "parameter")) {
      # get info from parameter element of module
      estimates_newrows <- get_parameter(
        json = json_list[[i]][[j]],
        fleet = fleet,
        module_name = module_name,
        module_type = module_type
      )
      # if new rows were returned by get_parameter(), then bind to end of tibble
      if (!is.null(estimates_newrows) > 0) {
        estimates <- dplyr::bind_rows(estimates, estimates_newrows)
      }
    } # end loop over parameters

    # add gradient based on parameter_id
    # for all parameters with id avialable (not -999),
    # TODO: check why length of final_gradient doesn't match number of parameters
    good_parameter_id <- !is.na(estimates[["parameter_id"]]) & estimates[["parameter_id"]] >= 0
    estimates[["gradient"]][good_parameter_id] <-
      json_list[["final_gradient"]][estimates[["parameter_id"]][good_parameter_id] + 1]

    # loop over derived quantities
    # in the future this could be merged with processing of parameters,
    # but easier to keep separate for now
    # j is the second level in the nested lists, with i as the first
    for (j in which(names(json_list[[i]]) == "derived_quantity")) {
      estimates_newrows <- get_derived_quantity(
        json = json_list[[i]][[j]],
        fleet = fleet,
        module_name = module_name,
        module_type = module_type
      )
      # if new rows were created above, bind to end of tibble
      if (nrow(estimates_newrows) > 0) {
        estimates <- dplyr::bind_rows(estimates, estimates_newrows)
      }
    } # end loop over derived quantities
  } # end loop over elements of json_list

  return(estimates)
}

#' Convert JSON output into the 'fits' tibble
#'
#' @param json_list A list of JSON information output from `Finalize()`.
#' @param data The FIMSFrame input to the model. If that object is not
#' available the function will return a more limited amount of information
#' @rdname FIMSOutput
#' @return
#' A tibble `estimates` to be included with other output in a list
get_fits <- function(json_list, data = NULL) {
  # get some summary information on the json_list
  # to figure out which elements are needed here
  json_summary <- get_json_summary(json_list)

  # create placeholder tibble similar to FIMSFrame but with a few additions
  fits <- dplyr::tibble(
    type = character(),
    fleet = integer(), # not found in FIMSFrame but can be used to fill in name
    name = character(),
    age = numeric(),
    year = numeric(), # simple but doesn't match datestart/dateend of FIMSFrame
    value = numeric(),
    unit = character(),
    uncertainty = numeric(),
    expected = numeric(), # not found in FIMSFrame
    likelihood = numeric(), # not found in FIMSFrame
    weight = numeric(), # not found in FIMSFrame
    distribution = character() # not found in FIMSFrame
  )

  # loop over highest level elements in JSON file which have name "module"
  for (i in which(names(json_list) == "module")) {
    # module names in fims-demo currently include
    # "data", "selectivity", "Fleet", "recruitment", "growth", "maturity", "Population"
    module_name <- json_list[[i]][["name"]]
    module_type <- json_list[[i]][["type"]]
    if (module_name == "Fleet") {
      # get values (e.g. from name == "index")
      # get observed_index_data_id
      # get index_likelihood_id
      # go find obs
      # placeholder for like

      # loop over derived quantities within the fleet module
      for (j in which(names(json_list[[i]]) == "derived_quantity")) {
        # JSON section for derived quantity within fleet module
        fits_newrows <- NULL
        json <- json_list[[i]][[j]]
        if (json[["name"]] == "index") {
          # get the input data (should be redundant with FIMSFrame
          # which is an optional input to this function)
          which_list_element <- json_summary |>
            dplyr::filter(
              type == "Index" &
                id == json_list[[i]][["observed_index_data_id"]]
            ) |>
            dplyr::pull(element)
          if (length(which_list_element) > 1) {
            stop("something wrong with matching index to fleet")
          }

          # fill in new rows for output tibble
          fits_newrows <- dplyr::tibble(
            type = "index",
            fleet = json_list[[i]][["id"]],
            name = NA,
            age = NA, # indices are not age-specific, so should be NA
            year = NA, # TODO: fill in based on dimensions or something
            value = json_list[[which_list_element]][["values"]],
            unit = NA, # TODO: add once available
            uncertainty = NA, # TODO: add once available
            expected = json[["values"]],
            # TODO: once likelihood values are available in JSON output,
            # we can probably get them just like value above only using
            # `index_likelihood_id`
            likelihood = NA,
            weight = 1.0,
            distribution = NA # TODO: hardwire to lognormal for now or add to JSON output?
          )
        }
        if (json[["name"]] == "age_composition") {
          # TODO: fill this in (as well as for any other data type we may use)
          # NOTE: JSON output has a mix of "AgeComp", "age_composition" and "agecomp" (in "agecomp_likelihood_id")
        }
        # if new rows were created above, bind to end of tibble
        if (!is.null(fits_newrows) && nrow(fits_newrows) > 0) {
          fits <- dplyr::bind_rows(fits, fits_newrows)
        }
      } # end loop over derived quantities
    }
  } # end loop over modules
  return(fits)
}
