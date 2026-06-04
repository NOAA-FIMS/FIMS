#' Augment a FIMSFit object for use with yardstick
#'
#' Returns a tidy tibble of observed-vs-expected pairs drawn from the output of
#' [get_estimates()].  Only rows that have both an observed value and a
#' model-expected value are included (i.e. data-likelihood rows), so parameter
#' rows without data observations are automatically dropped.
#'
#' The returned tibble follows the conventions expected by every
#' [yardstick::metric_set()] metric function:
#'
#' | Column        | Role                                                      |
#' |---------------|-----------------------------------------------------------|
#' | `.truth`      | Observed data value (maps from `observed`)                |
#' | `.pred`       | Model-expected value (maps from `expected`)               |
#' | `.weight`     | Inverse-variance weight from `uncertainty` (optional)     |
#' | `label`       | Parameter / quantity label, e.g. `"landings_expected"`    |
#' | `fleet`       | Fleet identifier (integer)                                |
#' | `module_id`   | Unique module identifier                                  |
#' | `distribution`| Likelihood distribution used for this data stream         |
#' | `year_i`      | Year index (present when available in the estimates)      |
#' | `age_i`       | Age index  (present when available in the estimates)      |
#'
#' @param x A `FIMSFit` object returned from [fit_fims()].
#' @param include_weights Logical (default `TRUE`).  When `TRUE` and
#'   `uncertainty` is available, a `.weight` column is added equal to
#'   `1 / uncertainty^2` (inverse-variance weights).  Rows where
#'   `uncertainty` is `NA` or zero receive `NA` weights, which yardstick
#'   silently drops when calling weighted metrics.
#' @param ... Unused; present for S3 method compatibility.
#'
#' @return A [tibble::tibble()] with at least the columns `.truth`, `.pred`,
#'   and optional `.weight`, plus grouping-metadata columns.
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
#' # Tidy tibble of observed vs. expected, ready for any yardstick metric
#' augment(fit)
#' }
#'
#' @seealso [get_fit_metrics()], [get_estimates()], [yardstick::metrics()]
#' @exportS3Method generics::augment
augment.FIMSFit <- function(x, include_weights = TRUE, ...) {
  # augment() is a filtered, renamed view of get_estimates().
  # this function only selects the
  # data-fit rows (where both observed and expected are non-NA) and maps
  # column names to the yardstick convention (.truth, .pred, .weight).
  # After applying fimsfit-patches.R the gradient is stored in x@gradient at
  # fit time, so this call is safe after clear().
  estimates <- get_estimates(x)

  # Keep only rows that are data-fit rows: both observed and expected must be
  # present.  Parameter rows (fixed_effects, random_effects) that have no
  # observed data counterpart are excluded.
  fit_rows <- estimates |>
    dplyr::filter(
      !is.na(.data$observed),
      !is.na(.data$expected)
    )

  if (nrow(fit_rows) == 0) {
    cli::cli_warn(c(
      "!" = "No observed/expected pairs found in the FIMSFit estimates.",
      "i" = "Returning an empty tibble. Check that the model has data-likelihood components."
    ))
    return(tibble::tibble(
      .truth       = numeric(),
      .pred        = numeric(),
      label        = character(),
      fleet        = character(),
      module_id    = integer(),
      distribution = character()
    ))
  }

  # Determine which optional index columns are present in the output
  # (year_i, age_i, length_i, season_i, …).  These are carried through so
  # users can group metrics by, e.g., year.
  index_cols <- names(fit_rows)[grepl("_i$", names(fit_rows))]

  # Core set of metadata columns to retain for grouping / filtering
  meta_cols <- intersect(
    c("label", "module_id", "module_type", "fleet", "distribution",
      "estimation_type", index_cols),
    names(fit_rows)
  )

  out <- fit_rows |>
    dplyr::select(
      dplyr::all_of(meta_cols),
      ".truth"  = "observed",
      ".pred"   = "expected",
      dplyr::any_of("uncertainty")
    ) |>
    dplyr::mutate(
      .truth = as.numeric(.data$.truth),
      .pred  = as.numeric(.data$.pred)
    )

  if (include_weights && "uncertainty" %in% names(out)) {
    out <- out |>
      dplyr::mutate(
        .weight = dplyr::if_else(
          !is.na(.data$uncertainty) & .data$uncertainty > 0,
          1 / .data$uncertainty^2,
          NA_real_
        )
      ) |>
      dplyr::select(-"uncertainty")
  } else if ("uncertainty" %in% names(out)) {
    out <- dplyr::select(out, -"uncertainty")
  }

  out
}

#' Compute yardstick metrics for a fitted FIMS model
#'
#' Extracts observed-vs-expected pairs from a `FIMSFit` object via
#' `augment.FIMSFit()` and evaluates a [yardstick::metric_set()] over them.
#'
#' By default the metrics are computed over **all** data streams combined.
#' Pass one or more column names to `group_by` to get per-stream breakdowns
#' (e.g., `group_by = "label"` gives one row per data-stream label such as
#' `"landings_expected"`, `"age_comp_expected"`, etc.).
#'
#' @param x A `FIMSFit` object returned from [fit_fims()].
#' @param metrics A [yardstick::metric_set()].  Defaults to
#'   `yardstick::metric_set(yardstick::rmse, yardstick::mae, yardstick::rsq)`.
#'   Any regression metric that accepts `truth` / `estimate` (and optionally
#'   `case_weights`) can be included.
#' @param group_by A character vector of column names in the augmented tibble
#'   to group by before computing metrics.  Common choices:
#'   * `"label"`        – one row-set per data-stream label
#'   * `"fleet"`        – one row-set per fleet
#'   * `"distribution"` – one row-set per likelihood distribution
#'   * `c("label", "fleet")` – per label × fleet combination
#'   Defaults to `NULL` (no grouping).
#' @param weighted Logical (default `FALSE`).  When `TRUE`, inverse-variance
#'   weights from `uncertainty` are passed to the metric functions via the
#'   `case_weights` argument.  Only metrics that accept `case_weights` will
#'   use them; others silently ignore the column.
#' @param ... Additional arguments forwarded to `augment.FIMSFit()`.
#'
#' @return A [tibble::tibble()] in the standard yardstick result format:
#'   columns `.metric`, `.estimator`, `.estimate`, plus any grouping columns.
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
#' # Overall RMSE / MAE / R² across all data streams
#' get_fit_metrics(fit)
#'
#' # Per-data-stream: one row-set per label
#' # (e.g. "landings_expected", "index_expected", "age_comp_expected")
#' get_fit_metrics(fit, group_by = "label")
#'
#' # Per-fleet with a custom metric set
#' get_fit_metrics(
#'   fit,
#'   metrics  = yardstick::metric_set(yardstick::rmse, yardstick::mape),
#'   group_by = "fleet"
#' )
#'
#' # Inverse-variance weighted metrics
#' get_fit_metrics(fit, weighted = TRUE)
#' }
#'
#' @seealso `augment.FIMSFit()`, [yardstick::metric_set()]
#' @export
get_fit_metrics <- function(
    x,
    metrics   = yardstick::metric_set(
      yardstick::rmse,
      yardstick::mae,
      yardstick::rsq
    ),
    group_by  = NULL,
    weighted  = FALSE,
    ...
) {
  if (!is.FIMSFit(x)) {
    cli::cli_abort("{.arg x} must be a {.cls FIMSFit} object.")
  }
  if (!requireNamespace("yardstick", quietly = TRUE)) {
    cli::cli_abort(
      "Package {.pkg yardstick} is required. Install it with
       {.code install.packages('yardstick')}."
    )
  }

  aug <- augment.FIMSFit(x, include_weights = weighted, ...)

  if (nrow(aug) == 0) {
    cli::cli_warn("Augmented data is empty; returning empty metrics tibble.")
    return(tibble::tibble(
      .metric    = character(),
      .estimator = character(),
      .estimate  = numeric()
    ))
  }

  # Validate grouping columns
  if (!is.null(group_by)) {
    bad <- setdiff(group_by, names(aug))
    if (length(bad) > 0) {
      cli::cli_abort(c(
        "Column{?s} {.val {bad}} not found in the augmented tibble.",
        "i" = "Available columns: {.val {names(aug)}}."
      ))
    }
    aug <- dplyr::group_by(aug, dplyr::across(dplyr::all_of(group_by)))
  }

  # Build the metric call: with or without case_weights.
  # .truth, .pred, and .weight are column names passed to yardstick's NSE
  # interface (truth = .truth, etc.). The .data$ pronoun used for dplyr verbs
  # does not apply here; globalVariables() is used instead to suppress the
  # R CMD CHECK notes for these three names.
  if (weighted && ".weight" %in% names(aug)) {
    result <- metrics(
      data          = aug,
      truth         = .truth,
      estimate      = .pred,
      case_weights  = .weight
    )
  } else {
    result <- metrics(
      data     = aug,
      truth    = .truth,
      estimate = .pred
    )
  }

  result
}

# .truth, .pred, and .weight are column names passed via NSE to yardstick
# metric functions (e.g. metrics(data, truth = .truth, estimate = .pred)).
# The .data$ pronoun is dplyr-specific and does not work in this context.
utils::globalVariables(c(".truth", ".pred", ".weight"))

# .env is an rlang environment pronoun used in dplyr::filter() to explicitly
# reference function arguments rather than columns, e.g.:
#   dplyr::filter(aug, .data$module_id == .env$module_id)
# Importing it suppresses the R CMD CHECK note.
#' @importFrom rlang .env
NULL

#' Extract a single data stream from a FIMSFit augmented tibble
#'
#' Convenience filter to pull out one specific data stream (e.g. the landings
#' for a given module) so you can pass it directly to any yardstick metric or
#' plot it.
#'
#' @details
#' In the FIMS output the `fleet` column is `NA` for derived-quantity rows
#' (which is where all observed/expected pairs live). Use `module_id` instead
#' to distinguish fleets and surveys — this matches the convention used in the
#' FIMS vignettes, where `module_id == 1` is the first fishing fleet and
#' `module_id == 2` is the first survey.  To discover which `module_id` values
#' are present in your fit, inspect `augment(fit)` directly.
#'
#' @param x A `FIMSFit` object **or** an already-augmented tibble from
#'   `augment.FIMSFit()`.
#' @param stream_label Character scalar.  The value of the `label` column to
#'   retain, e.g. `"landings_expected"`, `"index_expected"`,
#'   `"agecomp_expected"`, or `"lengthcomp_expected"`.  If `NULL` (default),
#'   no filtering on label is done.
#' @param module_id Integer scalar.  The `module_id` of the fleet or survey to
#'   retain (e.g. `1` for the first fishing fleet, `2` for the first survey in
#'   `data_big`).  If `NULL` (default), all modules are included.
#' @param ... Forwarded to `augment.FIMSFit()` when `x` is a `FIMSFit`.
#'
#' @return A [tibble::tibble()] subset of the augmented data.
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
#' # Landings for the fishing fleet (module_id 1) — compute RMSE
#' get_fit_stream(fit, stream_label = "landings_expected", module_id = 1) |>
#'   yardstick::rmse(truth = .truth, estimate = .pred)
#'
#' # Survey index stream (module_id 2 in the data_big example)
#' get_fit_stream(fit, stream_label = "index_expected", module_id = 2)
#'
#' # All streams for fleet 1 (landings + age comp + length comp)
#' get_fit_stream(fit, module_id = 1)
#' }
#' @export
get_fit_stream <- function(x, stream_label = NULL, module_id = NULL, ...) {
  aug <- if (is.FIMSFit(x)) augment.FIMSFit(x, ...) else x

  if (!is.null(stream_label)) {
    aug <- dplyr::filter(aug, .data$label == stream_label)
  }
  if (!is.null(module_id)) {
    aug <- dplyr::filter(aug, .data$module_id == .env$module_id)
  }
  aug
}

