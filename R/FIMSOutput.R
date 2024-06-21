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
#' @param obj The output from `obj$report(obj$env$last.par.best)`.
#' @param sdr sdr The returned object from running [TMB::sdreport()] on `obj`.
#' @param call The call the users made to run FIMS. The default is `NULL`.
#'   This is not available for FIMS yet.
#' @param data The input data used to fit the model. This needs to be a
#'   FIMSFrame object.
#' @return
#' An object of the S4 class `FIMSOutput` or one of its child classes is
#' validated and then returned. All objects will at a minimum have a slot
#' called `data` to store the input data frame. Additional slots are dependent
#' on the child class. Use [showClass()] to see all available slots.
create_fims_output <- function(obj, sdr, call = NULL, data) {
  # report <- obj$report(obj$env$last.par.best) # causes failure
  # SSB and Biomass
  # will pull the information from obj and/or sdreport for estimates par is a
  # placeholder The following only applies to SSB and Biomass and need to
  # modify to include other parameters and quantities
  estimate_biomass_tibble <- function(derived_quantity_name,
                                      sdr = sdr,
                                      data = data) {
    dplyr::tibble(
      label = derived_quantity_name,
      age = NA,
      time = 0:n_years(data),
      initial = NA,
      estimates = sdr$value[names(sdr$value) == derived_quantity_name],
      uncertainty = sdr$sd[names(sdr$value) == derived_quantity_name],
      likelihood = NA,
      gradient = NA,
      estimated = NA
    )
  }
  # NAA
  estimate_naa_tibble <- function(derived_quantity_name,
                                  sdr = sdr,
                                  data = data) {
    dplyr::tibble(
      label = derived_quantity_name,
      age = rep(1:n_ages(data), times = (n_years(data) + 1)),
      time = rep(0:n_years(data), each = n_ages(data)),
      initial = NA,
      estimates = sdr$value[names(sdr$value) == derived_quantity_name],
      uncertainty = sdr$sd[names(sdr$value) == derived_quantity_name],
      likelihood = NA,
      gradient = NA,
      estimated = NA
    )
  }

  estimates_biomass <- do.call(
    "rbind",
    lapply(
      c("SSB", "Biomass"),
      estimate_biomass_tibble,
      sdr = sdr,
      data = data
    )
  )

  # recruitment (need to add uncertainty)
  estimate_recruitment_tibble <- function(obj, data) {
    dplyr::tibble(
      label = "recruitment",
      age = NA,
      time = 1:(n_years(data) + 1),
      initial = NA,
      estimates = 0.5, # Should be report$recruitment[[1]],
      uncertainty = NA,
      likelihood = NA,
      gradient = NA,
      estimated = NA
    )
  }

  # the fishing mortality needs the year vector
  estimate_Fmort_tibble <- function(par_name, sdr = sdr) {
    dplyr::tibble(
      label = par_name,
      age = NA,
      time = NA,
      initial = NA,
      estimates = sdr$value[names(sdr$value) == par_name],
      uncertainty = sdr$sd[names(sdr$value) == par_name],
      likelihood = NA,
      gradient = NA,
      estimated = NA
    )
  }

  Fmort_estimate <- estimate_Fmort_tibble("FMort", sdr)

  # natural mortality
  estimate_M_tibble <- function(obj, data) {
    dplyr::tibble(
      label = "natM",
      age = rep(0:n_ages(data), times = n_years(data)),
      # TODO:
      # I think that we want the actual range of years rather than starting at 1
      time = rep(1:n_years(data), each = 1 + n_ages(data)),
      initial = NA,
      estimates = 0.2, # should be report$M[[1]],
      uncertainty = NA,
      likelihood = NA,
      gradient = NA,
      estimated = NA
    )
  }

  M_estimate <- estimate_M_tibble(obj = obj, data = data)

  estimates <- rbind(
    M_estimate,
    Fmort_estimate,
    estimates_biomass,
    estimate_recruitment_tibble(obj = obj, data = data),
    estimate_naa_tibble("NAA", sdr, data = data)
  )

  # will pull the information from obj and/or sdreport for fits
  # NA is a placeholder
  fits <- dplyr::tibble()
  # Need to run the following timestamp code before running FIMS and after.
  timestamp <- as.POSIXlt(Sys.time(), tz = "UTC")
  version <- utils::packageVersion("FIMS")
  # match.call has to be used to create the call when the user run FIMS
  # call <- match.call()

  # Fill the empty data frames with data extracted from the data file
  out <- new(
    "FIMSOutput",
    estimates = estimates,
    fits = fits,
    tmb = obj,
    call = call,
    timestamp = timestamp,
    version = version
  )
  return(out)
}
