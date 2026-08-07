# EDM User-Facing R Interface
# This file defines the EDMFit S4 class and high-level R wrapper functions
# for fitting and forecasting with Empirical Dynamic Modeling (EDM) in FIMS.
# These interfaces simplify interaction with the underlying C++ prediction
# framework implemented in previous phases (#1528, #1561, #1620).

# EDMFit S4 class --------------------------------------------------------

# methods::setClass: ----

# Register S3 classes used as slots (mirrors fimsfit.R pattern)
methods::setOldClass("package_version")

#' The EDMFit class
#'
#' @description
#' An S4 class storing the results of a fitted Empirical Dynamic Modeling (EDM)
#' model. Objects of this class are returned by [fit_edm()] and are the primary
#' input to [edm_forecast()].
#'
#' Slots are accessed using `get_*()` accessor functions rather than the `@`
#' operator directly.
#'
#' @slot method Character. The EDM prediction method used. One of
#'   `"simplex"`, `"smap"`, or `"gp"`.
#' @slot predictions Numeric vector. In-library (fitted) one-step-ahead
#'   predictions for each row of the delay embedding library.
#' @slot embedding_name Character. Name of the delay embedding used, as stored
#'   in the source `FIMSFrame`.
#' @slot embedding_dimension Integer. The embedding dimension \eqn{E} used.
#' @slot time_lag Integer. The time lag \eqn{\tau} used.
#' @slot n_library Integer. Number of library points (rows) in the embedding.
#' @slot parameters List. Method-specific hyperparameters:
#'   \describe{
#'     \item{simplex}{\code{n_neighbors} — number of nearest neighbours used.}
#'     \item{smap}{\code{theta} — nonlinearity weighting parameter.}
#'     \item{gp}{\code{phi}, \code{sigma2}, \code{ve} — ARD length-scales,
#'       signal variance, and process noise variance.}
#'   }
#' @slot version A \code{package_version}. The version of FIMS used.
#'
#' @seealso [fit_edm()], [edm_forecast()], [get_EDMFit]
#' @export
methods::setClass(
  Class = "EDMFit",
  slots = c(
    method = "character",
    predictions = "numeric",
    embedding_name = "character",
    embedding_dimension = "integer",
    time_lag = "integer",
    n_library = "integer",
    parameters = "list",
    version = "package_version"
  )
)

# methods::setValidity: ----

methods::setValidity("EDMFit", function(object) {
  errors <- character(0)

  valid_methods <- c("simplex", "smap", "gp")
  if (!object@method %in% valid_methods) {
    errors <- c(
      errors,
      paste0(
        "Invalid method '", object@method, "'. ",
        "Must be one of: ", paste(valid_methods, collapse = ", "), "."
      )
    )
  }

  if (length(object@predictions) == 0) {
    errors <- c(errors, "predictions slot must not be empty.")
  }

  if (object@embedding_dimension < 1L) {
    errors <- c(errors, "embedding_dimension must be >= 1.")
  }

  if (object@time_lag < 1L) {
    errors <- c(errors, "time_lag must be >= 1.")
  }

  if (object@n_library < 2L) {
    errors <- c(errors, "n_library must be >= 2.")
  }

  if (length(errors) == 0) TRUE else errors
})

# methods::setMethod: show ----

methods::setMethod(
  f = "show",
  signature = "EDMFit",
  definition = function(object) {
    cli::cli_inform(c(
      "i" = "An {.cls EDMFit} object (FIMS v.{object@version})",
      "i" = "Access slots with {.fn get_*} functions, e.g.,",
      "*" = "{.fn get_method}",
      "*" = "{.fn get_predictions}",
      "*" = "{.fn get_parameters}",
      "i" = "Use {.fn print} for a formatted summary."
    ))
  }
)

# methods::setMethod: print ----

methods::setMethod(
  f = "print",
  signature = "EDMFit",
  definition = function(x, ...) {
    n_pred <- length(x@predictions)
    rho <- if (n_pred > 1) {
      stats::cor(
        x@predictions[-n_pred],
        x@predictions[-1],
        use = "complete.obs"
      )
    } else {
      NA_real_
    }

    # Format method-specific parameter string
    param_str <- switch(x@method,
      simplex = paste0(
        "n_neighbors = ", x@parameters[["n_neighbors"]]
      ),
      smap = paste0(
        "theta = ", x@parameters[["theta"]]
      ),
      gp = paste0(
        "sigma2 = ", round(x@parameters[["sigma2"]], 4),
        ", ve = ", round(x@parameters[["ve"]], 4)
      )
    )

    cli::cli_inform(c(
      "i" = "EDM Fit Summary (FIMS v.{x@version})",
      "-" = "Method         : {.strong {toupper(x@method)}}",
      "-" = "Embedding      : {x@embedding_name}",
      "-" = "Dimension (E)  : {x@embedding_dimension}",
      "-" = "Time lag (tau) : {x@time_lag}",
      "-" = "Library size   : {x@n_library}",
      "-" = "Parameters     : {param_str}",
      "-" = "Predictions    : {n_pred} fitted values"
    ))
    invisible(x)
  }
)

# methods::setMethod: accessors ----

#' Get a slot from an EDMFit object
#'
#' Accessor functions for each slot of the [EDMFit-class] S4 class returned
#' by [fit_edm()]. These are the preferred way to access stored results.
#'
#' @param x An [EDMFit-class] object returned by [fit_edm()].
#' @name get_EDMFit
#' @seealso [fit_edm()], [edm_forecast()], [EDMFit-class]
NULL

#' @return
#' [get_method()] returns a character string: one of `"simplex"`,
#' `"smap"`, or `"gp"`.
#' @export
#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setGeneric("get_method", function(x) standardGeneric("get_method"))

#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setMethod("get_method", "EDMFit", function(x) x@method)

#' @return
#' [get_predictions()] returns a numeric vector of in-library one-step-ahead
#' predictions, one per library row of the delay embedding.
#' @export
#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setGeneric(
  "get_predictions",
  function(x) standardGeneric("get_predictions")
)

#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setMethod("get_predictions", "EDMFit", function(x) x@predictions)

#' @return
#' [get_parameters()] returns a named list of method-specific
#' hyperparameters used during fitting.
#' @export
#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setGeneric(
  "get_parameters",
  function(x) standardGeneric("get_parameters")
)

#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setMethod("get_parameters", "EDMFit", function(x) x@parameters)

#' @return
#' [get_embedding_name()] returns the character name of the delay
#' embedding used.
#' @export
#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setGeneric(
  "get_embedding_name",
  function(x) standardGeneric("get_embedding_name")
)

#' @rdname get_EDMFit
#' @keywords fit_edm
methods::setMethod(
  "get_embedding_name",
  "EDMFit",
  function(x) x@embedding_name
)

# is.EDMFit ----------------------------------------------------------------

#' Test whether an object is an EDMFit
#'
#' @param x Any R object.
#' @return Logical. `TRUE` if `x` is an [EDMFit-class] object.
#' @export
#' @keywords fit_edm
is.EDMFit <- function(x) methods::is(x, "EDMFit")

# fit_edm ----------------------------------------------------------------

#' Fit an EDM model to a time series
#'
#' @description
#' High-level interface for fitting an Empirical Dynamic Modeling (EDM)
#' prediction model to a univariate or multivariate time series stored in a
#' [FIMSFrame()] object. Supports Simplex Projection, S-Map, and GP-EDM
#' methods.
#'
#' @param data A `FIMSFrame` object containing the time series data.
#' @param method Character string specifying the EDM method to use. One of
#'   `"simplex"`, `"smap"`, or `"gp"`. Default is `"simplex"`.
#' @param embedding_name A single string matching one of the named delay
#'   embeddings in `data` (created via [create_edm_embedding()]). If `NULL`,
#'   uses the first available embedding.
#' @param E Positive integer. Embedding dimension (number of lagged
#'   coordinates). Ignored if `embedding_name` is specified.
#' @param tau Positive integer. Time lag between successive coordinates.
#'   Ignored if `embedding_name` is specified.
#' @param theta Numeric. Nonlinearity parameter for S-Map. Larger values
#'   weight nearby library points more strongly. Only used when
#'   `method = "smap"`. Default is `1.0`.
#' @param n_neighbors Positive integer. Number of nearest neighbors for
#'   Simplex Projection. Defaults to `E + 1` when set to `0`.
#'   Only used when `method = "simplex"`. Default is `0`.
#' @param forecast_horizon Positive integer. Number of steps ahead to forecast.
#'   Default is `1`.
#' @param ... Additional arguments passed to the underlying EDM C++ interface.
#'
#' @return An object of class [EDMFit-class].
#'
#' @seealso [edm_forecast()], [create_edm_embedding()], [FIMSFrame()]
#' @export
#' @examples
#' \dontrun{
#' data("data_big")
#' ff <- FIMSFrame(data_big)
#' ff <- create_edm_embedding(ff, type = "landings", fleet = "fishery1", E = 3, tau = 1)
#' fit <- fit_edm(ff, method = "simplex", embedding_name = "fishery1_landings")
#' print(fit)
#' }
fit_edm <- function(data,
                    method = c("simplex", "smap", "gp"),
                    embedding_name = NULL,
                    E = 3L,
                    tau = 1L,
                    theta = 1.0,
                    n_neighbors = 0L,
                    forecast_horizon = 1L,
                    ...) {
  method <- match.arg(method)
  stopifnot(inherits(data, "FIMSFrame"))
  # TODO: Implement method dispatch to C++ EDM interfaces (Commit 3)
  stop("fit_edm() is not yet implemented. Coming in Phase 4 (GSoC 2026).")
}

# edm_forecast -----------------------------------------------------------

#' Generate forecasts from a fitted EDM model
#'
#' @description
#' Generates out-of-sample empirical forecasts from a fitted [EDMFit-class]
#' object returned by [fit_edm()].
#'
#' @param fit An [EDMFit-class] object returned by [fit_edm()].
#' @param n_ahead Positive integer. Number of time steps to forecast ahead.
#'   Default is `1`.
#' @param ... Additional arguments passed to underlying prediction methods.
#'
#' @return A tibble with columns:
#'   \describe{
#'     \item{time}{Forecast time index.}
#'     \item{forecast}{Point forecast value.}
#'     \item{se}{Standard error of the forecast (if available).}
#'   }
#'
#' @seealso [fit_edm()], [EDMFit-class]
#' @export
#' @examples
#' \dontrun{
#' fit <- fit_edm(ff, method = "smap", embedding_name = "fishery1_landings")
#' forecasts <- edm_forecast(fit, n_ahead = 5)
#' }
edm_forecast <- function(fit, n_ahead = 1L, ...) {
  stopifnot(is.EDMFit(fit))
  # TODO: Implement forecast generation from EDMFit object (Commit 4)
  stop("edm_forecast() is not yet implemented. Coming in Phase 4 (GSoC 2026).")
}
