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

setClass(
  Class = "FIMSOutput",
  slots = c(
    estimates = "tibble",
    fits = "tibble",
    tmb = "list",
    sdreport = "list",
    call = "call",
    timestamp = "vector",
    version = "list"
  )
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
#' @param data A `data.frame` that contains the necessary columns
#'   to construct a output of FIMS of a given `FIMSOutput-class`.
#' @importFrom dplyr tibble
#' @return An object of the S4 class `FIMSOutput` or one of its child classes
#' is validated and then returned. All objects will at a minimum have a slot
#' called `data` to store the input data frame. Additional slots are dependent
#' on the child class. Use [showClass()] to see all available slots.
create_fims_output <- function(tmb, sdreport, call) {
  # SSB and Biomass
  # will pull the information from tmb and/or sdreport for estimates, par is a placeholder
  # The following only applies to SSB and Biomass and need to modify to include other parameters and quantities
  estimate_biomass_tibble <- function(derivedquanname, sdr = sdr) {
    tibble(
      label = derivedquanname, age = NA, time = 0:nyears, initial = NA, estimates = sdr$value[names(sdr$value) == derivedquanname],
      uncertainty = sdr$sd[names(sdr$value) == derivedquanname], likelihood = NA, gradient = NA, estimated = NA
    )
  }
  # NAA
  estimate_naa_tibble <- function(derivedquanname, sdr = sdr) {
    tibble(
      label = derivedquanname, age = rep(ages, times = (nyears + 1)), time = rep(0:nyears, each = nages), initial = NA, estimates = sdr$value[names(sdr$value) == derivedquanname],
      uncertainty = sdr$sd[names(sdr$value) == derivedquanname], likelihood = NA, gradient = NA, estimated = NA
    )
  }

  estimates_biomass <- do.call("rbind", lapply(c("SSB", "Biomass"), estimate_biomass_tibble, sdr = sdr))

  # recruitment (need to add uncertainty)
  estimate_recruitment_tibble <- function(obj) {
    tibble(
      label = "recruitment", age = NA, time = 1:(nyears + 1), initial = NA, estimates = obj$report()$recruitment[[1]],
      uncertainty = NA, likelihood = NA, gradient = NA, estimated = NA
    )
  }

  # the fising mortality needs the year vector
  estimate_Fmort_tibble <- function(parname, sdr = sdr) {
    tibble(
      label = parname, age = NA, time = NA, initial = NA, estimates = sdr$value[names(sdr$value) == parname],
      uncertainty = sdr$sd[names(sdr$value) == parname], likelihood = NA, gradient = NA, estimated = NA
    )
  }

  Fmort_estimate <- estimate_Fmort_tibble("FMort", sdr)

  # natural mortality
  estimate_M_tibble <- function(obj) {
    tibble(
      label = "natM", age = rep(ages, times = nyears), time = rep(1:nyears, each = nages), initial = NA, estimates = obj$report()$M[[1]],
      uncertainty = NA, likelihood = NA, gradient = NA, estimated = NA
    )
  }

  M_estimate <- estimate_M_tibble(obj = obj)

  estimates <- rbind(M_estimate, Fmort_estimate, estimates_biomass, estimate_recruitment_tibble(obj = obj), estimate_naa_tibble("NAA", sdr))

  # will pull the information from tmb and/or sdreport for fits, par is a placeholder
  fits <- tmb$par
  fits <- sdreport$par
  # Need to run the following timestamp code before running FIMS and after.
  timestamp <- as.POSIXlt(Sys.time(), tz = "UTC")
  # The following code does not work now but it is the correct comment for getting packageVersion
  version <- packageVersion("FIMS")
  # match.call has to be used to create the call when the user run FIMS
  # call <- match.call()

  # Fill the empty data frames with data extracted from the data file
  out <- new("FIMSOutput",
    estimates = estimates,
    fits = fits,
    tmb = tmb,
    sdreport = sdreport,
    call = call,
    timestamp = timestamp,
    version = version
  )
  return(out)
}
