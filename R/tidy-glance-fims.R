#' @importFrom generics tidy
#' @export
generics::tidy

#' @importFrom generics glance
#' @export
generics::glance

#' @importFrom generics augment
#' @export
generics::augment

#' Tidy a FIMSFit object into a parameter-level tibble
#'
#' Returns one row per estimated parameter following the
#' [generics::tidy()] convention.  Standard columns (`term`, `estimate`,
#' `std.error`, `statistic`, `p.value`) are always present; FIMS-specific
#' columns (`module_name`, `module_id`, `estimation_type`, `gradient`) are
#' appended so the full context is available for filtering and plotting.
#'
#' @section Parameter types:
#' FIMS distinguishes three `estimation_type` values:
#' \describe{
#'   \item{`"fixed_effects"`}{Directly optimized parameters (selectivity,
#'     log_Fmort, log_q, …).}
#'   \item{`"random_effects"`}{Integrated-out random effects (log_devs, …).}
#'   \item{`"derived_quantity"`}{Model outputs that are not parameters
#'     (spawning biomass, expected catches, …). Uncertainty here comes from
#'     the delta method via [TMB::sdreport()].}
#' }
#' Pass any subset of these strings to `parameters` to control which rows are
#' returned.
#'
#' @section Inference:
#' `statistic` and `p.value` are computed as a two-sided Wald z-test:
#' `z = estimate / std.error`,
#' `p = 2 * pnorm(-|z|)`.
#' These are asymptotically valid for fixed effects under regularity
#' conditions; treat them as approximate for random effects and derived
#' quantities.
#'
#' @param x A `FIMSFit` object returned from [fit_fims()].
#' @param parameters Character vector controlling which `estimation_type`
#'   values to include.  Defaults to `c("fixed_effects", "random_effects")`.
#'   Pass `"derived_quantity"` to include derived quantities such as spawning
#'   biomass and expected data values, or pass all three to get every row.
#' @param conf.int Logical (default `FALSE`).  When `TRUE`, `conf.low` and
#'   `conf.high` columns are added using a normal approximation:
#'   `estimate ± qnorm((1 + conf.level) / 2) * std.error`.
#' @param conf.level Numeric (default `0.95`).  The confidence level used when
#'   `conf.int = TRUE`.
#' @param ... Unused; present for S3 method compatibility.
#'
#' @return A [tibble::tibble()] with columns:
#' \describe{
#'   \item{`term`}{Parameter label (from `label` in [get_estimates()]).}
#'   \item{`estimate`}{Point estimate at the MLE.}
#'   \item{`std.error`}{Standard error from [TMB::sdreport()].}
#'   \item{`statistic`}{Wald z-statistic (`estimate / std.error`).}
#'   \item{`p.value`}{Two-sided p-value for the z-test.}
#'   \item{`conf.low`, `conf.high`}{Confidence bounds (only when
#'     `conf.int = TRUE`).}
#'   \item{`module_name`}{Name of the FIMS module (e.g. `"Selectivity"`).}
#'   \item{`module_id`}{Integer module identifier.}
#'   \item{`estimation_type`}{One of `"fixed_effects"`, `"random_effects"`,
#'     or `"derived_quantity"`.}
#'   \item{`gradient`}{Gradient of the log-likelihood at the MLE. Values
#'     close to zero indicate a well-converged parameter.}
#' }
#'
#' @examples
#' \dontrun{
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#'
#' fit <- create_default_parameters(
#'   configurations = create_default_configurations(data = data_4_model),
#'   data = data_4_model
#' ) |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#'
#' # Fixed and random effects (default)
#' tidy(fit)
#'
#' # Fixed effects only, with 95% confidence intervals
#' tidy(fit, parameters = "fixed_effects", conf.int = TRUE)
#'
#' # All rows including derived quantities
#' tidy(fit, parameters = c("fixed_effects", "random_effects", "derived_quantity"))
#' }
#'
#' @seealso [glance.FIMSFit()], [get_estimates()], [fit_fims()]
#' @exportS3Method generics::tidy
tidy.FIMSFit <- function(
  x,
  parameters = c("fixed_effects", "random_effects"),
  conf.int = FALSE,
  conf.level = 0.95,
  ...
) {
  valid_types <- c("fixed_effects", "random_effects", "derived_quantity")
  bad <- setdiff(parameters, valid_types)
  if (length(bad) > 0) {
    cli::cli_abort(c(
      "{.arg parameters} contains unknown estimation type{?s}: {.val {bad}}.",
      "i" = "Valid values are: {.val {valid_types}}."
    ))
  }

  # tidy() is a filtered, renamed view of get_estimates().
  # this function only selects the
  # parameter rows and maps column names to the generics/broom convention.
  # After applying fimsfit-patches.R the gradient is stored in x@gradient at
  # fit time, so this call is safe after clear().
  estimates <- get_estimates(x)

  # Metadata columns to carry through (drop index columns and data columns
  # that belong in augment(), not tidy())
  meta_cols <- intersect(
    c(
      "module_name", "module_id", "module_type", "fleet",
      "estimation_type", "gradient"
    ),
    names(estimates)
  )

  out <- estimates |>
    dplyr::filter(.data$estimation_type %in% parameters) |>
    dplyr::select(
      term      = .data$label,
      estimate  = .data$estimated,
      std.error = .data$uncertainty,
      dplyr::all_of(meta_cols)
    ) |>
    dplyr::mutate(
      estimate  = as.numeric(.data$estimate),
      std.error = as.numeric(.data$std.error),
      statistic = .data$estimate / .data$std.error,
      p.value   = 2 * stats::pnorm(-abs(.data$statistic))
    ) |>
    dplyr::relocate("statistic", "p.value", .after = "std.error")

  if (conf.int) {
    z <- stats::qnorm((1 + conf.level) / 2)
    out <- out |>
      dplyr::mutate(
        conf.low  = .data$estimate - z * .data$std.error,
        conf.high = .data$estimate + z * .data$std.error
      )
  }

  out
}

#' Glance at a FIMSFit object — one-row model summary
#'
#' Returns a single-row tibble of model-level diagnostics following the
#' [generics::glance()] convention.  Standard information-criterion columns
#' (`logLik`, `AIC`, `BIC`, `nobs`) are included alongside FIMS-specific
#' diagnostics (`max_gradient`, `marginal_nll`, `total_nll`, `converged`,
#' `terminal_ssb`).
#'
#' @section Information criteria:
#' AIC and BIC are computed from the **marginal** log-likelihood (i.e. after
#' integrating out random effects), using the number of **fixed-effect**
#' parameters as \eqn{k}:
#' \deqn{AIC = 2k - 2 \ell_m}
#' \deqn{BIC = k \log(n) - 2 \ell_m}
#' where \eqn{\ell_m} is the marginal log-likelihood and \eqn{n} is the total
#' number of data observations.  These values will be `NA` when
#' `optimize = FALSE` was passed to [fit_fims()].
#'
#' @param x A `FIMSFit` object returned from [fit_fims()].
#' @param ... Unused; present for S3 method compatibility.
#'
#' @return A [tibble::tibble()] with one row and the following columns:
#' \describe{
#'   \item{`logLik`}{Marginal log-likelihood at the MLE
#'     (`-opt$objective`).}
#'   \item{`AIC`}{Akaike information criterion (marginal, fixed effects only).}
#'   \item{`BIC`}{Bayesian information criterion (marginal, fixed effects only).}
#'   \item{`nobs`}{Total number of data observations (rows with both an
#'     observed and an expected value in [get_estimates()]).}
#'   \item{`npar_fixed`}{Number of fixed-effect parameters.}
#'   \item{`npar_random`}{Number of random-effect parameters.}
#'   \item{`marginal_nll`}{Marginal negative log-likelihood
#'     (`opt$objective`); `NA` if the model was not optimized.}
#'   \item{`total_nll`}{Total (joint) negative log-likelihood from the TMB
#'     report (`report$jnll`).}
#'   \item{`max_gradient`}{Maximum absolute gradient at the MLE. Values
#'     below 0.001 are generally considered well-converged.}
#'   \item{`converged`}{Logical; `TRUE` when `opt$convergence == 0` **and**
#'     `max_gradient < 0.001`.}
#'   \item{`terminal_ssb`}{Spawning stock biomass at the terminal year,
#'     extracted from the TMB report. Returns a list-column when multiple
#'     populations are present.}
#'   \item{`fims_version`}{The version of FIMS used to fit the model.}
#'   \item{`runtime_secs`}{Total wall-clock time of the fit in seconds.}
#' }
#'
#' @examples
#' \dontrun{
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#'
#' fit <- create_default_parameters(
#'   configurations = create_default_configurations(data = data_4_model),
#'   data = data_4_model
#' ) |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#'
#' glance(fit)
#'
#' # Compare sensitivity runs in one table
#' dplyr::bind_rows(
#'   glance(age_only_fit) |> dplyr::mutate(model = "age_only"),
#'   glance(length_only_fit) |> dplyr::mutate(model = "length_only")
#' )
#' }
#'
#' @seealso `tidy.FIMSFit()`, [get_estimates()], [fit_fims()]
#' @exportS3Method generics::glance
glance.FIMSFit <- function(x, ...) {
  opt <- get_opt(x)
  report <- get_report(x)
  npar <- get_number_of_parameters(x)

  # parameter count
  npar_fixed <- as.integer(npar[["fixed_effects"]])
  npar_random <- as.integer(npar[["random_effects"]])

  # likelihood
  # opt is an empty list when optimize = FALSE
  optimized <- length(opt) > 0
  marginal_nll <- if (optimized) opt[["objective"]] else NA_real_
  log_lik <- if (optimized) -marginal_nll else NA_real_
  total_nll <- if (!is.null(report[["jnll"]])) report[["jnll"]] else NA_real_

  # observations
  # Count rows that have both observed and expected values.
  #
  # don't call get_estimates() here. That function calls
  # reshape_tmb_estimates() which in turn calls obj$gr() —- this
  # function can segfault after clear() has freed the C++ memory.
  # reshape_json_estimates() only parses the stored JSON string (x@model_output)
  # and is safe to call at any time.
  json_estimates <- reshape_json_estimates(get_model_output(x))
  nobs <- sum(!is.na(json_estimates[["observed"]]) & !is.na(json_estimates[["expected"]]))

  # information criteria
  aic <- if (optimized) 2 * npar_fixed - 2 * log_lik else NA_real_
  bic <- if (optimized && nobs > 0) {
    npar_fixed * log(nobs) - 2 * log_lik
  } else {
    NA_real_
  }

  # convergence
  max_grad <- get_max_gradient(x)
  converged <- if (optimized) {
    isTRUE(opt[["convergence"]] == 0L) && isTRUE(max_grad < 0.001)
  } else {
    NA
  }

  # terminal spawning biomass
  # spawning_biomass is a list, one element per population; take the last
  # value from each and store as a list-column (supports multi-population).
  ssb_list <- report[["spawning_biomass"]]
  terminal_ssb <- if (!is.null(ssb_list) && length(ssb_list) > 0) {
    list(vapply(ssb_list, utils::tail, n = 1L, FUN.VALUE = numeric(1L)))
  } else {
    list(NA_real_)
  }

  # run time
  timing <- get_timing(x)
  runtime_secs <- as.numeric(timing[["time_total"]], units = "secs")

  tibble::tibble(
    logLik       = log_lik,
    AIC          = aic,
    BIC          = bic,
    nobs         = nobs,
    npar_fixed   = npar_fixed,
    npar_random  = npar_random,
    marginal_nll = marginal_nll,
    total_nll    = total_nll,
    max_gradient = max_grad,
    converged    = converged,
    terminal_ssb = terminal_ssb,
    fims_version = as.character(get_version(x)),
    runtime_secs = runtime_secs
  )
}
