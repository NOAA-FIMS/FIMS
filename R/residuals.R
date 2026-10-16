# This file contains functions to calculate residuals for the data in the
# tibble returned by get_estimates(). The argument names and the use of one
# function per distribution follow residuals.sdmTMB() in the sdmTMB package.

#' Calculate residuals for the data in a FIMS estimates tibble
#'
#' @description
#' Calculates residuals for every data observation in the tibble returned by
#' [get_estimates()]. [get_estimates()] calls this function, so the tibble it
#' returns already has a `resids` column next to `observed` and `expected`.
#' Call this function yourself to recalculate the residuals, e.g., for
#' estimates saved before the column was added.
#'
#' @details
#' Only Pearson residuals, i.e., `type = "pearson"`, are available. A Pearson
#' residual is the observed value minus its expected value on the scale of the
#' data, divided by the standard deviation the likelihood implies for that
#' observation. The `type` argument follows `residuals.sdmTMB()` in the
#' \pkg{sdmTMB} package so other types, e.g., one-step-ahead residuals, can be
#' added later.
#'
#' The calculation depends on the distribution used for each data stream:
#' * `normal`: `expected` is the mean, \eqn{\mu}, and \eqn{\sigma} is
#'   `exp(log_sd)`. The residual is \eqn{(y - \mu) / \sigma}.
#' * `log_normal`: `expected` is the mean on the log scale, \eqn{\mu}, and
#'   \eqn{\sigma} is `exp(log_sd)`, the standard deviation on the log scale.
#'   The residual uses the mean, \eqn{\exp(\mu + \sigma^2 / 2)}, and the
#'   variance, \eqn{\exp(2\mu + \sigma^2)(\exp(\sigma^2) - 1)}, on the scale
#'   of the data.
#' * `multinomial`: `observed` is the number in each bin and `expected` is the
#'   expected proportion in each bin, \eqn{p}. The sample size, \eqn{N}, is the
#'   sum of the observations in a composition, i.e., across the bins for one
#'   fleet, data type, and year, which is the size used in the likelihood. The
#'   residual is \eqn{(y - Np) / \sqrt{Np(1 - p)}}.
#'
#' Residuals are `NA` for rows that are not data, i.e., `input_type` is not
#' `"data"`; for missing observations, i.e., `NA` or -999; for rows where the
#' variance is not positive, e.g., an expected proportion of zero; and for any
#' other distribution. If any bin of a multinomial composition is missing, all
#' bins of that composition are `NA` because FIMS leaves the whole composition
#' out of the likelihood.
#'
#' @param estimates A tibble returned from [get_estimates()]. It must have the
#'   columns `observed`, `expected`, `distribution`, and `input_type`. The
#'   `log_sd` column is needed for normal and log-normal data.
#' @param type A string specifying the type of residual. Only `"pearson"` is
#'   available at this time.
#' @return
#' The `estimates` tibble with a `resids` column placed after `observed`.
#' An existing `resids` column is replaced.
#' @seealso [get_estimates()]
#' @export
#' @keywords diagnostics
#' @examples
#' data("estimates_big_optimized")
#' calculate_residuals(estimates_big_optimized) |>
#'   dplyr::filter(!is.na(resids)) |>
#'   dplyr::select(label, fleet, year_i, age, length, observed, resids)
calculate_residuals <- function(estimates, type = c("pearson")) {
  type <- rlang::arg_match(type)
  required_columns <- c("observed", "expected", "distribution", "input_type")
  missing_columns <- setdiff(required_columns, names(estimates))
  if (length(missing_columns) > 0) {
    cli::cli_abort(c(
      "{.arg estimates} is missing the column{?s} {.var {missing_columns}}.",
      "i" = "Use the tibble returned from {.fn get_estimates}."
    ))
  }

  # Replace an existing column rather than adding a second one
  estimates <- dplyr::select(estimates, -dplyr::any_of("resids"))
  n_rows <- NROW(estimates)
  is_data <- estimates[["input_type"]] %in% "data"
  distribution <- estimates[["distribution"]]
  observed <- as.numeric(estimates[["observed"]])
  # FIMS stores missing data as -999
  observed[observed %in% -999] <- NA_real_
  expected <- as.numeric(estimates[["expected"]])
  expected[!is.finite(expected)] <- NA_real_
  sd <- if ("log_sd" %in% names(estimates)) {
    exp(as.numeric(estimates[["log_sd"]]))
  } else {
    rep(NA_real_, n_rows)
  }

  residual_values <- rep(NA_real_, n_rows)

  rows <- which(is_data & distribution %in% "normal")
  residual_values[rows] <- pearson_normal(observed[rows], expected[rows], sd[rows])

  rows <- which(is_data & distribution %in% "log_normal")
  residual_values[rows] <- pearson_log_normal(
    observed[rows], expected[rows], sd[rows]
  )

  rows <- which(is_data & distribution %in% "multinomial")
  if (length(rows) > 0) {
    # A composition is one fleet, data type, and time step, and the bins,
    # i.e., age or length, are summed over. The index columns ending in _i,
    # e.g., year_i, identify the time step.
    composition_columns <- intersect(
      c("module_id", "label", grep("_i$", names(estimates), value = TRUE)),
      names(estimates)
    )
    composition <- estimates[rows, composition_columns] |>
      dplyr::group_by(dplyr::across(dplyr::all_of(composition_columns))) |>
      dplyr::group_indices()
    # sum() returns NA when any bin is missing, which makes every bin in that
    # composition NA, matching the likelihood
    size <- stats::ave(observed[rows], composition, FUN = sum)
    residual_values[rows] <- pearson_multinomial(observed[rows], expected[rows], size)
  }

  estimates |>
    dplyr::mutate(resids = residual_values, .after = "observed")
}

#' Pearson residuals for each distribution
#'
#' @description
#' Internal helpers used by [calculate_residuals()], one per distribution,
#' similar to the `qres_*()` functions in \pkg{sdmTMB}. Each returns `NA` when
#' the variance is not positive and finite.
#' @param observed A vector of observations on the scale of the data.
#' @param expected A vector of expected values as reported by FIMS, i.e., the
#'   mean for normal, the mean on the log scale for log-normal, and the
#'   proportion for multinomial.
#' @param sd A vector of standard deviations, i.e., `exp(log_sd)`, which is on
#'   the log scale for log-normal.
#' @param size A vector of multinomial sample sizes, one per observation.
#' @return A numeric vector of residuals the same length as `observed`.
#' @noRd
pearson_normal <- function(observed, expected, sd) {
  pearson_residual(observed - expected, sd^2)
}

#' @noRd
pearson_log_normal <- function(observed, expected, sd) {
  mean <- exp(expected + sd^2 / 2)
  variance <- exp(2 * expected + sd^2) * (exp(sd^2) - 1)
  pearson_residual(observed - mean, variance)
}

#' @noRd
pearson_multinomial <- function(observed, expected, size) {
  pearson_residual(
    observed - size * expected,
    size * expected * (1 - expected)
  )
}

#' @noRd
pearson_residual <- function(response_residual, variance) {
  out <- rep(NA_real_, length(response_residual))
  valid <- is.finite(variance) & variance > 0
  out[valid] <- response_residual[valid] / sqrt(variance[valid])
  out[!is.finite(out)] <- NA_real_
  out
}
