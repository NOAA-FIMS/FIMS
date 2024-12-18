# Developers: ----

# This file defines the parent class of FIMSFit and its potential children. The
# class is an S4 class with accessors and validators but no setters. For more
# details on how to create an S4 class in FIMS please see R/fimsframe.R

# TODO: ----

# TODO: Fix "no metadata object found to revise superClass" in sdreportOrList
# TODO: Write more validity checks for FIMSFit
# TODO: Better document the return of [get_estimates()], i.e., columns
# TODO: Decide if the error from is.FIMSFits should be a single FALSE or stop
# TODO: Decide if "total" should be a part of number_of_parameters because it
#       can be calculated from fixed_effects + random_effects and would need to
#       be calculated in print.FITFims()
# TODO: Determine if report should always use last.par.best
# TODO: Make a helper function to add lower and upper CI for users in estimates
# TODO: Add Terminal SB to print()

# methods::setClass: ----

# Need to use an S3 class for the following S4 class
methods::setOldClass(Classes = "package_version")
methods::setOldClass(Classes = "difftime")
methods::setOldClass(Classes = "sdreport")
# Join sdreport and list into a class incase the sdreport is not created
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
    number_of_parameters = "integer",
    timing = "difftime",
    version = "package_version"
  )
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
    div_digit <- cli::cli_div(theme = list(.val = list(digits = 5)))
    terminal_ssb <- sapply(
      x@report[["ssb"]],
      function(y) utils::tail(y, 1)
    )
    cli::cli_inform(c(
      "i" = "FIMS model version: {.val {x@version}}",
      "i" = "Total run time was {.val {rt}} {ru}",
      "i" = "Number of parameters: {number_of_parameters}",
      "i" = "Maximum gradient= {.val {x@max_gradient}}",
      "i" = "Negative log likelihood (NLL):",
      "*" = "Marginal NLL= {.val {x@opt$objective}}",
      "*" = "Total NLL= {.val {x@report$jnll}}",
      # TODO: x@rep[["sb"]] does not exist
      "i" = "Terminal SB= "
    ))
    cli::cli_end(div_digit)
  }
)

# methods::setMethod: accessors ----

# Accessor functions for a FIMSFit object
# 1 setGeneric() per slot but potentially >1 setMethod() per setGeneric()

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
methods::setGeneric("get_input", function(x) standardGeneric("get_input"))
#' @export
#' @rdname get_FIMSFit
methods::setMethod("get_input", "FIMSFit", function(x) x@input)

#' @return
#' [get_report()] returns the TMB report, where anything that is flagged as
#' reportable in the C++ code is returned.
#' @export
#' @rdname get_FIMSFit
setGeneric("get_report", function(x) standardGeneric("get_report"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_report", "FIMSFit", function(x) x@report)

#' @return
#' [get_obj()] returns the output from [TMB::MakeADFun()].
#' @export
#' @rdname get_FIMSFit
setGeneric("get_obj", function(x) standardGeneric("get_obj"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_obj", "FIMSFit", function(x) x@obj)

#' @return
#' [get_opt()] returns the output from [nlminb()], which is the minimizer used
#' in [fit_fims()].
#' @export
#' @rdname get_FIMSFit
setGeneric("get_opt", function(x) standardGeneric("get_opt"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_opt", "FIMSFit", function(x) x@opt)

#' @return
#' [get_max_gradient()] returns the maximum gradient found when optimizing the
#' model.
#' @export
#' @rdname get_FIMSFit
setGeneric("get_max_gradient", function(x) standardGeneric("get_max_gradient"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_max_gradient", "FIMSFit", function(x) x@max_gradient)


#' @return
#' [get_sdreport()] returns the list from [TMB::sdreport()].
#' @export
#' @rdname get_FIMSFit
setGeneric("get_sdreport", function(x) standardGeneric("get_sdreport"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_sdreport", "FIMSFit", function(x) x@sdreport)

#' @return
#' [get_estimates()] returns a tibble of parameter values and their
#' uncertainties from a fitted model.
#' @export
#' @rdname get_FIMSFit
setGeneric("get_estimates", function(x) standardGeneric("get_estimates"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_estimates", "FIMSFit", function(x) x@estimates)

#' @return
#' [get_number_of_parameters()] returns a vector of integers specifying the
#' number of fixed-effect parameters and the number of random-effect parameters
#' in the model.
#' @export
#' @rdname get_FIMSFit
setGeneric(
  "get_number_of_parameters",
  function(x) standardGeneric("get_number_of_parameters")
)
#' @export
#' @rdname get_FIMSFit
setMethod(
  "get_number_of_parameters",
  "FIMSFit",
  function(x) x@get_number_of_parameters
)

#' @return
#' [get_timing()] returns the amount of time it took to run the model in
#' seconds as a `difftime` object.
#' @export
#' @rdname get_FIMSFit
setGeneric("get_timing", function(x) standardGeneric("get_timing"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_timing", "FIMSFit", function(x) x@timing)

#' @return
#' [get_version()] returns the `package_version` of FIMS that was used to fit
#' the model.
#' @export
#' @rdname get_FIMSFit
setGeneric("get_version", function(x) standardGeneric("get_version"))
#' @export
#' @rdname get_FIMSFit
setMethod("get_version", "FIMSFit", function(x) x@version)

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

#' Check if an object is a list of FIMSFit objects
#'
#' @param x List of fits returned from multiple calls to [fit_fims()].
#' @keywords fit_fims
#' @export
is.FIMSFits <- function(x) {
  if (!is.list(x)) {
    cli::cli_warn(
      message = c("x" = "{.par x} is not a list -- something went wrong.")
    )
    return(FALSE)
  }
  all(sapply(x, function(i) inherits(i, "FIMSFit")))
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
#'       A object with the `sdreport` class containing the output from
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
create_FIMSFit <- function(
  input,
  obj,
  opt = list(),
  sdreport = list(),
  timing = c("time_total" = as.difftime(0, units = "secs")),
  version = utils::packageVersion("FIMS")
) {
  # Determine the number of parameters
  n_total <- length(obj[["env"]][["last.par.best"]])
  n_fixed_effects <- length(obj[["par"]])
  number_of_parameters <- c(
    total = n_total,
    fixed_effects = n_fixed_effects,
    random_effects = n_total - n_fixed_effects
  )
  rm(n_total, n_fixed_effects)

  # Calculate the maximum gradient
  max_gradient <- if (length(opt) > 0) {
    max(abs(obj[["gr"]](opt[["par"]])))
  } else {
    NA_real_
  }

  # Get the report
  report <- if (length(opt) == 0) {
    obj[["report"]](obj[["env"]][["last.par.best"]])
  } else {
    obj[["report"]]()
  }

  if (length(sdreport) > 0) {
    std <- summary(sdreport)
    estimates <- tibble::tibble(
      as.data.frame(std)
    ) |>
      dplyr::rename(value = "Estimate", se = "Std. Error") |>
      dplyr::mutate(
        name = dimnames(std)[[1]],
        .before = "value"
      )
  } else {
    estimates <- tibble::tibble(
      name = names(obj[["par"]]),
      value = obj[["env"]][["parList"]](),
      se = NA_real_
    )
  }

  fit <- methods::new(
    "FIMSFit",
    input = input,
    obj = obj,
    opt = opt,
    max_gradient = max_gradient,
    report = report,
    sdreport = sdreport,
    estimates = estimates,
    number_of_parameters = number_of_parameters,
    timing = timing,
    version = version
  )
  fit
}

#' Fit a FIMS model (BETA)
#'
#' @param input Input list as returned by [create_default_parameters()].
#' @param get_sd A boolean specifying if the [TMB::sdreport()] should be
#'   calculated?
#' @param save_sd A logical, with the default `TRUE`, indicating whether the
#'   sdreport is returned in the output. If `FALSE`, the slot for the report
#'   will be empty.
#' @param number_of_loops A positive integer specifying the number of
#'   iterations of the optimizer that will be performed to improve the
#'   gradient.
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
#' * [create_FIMSFit()]
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
  if (!is.null(input$random)) {
    cli::cli_abort("Random effects declared but not implemented yet.")
  }
  if (number_of_newton_steps > 0) {
    cli::cli_abort("Newton steps not implemented yet.")
  }
  if (number_of_loops < 0) {
    cli::cli_abort("number_of_loops ({.par {number_of_loops}}) must be >= 0.")
  }
  obj <- MakeADFun(
    data = list(),
    parameters = input$parameters,
    map = input$map,
    random = input$random,
    DLL = "FIMS",
    silent = TRUE
  )
  if (!optimize) {
    initial_fit <- create_FIMSFit(
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
    for (ii in 2:number_of_loops) {
      # TODO: Shouldn't this be dictated by verbose?
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
  fit <- create_FIMSFit(
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

#we create an as.list method for this new FIMSFit
setMethod("as.list",signature(x="FIMSFit"),function(x) {
  mapply(function(y) {
    #apply as.list if the slot is again an user-defined object
    #therefore, as.list gets applied recursively
    if (inherits(slot(x,y),"FIMSFit")) {
      as.list(slot(x,y))
    } else {
      #otherwise just return the slot
      slot(x,y)
    }
  },
  slotNames(class(x)),
  SIMPLIFY=FALSE)
})
