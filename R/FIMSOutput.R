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
#' @param obj The output from `obj$report(obj$env$last.par.best)`.
#' @param sdr sdr The returned object from running [TMB::sdreport(obj)].
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
  # report <- obj$report(obj$env$last.par.best)

  # The code will break if
  # * The age bins are not the same as the ages in the data, e.g., if age 3 is
  #   not in the data or if there is a plus group that is created internally
  # * The model reports values for age zero fish but age zero is not in the data
  # Questions:
  # * How should the initial year should be labeled, e.g., should it always be
  #   zero or one minus the initial year?
  # * Should we be reporting the covariance from sdr?
  # * Should time by year because that is what it actually is at the moment?
  # * What order is the index and fishing mortality information given in?
  # Things to change in C++ code
  # [ ] SSB should be SB
  # [ ] Recruitment should be in sdr to get uncertainty
  # Things to change in FIMSFrame
  # [x] fleets(data) should give fleet names not the number of fleets
  # [x] need n_fleets(data)

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
    estimates = get_estimates(sdr, data),
    fits = get_fits(),
    tmb = obj,
    call = call,
    timestamp = timestamp,
    version = version
  )
  return(out)
}

# Questions
# * Is there a purrr or dplyr::split way to make the indexing more robust?
#   E.g., if if the name is length 30 then, length 31 then, length 720 then as
#   in can you use n_fleets, n_ages, n_years to determine the order of the
#   index?
get_estimates <- function(sdr, data) {
  # Need to define tibble with zero rows so NA will be used to fill in
  # missing sections when combining estimates and derived quantities later
  estimates_outline <- dplyr::tibble(
    label = character(),
    fleet = character(),
    age = numeric(),
    time = numeric(),
    initial = numeric(),
    estimate = numeric(),
    uncertainty = numeric(),
    likelihood = numeric(),
    gradient = numeric(),
    estimated = logical()
  )

  # Rules of reporting in sdreport
  # Ages are always reported by age then by year, e.g.,
  # age 1, age 2, age 3 for year 1 then age 1, age 2, age 3 for year 2
  year_int_line <- (start_year(data) - 1):end_year(data)
  age_int_line <- 1:n_ages(data)
  year_vector <- c(
    # Numbers at age
    rep(year_int_line, each = n_ages(data)),
    # Biomass
    rep(year_int_line, 1),
    # Spawning biomass
    rep(year_int_line, 1),
    # Ln Recruitment Deviations
    rep(year_int_line[-1], 1),
    # Fishing mortality
    rep(year_int_line[-1], times = n_fleets(data)),
    # Index
    rep(year_int_line[-1], times = n_fleets(data)),
    # Catch numbers at age
    rep(rep(year_int_line[-1], each = n_ages(data)), times = n_fleets(data))
  )
  age_vector <- c(
    # Numbers at age
    rep(age_int_line, times = length(year_int_line)),
    # Biomass
    rep(NA, length(year_int_line)),
    # Spawning biomass
    rep(NA, length(year_int_line)),
    # Ln Recruitment Deviations
    rep(NA, n_years(data)),
    # Fishing mortality
    rep(NA, each = n_years(data) * n_fleets(data)),
    # Index
    rep(NA, each = n_years(data) * n_fleets(data)),
    # Catch numbers at age
    rep(rep(age_int_line, times = n_years(data)), times = n_fleets(data))
  )
  fleet_vector <- c(
    # Numbers at age
    rep(NA, n_ages(data) * length(year_int_line)),
    # Biomass
    rep(NA, length(year_int_line)),
    # Spawning biomass
    rep(NA, length(year_int_line)),
    # Ln Recruitment Deviations
    rep(NA, n_years(data)),
    # Fishing mortality
    rep(fleets(data), each = n_years(data)),
    # Index
    rep(fleets(data), each = n_years(data)),
    # Catch numbers at age
    rep(fleets(data), each = n_ages(data) * n_years(data))
  )
  sdr_tibble <- tibble::tibble(
    label = names(sdr[["value"]]),
    fleet = fleet_vector,
    age = age_vector,
    time = year_vector,
    estimate = sdr[["value"]],
    uncertainty = sdr[["sd"]]
  )

  fixed_effects_tibble <- dplyr::tibble(
    label = "",
    estimate = sdr[["par.fixed"]],
    uncertainty = sqrt(diag(sdr[["cov.fixed"]])),
    gradient = sdr[["gradient.fixed"]][1, ]
  )
  estimates <- estimates_outline |>
    dplyr::full_join(
      sdr_tibble,
      by = c("label", "fleet", "age", "time", "estimate", "uncertainty")
    ) |>
    dplyr::full_join(
      fixed_effects_tibble,
      by = c("label", "estimate", "uncertainty", "gradient")
    ) |>
    dplyr::mutate(
      label = dplyr::case_when(
        label == "Biomass" ~ "pop-b",
        label == "ExpectedIndex" ~ "pop-index",
        label == "FMort" ~ "pop-F_mort",
        label == "LogRecDev" ~ "rec-ln_rec_dev",
        grepl("NAA", label) ~ "pop-naa",
        label == "SSB" ~ "pop-sb",
        .default = label
      )
    )
  return(estimates)
}

get_fits <- function(obj, sdr) {
  fits <- dplyr::tibble(
  )
  return(fits)
}
