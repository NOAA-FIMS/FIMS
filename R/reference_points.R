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
#' @param max_iterations Maximum bisection steps for SPR, or golden-section
#'   steps per candidate interval for MSY.
#' @param grid_intervals Number of initial intervals in the MSY grid search.
#' @return A list with the input snapshot, unfished per-recruit biomass,
#'   an expanded `spr` data frame, an `msy` list (or `NULL`), a common
#'   `reference_points` data frame, `conventions`, and solver `settings`.
#'   Each SPR row includes per-recruit spawning biomass, biomass, total and
#'   objective yields, and a `fleet_yield_per_recruit` list column. With supported
#'   recruitment inputs, it also includes equilibrium recruitment, spawning
#'   biomass, total biomass, total and objective yields, a `fleet_yield` list
#'   column, and a separate logical `collapsed` flag. Without a recruitment
#'   baseline (or with an unsupported recruitment type in SPR-only mode),
#'   equilibrium fields and `collapsed` are `NA`. Invalid supplied Beverton-Holt
#'   parameters are errors. Quantities at unconverged candidates are returned
#'   for diagnosis; they must not be interpreted as achieved target benchmarks.
#'   `reference_points` uses identical columns for MSY and every SPR target,
#'   with catches in `fleet_yield_<id>` columns (one-based fleet positions when
#'   no `inputs$fleet_ids` are supplied). `total_biomass` is total stock biomass;
#'   `spawning_biomass` is mature female biomass. The existing `biomass`, `spr`,
#'   and MSY `yield` fields remain available for compatibility.
#'   SPR `lower_bound`, `upper_bound`, and `bracket_width` describe the last
#'   searched interval, not a guaranteed F error bound; `not_bracketed` means
#'   that interval did not bracket the target. MSY `bracket_width` is the largest
#'   remaining refinement interval, `iterations` sums steps across intervals,
#'   and `intervals_refined` counts them. MSY table residual and bounds are `NA`.
#'   Biological collapse is separate from numerical solver status.
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
#' MSY uses a grid followed by golden-section refinement of candidate peaks,
#' including plateaus, and both boundary-adjacent intervals. Check sensitivity
#' to bounds and grid resolution for unusual fishing patterns. For approximate parameter uncertainty intervals, see
#' [calculate_reference_points_uncertainty()].
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
  if (!is.null(inputs$fleet_ids) &&
    (length(inputs$fleet_ids) != length(inputs$fleets) ||
      anyNA(inputs$fleet_ids) || anyDuplicated(inputs$fleet_ids) ||
      any(!nzchar(as.character(inputs$fleet_ids))))) {
    stop("fleet_ids must contain one unique nonmissing ID per fleet", call. = FALSE)
  }
  result <- calculate_reference_points_cpp(
    inputs, spr_targets, msy, max_f, tolerance,
    as.integer(max_iterations), as.integer(grid_intervals)
  )
  result$settings <- list(
    max_f = max_f, tolerance = tolerance, max_iterations = max_iterations,
    grid_intervals = grid_intervals
  )
  result$reference_points <- reference_point_table(result)
  result$conventions <- list(
    fishing_mortality = "Sum of fleet F coefficients; selectivity is used as supplied",
    effective_fishing_mortality = "Population f_multiplier times the sum of fleet Fmort coefficients across the population's fleets; excludes index catchability, catch shares, and age-averaged or apical F",
    fleet_shares = "Fixed mortality coefficient shares, not catch shares",
    spawning_biomass = "Beginning-of-year mature female biomass",
    total_biomass = "Beginning-of-year total stock biomass",
    spr_baseline = "Unfished per-recruit spawning biomass of the input snapshot",
    recruitment_baseline = "Supplied stock-recruit phi0; never rebased automatically",
    msy_objective = "Equilibrium yield of fleets with include_in_msy = TRUE",
    stock_status = "Annual beginning-of-year SB / SB_MSY and effective F / F_MSY relative to a single reference-year MSY benchmark; changes in fleet allocation or selectivity over time can limit interpretation of F / F_MSY as a like-for-like comparison of fishing pressure"
  )
  if (!is.null(inputs$historical) || !is.null(inputs$spawning_biomass)) {
    status_info <- build_stock_status(result$inputs, result)
    result$stock_status <- status_info$stock_status
    result$stock_status_reference <- status_info$reference
  }
  result
}

# Construct an additive reporting view without changing legacy result objects.
reference_point_table <- function(result) {
  fields <- c(
    "target", "fishing_mortality", "achieved_spr", "spawning_biomass",
    "total_biomass", "recruitment", "objective_yield", "total_yield",
    "spawning_biomass_per_recruit", "biomass_per_recruit",
    "objective_yield_per_recruit", "total_yield_per_recruit",
    "residual", "lower_bound", "upper_bound", "bracket_width",
    "iterations", "status", "collapsed"
  )
  rows <- result$spr[, fields, drop = FALSE]
  rows$reference_point <- if (nrow(rows)) paste0("SPR_", result$spr$target) else character()
  fleet_yield <- result$spr$fleet_yield
  if (!is.null(result$msy)) {
    msy <- result$msy
    msy[c("target", "residual", "lower_bound", "upper_bound")] <- rep(list(NA_real_), 4)
    msy <- as.data.frame(msy[fields], stringsAsFactors = FALSE)
    msy$reference_point <- "MSY"
    rows <- rbind(msy, rows)
    fleet_yield <- c(list(result$msy$fleet_yield), fleet_yield)
  }
  ids <- result$inputs$fleet_ids
  if (is.null(ids)) ids <- seq_along(result$inputs$fleets)
  for (i in seq_along(ids)) {
    rows[[paste0("fleet_yield_", ids[i])]] <- vapply(fleet_yield, `[`, numeric(1), i)
  }
  rows <- rows[, c("reference_point", setdiff(names(rows), "reference_point")), drop = FALSE]
  rownames(rows) <- NULL
  rows
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
#' The returned snapshot records population ID, year index, ages, fleet IDs,
#' `f_multiplier`, and `effective_fishing_mortality` (the multiplier times the
#' sum of the reference year's fleet F coefficients). The multiplier cancels
#' from normalized fleet shares but must be included in historical F/reference-F
#' ratios. Such ratios require matching allocation and selectivity conventions;
#' overriding shares defines a different fishing pattern. Index catchability
#' must not be multiplied into these fishing coefficients.
#' Catch weights follow the population weights used by `CatchAtAge`. The model's
#' initial `phi0` is retained for recruitment even if reference-year biology
#' differs. Thus unfished equilibrium recruitment in that year need not be R0.
#' A warning is issued when a fleet with zero reference-year fishing mortality
#' is assigned a positive reference-point share.
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
    inputs <- apply_include_in_msy(inputs, include_in_msy)
  }
  calculate_reference_points(
    inputs, spr_targets, msy, max_f, tolerance, max_iterations, grid_intervals
  )
}

valid_positive_index <- function(x) {
  is.numeric(x) && length(x) == 1L && is.finite(x) &&
    x >= 1 && x <= .Machine$integer.max && x == floor(x)
}

apply_include_in_msy <- function(inputs, include_in_msy) {
  if (is.null(include_in_msy)) return(inputs)
  if (!is.logical(include_in_msy) || anyNA(include_in_msy) ||
    length(include_in_msy) != length(inputs$fleets)) {
    stop("include_in_msy must contain one TRUE or FALSE per fleet", call. = FALSE)
  }
  if (!is.null(names(include_in_msy))) {
    ids <- if (!is.null(inputs$fleet_ids)) {
      as.character(inputs$fleet_ids)
    } else {
      as.character(seq_along(inputs$fleets))
    }
    if (anyDuplicated(names(include_in_msy)) ||
      !setequal(names(include_in_msy), ids)) {
      stop("include_in_msy names must match the population's fleet IDs", call. = FALSE)
    }
    include_in_msy <- include_in_msy[ids]
  }
  for (i in seq_along(inputs$fleets)) {
    inputs$fleets[[i]]$include_in_msy <- unname(include_in_msy[i])
  }
  inputs
}

normalize_historical_series <- function(inputs, population = NULL,
                                        years = NULL, calendar_years = NULL,
                                        include_terminal_biomass = TRUE) {
  if (!is.logical(include_terminal_biomass) ||
    length(include_terminal_biomass) != 1L || is.na(include_terminal_biomass)) {
    stop("include_terminal_biomass must be TRUE or FALSE", call. = FALSE)
  }
  if (!is.null(population) && !valid_positive_index(population)) {
    stop("population must be a positive integer index", call. = FALSE)
  }
  hist <- if (!is.null(inputs$historical)) inputs$historical else inputs
  if (!is.list(hist) || is.null(hist$spawning_biomass)) {
    stop(
      "inputs must contain historical spawning_biomass and fishing mortality series",
      call. = FALSE
    )
  }
  sb <- as.numeric(hist$spawning_biomass)
  n_raw <- length(sb)
  if (!n_raw || any(is.finite(sb) & sb < 0)) {
    stop("historical spawning_biomass must be a nonempty nonnegative vector", call. = FALSE)
  }
  if (!is.null(hist$population_id)) {
    pop_vec <- hist$population_id
    if (!is.numeric(pop_vec) || !(length(pop_vec) %in% c(1L, n_raw)) ||
      anyNA(pop_vec) || any(!is.finite(pop_vec) | pop_vec < 1 | pop_vec != floor(pop_vec))) {
      stop("historical population_id must contain positive integer IDs", call. = FALSE)
    }
    pop_vec <- rep(as.integer(pop_vec), length.out = n_raw)
  } else {
    default_pop <- if (!is.null(inputs$population_id)) {
      if (!valid_positive_index(inputs$population_id)) {
        stop("inputs$population_id must be a positive integer ID", call. = FALSE)
      }
      as.integer(inputs$population_id)
    } else if (!is.null(population)) {
      as.integer(population)
    } else {
      1L
    }
    pop_vec <- rep(default_pop, n_raw)
  }
  target_pop <- if (!is.null(population)) {
    as.integer(population)
  } else if (!is.null(inputs$population_id)) {
    as.integer(inputs$population_id)
  } else if (length(unique(pop_vec)) == 1L) {
    unique(pop_vec)
  } else {
    stop("Specify population when historical series contain multiple populations", call. = FALSE)
  }
  if (!is.null(inputs$population_id) &&
    as.integer(inputs$population_id) != target_pop) {
    stop("Requested population does not match reference snapshot population_id", call. = FALSE)
  }
  keep_pop <- which(pop_vec == target_pop)
  if (!length(keep_pop)) {
    stop("Requested population ID not found in historical series", call. = FALSE)
  }
  if (!is.null(hist$year_index)) {
    yr_all <- hist$year_index
    if (!is.numeric(yr_all) || length(yr_all) != n_raw || anyNA(yr_all) ||
      any(!is.finite(yr_all) | yr_all < 1 | yr_all != floor(yr_all))) {
      stop("historical year_index must contain positive integer indices", call. = FALSE)
    }
    year_index <- as.integer(yr_all[keep_pop])
  } else {
    year_index <- seq_along(keep_pop)
  }
  if (anyDuplicated(year_index)) {
    stop("historical year_index must be unique within each population", call. = FALSE)
  }
  sb <- sb[keep_pop]
  n_pop <- length(year_index)
  cal_year <- rep(NA_real_, n_pop)
  if (!is.null(hist$calendar_year)) {
    if (!is.numeric(hist$calendar_year) || length(hist$calendar_year) != n_raw) {
      stop("historical calendar_year must match spawning_biomass length", call. = FALSE)
    }
    cal_year <- as.numeric(hist$calendar_year[keep_pop])
  }

  # Align fishing mortality components by explicit identifiers or CatchAtAge length rules.
  order_idx <- order(year_index)
  year_index <- year_index[order_idx]
  sb <- sb[order_idx]
  cal_year <- cal_year[order_idx]
  sorted_keep <- keep_pop[order_idx]

  extract_f_vector <- function(x, name) {
    if (is.null(x)) return(NULL)
    if (is.data.frame(x)) {
      if (!("year_index" %in% names(x)) || !(name %in% names(x))) {
        stop(name, " data frame must contain year_index and ", name, call. = FALSE)
      }
      sub_x <- x
      if ("population_id" %in% names(sub_x)) {
        sub_x <- sub_x[sub_x$population_id == target_pop, , drop = FALSE]
      }
      if (anyDuplicated(sub_x$year_index)) {
        stop(name, " data frame has duplicate year_index values", call. = FALSE)
      }
      idx <- match(year_index, sub_x$year_index)
      out <- as.numeric(sub_x[[name]][idx])
    } else if (!is.null(names(x)) && all(nzchar(names(x)))) {
      if (anyDuplicated(names(x))) {
        stop("Named ", name, " must have unique year_index names", call. = FALSE)
      }
      out <- unname(as.numeric(x[as.character(year_index)]))
    } else {
      x_num <- as.numeric(x)
      if (length(x_num) == n_raw) {
        out <- x_num[sorted_keep]
      } else if (n_pop == n_raw && length(x_num) == n_pop - 1L && n_pop >= 2L) {
        out <- c(x_num, NA_real_)
      } else if (length(x_num) == n_pop) {
        out <- x_num
      } else {
        stop(
          name, " must match spawning_biomass length (or minus one for terminal biomass)",
          call. = FALSE
        )
      }
    }
    if (any(is.finite(out) & out < 0)) {
      stop("historical ", name, " must be nonnegative", call. = FALSE)
    }
    out
  }

  fleet_fmort <- hist$fleet_fmort
  if (is.null(fleet_fmort)) fleet_fmort <- hist$fmort
  computed_fleet_f <- NULL
  if (!is.null(fleet_fmort)) {
    if (is.matrix(fleet_fmort)) {
      if (!is.numeric(fleet_fmort) || any(is.finite(fleet_fmort) & fleet_fmort < 0)) {
        stop("fleet_fmort matrix must be nonnegative numeric values", call. = FALSE)
      }
      n_fleets <- length(inputs$fleets)
      if (n_fleets > 0L && nrow(fleet_fmort) == n_fleets &&
        ncol(fleet_fmort) %in% c(n_pop, n_pop - 1L) && nrow(fleet_fmort) != ncol(fleet_fmort)) {
        computed_fleet_f <- colSums(fleet_fmort)
      } else if (nrow(fleet_fmort) %in% c(n_pop, n_pop - 1L)) {
        computed_fleet_f <- rowSums(fleet_fmort)
      } else if (ncol(fleet_fmort) %in% c(n_pop, n_pop - 1L)) {
        computed_fleet_f <- colSums(fleet_fmort)
      } else {
        stop("fleet_fmort matrix dimensions must match historical years", call. = FALSE)
      }
    } else if (is.list(fleet_fmort)) {
      if (!length(fleet_fmort) || any(!vapply(fleet_fmort, is.numeric, logical(1)))) {
        stop("fleet_fmort list must contain numeric fleet Fmort vectors", call. = FALSE)
      }
      lens <- lengths(fleet_fmort)
      if (length(unique(lens)) != 1L) {
        stop("All fleet_fmort vectors must have the same length", call. = FALSE)
      }
      mat <- do.call(cbind, lapply(fleet_fmort, as.numeric))
      if (any(is.finite(mat) & mat < 0)) {
        stop("fleet_fmort values must be nonnegative", call. = FALSE)
      }
      computed_fleet_f <- rowSums(mat)
    } else {
      stop("fleet_fmort must be a numeric matrix or list of fleet vectors", call. = FALSE)
    }
  }

  f_mult <- extract_f_vector(hist$f_multiplier, "f_multiplier")
  fleet_f <- extract_f_vector(
    if (!is.null(hist$fleet_fishing_mortality)) hist$fleet_fishing_mortality else computed_fleet_f,
    "fleet_fishing_mortality"
  )
  f_eff <- extract_f_vector(hist$effective_fishing_mortality, "effective_fishing_mortality")

  if (is.null(f_mult) && !is.null(fleet_f) && is.null(f_eff)) {
    default_mult <- if (!is.null(inputs$f_multiplier) && length(inputs$f_multiplier) == 1L) {
      as.numeric(inputs$f_multiplier)
    } else {
      1
    }
    if (!is.finite(default_mult) || default_mult < 0) {
      stop("f_multiplier must be finite and nonnegative", call. = FALSE)
    }
    f_mult <- ifelse(is.na(fleet_f), NA_real_, default_mult)
  }
  if (!is.null(f_mult) && !is.null(fleet_f)) {
    product_f <- f_mult * fleet_f
    if (!is.null(f_eff)) {
      both_finite <- is.finite(f_eff) & is.finite(product_f)
      if (any(abs(f_eff[both_finite] - product_f[both_finite]) >
        1e-8 * pmax(1, abs(product_f[both_finite])))) {
        stop(
          "effective_fishing_mortality does not match f_multiplier * fleet_fishing_mortality",
          call. = FALSE
        )
      }
    } else {
      f_eff <- product_f
    }
  } else if (!is.null(f_eff)) {
    if (is.null(f_mult)) {
      f_mult <- rep(NA_real_, n_pop)
      if (!is.null(inputs$year_index) && !is.null(inputs$f_multiplier) &&
        length(inputs$f_multiplier) == 1L) {
        ref_pos <- match(as.integer(inputs$year_index), year_index)
        if (!is.na(ref_pos) && !is.na(f_eff[ref_pos])) {
          f_mult[ref_pos] <- as.numeric(inputs$f_multiplier)
        }
      }
    }
    if (is.null(fleet_f)) {
      fleet_f <- ifelse(is.finite(f_mult) & f_mult > 0, f_eff / f_mult, NA_real_)
    }
  } else {
    stop(
      "Supply effective_fishing_mortality or f_multiplier and fleet_fishing_mortality",
      call. = FALSE
    )
  }

  # Align optional calendar_years before subsetting requested years.
  if (is.null(calendar_years) && !is.null(inputs$calendar_years)) {
    calendar_years <- inputs$calendar_years
  }
  if (!is.null(calendar_years)) {
    if (inherits(calendar_years, "FIMSFrame")) {
      cal_seq <- seq.int(get_start_year(calendar_years), get_end_year(calendar_years) + 1L)
      idx <- match(year_index, seq_along(cal_seq))
      if (anyNA(idx)) {
        stop("FIMSFrame calendar years do not cover historical year_index", call. = FALSE)
      }
      cal_year <- as.numeric(cal_seq[idx])
    } else if (is.data.frame(calendar_years)) {
      if (!all(c("year_index", "calendar_year") %in% names(calendar_years))) {
        stop("calendar_years data frame must have year_index and calendar_year columns", call. = FALSE)
      }
      sub_cal <- calendar_years
      if ("population_id" %in% names(sub_cal)) {
        sub_cal <- sub_cal[sub_cal$population_id == target_pop, , drop = FALSE]
      }
      if (anyDuplicated(sub_cal$year_index)) {
        stop("calendar_years data frame has duplicate year_index values", call. = FALSE)
      }
      idx <- match(year_index, sub_cal$year_index)
      if (anyNA(idx)) {
        stop("calendar_years data frame does not cover all historical year_index values", call. = FALSE)
      }
      cal_year <- as.numeric(sub_cal$calendar_year[idx])
    } else if (is.numeric(calendar_years)) {
      if (anyNA(calendar_years) || any(!is.finite(calendar_years))) {
        stop("calendar_years must be finite numeric values", call. = FALSE)
      }
      if (!is.null(names(calendar_years)) && all(nzchar(names(calendar_years)))) {
        if (anyDuplicated(names(calendar_years)) ||
          !all(as.character(year_index) %in% names(calendar_years))) {
          stop("Named calendar_years must contain unique names for all year_index values", call. = FALSE)
        }
        cal_year <- unname(as.numeric(calendar_years[as.character(year_index)]))
      } else if (length(calendar_years) == n_pop) {
        cal_year <- as.numeric(calendar_years)
      } else if (length(calendar_years) == n_pop - 1L && n_pop >= 2L && is.na(f_eff[n_pop])) {
        next_cal <- if (length(calendar_years) >= 1L) {
          as.numeric(calendar_years[length(calendar_years)]) + 1
        } else {
          NA_real_
        }
        cal_year <- c(as.numeric(calendar_years), next_cal)
      } else if (!is.null(years) && length(calendar_years) == length(years)) {
        cal_year <- rep(NA_real_, n_pop)
        idx_years <- match(as.integer(years), year_index)
        if (!anyNA(idx_years)) cal_year[idx_years] <- as.numeric(calendar_years)
      } else {
        stop("calendar_years length must match the historical series", call. = FALSE)
      }
    } else {
      stop("Invalid calendar_years specification", call. = FALSE)
    }
  }

  is_terminal_extra <- rep(FALSE, n_pop)
  if (n_pop >= 2L && is.na(f_eff[n_pop]) && all(!is.na(f_eff[-n_pop]))) {
    is_terminal_extra[n_pop] <- TRUE
  }

  full_cal_map <- stats::setNames(cal_year, as.character(year_index))
  full_f_eff_map <- stats::setNames(f_eff, as.character(year_index))

  if (!is.null(years)) {
    if (!is.numeric(years) || !length(years) || anyNA(years) ||
      any(!is.finite(years) | years < 1 | years != floor(years)) ||
      anyDuplicated(years)) {
      stop("years must be unique positive integer indices", call. = FALSE)
    }
    years <- as.integer(years)
    idx <- match(years, year_index)
    if (anyNA(idx)) {
      stop("Requested years are not present in the population's historical series", call. = FALSE)
    }
  } else if (!include_terminal_biomass) {
    idx <- which(!is_terminal_extra)
  } else {
    idx <- seq_len(n_pop)
  }

  list(
    population_id = rep(target_pop, length(idx)),
    target_population = target_pop,
    year_index = year_index[idx],
    calendar_year = cal_year[idx],
    spawning_biomass = sb[idx],
    f_multiplier = f_mult[idx],
    fleet_fishing_mortality = fleet_f[idx],
    effective_fishing_mortality = f_eff[idx],
    is_terminal_extra = is_terminal_extra[idx],
    full_calendar_map = full_cal_map,
    full_f_eff_map = full_f_eff_map
  )
}

build_stock_status <- function(inputs, reference_points, population = NULL,
                               reference_year = NULL, years = NULL,
                               calendar_years = NULL,
                               include_terminal_biomass = TRUE) {
  hist <- normalize_historical_series(
    inputs = inputs, population = population, years = years,
    calendar_years = calendar_years,
    include_terminal_biomass = include_terminal_biomass
  )
  if (!is.null(reference_year)) {
    if (!valid_positive_index(reference_year)) {
      stop("reference_year must be a positive integer index", call. = FALSE)
    }
    reference_year <- as.integer(reference_year)
    if (!is.null(inputs$year_index) &&
      as.integer(inputs$year_index) != reference_year) {
      stop("reference_year does not match inputs$year_index", call. = FALSE)
    }
  } else if (!is.null(inputs$year_index)) {
    if (!valid_positive_index(inputs$year_index)) {
      stop("inputs$year_index must be a positive integer index", call. = FALSE)
    }
    reference_year <- as.integer(inputs$year_index)
  } else {
    reference_year <- NA_integer_
  }

  if (!is.na(reference_year) && !is.null(inputs$effective_fishing_mortality) &&
    as.character(reference_year) %in% names(hist$full_f_eff_map)) {
    ref_f_hist <- unname(hist$full_f_eff_map[[as.character(reference_year)]])
    ref_f_snap <- as.numeric(inputs$effective_fishing_mortality)
    if (is.finite(ref_f_hist) && is.finite(ref_f_snap) &&
      abs(ref_f_hist - ref_f_snap) > 1e-8 * max(1, abs(ref_f_snap))) {
      stop(
        "Reference-year effective_fishing_mortality in snapshot does not match historical series",
        call. = FALSE
      )
    }
  }

  msy <- reference_points$msy
  if (is.null(msy)) {
    msy_status <- "unavailable"
    msy_collapsed <- NA
    sb_msy <- NA_real_
    f_msy <- NA_real_
    sb_base_reason <- "msy_unavailable"
    f_base_reason <- "msy_unavailable"
  } else {
    msy_status <- if (!is.null(msy$status) && length(msy$status) == 1L) {
      as.character(msy$status)
    } else {
      "unavailable"
    }
    msy_collapsed <- if (!is.null(msy$collapsed) && length(msy$collapsed) == 1L) {
      as.logical(msy$collapsed)
    } else {
      NA
    }
    sb_msy <- if (!is.null(msy$spawning_biomass) && length(msy$spawning_biomass) == 1L) {
      as.numeric(msy$spawning_biomass)
    } else {
      NA_real_
    }
    f_msy <- if (!is.null(msy$fishing_mortality) && length(msy$fishing_mortality) == 1L) {
      as.numeric(msy$fishing_mortality)
    } else {
      NA_real_
    }
    sb_base_reason <- if (!identical(msy_status, "converged")) {
      msy_status
    } else if (!is.finite(sb_msy)) {
      "nonfinite_sb_msy"
    } else if (sb_msy <= 0) {
      "nonpositive_sb_msy"
    } else {
      "ok"
    }
    f_base_reason <- if (!identical(msy_status, "converged")) {
      msy_status
    } else if (!is.finite(f_msy)) {
      "nonfinite_f_msy"
    } else if (f_msy <= 0) {
      "nonpositive_f_msy"
    } else {
      "ok"
    }
  }

  n <- length(hist$year_index)
  sb_ratio <- rep(NA_real_, n)
  f_ratio <- rep(NA_real_, n)
  sb_diag <- rep(sb_base_reason, n)
  f_diag <- rep(f_base_reason, n)

  for (k in seq_len(n)) {
    sb_k <- hist$spawning_biomass[k]
    if (identical(sb_base_reason, "ok")) {
      if (is.finite(sb_k) && sb_k >= 0) {
        sb_ratio[k] <- sb_k / sb_msy
        sb_diag[k] <- "ok"
      } else {
        sb_diag[k] <- "missing_numerator"
      }
    }
    f_k <- hist$effective_fishing_mortality[k]
    if (hist$is_terminal_extra[k] && is.na(f_k)) {
      f_ratio[k] <- NA_real_
      f_diag[k] <- if (identical(f_base_reason, "ok")) {
        "terminal_biomass_only"
      } else {
        f_base_reason
      }
    } else if (identical(f_base_reason, "ok")) {
      if (is.finite(f_k) && f_k >= 0) {
        f_ratio[k] <- f_k / f_msy
        f_diag[k] <- "ok"
      } else {
        f_diag[k] <- "missing_numerator"
      }
    }
  }

  fleet_ids <- if (!is.null(inputs$fleet_ids)) {
    as.integer(inputs$fleet_ids)
  } else {
    seq_along(inputs$fleets)
  }
  fleet_shares <- vapply(inputs$fleets, `[[`, numeric(1), "share")
  include_flags <- vapply(inputs$fleets, function(fleet) {
    is.null(fleet$include_in_msy) || isTRUE(fleet$include_in_msy)
  }, logical(1))
  if (length(fleet_ids)) {
    names(fleet_shares) <- as.character(fleet_ids)
    names(include_flags) <- as.character(fleet_ids)
  }
  ref_cal_year <- if (!is.na(reference_year) &&
    as.character(reference_year) %in% names(hist$full_calendar_map)) {
    unname(hist$full_calendar_map[[as.character(reference_year)]])
  } else {
    NA_real_
  }

  reference_info <- list(
    population_id = hist$target_population,
    reference_year = reference_year,
    calendar_year = ref_cal_year,
    fleet_ids = fleet_ids,
    fleet_shares = fleet_shares,
    include_in_msy = include_flags,
    sb_msy = sb_msy,
    f_msy = f_msy,
    status = msy_status,
    collapsed = msy_collapsed,
    sb_diagnostic = sb_base_reason,
    f_diagnostic = f_base_reason,
    recruitment = inputs$recruitment
  )

  table <- data.frame(
    population_id = hist$population_id,
    year_index = hist$year_index,
    calendar_year = hist$calendar_year,
    reference_year = rep(reference_year, n),
    spawning_biomass = hist$spawning_biomass,
    f_multiplier = hist$f_multiplier,
    fleet_fishing_mortality = hist$fleet_fishing_mortality,
    effective_fishing_mortality = hist$effective_fishing_mortality,
    sb_msy = rep(sb_msy, n),
    f_msy = rep(f_msy, n),
    sb_over_sb_msy = sb_ratio,
    f_over_f_msy = f_ratio,
    status = rep(msy_status, n),
    collapsed = rep(msy_collapsed, n),
    sb_diagnostic = sb_diag,
    f_diagnostic = f_diag,
    stringsAsFactors = FALSE
  )
  rownames(table) <- NULL
  attr(table, "reference") <- reference_info

  list(
    stock_status = table,
    reference = reference_info,
    reference_points = reference_points,
    conventions = reference_points$conventions
  )
}

#' Calculate annual MSY-relative stock status
#'
#' Calculate annual beginning-of-year spawning biomass relative to \eqn{SB_{\mathrm{MSY}}}
#' (`sb_over_sb_msy`) and annual effective fishing mortality relative to
#' \eqn{F_{\mathrm{MSY}}} (`f_over_f_msy`) against a single Beverton-Holt MSY
#' benchmark solved for a selected population and reference year.
#'
#' @inheritParams calculate_reference_points
#' @param reference_points Optional result from [calculate_reference_points()]
#'   or [get_reference_points()]. When `NULL`, [calculate_reference_points()] is
#'   called once using `inputs`.
#' @param population Optional positive integer population ID used to select and
#'   verify rows when `inputs` contains population identifiers.
#' @param reference_year Optional one-based index of the MSY reference year.
#'   Defaults to `inputs$year_index` when available.
#' @param years Optional vector of unique one-based year indices to return.
#'   Defaults to all historical years for the selected population.
#' @param calendar_years Optional calendar years corresponding to the historical
#'   series: a numeric vector, a named vector keyed by `year_index`, a data
#'   frame with `year_index` and `calendar_year` columns, or a [FIMSFrame]
#'   object.
#' @param include_terminal_biomass Logical (default `TRUE`). When `TRUE` and
#'   `years = NULL`, retains `CatchAtAge`'s extra terminal beginning-of-year
#'   spawning biomass observation (`n_years + 1`) with missing fishing mortality
#'   and missing `f_over_f_msy`.
#' @param reference_year One-based index of the modeled MSY reference year (or
#'   `year` in [get_stock_status()]).
#' @param fit Optional [FIMSFit] object used to synchronize fitted fixed and
#'   random parameters before evaluating a live `CatchAtAge` model.
#' @return A list containing:
#'   \describe{
#'     \item{`stock_status`}{Data frame with `population_id`, `year_index`,
#'       `calendar_year`, `reference_year`, `spawning_biomass`, `f_multiplier`,
#'       `fleet_fishing_mortality`, `effective_fishing_mortality`, `sb_msy`,
#'       `f_msy`, `sb_over_sb_msy`, `f_over_f_msy`, `status`, `collapsed`,
#'       `sb_diagnostic`, and `f_diagnostic`.}
#'     \item{`reference`}{List recording `population_id`, `reference_year`,
#'       `calendar_year`, `fleet_ids`, `fleet_shares`, `include_in_msy`,
#'       `sb_msy`, `f_msy`, `status`, `collapsed`, `sb_diagnostic`,
#'       `f_diagnostic`, and `recruitment`.}
#'     \item{`reference_points`}{Result from [calculate_reference_points()] for
#'       the single solved benchmark.}
#'     \item{`conventions`}{List of definitions and interpretation notes.}
#'   }
#' @details
#' For each requested year \eqn{t}, `sb_over_sb_msy[t] = SB[t] / SB_MSY` and
#' `f_over_f_msy[t] = F_eff[t] / F_MSY`, where `SB_MSY` and `F_MSY` are read
#' from `msy$spawning_biomass` and `msy$fishing_mortality`. Historical
#' effective fishing mortality is `F_eff[t] = f_multiplier[t] * sum_v(Fmort[v, t])`
#' across all fleets of the population. This matches the MSY solver's
#' fishing-mortality coefficient scale: survey fleets are excluded through zero
#' fishing coefficients, while fishing fleets with `include_in_msy = FALSE`
#' retain their fishing mortality in `F_eff[t]` even though their yield is
#' excluded from the MSY objective. Index catchability, catch shares, and
#' age-averaged or apical fishing mortality are outside this coefficient
#' definition.
#'
#' Because `F_MSY` is solved under fixed reference-year fleet shares and
#' selectivities, changes in fleet allocation or selectivity across years can
#' limit interpretation of `f_over_f_msy` as a like-for-like comparison of
#' fishing pressure. When the MSY benchmark is unavailable, unconverged (such as
#' `upper_bound`, `no_positive_yield`, or `iteration_limit`), or has a nonfinite
#' or nonpositive denominator, affected ratios are returned as `NA_real_` and
#' the reason is recorded in `sb_diagnostic` and `f_diagnostic`. True zero
#' historical numerators are preserved as zero ratios when the benchmark
#' denominator is valid.
#' @export
#' @examples
#' inputs <- list(
#'   natural_mortality = c(0.2, 0.2), weight = c(1, 1),
#'   maturity = c(0, 1), proportion_female = c(1, 1),
#'   fleets = list(list(share = 1, selectivity = c(1, 1), weight = c(1, 1))),
#'   recruitment = list(
#'     type = "beverton_holt", rzero = 1000, steepness = 0.75,
#'     phi0 = 1 / expm1(0.2)
#'   ),
#'   population_id = 1L,
#'   year_index = 2L,
#'   historical = data.frame(
#'     population_id = c(1L, 1L, 1L),
#'     year_index = 1:3,
#'     calendar_year = 2020:2022,
#'     spawning_biomass = c(2500, 1800, 1600),
#'     f_multiplier = c(1, 1, NA_real_),
#'     fleet_fishing_mortality = c(0.25, 0.35, NA_real_),
#'     effective_fishing_mortality = c(0.25, 0.35, NA_real_)
#'   )
#' )
#' calculate_stock_status(inputs)$stock_status
calculate_stock_status <- function(inputs, reference_points = NULL,
                                   population = NULL, reference_year = NULL,
                                   years = NULL, calendar_years = NULL,
                                   include_terminal_biomass = TRUE,
                                   spr_targets = 0.4, msy = TRUE, max_f = 5,
                                   tolerance = 1e-8, max_iterations = 100L,
                                   grid_intervals = 100L) {
  if (!is.list(inputs)) stop("inputs must be a list", call. = FALSE)
  if (is.null(reference_points) &&
    all(c("inputs", "spr", "reference_points") %in% names(inputs))) {
    reference_points <- inputs
    inputs <- reference_points$inputs
  } else if (is.list(inputs$inputs) &&
    (!is.null(inputs$historical) || !is.null(inputs$spawning_biomass))) {
    merged <- inputs$inputs
    if (!is.null(inputs$historical)) {
      merged$historical <- inputs$historical
    } else {
      merged$historical <- inputs[intersect(
        names(inputs),
        c(
          "population_id", "year_index", "calendar_year", "spawning_biomass",
          "f_multiplier", "fleet_fishing_mortality",
          "effective_fishing_mortality", "fleet_fmort", "fmort"
        )
      )]
    }
    if (!is.null(inputs$calendar_years) && is.null(merged$calendar_years)) {
      merged$calendar_years <- inputs$calendar_years
    }
    inputs <- merged
  }
  if (is.null(reference_points)) {
    reference_points <- calculate_reference_points(
      inputs, spr_targets = spr_targets, msy = msy, max_f = max_f,
      tolerance = tolerance, max_iterations = max_iterations,
      grid_intervals = grid_intervals
    )
  }
  build_stock_status(
    inputs = inputs, reference_points = reference_points,
    population = population, reference_year = reference_year,
    years = years, calendar_years = calendar_years,
    include_terminal_biomass = include_terminal_biomass
  )
}

#' @rdname calculate_stock_status
#' @param model A live `CatchAtAge` Rcpp object after [CreateTMBModel()], or a
#'   [FIMSFit] object whose live `CatchAtAge` model is active in memory.
#' @param year One-based index of the modeled MSY reference year (alias for
#'   `reference_year`).
#' @export
get_stock_status <- function(model, population = 1L, year = NULL,
                             reference_year = year, years = NULL,
                             calendar_years = NULL,
                             include_terminal_biomass = TRUE,
                             fleet_shares = NULL, spr_targets = 0.4,
                             msy = TRUE, max_f = 5, tolerance = 1e-8,
                             max_iterations = 100L, grid_intervals = 100L,
                             include_in_msy = NULL, fit = NULL) {
  if (is.FIMSFit(model)) {
    if (is.null(fit)) fit <- model
    model <- get_input(fit)[["model"]]
  }
  if (!methods::is(model, "Rcpp_CatchAtAge")) {
    stop("model must be a live CatchAtAge object or FIMSFit with a live model", call. = FALSE)
  }
  if (is.null(reference_year) || !valid_positive_index(population) ||
    !valid_positive_index(reference_year)) {
    stop("population and year (or reference_year) must be positive integer indices", call. = FALSE)
  }
  population <- as.integer(population)
  reference_year <- as.integer(reference_year)
  if (!is.null(fit)) {
    if (!is.FIMSFit(fit)) stop("fit must be a FIMSFit object", call. = FALSE)
    orig_fixed <- get_fixed()
    orig_random <- get_random()
    on.exit(
      restore_live_model_state(
        orig_fixed, orig_random, model = model,
        population = population, year = reference_year
      ),
      add = TRUE
    )
    sync_live_model_to_fit(fit, orig_fixed = orig_fixed, orig_random = orig_random)
  }
  reference_points <- get_reference_points(
    model = model, population = population, year = reference_year,
    fleet_shares = fleet_shares, spr_targets = spr_targets, msy = msy,
    max_f = max_f, tolerance = tolerance, max_iterations = max_iterations,
    grid_intervals = grid_intervals, include_in_msy = include_in_msy
  )
  calculate_stock_status(
    inputs = reference_points$inputs, reference_points = reference_points,
    population = population, reference_year = reference_year,
    years = years, calendar_years = calendar_years,
    include_terminal_biomass = include_terminal_biomass
  )
}

restore_live_model_state <- function(orig_fixed, orig_random, model = NULL,
                                     population = 1L, year = 1L) {
  set_fixed(orig_fixed)
  set_random(orig_random)
  if (!is.null(model) && methods::is(model, "Rcpp_CatchAtAge")) {
    tryCatch(
      suppressWarnings(
        model$reference_point_inputs(as.integer(population), as.integer(year), numeric())
      ),
      error = function(e) NULL
    )
  }
  invisible(NULL)
}

extract_fit_parameter_vectors <- function(fit, orig_fixed = get_fixed(),
                                          orig_random = get_random()) {
  if (!is.FIMSFit(fit)) stop("fit must be a FIMSFit object", call. = FALSE)
  sdreport <- get_sdreport(fit)
  opt <- get_opt(fit)
  obj <- get_obj(fit)
  par_fixed <- if (length(sdreport) > 0L && !is.null(sdreport[["par.fixed"]])) {
    sdreport[["par.fixed"]]
  } else if (length(opt) > 0L && !is.null(opt[["par"]])) {
    opt[["par"]]
  } else if (length(obj) > 0L && !is.null(obj[["par"]])) {
    obj[["par"]]
  } else {
    orig_fixed
  }
  par_random <- if (length(sdreport) > 0L && !is.null(sdreport[["par.random"]])) {
    sdreport[["par.random"]]
  } else if (length(obj) > 0L && !is.null(obj[["env"]]) &&
    is.function(obj[["env"]][["parList"]])) {
    obj[["env"]]$parList()[["re"]]
  } else {
    orig_random
  }
  list(fixed = par_fixed, random = par_random)
}

expand_live_parameters <- function(par_fixed, par_random = NULL,
                                   orig_fixed = get_fixed(),
                                   orig_random = get_random(),
                                   cpp_fixed_names = NULL,
                                   cpp_random_names = NULL,
                                   fit = NULL) {
  if (is.null(cpp_fixed_names) && length(orig_fixed)) {
    cpp_fixed_names <- names(get_parameter_names(as.list(orig_fixed)))
  }
  if (is.null(cpp_random_names) && length(orig_random)) {
    cpp_random_names <- names(get_random_names(as.list(orig_random)))
  }
  if (!is.numeric(par_fixed) || any(!is.finite(par_fixed))) {
    stop("Fixed parameters must be finite numeric values", call. = FALSE)
  }
  if (!is.null(par_random) && length(par_random) > 0L &&
    (!is.numeric(par_random) || any(!is.finite(par_random)))) {
    stop("Random parameters must be finite numeric values", call. = FALSE)
  }
  # Use TMB parList mapping when a FIMSFit object defines a parameter map.
  if (!is.null(fit) && is.FIMSFit(fit)) {
    obj <- get_obj(fit)
    has_map <- length(obj[["env"]][["map"]]) > 0L
    if (has_map && is.function(obj[["env"]][["parList"]])) {
      full_par <- obj[["env"]][["last.par.best"]]
      rand_idx <- obj[["env"]][["random"]]
      if (length(rand_idx) > 0L) {
        fixed_idx <- setdiff(seq_along(full_par), rand_idx)
        if (length(par_fixed) != length(fixed_idx)) {
          stop("Fixed parameter length does not match fitted model", call. = FALSE)
        }
        full_par[fixed_idx] <- as.numeric(par_fixed)
        if (!is.null(par_random)) {
          if (length(par_random) != length(rand_idx)) {
            stop("Random parameter length does not match fitted model", call. = FALSE)
          }
          full_par[rand_idx] <- as.numeric(par_random)
        }
        par_list <- obj[["env"]]$parList(x = as.numeric(par_fixed), par = full_par)
      } else {
        if (length(par_fixed) != length(obj[["par"]])) {
          stop("Fixed parameter length does not match fitted model", call. = FALSE)
        }
        par_list <- obj[["env"]]$parList(as.numeric(par_fixed))
      }
      p_vec <- as.numeric(par_list[["p"]])
      re_vec <- as.numeric(par_list[["re"]])
      if (length(p_vec) != length(orig_fixed) || length(re_vec) != length(orig_random)) {
        stop("Mapped parameter lengths do not match live CatchAtAge model", call. = FALSE)
      }
      return(list(fixed = p_vec, random = re_vec))
    }
  }
  if (length(par_fixed) != length(orig_fixed)) {
    stop("Fixed parameter length must match the live model", call. = FALSE)
  }
  if (!is.null(names(par_fixed)) && !is.null(cpp_fixed_names) &&
    length(cpp_fixed_names) == length(par_fixed) &&
    !identical(names(par_fixed), cpp_fixed_names)) {
    stop("Fixed parameter names must match the live model in order", call. = FALSE)
  }
  if (is.null(par_random)) {
    re_vec <- as.numeric(orig_random)
  } else {
    if (length(par_random) != length(orig_random)) {
      stop("Random parameter length must match the live model", call. = FALSE)
    }
    if (!is.null(names(par_random)) && !is.null(cpp_random_names) &&
      length(cpp_random_names) == length(par_random) &&
      !identical(names(par_random), cpp_random_names)) {
      stop("Random parameter names must match the live model in order", call. = FALSE)
    }
    re_vec <- as.numeric(par_random)
  }
  list(fixed = as.numeric(par_fixed), random = re_vec)
}

sync_live_model_to_fit <- function(fit, orig_fixed = get_fixed(),
                                   orig_random = get_random()) {
  vecs <- extract_fit_parameter_vectors(
    fit, orig_fixed = orig_fixed, orig_random = orig_random
  )
  expanded <- expand_live_parameters(
    vecs$fixed, vecs$random, orig_fixed = orig_fixed,
    orig_random = orig_random, fit = fit
  )
  if (length(expanded$fixed)) set_fixed(expanded$fixed)
  if (length(expanded$random)) set_random(expanded$random)
  invisible(expanded)
}

