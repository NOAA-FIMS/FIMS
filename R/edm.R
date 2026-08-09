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

# select_n_neighbors ----------------------------------------------------

#' Select the optimal number of nearest neighbours for Simplex Projection
#'
#' @description
#' Evaluates Simplex Projection prediction skill across a range of
#' `n_neighbors` values using in-library (self-prediction) assessment.
#' For each candidate value the function computes the Pearson correlation
#' (\eqn{\rho}) and root-mean-square error (RMSE) between the in-library
#' predictions and the embedding target values, then flags the value that
#' maximises \eqn{\rho}.
#'
#' @param data A [FIMSFrame()] object with at least one delay embedding
#'   created by [create_edm_embedding()].
#' @param embedding_name A single string matching a named embedding in `data`.
#'   If `NULL`, the first available embedding is used.
#' @param k_min Positive integer. Minimum number of neighbours to evaluate.
#'   Defaults to `E + 1`.
#' @param k_max Positive integer. Maximum number of neighbours to evaluate.
#'   Defaults to `min(floor(n_library / 2), E + 10)`.
#'
#' @return A [tibble::tibble()] with one row per candidate value and columns:
#'   \describe{
#'     \item{n_neighbors}{Candidate neighbour count.}
#'     \item{rho}{Pearson correlation between predictions and targets.}
#'     \item{rmse}{Root-mean-square prediction error.}
#'     \item{optimal}{Logical. `TRUE` for the row that maximises `rho`.}
#'   }
#'
#' @seealso [fit_edm()], [create_edm_embedding()]
#' @export
#' @examples
#' \dontrun{
#' data("data_big")
#' ff <- FIMSFrame(data_big)
#' ff <- create_edm_embedding(ff, series_type = "landings", series_name = "fleet1",
#'                            E = 3L, tau = 1L)
#' skill <- select_n_neighbors(ff)
#' print(skill)
#' }
select_n_neighbors <- function(data,
                               embedding_name = NULL,
                               k_min = NULL,
                               k_max = NULL) {

  # --- Input validation ---------------------------------------------------
  if (!methods::is(data, "FIMSFrame")) {
    cli::cli_abort("{.arg data} must be a {.cls FIMSFrame} object.")
  }

  embeddings <- get_edm_embeddings(data)
  if (length(embeddings) == 0L) {
    cli::cli_abort(c(
      "No delay embeddings found in {.arg data}.",
      "i" = "Use {.fn create_edm_embedding} to build one first."
    ))
  }

  if (is.null(embedding_name)) {
    embedding_name <- names(embeddings)[[1L]]
  }
  if (!embedding_name %in% names(embeddings)) {
    cli::cli_abort(c(
      "Embedding {.val {embedding_name}} not found.",
      "i" = "Available: {.val {names(embeddings)}}."
    ))
  }

  emb      <- embeddings[[embedding_name]]
  E_used   <- emb$E
  tau_used <- emb$tau

  # --- Rebuild DelayEmbedding ---------------------------------------------
  series_data <- dplyr::filter(
    get_data(data),
    .data[["type"]]  == emb$series_type,
    .data[["fleet"]] == emb$series_name
  ) |>
    dplyr::arrange(.data[["timing"]]) |>
    dplyr::pull(.data[["value"]])

  lib_de <- methods::new(DelayEmbedding)
  if (isTRUE(emb$drop_missing)) {
    lib_de$construct_drop_missing(
      as.numeric(series_data), as.integer(E_used), as.integer(tau_used), -999.0
    )
  } else {
    lib_de$construct(
      as.numeric(series_data), as.integer(E_used), as.integer(tau_used)
    )
  }
  lib_id       <- lib_de$get_id()
  n_lib        <- lib_de$n_rows
  target_vals  <- lib_de$target_values$get_values()

  # --- Define search range ------------------------------------------------
  k_min_used <- if (is.null(k_min)) as.integer(E_used + 1L) else as.integer(k_min)
  k_max_used <- if (is.null(k_max)) {
    as.integer(min(floor(n_lib / 2L), E_used + 10L))
  } else {
    as.integer(k_max)
  }
  k_max_used <- max(k_max_used, k_min_used)  # guard against degenerate range

  k_seq <- seq.int(k_min_used, k_max_used)

  # --- Evaluate each candidate --------------------------------------------
  rho_vec  <- numeric(length(k_seq))
  rmse_vec <- numeric(length(k_seq))

  for (i in seq_along(k_seq)) {
    k  <- k_seq[[i]]
    sp <- methods::new(SimplexProjection)
    sp$embedding_dimension <- as.integer(E_used)
    sp$n_neighbors         <- as.integer(k)
    preds <- sp$predict(lib_id, lib_id)

    rho_vec[[i]]  <- stats::cor(preds, target_vals, use = "complete.obs")
    rmse_vec[[i]] <- sqrt(mean((preds - target_vals)^2, na.rm = TRUE))
  }

  best_idx <- which.max(rho_vec)

  tibble::tibble(
    n_neighbors = k_seq,
    rho         = rho_vec,
    rmse        = rmse_vec,
    optimal     = seq_along(k_seq) == best_idx
  )
}

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
#'   Only used when `method = "simplex"`. Ignored when `auto_select = TRUE`.
#'   Default is `0`.
#' @param auto_select Logical. When `TRUE` and `method = "simplex"`, calls
#'   [select_n_neighbors()] internally to choose the `n_neighbors` value that
#'   maximises in-library Pearson correlation (\eqn{\rho}). Default is `FALSE`.
#' @param gp_phi_init Numeric vector of length `E`. Initial ARD inverse
#'   length-scale values for GP-EDM. Defaults to `rep(0.5, E)`. Only used
#'   when `method = "gp"`.
#' @param gp_sigma2_init Positive numeric. Initial signal variance for GP-EDM.
#'   Default is `1.0`. Only used when `method = "gp"`.
#' @param gp_ve_init Positive numeric. Initial process noise variance for
#'   GP-EDM. Default is `0.1`. Only used when `method = "gp"`.
#' @param forecast_horizon Positive integer. Number of steps ahead to forecast.
#'   Default is `1`.
#' @param ... Additional arguments passed to the underlying EDM C++ interface.
#'
#' @return An object of class [EDMFit-class].
#'
#' @seealso [edm_forecast()], [select_n_neighbors()], [create_edm_embedding()],
#'   [FIMSFrame()]
#' @export
#' @examples
#' \dontrun{
#' data("data_big")
#' ff <- FIMSFrame(data_big)
#' ff <- create_edm_embedding(ff, series_type = "landings", series_name = "fleet1",
#'                            E = 3L, tau = 1L)
#' # Auto-select optimal n_neighbors via cross-validation
#' fit <- fit_edm(ff, method = "simplex", auto_select = TRUE)
#' # GP-EDM with custom initial hyperparameters
#' fit_gp <- fit_edm(ff, method = "gp", gp_sigma2_init = 2.0, gp_ve_init = 0.05)
#' print(fit)
#' }
fit_edm <- function(data,
                    method = c("simplex", "smap", "gp"),
                    embedding_name = NULL,
                    E = 3L,
                    tau = 1L,
                    theta = 1.0,
                    n_neighbors = 0L,
                    auto_select = FALSE,
                    gp_phi_init = NULL,
                    gp_sigma2_init = 1.0,
                    gp_ve_init = 0.1,
                    forecast_horizon = 1L,
                    ...) {
  method <- match.arg(method)

  # --- Input validation ---------------------------------------------------
  if (!methods::is(data, "FIMSFrame")) {
    cli::cli_abort("{.arg data} must be a {.cls FIMSFrame} object.")
  }

  # --- Resolve embedding --------------------------------------------------
  embeddings <- get_edm_embeddings(data)
  if (length(embeddings) == 0L) {
    cli::cli_abort(c(
      "No delay embeddings found in {.arg data}.",
      "i" = "Use {.fn create_edm_embedding} to build one first."
    ))
  }

  if (is.null(embedding_name)) {
    embedding_name <- names(embeddings)[[1L]]
    cli::cli_inform(
      "Using first available embedding: {.val {embedding_name}}"
    )
  }

  if (!embedding_name %in% names(embeddings)) {
    cli::cli_abort(c(
      "Embedding {.val {embedding_name}} not found in {.arg data}.",
      "i" = "Available embeddings: {.val {names(embeddings)}}."
    ))
  }

  emb    <- embeddings[[embedding_name]]
  E_used   <- emb$E
  tau_used <- emb$tau

  # --- Reconstruct Rcpp DelayEmbedding from the stored series -------------
  # Re-extract the original series so the Rcpp object holds live pointers.
  series_data <- dplyr::filter(
    get_data(data),
    .data[["type"]]  == emb$series_type,
    .data[["fleet"]] == emb$series_name
  ) |>
    dplyr::arrange(.data[["timing"]]) |>
    dplyr::pull(.data[["value"]])

  lib_de <- methods::new(DelayEmbedding)
  if (isTRUE(emb$drop_missing)) {
    lib_de$construct_drop_missing(
      as.numeric(series_data),
      as.integer(E_used),
      as.integer(tau_used),
      -999.0
    )
  } else {
    lib_de$construct(
      as.numeric(series_data),
      as.integer(E_used),
      as.integer(tau_used)
    )
  }

  n_lib <- lib_de$n_rows

  if (n_lib < 2L) {
    cli::cli_abort(c(
      "Library has fewer than 2 rows after constructing the delay embedding.",
      "i" = "Check that {.arg data} has enough observations for E={E_used}, tau={tau_used}."
    ))
  }

  # --- Dispatch to Rcpp predictor -----------------------------------------
  # In-library prediction: library == test (same DelayEmbedding id).
  lib_id <- lib_de$get_id()

  result <- switch(method,

    simplex = {
      k <- if (isTRUE(auto_select)) {
        # Use cross-validation to find optimal n_neighbors
        skill <- select_n_neighbors(data, embedding_name = embedding_name)
        best  <- skill$n_neighbors[skill$optimal]
        cli::cli_inform(
          "Auto-selected n_neighbors = {best} (rho = {round(skill$rho[skill$optimal], 3)})"
        )
        as.integer(best)
      } else if (as.integer(n_neighbors) == 0L) {
        as.integer(E_used + 1L)
      } else {
        as.integer(n_neighbors)
      }
      sp <- methods::new(SimplexProjection)
      sp$embedding_dimension <- as.integer(E_used)
      sp$n_neighbors         <- k
      preds  <- sp$predict(lib_id, lib_id)
      params <- list(n_neighbors = k, auto_selected = isTRUE(auto_select))
      list(predictions = preds, parameters = params)
    },

    smap = {
      sm <- methods::new(SMapProjection)
      sm$embedding_dimension <- as.integer(E_used)
      sm$theta               <- as.numeric(theta)
      sm$kernel              <- "exponential"
      preds  <- sm$predict(lib_id, lib_id)
      params <- list(theta = as.numeric(theta), kernel = "exponential")
      list(predictions = preds, parameters = params)
    },

    gp = {
      # Use user-supplied initial hyperparameters or fall back to defaults
      phi_init <- if (!is.null(gp_phi_init)) {
        as.numeric(gp_phi_init)
      } else {
        rep(0.5, E_used)
      }
      if (length(phi_init) != E_used) {
        cli::cli_abort(c(
          "{.arg gp_phi_init} must have length equal to E ({E_used}), ",
          "got length {length(phi_init)}."
        ))
      }
      gp <- methods::new(GPEdmProjection)
      gp$embedding_dimension <- as.integer(E_used)
      gp$phi                 <- phi_init
      gp$sigma2              <- as.numeric(gp_sigma2_init)
      gp$ve                  <- as.numeric(gp_ve_init)
      fitted <- gp$fit(lib_id)   # optimises phi, sigma2, ve in-place via MAP
      preds  <- gp$predict(lib_id, lib_id)
      params <- list(
        phi          = fitted$phi,
        sigma2       = fitted$sigma2,
        ve           = fitted$ve,
        phi_init     = phi_init,
        sigma2_init  = as.numeric(gp_sigma2_init),
        ve_init      = as.numeric(gp_ve_init)
      )
      list(predictions = preds, parameters = params)
    }
  )

  # --- Construct and return EDMFit ----------------------------------------
  methods::new(
    "EDMFit",
    method             = method,
    predictions        = as.numeric(result$predictions),
    embedding_name     = embedding_name,
    embedding_dimension = as.integer(E_used),
    time_lag           = as.integer(tau_used),
    n_library          = as.integer(n_lib),
    parameters         = result$parameters,
    version            = utils::packageVersion("FIMS")
  )
}

#' Generate forecasts from a fitted EDM model
#'
#' @description
#' Generates out-of-sample empirical forecasts from a fitted [EDMFit-class]
#' object returned by [fit_edm()]. For multi-step forecasting (`n_ahead > 1`),
#' each predicted value is iteratively appended to the series and used as the
#' query state for the next step.
#'
#' @param fit An [EDMFit-class] object returned by [fit_edm()].
#' @param data The [FIMSFrame()] object that was passed to [fit_edm()].
#'   Required to reconstruct the time series for building forecast query points.
#' @param n_ahead Positive integer. Number of time steps to forecast ahead.
#'   Default is `1`.
#' @param ... Additional arguments (reserved for future use).
#'
#' @return A [tibble::tibble()] with columns:
#'   \describe{
#'     \item{step}{Integer forecast horizon (1 to `n_ahead`).}
#'     \item{time}{Forecast time index (last observed time + step).}
#'     \item{forecast}{Point forecast value.}
#'     \item{se}{Standard error. Currently `NA` for all methods; GP posterior
#'       variance will be added in a future release.}
#'   }
#'
#' @seealso [fit_edm()], [EDMFit-class]
#' @export
#' @examples
#' \dontrun{
#' data("data_big")
#' ff <- FIMSFrame(data_big)
#' ff <- create_edm_embedding(ff, series_type = "landings", series_name = "fleet1",
#'                            E = 3L, tau = 1L)
#' fit <- fit_edm(ff, method = "simplex")
#' forecasts <- edm_forecast(fit, ff, n_ahead = 5)
#' print(forecasts)
#' }
edm_forecast <- function(fit, data, n_ahead = 1L, ...) {

  # --- Input validation ---------------------------------------------------
  if (!is.EDMFit(fit)) {
    cli::cli_abort(
      "{.arg fit} must be an {.cls EDMFit} object returned by {.fn fit_edm}."
    )
  }
  if (!methods::is(data, "FIMSFrame")) {
    cli::cli_abort("{.arg data} must be a {.cls FIMSFrame} object.")
  }
  n_ahead <- as.integer(n_ahead)
  if (n_ahead < 1L) {
    cli::cli_abort("{.arg n_ahead} must be a positive integer, got {n_ahead}.")
  }

  # --- Resolve embedding metadata -----------------------------------------
  embedding_name <- get_embedding_name(fit)
  embeddings     <- get_edm_embeddings(data)

  if (!embedding_name %in% names(embeddings)) {
    cli::cli_abort(c(
      "Embedding {.val {embedding_name}} not found in {.arg data}.",
      "i" = "Was {.arg data} the same {.cls FIMSFrame} passed to {.fn fit_edm}?"
    ))
  }

  emb      <- embeddings[[embedding_name]]
  E_used   <- fit@embedding_dimension
  tau_used <- fit@time_lag
  method   <- get_method(fit)
  params   <- get_parameters(fit)

  # --- Re-extract the original time series --------------------------------
  series_tbl <- dplyr::filter(
    get_data(data),
    .data[["type"]]  == emb$series_type,
    .data[["fleet"]] == emb$series_name
  ) |>
    dplyr::arrange(.data[["timing"]])

  full_series <- series_tbl$value
  last_time   <- max(series_tbl$timing)

  # --- Rebuild library DelayEmbedding -------------------------------------
  lib_de <- methods::new(DelayEmbedding)
  if (isTRUE(emb$drop_missing)) {
    lib_de$construct_drop_missing(
      as.numeric(full_series),
      as.integer(E_used),
      as.integer(tau_used),
      -999.0
    )
  } else {
    lib_de$construct(
      as.numeric(full_series),
      as.integer(E_used),
      as.integer(tau_used)
    )
  }
  lib_id <- lib_de$get_id()

  # Helper: predict one step ahead using the last row of test_de
  .predict_last <- function(test_series) {
    test_de <- methods::new(DelayEmbedding)
    test_de$construct(as.numeric(test_series), as.integer(E_used), as.integer(tau_used))
    if (test_de$n_rows == 0L) {
      cli::cli_abort(c(
        "Cannot build a test query from the extended series.",
        "i" = "Series length {length(test_series)} is too short for E={E_used}, tau={tau_used}."
      ))
    }
    test_id <- test_de$get_id()
    preds <- switch(method,
      simplex = {
        sp <- methods::new(SimplexProjection)
        sp$embedding_dimension <- as.integer(E_used)
        sp$n_neighbors         <- as.integer(params[["n_neighbors"]])
        sp$predict(lib_id, test_id)
      },
      smap = {
        sm <- methods::new(SMapProjection)
        sm$embedding_dimension <- as.integer(E_used)
        sm$theta               <- as.numeric(params[["theta"]])
        sm$kernel              <- "exponential"
        sm$predict(lib_id, test_id)
      },
      gp = {
        gp <- methods::new(GPEdmProjection)
        gp$embedding_dimension <- as.integer(E_used)
        gp$phi                 <- as.numeric(params[["phi"]])
        gp$sigma2              <- as.numeric(params[["sigma2"]])
        gp$ve                  <- as.numeric(params[["ve"]])
        gp$predict(lib_id, test_id)
      }
    )
    # Return the LAST prediction (the most recent query state)
    utils::tail(preds, 1L)
  }

  # --- Iterative multi-step forecasting -----------------------------------
  # Each step: take the last prediction, append it to the extended series,
  # rebuild the test embedding, and predict again.
  extended_series <- as.numeric(full_series)
  forecasts       <- numeric(n_ahead)

  for (h in seq_len(n_ahead)) {
    forecasts[h]    <- .predict_last(extended_series)
    extended_series <- c(extended_series, forecasts[h])
  }

  # --- Return tidy tibble -------------------------------------------------
  tibble::tibble(
    step     = seq_len(n_ahead),
    time     = last_time + seq_len(n_ahead),
    forecast = forecasts,
    se       = NA_real_
  )
}

