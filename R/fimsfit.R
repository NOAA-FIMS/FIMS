# To remove the NOTE
# no visible binding for global variable
utils::globalVariables(c(
  "parameter_id", "module_name", "module_id", "label", "initial.x", "initial.y",
  "estimate.x", "estimate.y",
  "derived_quantity_id",
  "distribution", "module_type", "n", "type_id", "values",
  "module_name.x", "module_name.y",
  "module_id.x", "module_id.y",
  "module_id_init",
  "module_type.x", "module_type.y"
))

# Developers: ----

# This file defines the parent class of FIMSFit and its potential children. The
# class is an S4 class with accessors and validators but no setters. For more
# details on how to create an S4 class in FIMS please see R/fimsframe.R

# TODO: ----

# TODO: Fix "no metadata object found to revise superClass" in sdreportOrList
# TODO: Write more validity checks for FIMSFit
# TODO: Better document the return of [get_estimates()], i.e., columns
# TODO: Make a helper function to add lower and upper CI for users in estimates

# methods::setClass: ----

# Need to use an S3 class for the following S4 class
methods::setOldClass(Classes = "package_version")
methods::setOldClass(Classes = "difftime")
methods::setOldClass(Classes = "sdreport")
# Join sdreport and list into a class in case the sdreport is not created
methods::setClassUnion("sdreportOrList", members = c("sdreport", "list"))

methods::setClass(
  Class = "FIMSFit",
  slots = c(
    input = "list",
    obj = "list",
    opt = "list",
    max_gradient = "numeric",
    report = "list",
    sdreport = "sdreportOrList",
    estimates = "tbl_df",
    fits = "tbl_df",
    number_of_parameters = "integer",
    timing = "difftime",
    version = "package_version"
  )
)

# methods::setMethod: printers ----

methods::setMethod(
  f = "show",
  signature = "FIMSFit",
  definition = function(object) {
    cli::cli_inform(c(
      "i" = "The object is of the class FIMSFit v.{get_version(object)}",
      "i" = "The slots can be accessed using {.fn get_*} functions, e.g.,",
      "*" = "{.fn get_estimates}",
      "*" = "{.fn get_obj}",
      "*" = "{.fn get_version}",
      "i" = "The following slots are available: {methods::slotNames(object)}.",
      "i" = "Use {.fn print} to see a summary of the fit."
    ))
  }
)

methods::setMethod(
  f = "print",
  signature = "FIMSFit",
  definition = function(x) {
    rt <- as.numeric(x@timing[["time_total"]], units = "secs")
    ru <- "seconds"
    if (rt > 60 * 60 * 24) {
      rt <- rt / (60 * 60 * 24)
      ru <- "days"
    } else if (rt > 60 * 60) {
      rt <- rt / (60 * 60)
      ru <- "hours"
    } else if (rt > 60) {
      rt <- rt / 60
      ru <- "minutes"
    }

    number_of_parameters <- paste(
      names(x@number_of_parameters),
      x@number_of_parameters,
      sep = "="
    )
    total_parameters <- sum(x@number_of_parameters)
    all_parameters_info <- c(number_of_parameters, paste(
      "total",
      total_parameters,
      sep = "="
    ))
    div_digit <- cli::cli_div(theme = list(.val = list(digits = 5)))
    terminal_ssb <- sapply(
      x@report[["ssb"]],
      function(y) utils::tail(y, 1)
    )
    cli::cli_inform(c(
      "i" = "FIMS model version: {.val {x@version}}",
      "i" = "Total run time was {.val {rt}} {ru}",
      "i" = "Number of parameters: {all_parameters_info}",
      "i" = "Maximum gradient= {.val {x@max_gradient}}",
      "i" = "Negative log likelihood (NLL):",
      "*" = "Marginal NLL= {.val {x@opt$objective}}",
      "*" = "Total NLL= {.val {x@report$jnll}}",
      "i" = "Terminal SB= {.val {terminal_ssb}}"
    ))
    cli::cli_end(div_digit)
  }
)

# methods::setMethod: accessors ----

# Accessor functions for a FIMSFit object
# 1 methods::setGeneric() per slot but potentially >1 methods::setMethod() per methods::setGeneric()

#' Get a slot in a FIMSFit object
#'
#' There is an accessor function for each slot in the S4 class `FIMSFit`, where
#' the function is named `get_*()` and the star can be replaced with the slot
#' name, e.g., [get_input()]. These accessor functions are the preferred way
#' to access objects stored in the available slots.
#'
#' @param x Output returned from [fit_fims()].
#' @name get_FIMSFit
#' @seealso
#' * [fit_fims()]
#' * [create_default_parameters()]
NULL

#' @return
#' [get_input()] returns the list that was used to fit the FIMS model, which
#' is the returned object from [create_default_parameters()].
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_input", function(x) standardGeneric("get_input"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_input", "FIMSFit", function(x) x@input)

#' @return
#' [get_report()] returns the TMB report, where anything that is flagged as
#' reportable in the C++ code is returned.
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_report", function(x) standardGeneric("get_report"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_report", "FIMSFit", function(x) x@report)

#' @return
#' [get_obj()] returns the output from [TMB::MakeADFun()].
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_obj", function(x) standardGeneric("get_obj"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_obj", "FIMSFit", function(x) x@obj)

#' @return
#' [get_opt()] returns the output from [nlminb()], which is the minimizer used
#' in [fit_fims()].
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_opt", function(x) standardGeneric("get_opt"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_opt", "FIMSFit", function(x) x@opt)

#' @return
#' [get_max_gradient()] returns the maximum gradient found when optimizing the
#' model.
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_max_gradient", function(x) standardGeneric("get_max_gradient"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_max_gradient", "FIMSFit", function(x) x@max_gradient)


#' @return
#' [get_sdreport()] returns the list from [TMB::sdreport()].
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_sdreport", function(x) standardGeneric("get_sdreport"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_sdreport", "FIMSFit", function(x) x@sdreport)

#' @return
#' [get_estimates()] returns a tibble of parameter values and their
#' uncertainties from a fitted model.
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_estimates", function(x) standardGeneric("get_estimates"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_estimates", "FIMSFit", function(x) x@estimates)

#' @return
#' [get_fits()] returns a tibble of input data and corresponding
#'  expected values, likelihoods, and additional metadata from a fitted model.
#'  The tibble includes the following variables:
#' \itemize{
#'  \item{module_name: Character string that describes the name of the module,
#'  e.g., `"Data"`.}
#'  \item{module_id: Integer that provides identifier for linking outputs.}
#'  \item{label: Character string that describes type of data.}
#'  \item{data_id: Not yet implemented/NA: Integer that will provide
#'  unique identifier for data.}
#'  \item{fleet_name: Not yet implemented/NA: Character string that
#' will provide fleet name corresponding to name provided via FIMSFrame.}
#'  \item{unit: Not yet implemented/NA: Character string that will
#'  describe appropriate units for the data inputs and expected values.}
#'  \item{uncertainty: Not yet implemented/NA: Character string that will
#'  describe the uncertainty specified for the data input value.}
#'  \item{age: Not yet implemented/NA: Integer that will provide the age
#'  affiliated with a data input, where appropriate.}
#'  \item{length: Not yet implemented/NA: Integer that will provide the length
#'  affiliated with a data input, where appropriate.}
#'  \item{datestart: Not yet implemented/NA: Character string that will provide
#'  the start date for the data input, corresponding to the value provided in
#'  the input data.}
#'  \item{dateend: Not yet implemented/NA: Character string that will provide
#'  the end date for the data input, corresponding to the value provided in
#'  the input data.}
#'  \item{year: Not yet implemented/NA: Integer that will provide model year
#'  for the data input.}
#'  \item{init: Numeric that provides the initial value for the data input.}
#'  \item{expected: Numeric that provides the expected value for the data input.
#'  *NOTE: units for provided init and expected values need to be standardized.}
#'  \item{log_like: Numeric that provides log-likelihood for expected value.}
#'  \item{distribution: Character string that indicates the distribution used
#'  for the log-likelihood estimation.}
#'  \item{re_estimated: Logical that indicates whether any random effects were
#'  estimated during model fitting. Log-likelihood values should not be directly
#'  compared between models with and without estimation of random effects.}
#'  \item{log_like_cv: Not yet implemented/NA: Numeric that will indicate
#'  corresponding uncertainty for the log-likelihood value.}
#'  \item{weight: Numeric that indicates data weighting applied to each data
#'  value; manually fixed at 1. *NOTE: Will need to be made responsive to
#'  user-specified or user-estimated data weighting once data weighting is
#'  added to FIMS as a feature.}
#' }
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_fits", function(x) standardGeneric("get_fits"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_fits", "FIMSFit", function(x) x@fits)

#' @return
#' [get_number_of_parameters()] returns a vector of integers specifying the
#' number of fixed-effect parameters and the number of random-effect parameters
#' in the model.
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric(
  "get_number_of_parameters",
  function(x) standardGeneric("get_number_of_parameters")
)
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod(
  "get_number_of_parameters",
  "FIMSFit",
  function(x) x@number_of_parameters
)

#' @return
#' [get_timing()] returns the amount of time it took to run the model in
#' seconds as a `difftime` object.
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_timing", function(x) standardGeneric("get_timing"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_timing", "FIMSFit", function(x) x@timing)

#' @return
#' [get_version()] returns the `package_version` of FIMS that was used to fit
#' the model.
#' @export
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setGeneric("get_version", function(x) standardGeneric("get_version"))
#' @rdname get_FIMSFit
#' @keywords fit_fims
methods::setMethod("get_version", "FIMSFit", function(x) x@version)

# methods::setValidity ----

methods::setValidity(
  Class = "FIMSFit",
  method = function(object) {
    errors <- character()

    # Check that obj is from TMB::MakeADFun()
    TMB_MakeADFun_names <- c(
      "par", "fn", "gr", "he", "hessian", "method", "retape", "env", "report",
      "simulate"
    )
    if (!setequal(names(object@obj), TMB_MakeADFun_names)) {
      errors <- c(
        errors,
        "obj must be a list returned from TMB::MakeADFun() but it does not
        appear to be so because it does not have the standard names."
      )
    }

    # Return
    if (length(errors) == 0) {
      return(TRUE)
    } else {
      return(errors)
    }
  }
)

# methods::setMethod: is.FIMSFit ----

#' Check if an object is of class FIMSFit
#'
#' @param x Returned list from [fit_fims()].
#' @keywords fit_fims
#' @export
is.FIMSFit <- function(x) {
  inherits(x, "FIMSFit")
}

# Constructors ----

#' Class constructors for class `FIMSFit` and associated child classes
#'
#' Create an object with the class of `FIMSFit` after running a FIMS model. This
#' is typically done within [fit_fims()] but it can be create manually by the
#' user if they have used their own bespoke code to fit a FIMS model.
#'
#' @inheritParams fit_fims
#' @param obj An object returned from [TMB::MakeADFun()].
#' @param opt An object returned from an optimizer, typically from
#'   [stats::nlminb()], used to fit a TMB model.
#' @param sdreport An object of the `sdreport` class as returned from
#'   [TMB::sdreport()].
#' @param timing A vector of at least length one, where all entries are of the
#'   `timediff` class and at least one is named "time_total". This information
#'   is available in [fit_fims()] and added to this argument internally but if
#'   you are a power user you can calculate the time it took to run your model
#'   by subtracting two [Sys.time()] objects.
#' @param version The version of FIMS that was used to optimize the model. If
#'   [fit_fims()] was not used to optimize the model, then the default is to
#'   use the current version of the package that is loaded.
#'
#' @return
#' An object with an S4 class of `FIMSFit` is returned. The object will have the
#' following slots:
#'   \describe{
#'     \item{\code{input}:}{
#'       A list containing the model setup in the same form it was passed.
#'     }
#'     \item{\code{obj}:}{
#'       A list returned from [TMB::MakeADFun()] in the same form it was passed.
#'     }
#'     \item{\code{opt}:}{
#'       A list containing the optimized model in the same form it was passed.
#'     }
#'     \item{\code{max_gradient}:}{
#'       The maximum gradient found when optimizing the model. The default is
#'       `NA`, which means that the model was not optimized.
#'     }
#'     \item{\code{report}:}{
#'       A list containing the model report from `obj[["report"]]()`.
#'     }
#'     \item{\code{sdreport}:}{
#'       An object with the `sdreport` class containing the output from
#'       `TMB::sdreport(obj)`.
#'     }
#'     \item{\code{estimates}:}{
#'       A table of parameter values and their uncertainty.
#'     }
#'     \item{\code{timing}:}{
#'       The length of time it took to run the model if it was optimized.
#'     }
#'     \item{\code{version}:}{
#'       The package version of FIMS used to fit the model or at least the
#'       version used to create this output, which will not always be the same
#'       if you are running this function yourself.
#'     }
#'  }
#' @keywords fit_fims
#' @export
FIMSFit <- function(
    input,
    obj,
    opt = list(),
    sdreport = list(),
    timing = c("time_total" = as.difftime(0, units = "secs")),
    version = utils::packageVersion("FIMS")) {
  # Determine the number of parameters
  n_total <- length(obj[["env"]][["last.par.best"]])
  n_fixed_effects <- length(obj[["par"]])
  n_random_effects <- length(obj[["env"]][["parList()"]][["re"]])
  number_of_parameters <- c(
    fixed_effects = n_fixed_effects,
    random_effects = n_random_effects
  )
  rm(n_total, n_fixed_effects, n_random_effects)

  # Calculate the maximum gradient
  max_gradient <- if (length(opt) > 0) {
    max(abs(obj[["gr"]](opt[["par"]])))
  } else {
    NA_real_
  }

  # Rename parameters instead of "p"
  parameter_names <- names(get_parameter_names(obj[["par"]]))
  names(obj[["par"]]) <- parameter_names
  random_effects_names <- names(get_random_names(obj[["env"]][["parList()"]][["re"]]))

  # Get the report
  report <- if (length(opt) == 0) {
    obj[["report"]](obj[["env"]][["last.par.best"]])
  } else {
    obj[["report"]]()
  }

  if (length(sdreport) > 0) {
    # rename the sdreport
    names(sdreport[["par.fixed"]]) <- parameter_names
    dimnames(sdreport[["cov.fixed"]]) <- list(parameter_names, parameter_names)
  }

  # Reshape the TMB estimates
  # If the model is not optimized, opt is an empty list and is not used in
  # reshape_tmb_estimates().
  tmb_estimates <- reshape_tmb_estimates(
    obj = obj,
    sdreport = sdreport,
    opt = opt,
    parameter_names = parameter_names
  )

  # Create JSON output for FIMS run
  finalized_fims <- input[["model"]]$get_output()

  return(finalized_fims)
  # Reshape the JSON estimates
  json_estimates <- reshape_json_estimates(finalized_fims, opt)

  # Merge json_estimates into tmb_estimates based on common columns
  # TODO: need to update the derived quantities section of the tibble
  # The outputs from TMB and JSON are not the same, difficult to join them
  estimates <- dplyr::full_join(
    tmb_estimates,
    json_estimates,
    by = dplyr::join_by(
      parameter_id,
      module_name,
      module_id,
      label,
      estimation_type
    )
  ) |>
    dplyr::mutate(
      # if estimation_type = "constant" and initial.x = NA, then set initial.x = initial.y
      # and set estimate.x = estimate.y. Here .x represents the TMB values and .y
      # represents the JSON values. json_estimates have values
      # for all parameters, including constant (not estimated) parameters.
      initial.x = ifelse(
        estimation_type == "constant" & is.na(initial.x),
        initial.y,
        initial.x
      ),
      estimate.x = ifelse(
        estimation_type == "constant" & is.na(estimate.x),
        estimate.y,
        estimate.x
      )
    ) |>
    # Select the relevant columns for the final output
    # Drop the initial and estimate columns from the json_estimates and
    # use values from tmb_estimates. The values from json_estimates are
    # slightly different from the values from tmb_estimates, most likely
    # due to rounding differences.
    dplyr::select(
      -c(initial.y, estimate.y)
    ) |>
    # Drop .x suffix from column names
    dplyr::rename(
      initial = initial.x,
      estimate = estimate.x
    ) |>
    # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
    dplyr::relocate(module_name, module_id, module_type, label, type, type_id, .before = tidyselect::everything()) |>
    # Reorder the rows by `parameter_id`
    dplyr::arrange(parameter_id) |>
    # Add derived quantity IDs to the tibble for merging with the JSON output
    # TODO: Refactor once we can reliably extract unique IDs from
    # both the JSON and TMB outputs.
    dplyr::group_by(label) |>
    dplyr::mutate(
      derived_quantity_id = ifelse(
        is.na(parameter_id),
        paste0(label, "_", seq_len(dplyr::n())),
        NA_character_
      )
    )

  # Reshape the JSON derived quantities
  json_derived_quantities <- reshape_json_derived_quantities(finalized_fims) |>
    # Add derived quantity IDs to the tibble for merging with the JSON output
    # TODO: Refactor once we can reliably extract unique IDs from
    # both the JSON and TMB outputs.
    dplyr::group_by(name) |>
    dplyr::mutate(
      derived_quantity_id = paste0(name, "_", seq_len(dplyr::n()))
    ) |>
    # Rename the columns to match the tmb_estimates
    dplyr::rename(
      label = name,
      estimate = values
    )

  # Merge json_derived_quantities into estimates based on common columns
  estimates <- dplyr::full_join(
    estimates,
    json_derived_quantities,
    by = dplyr::join_by(
      derived_quantity_id,
      label
    )
  ) |>
    # Fill missing values in .x columns (TMB output) using corresponding values
    # from .y columns (JSON output).
    dplyr::mutate(
      module_name.x = dplyr::coalesce(module_name.x, module_name.y),
      module_id.x = dplyr::coalesce(module_id.x, module_id.y),
      module_type.x = dplyr::coalesce(module_type.x, module_type.y),
      estimate.x = dplyr::coalesce(estimate.x, estimate.y)
    ) |>
    # Drop .x suffix from column names
    dplyr::rename(
      module_name = module_name.x,
      module_id = module_id.x,
      module_type = module_type.x,
      estimate = estimate.x
    ) |>
    # Select the relevant columns for the final output
    dplyr::select(
      -c(
        derived_quantity_id, module_name.y, module_id.y, module_type.y,
        estimate.y
      )
    ) |>
    dplyr::ungroup()

  # Create fits tibble
  # TODO: Develop better/more reliable method for obtaining 'module_name'/'module_id'/'label' from output
  # TODO: Standardize 'init' and 'expected' units for distributions Dlnorm, Dmultinom
  # TODO: Develop means to provide values for remaining columns with NAs

  # Obtain preliminary 'init' values for data inputs in JSON, using 'values'
  # Necessary to obtain 'label' values (e.g., "Index", "AgeComp", etc.) for
  # final 'init', 'expected', etc.
  data_init_res <- reshape_json_values(finalized_fims) |>
    dplyr::filter(module_name == "data") |> # remove EWAA
    dplyr::rename(
      init_values = value,
      label = module_type,
      module_id_init = module_id
    ) |>
    dplyr::arrange(module_id_init) # ensure this aligns with 'data_fits_res'

  # Obtain 'log_like', 'distribution', 'init', and 'expected'
  data_fits_res <- reshape_json_fits(finalized_fims) |>
    dplyr::select(-module_name) |>
    dplyr::rename(distribution = module_type) |>
    # manually standardize distribution names
    dplyr::mutate(
      distribution = dplyr::case_when(
        distribution == "log_normal" ~ "Dlnorm",
        .default = distribution
      )
    ) |>
    # remove likelihoods not affiliated with data (e.g., recruitment)
    dplyr::filter(module_id %in% unique(data_init_res$module_id_init)) |>
    # ensure module_id order aligns with 'data_init_res'
    dplyr::arrange(module_id) |>
    # assign T/F to 're_estimated' based on estimation_type == "random_effects"
    dplyr::mutate(re_estimated = any(json_estimates$estimation_type == "random_effects"))

  # Join, format data into final 'fits' tibble
  fits <- data_init_res |>
    cbind(data_fits_res) |> # Consider test to ensure 'init_values' = 'init'
    # Provide blank fields for currently missing values
    dplyr::mutate(
      data_id = NA_integer_, # not available from JSON output
      fleet_name = NA_character_, # not available from JSON output
      unit = NA_character_, # not available from JSON output
      uncertainty = NA_real_, # not available from JSON output
      age = NA_integer_, # not available from JSON output
      length = NA_integer_, # not available from JSON output
      datestart = NA_character_, # not available from JSON output
      dateend = NA_character_, # not available from JSON output
      year = NA_integer_, # not available from JSON output
      log_like_cv = NA_real_, # future feature
      weight = 1.0 # future feature; fixed at 1.0 for time being
    ) |>
    dplyr::select(
      "module_name", "module_id", "label", "data_id", "fleet_name",
      "unit", "uncertainty", "age", "length", "datestart",
      "dateend", "year", "init", "expected", "log_like",
      "distribution", "re_estimated", "log_like_cv", "weight"
    ) |>
    tibble::as_tibble()

  fit <- methods::new(
    "FIMSFit",
    input = input,
    obj = obj,
    opt = opt,
    max_gradient = max_gradient,
    report = report,
    sdreport = sdreport,
    estimates = estimates,
    fits = fits,
    number_of_parameters = number_of_parameters,
    timing = timing,
    version = version
  )
  fit
}

#' Fit a FIMS model (BETA)
#'
#' @param input Input list as returned by [initialize_fims()].
#' @param get_sd A boolean specifying if the [TMB::sdreport()] should be
#'   calculated?
#' @param save_sd A logical, with the default `TRUE`, indicating whether the
#'   sdreport is returned in the output. If `FALSE`, the slot for the report
#'   will be empty.
#' @param number_of_loops A positive integer specifying the number of
#'   iterations of the optimizer that will be performed to improve the
#'   gradient. The default is three, leading to four total optimization steps.
#' @param optimize Optimize (TRUE, default) or (FALSE) build and return
#'   a list containing the obj and report slot.
#' @param number_of_newton_steps The number of Newton steps using the inverse
#'   Hessian to do after optimization. Not yet implemented.
#' @param control A list of optimizer settings passed to [stats::nlminb()]. The
#'   the default is a list of length three with `eval.max = 1000`,
#'   `iter.max = 10000`, and `trace = 0`.
#' @param filename Character string giving a file name to save the fitted
#'   object as an RDS object. Defaults to 'fit.RDS', and a value of NULL
#'   indicates not to save it. If specified, it must end in .RDS. The file is
#'   written to folder given by `input[["path"]]`. Not yet implemented.
#' @return
#' An object of class `FIMSFit` is returned, where the structure is the same
#' regardless if `optimize = TRUE` or not. Uncertainty information is only
#' included in the `estimates` slot if `get_sd = TRUE`.
#' @seealso
#' * [FIMSFit()]
#' @details This function is a beta version still and subject to change
#'   without warning.
#' @keywords fit_fims
#' @export
fit_fims <- function(input,
                     get_sd = TRUE,
                     save_sd = TRUE,
                     number_of_loops = 3,
                     optimize = TRUE,
                     number_of_newton_steps = 0,
                     control = list(
                       eval.max = 10000,
                       iter.max = 10000,
                       trace = 0
                     ),
                     filename = NULL) {
  # See issue 455 of sdmTMB to see what should be used.
  # https://github.com/pbs-assess/sdmTMB/issues/455
  # NOTE: When we add implementation for newton step we need to
  # review the above github issue to make sure we maintain continuity
  # between outputs as last.par may not equal last.par.best due to
  # the smallest newton gradient solution not matching the smallest
  # likelihood value. This can cause sanity issues in output reporting.
  if (number_of_newton_steps > 0) {
    cli::cli_abort("Newton steps not implemented yet.")
  }
  if (number_of_loops < 0) {
    cli::cli_abort("number_of_loops ({.par {number_of_loops}}) must be >= 0.")
  }
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = input$parameters,
    map = input$map,
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  if (!optimize) {
    initial_fit <- FIMSFit(
      input = input,
      obj = obj,
      timing = c("time_total" = as.difftime(0, units = "secs"))
    )
    return(initial_fit)
  }
  if (!is_fims_verbose()) {
    control$trace <- 0
  }
  ## optimize and compare
  cli::cli_inform(c("v" = "Starting optimization ..."))
  t0 <- Sys.time()
  opt <- with(
    obj,
    nlminb(
      start = par,
      objective = fn,
      gradient = gr,
      control = control
    )
  )
  maxgrad0 <- maxgrad <- max(abs(obj$gr(opt$par)))
  if (number_of_loops > 0) {
    cli::cli_inform(c(
      "i" = "Restarting optimizer {number_of_loops} times to improve gradient."
    ))
    for (ii in 1:number_of_loops) {
      # control$trace is reset to zero regardless of verbosity because the
      # differences in values printed out using control$trace will be
      # negligible between these different runs and is not worth printing
      control$trace <- 0
      opt <- with(
        obj,
        nlminb(
          start = opt[["par"]],
          objective = fn,
          gradient = gr,
          control = control
        )
      )
      maxgrad <- max(abs(obj[["gr"]](opt[["par"]])))
    }
    div_digit <- cli::cli_div(theme = list(.val = list(digits = 5)))
    cli::cli_inform(c(
      "i" = "Maximum gradient went from {.val {maxgrad0}} to
            {.val {maxgrad}} after {number_of_loops} steps."
    ))
    cli::cli_end(div_digit)
  }
  time_optimization <- Sys.time() - t0
  cli::cli_inform(c("v" = "Finished optimization"))

  time_sdreport <- NA
  if (get_sd) {
    t2 <- Sys.time()
    sdreport <- TMB::sdreport(obj)
    cli::cli_inform(c("v" = "Finished sdreport"))
    time_sdreport <- Sys.time() - t2
  } else {
    sdreport <- list()
    time_sdreport <- as.difftime(0, units = "secs")
  }

  timing <- c(
    time_optimization = time_optimization,
    time_sdreport = time_sdreport,
    time_total = Sys.time() - t0
  )
  fit <- FIMSFit(
    input = input,
    obj = obj,
    opt = opt,
    sdreport = sdreport,
    timing = timing
  )
  print(fit)
  if (!is.null(filename)) {
    cli::cli_warn(c(
      "i" = "Saving output to file is not yet implemented."
    ))
    # saveRDS(fit, file=file.path(input[["path"]], filename))
  }
  return(fit)
}

# we create an as.list method for this new FIMSFit
methods::setMethod("as.list", signature(x = "FIMSFit"), function(x) {
  mapply(
    function(y) {
      # apply as.list if the slot is again an user-defined object
      # therefore, as.list gets applied recursively
      if (inherits(slot(x, y), "FIMSFit")) {
        as.list(slot(x, y))
      } else {
        # otherwise just return the slot
        slot(x, y)
      }
    },
    slotNames(class(x)),
    SIMPLIFY = FALSE
  )
})
