# EDM User-Facing R Interface
# This file contains high-level R wrapper functions for fitting and
# forecasting with Empirical Dynamic Modeling (EDM) methods in FIMS.
# These wrappers simplify interaction with the underlying C++ prediction
# framework implemented in previous phases (#1528, #1561, #1620).

# fit_edm ----------------------------------------------------------------

#' Fit an EDM model to a time series
#'
#' @description
#' High-level interface for fitting an Empirical Dynamic Modeling (EDM)
#' prediction model to a univariate or multivariate time series stored in a
#' `FIMSFrame` object. Supports Simplex Projection, S-Map, and GP-EDM methods.
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
#' @return An object of class `EDMFit` containing:
#'   \describe{
#'     \item{method}{The EDM method used.}
#'     \item{predictions}{Numeric vector of one-step-ahead predictions.}
#'     \item{embedding}{The `DelayEmbeddingMatrix` used for fitting.}
#'     \item{parameters}{List of method-specific hyperparameters.}
#'   }
#'
#' @seealso [edm_forecast()], [create_edm_embedding()], [FIMSFrame()]
#' @export
#' @examples
#' \dontrun{
#' data("data_big")
#' ff <- FIMSFrame(data_big)
#' ff <- create_edm_embedding(ff, type = "landings", fleet = "fishery1", E = 3, tau = 1)
#' fit <- fit_edm(ff, method = "simplex", embedding_name = "fishery1_landings")
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
  # TODO: Implement method dispatch to C++ EDM interfaces
  stop("fit_edm() is not yet implemented. Coming in Phase 4 (GSoC 2026).")
}

# edm_forecast -----------------------------------------------------------

#' Generate forecasts from a fitted EDM model
#'
#' @description
#' Generates out-of-sample empirical forecasts from a fitted `EDMFit` object
#' returned by [fit_edm()].
#'
#' @param fit An `EDMFit` object returned by [fit_edm()].
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
#' @seealso [fit_edm()]
#' @export
#' @examples
#' \dontrun{
#' fit <- fit_edm(ff, method = "smap", embedding_name = "fishery1_landings")
#' forecasts <- edm_forecast(fit, n_ahead = 5)
#' }
edm_forecast <- function(fit, n_ahead = 1L, ...) {
  stopifnot(inherits(fit, "EDMFit"))
  # TODO: Implement forecast generation from EDMFit object
  stop("edm_forecast() is not yet implemented. Coming in Phase 4 (GSoC 2026).")
}
