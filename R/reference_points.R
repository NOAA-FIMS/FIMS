#' Calculate equilibrium reference points
#'
#' Calculate spawning potential ratio (SPR) targets and optional Beverton-Holt
#' maximum sustainable yield (MSY), with fixed biology and fleet allocation.
#'
#' @param inputs A list containing age-specific `natural_mortality`, `weight`,
#'   `maturity`, `proportion_female`, and `fleets`. Each fleet is a list with
#'   `share`, `selectivity`, and `weight`, plus optional logical `include_in_msy`
#'   (default `TRUE`). Excluded fleets still contribute mortality and catch.
#'   For MSY, also supply `recruitment`, a
#'   list with `type = "beverton_holt"`, `rzero`, `steepness`, and `phi0`
#'   (unfished spawning biomass per recruit defining the recruitment baseline).
#'   The `inputs` returned by [get_reference_points()] can be reused here.
#' @param spr_targets Numeric SPR targets in (0, 1].
#' @param msy Calculate MSY as well as SPR targets?
#' @param max_f Upper bound on fishing mortality. Fleets receive
#'   `F * share * selectivity`; selectivity is not normalized automatically.
#' @param tolerance Positive absolute solver tolerance: SPR residual for SPR,
#'   and width of the fishing mortality interval for MSY.
#' @param max_iterations Maximum solver refinement iterations.
#' @param grid_intervals Number of initial intervals in the MSY grid search.
#' @return A list with the input snapshot, unfished per-recruit biomass,
#'   an `spr` data frame, an `msy` list (or `NULL`), and solver `settings`.
#'   Check `status` before using a candidate. SPR status is `converged`,
#'   `not_bracketed`, or `iteration_limit`. MSY status is `converged`,
#'   `upper_bound`, `no_positive_yield`, or `iteration_limit`. An upper-bound
#'   MSY candidate is not an interior optimum; increase `max_f` and reassess.
#'   Within `msy`, `objective_yield` is the optimized yield from included fleets.
#'   `total_yield` includes all fleets' catch at that same F. The legacy `yield`
#'   field remains an alias for `total_yield`, not necessarily the MSY objective.
#'   `fleet_yield` reports catches for all fleets, including excluded fleets.
#' @details
#' Ages are annual and the final age is a plus group. Natural mortality must be
#' strictly positive. Biomass is measured at the start of the year. Fleet shares
#' must sum to one; a zero share excludes a survey fleet. An empty fleet list
#' is allowed for per-recruit and SPR calculations. MSY requires at least one
#' fleet with `include_in_msy = TRUE`; an all-excluded objective is an error.
#' Inclusion flags do not change or renormalize fleet shares and do not affect
#' SPR. MSY uses mean recruitment without deviations, with steepness
#' strictly between 0.2 and 1. Nonpositive recruitment equilibria are extinction.
#'
#' MSY uses a grid followed by golden-section refinement of the best sampled
#' region. Check sensitivity to bounds and grid resolution for unusual fishing
#' patterns. These post-fit calculations do not provide standard errors.
#' @export
#' @examples
#' inputs <- list(
#'   natural_mortality = c(0.2, 0.2), weight = c(1, 1),
#'   maturity = c(0, 1), proportion_female = c(1, 1),
#'   fleets = list(list(share = 1, selectivity = c(1, 1), weight = c(1, 1)))
#' )
#' calculate_reference_points(inputs, msy = FALSE)
calculate_reference_points <- function(inputs, spr_targets = 0.4, msy = TRUE,
                                       max_f = 5, tolerance = 1e-8,
                                       max_iterations = 100L,
                                       grid_intervals = 100L) {
  scalar <- function(x, name, lower, upper = Inf, integer = FALSE) {
    if (!is.numeric(x) || length(x) != 1L || !is.finite(x) ||
      x < lower || x > upper || (integer && x != floor(x))) {
      stop("Invalid ", name, call. = FALSE)
    }
  }
  if (!is.list(inputs)) stop("inputs must be a list", call. = FALSE)
  if (!is.numeric(spr_targets) || anyNA(spr_targets) ||
    any(!is.finite(spr_targets) | spr_targets <= 0 | spr_targets > 1)) {
    stop("spr_targets must be numeric values in (0, 1]", call. = FALSE)
  }
  if (!is.logical(msy) || length(msy) != 1L || is.na(msy)) {
    stop("msy must be TRUE or FALSE", call. = FALSE)
  }
  scalar(max_f, "max_f", .Machine$double.xmin)
  scalar(tolerance, "tolerance", .Machine$double.xmin)
  if (tolerance >= 1) stop("tolerance must be less than one", call. = FALSE)
  scalar(max_iterations, "max_iterations", 1, .Machine$integer.max, TRUE)
  scalar(grid_intervals, "grid_intervals", 2, .Machine$integer.max, TRUE)
  result <- calculate_reference_points_cpp(
    inputs, spr_targets, msy, max_f, tolerance,
    as.integer(max_iterations), as.integer(grid_intervals)
  )
  result$settings <- list(
    max_f = max_f, tolerance = tolerance, max_iterations = max_iterations,
    grid_intervals = grid_intervals
  )
  result
}

#' @rdname calculate_reference_points
#' @param model A live `CatchAtAge` Rcpp object after [CreateTMBModel()].
#' @param population A population ID belonging to `model`.
#' @param year One-based index of the modeled reference year, not a calendar year.
#' @param fleet_shares Optional numeric shares in ascending fleet ID order.
#'   By default, the reference year's fleet fishing mortality coefficients
#'   determine the shares. If all are zero, supply shares explicitly.
#' @param include_in_msy Optional logical vector with one flag per fleet.
#'   Defaults to all `TRUE`. Unnamed flags follow ascending fleet ID order.
#'   Named flags must use the fleet IDs and may be provided in any order.
#'   The flags are saved in the returned snapshot; they do not modify live fleets.
#' @details
#' `get_reference_points()` refreshes the live model's derived quantities and
#' copies the chosen year's biology. It uses the current internal double
#' parameters, as `model$get_output()` does. After fitting, synchronize these
#' with [set_fixed()] and [set_random()] before calling. It does not retrieve
#' estimates from a saved `FIMSFit`, change parameters, or refit the model.
#' The returned snapshot records population ID, year index, ages, and fleet IDs.
#' Catch weights follow the population weights used by `CatchAtAge`. The model's
#' initial `phi0` is retained for recruitment even if reference-year biology
#' differs. Thus unfished equilibrium recruitment in that year need not be R0.
#' @export
get_reference_points <- function(model, population, year, fleet_shares = NULL,
                                 spr_targets = 0.4, msy = TRUE, max_f = 5,
                                 tolerance = 1e-8, max_iterations = 100L,
                                 grid_intervals = 100L, include_in_msy = NULL) {
  valid_index <- function(x) {
    is.numeric(x) && length(x) == 1L && is.finite(x) &&
      x >= 1 && x <= .Machine$integer.max && x == floor(x)
  }
  if (!methods::is(model, "Rcpp_CatchAtAge")) {
    stop("model must be a live CatchAtAge object", call. = FALSE)
  }
  if (!valid_index(population) || !valid_index(year)) {
    stop("population and year must be positive integer indices", call. = FALSE)
  }
  if (is.null(fleet_shares)) fleet_shares <- numeric()
  if (!is.numeric(fleet_shares) || any(!is.finite(fleet_shares))) {
    stop("fleet_shares must be finite numeric values", call. = FALSE)
  }
  inputs <- model$reference_point_inputs(
    as.integer(population), as.integer(year), fleet_shares
  )
  if (!is.null(include_in_msy)) {
    if (!is.logical(include_in_msy) || anyNA(include_in_msy) ||
      length(include_in_msy) != length(inputs$fleets)) {
      stop("include_in_msy must contain one TRUE or FALSE per fleet", call. = FALSE)
    }
    if (!is.null(names(include_in_msy))) {
      ids <- as.character(inputs$fleet_ids)
      if (anyDuplicated(names(include_in_msy)) ||
        !setequal(names(include_in_msy), ids)) {
        stop("include_in_msy names must match the population's fleet IDs", call. = FALSE)
      }
      include_in_msy <- include_in_msy[ids]
    }
    for (i in seq_along(inputs$fleets)) {
      inputs$fleets[[i]]$include_in_msy <- unname(include_in_msy[i])
    }
  }
  calculate_reference_points(
    inputs, spr_targets, msy, max_f, tolerance, max_iterations, grid_intervals
  )
}
