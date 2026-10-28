reference_point_test_inputs <- function() {
  list(
    natural_mortality = c(0.2, 0.2), weight = c(1, 1),
    maturity = c(0, 1), proportion_female = c(1, 1),
    fleets = list(list(share = 1, selectivity = c(1, 1), weight = c(1, 1))),
    recruitment = list(
      type = "beverton_holt", rzero = 1000, steepness = 0.75,
      phi0 = 1 / expm1(0.2)
    )
  )
}

test_that("reference points agree with analytic SPR and an independent MSY optimizer", {
  #' @description R results agree with analytic per-recruit formulas and R's optimizer.
  inputs <- reference_point_test_inputs()
  result <- calculate_reference_points(inputs, spr_targets = c(1, 0.4))
  expect_equal(result$spr$status, c("converged", "converged"))
  expect_equal(result$spr$fishing_mortality[1], 0)
  expect_equal(result$spr$fishing_mortality[2], log1p(expm1(0.2) / 0.4) - 0.2,
    tolerance = 1e-7
  )
  yield <- function(f) {
    sbpr <- 1 / expm1(0.2 + f)
    r <- max(0, 1000 * (0.8 * 0.75 - 0.2 * 0.25 * inputs$recruitment$phi0 / sbpr) / 0.55)
    r * f / (0.2 + f)
  }
  expected <- optimize(yield, c(0, 1), maximum = TRUE, tol = 1e-10)
  expect_equal(result$msy$status, "converged")
  expect_equal(result$msy$fishing_mortality, expected$maximum, tolerance = 1e-6)
  expect_equal(result$msy$yield, expected$objective, tolerance = 1e-8)
  expect_equal(result$inputs, inputs)
  expect_equal(result, calculate_reference_points(inputs, spr_targets = c(1, 0.4)))
})

test_that("reference point R interface validates requests and reports failed bounds", {
  #' @description Failed bounds and unsupported recruitment remain explicit to R users.
  inputs <- reference_point_test_inputs()
  result <- calculate_reference_points(inputs, max_f = 0.001)
  expect_equal(result$spr$status, "not_bracketed")
  expect_equal(result$msy$status, "upper_bound")
  inputs$recruitment <- NULL
  expect_null(calculate_reference_points(inputs, msy = FALSE)$msy)
  expect_error(calculate_reference_points(inputs), "Beverton-Holt")
  inputs$recruitment <- list(type = "unsupported")
  expect_error(calculate_reference_points(inputs), "only Beverton-Holt")
  expect_error(calculate_reference_points(inputs, max_iterations = 1.2), "max_iterations")
  expect_error(calculate_reference_points(inputs, spr_targets = NA_real_), "spr_targets")
  expect_error(calculate_reference_points(inputs, max_f = Inf), "max_f")
  expect_error(calculate_reference_points(inputs, msy = NA), "msy")
  inputs$weight <- 1
  expect_error(calculate_reference_points(inputs, msy = FALSE), "match ages")
})

make_reference_point_test_model <- function(multiplier = 1) {
  growth <- methods::new(EWAAGrowth)
  growth$n_years$set(2L)
  growth$ages[] <- 1:3
  growth$weights[] <- c(1, 2, 3, 2, 3, 4, 2, 3, 4)
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[] <- 2
  maturity$slope[] <- 1
  recruitment <- methods::new(BevertonHoltRecruitment)
  process <- methods::new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(process$get_id())
  recruitment$n_years$set(2L)
  recruitment$log_rzero[] <- log(1000)
  recruitment$log_rzero$set_estimation_types("fixed_effects")
  recruitment$logit_steep[] <- log((0.75 - 0.2) / (1 - 0.75))
  recruitment$log_devs[] <- 0
  selectivity <- methods::new(LogisticSelectivity)
  selectivity$inflection_point[] <- 2
  selectivity$slope[] <- 1
  population <- methods::new(Population)
  population$n_years$set(2L)
  population$n_ages$set(3L)
  population$n_fleets$set(2L)
  population$ages[] <- 1:3
  population$log_M[] <- log(c(0.2, 0.4, 0.6, 0.3, 0.4, 0.5))
  population$log_f_multiplier[] <- rep(log(multiplier), 2)
  population$log_init_naa[] <- log(c(1000, 500, 500))
  population$log_init_naa$set_estimation_types("fixed_effects")
  population$proportion_female[] <- 0.5
  population$SetGrowthID(growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$SetRecruitmentID(recruitment$get_id())
  for (share in c(0.25, 0.75)) {
    fleet <- methods::new(Fleet)
    fleet$n_years$set(2L)
    fleet$n_ages$set(3L)
    fleet$n_lengths$set(0L)
    fleet$log_Fmort[] <- log(c(0.2, 0.4) * share)
    fleet$log_q[] <- 0
    fleet$SetSelectivityID(selectivity$get_id())
    population$AddFleet(fleet$get_id())
  }
  model <- methods::new(CatchAtAge)
  model$AddPopulation(population$get_id())
  CreateTMBModel()
  list(model = model, population = population)
}

test_that("live population reference points preserve parameters and selected biology", {
  #' @description Live-model snapshots retain reference-year inputs and recruitment baseline.
  clear()
  on.exit(clear(), add = TRUE)
  live <- make_reference_point_test_model()
  parameters <- get_fixed()
  result <- get_reference_points(live$model, live$population$get_id(), 2L)
  expect_equal(result$inputs$natural_mortality, c(0.3, 0.4, 0.5))
  expect_equal(result$inputs$weight, c(2, 3, 4))
  expect_equal(result$inputs$proportion_female, rep(0.5, 3))
  expect_equal(vapply(result$inputs$fleets, `[[`, numeric(1), "share"), c(0.25, 0.75))
  expect_equal(result$inputs$year_index, 2L)
  expect_equal(length(result$inputs$fleet_ids), 2L)
  expect_equal(result$inputs$recruitment$steepness, 0.75)
  expect_equal(result$inputs$recruitment$rzero, 1000)
  expect_equal(get_fixed(), parameters)
  expect_equal(calculate_reference_points(result$inputs), result)
  expect_equal(get_reference_points(live$model, live$population$get_id(), 2L), result)
  # Synchronizing new fitted parameters must update the next snapshot.
  set_fixed(parameters + log(2))
  updated <- get_reference_points(live$model, live$population$get_id(), 2L)
  expect_equal(updated$inputs$recruitment$rzero, 2000)
  expect_equal(updated$msy$yield, 2 * result$msy$yield)
  expect_equal(get_fixed(), parameters + log(2))
  set_fixed(parameters)
  first <- get_reference_points(live$model, live$population$get_id(), 1L)
  # Independent age-varying survival, including the terminal geometric tail.
  unfished_numbers <- c(1, exp(-0.2), exp(-0.6) / -expm1(-0.6))
  expected_phi0 <- sum(unfished_numbers * c(1, 2, 3) * plogis((1:3) - 2) * 0.5)
  expect_equal(first$inputs$recruitment$phi0, expected_phi0)
  expect_equal(first$inputs$recruitment$phi0, result$inputs$recruitment$phi0)
  expect_equal(first$unfished_per_recruit$spawning_biomass, first$inputs$recruitment$phi0)
  expect_false(isTRUE(all.equal(
    result$unfished_per_recruit$spawning_biomass,
    result$inputs$recruitment$phi0
  )))
  override <- get_reference_points(live$model, live$population$get_id(), 2L,
    fleet_shares = c(1, 0)
  )
  expect_equal(override$inputs$fleets[[2]]$share, 0)
  expect_error(get_reference_points(live$model, live$population$get_id(), 3L), "reference year")
  expect_error(get_reference_points(live$model, 9999L, 1L), "population ID")
  expect_error(get_reference_points(live$model, live$population$get_id(), 1.5), "integer")
  clear()
  expect_error(get_reference_points(live$model, live$population$get_id(), 1L), "Create the model")
})

multifleet_reference_inputs <- function() {
  list(
    natural_mortality = c(0.2, 0.4, 0.6), weight = c(1, 2, 3),
    maturity = c(0, 0.5, 1), proportion_female = rep(0.5, 3),
    fleets = list(
      list(share = 0.65, selectivity = c(0.2, 0.7, 1), weight = c(1, 2, 3)),
      list(share = 0.35, selectivity = c(0.8, 1, 0.4), weight = c(0.8, 3, 4.5))
    ),
    recruitment = list(
      type = "beverton_holt", rzero = 1000, steepness = 0.75,
      phi0 = 0.5 * exp(-0.2) + 1.5 * exp(-0.6) / -expm1(-0.6)
    )
  )
}

# Independent equations: cumulative survival and the closed-form biomass
# equilibrium. This helper never calls the C++ reference-point engine.
independent_multifleet_equilibrium <- function(inputs, f) {
  fleet_f <- vapply(inputs$fleets, function(fleet) {
    f * fleet$share * fleet$selectivity
  }, numeric(length(inputs$natural_mortality)))
  z <- inputs$natural_mortality + rowSums(fleet_f)
  numbers <- exp(-c(0, head(cumsum(z), -1)))
  last <- length(numbers)
  numbers[last] <- numbers[last] / -expm1(-z[last])
  phi <- sum(numbers * inputs$weight * inputs$maturity * inputs$proportion_female)
  sr <- inputs$recruitment
  spawning_biomass <- max(
    0,
    (4 * sr$steepness * sr$rzero * phi - sr$rzero * sr$phi0 * (1 - sr$steepness)) /
      (5 * sr$steepness - 1)
  )
  recruitment <- spawning_biomass / phi
  fleet_yield <- vapply(seq_along(inputs$fleets), function(k) {
    recruitment * sum(numbers * fleet_f[, k] * -expm1(-z) / z * inputs$fleets[[k]]$weight)
  }, numeric(1))
  included <- vapply(inputs$fleets, function(fleet) {
    is.null(fleet$include_in_msy) || fleet$include_in_msy
  }, logical(1))
  list(
    objective_yield = sum(fleet_yield[included]), total_yield = sum(fleet_yield),
    fleet_yield = fleet_yield, biomass = recruitment * sum(numbers * inputs$weight),
    spawning_biomass = spawning_biomass, recruitment = recruitment
  )
}

test_that("multifleet MSY agrees with independent equations and optimization", {
  #' @description Different fleet selectivities, weights, and inclusion flags match an independent optimizer.
  for (flags in list(c(TRUE, TRUE), c(TRUE, FALSE), c(FALSE, TRUE))) {
    inputs <- multifleet_reference_inputs()
    for (i in seq_along(flags)) inputs$fleets[[i]]$include_in_msy <- flags[i]
    objective <- function(f) independent_multifleet_equilibrium(inputs, f)$objective_yield
    grid <- seq(0, 5, length.out = 2001)
    grid_yield <- vapply(grid, objective, numeric(1))
    peak <- which.max(grid_yield)
    expected_optimum <- optimize(objective, grid[c(peak - 1, peak + 1)],
      maximum = TRUE, tol = 1e-10
    )
    expected <- independent_multifleet_equilibrium(inputs, expected_optimum$maximum)
    result <- calculate_reference_points(inputs)
    expect_equal(result$msy$status, "converged")
    expect_equal(result$msy$fishing_mortality, expected_optimum$maximum, tolerance = 1e-6)
    expect_gte(result$msy$objective_yield + 1e-7, max(grid_yield))
    for (field in names(expected)) {
      expect_equal(result$msy[[field]], expected[[field]], tolerance = 1e-6)
    }
    # Legacy yield always means total catch, including excluded bycatch fleets.
    expect_identical(result$msy$yield, result$msy$total_yield)
    if (!all(flags)) expect_gt(result$msy$total_yield, result$msy$objective_yield)
    # Change both the bound and grid spacing to check this example's stability.
    finer <- calculate_reference_points(inputs, max_f = 10, grid_intervals = 400)
    expect_equal(finer$msy$status, "converged")
    expect_equal(finer$msy$fishing_mortality, result$msy$fishing_mortality, tolerance = 1e-6)
    expect_equal(finer$msy$objective_yield, result$msy$objective_yield, tolerance = 1e-8)
  }
})

test_that("fleet inclusion defaults and validation are explicit", {
  #' @description Missing flags preserve legacy results; all-excluded MSY and malformed flags are errors.
  inputs <- multifleet_reference_inputs()
  default <- calculate_reference_points(inputs)
  for (i in seq_along(inputs$fleets)) inputs$fleets[[i]]$include_in_msy <- TRUE
  explicit <- calculate_reference_points(inputs)
  expect_identical(default$msy, explicit$msy)
  expect_identical(default$msy$objective_yield, default$msy$total_yield)
  for (bad in list(NA, 0, 1, "FALSE", c(TRUE, FALSE), logical(), NULL)) {
    inputs$fleets[[1]]["include_in_msy"] <- list(bad)
    expect_error(calculate_reference_points(inputs), "include_in_msy must be TRUE or FALSE")
  }
  for (i in seq_along(inputs$fleets)) inputs$fleets[[i]]$include_in_msy <- FALSE
  expect_error(calculate_reference_points(inputs), "at least one included fleet")
  excluded <- calculate_reference_points(inputs, msy = FALSE)$spr
  unchanged <- setdiff(names(default$spr), c("objective_yield", "objective_yield_per_recruit"))
  expect_identical(excluded[unchanged], default$spr[unchanged])
  expect_equal(excluded$objective_yield, 0)
})

test_that("live fleet inclusion is saved without changing the fishing pattern", {
  #' @description Live-model flags follow fleet IDs, survive serialization, and preserve shares and SPR.
  clear()
  on.exit(clear(), add = TRUE)
  live <- make_reference_point_test_model()
  default <- get_reference_points(live$model, live$population$get_id(), 2L)
  ids <- as.character(default$inputs$fleet_ids)
  # Deliberately reverse named order to verify mapping by ID, not position.
  flags <- setNames(c(FALSE, TRUE), rev(ids))
  excluded <- get_reference_points(live$model, live$population$get_id(), 2L,
    include_in_msy = flags
  )
  expect_equal(vapply(excluded$inputs$fleets, `[[`, logical(1), "include_in_msy"), c(TRUE, FALSE))
  expect_equal(vapply(excluded$inputs$fleets, `[[`, numeric(1), "share"), c(0.25, 0.75))
  unchanged <- setdiff(names(default$spr), c("objective_yield", "objective_yield_per_recruit"))
  expect_identical(default$spr[unchanged], excluded$spr[unchanged])
  expect_equal(excluded$msy$objective_yield, excluded$msy$fleet_yield[1])
  expect_gt(excluded$msy$fleet_yield[2], 0)
  expect_equal(get_reference_points(live$model, live$population$get_id(), 2L,
    include_in_msy = c(TRUE, FALSE)
  ), excluded)
  # A later call without overrides must not retain the previous inclusion flags.
  expect_equal(get_reference_points(live$model, live$population$get_id(), 2L), default)
  expect_error(get_reference_points(live$model, live$population$get_id(), 2L,
    include_in_msy = c(FALSE, FALSE)
  ), "at least one included fleet")
  expect_error(get_reference_points(live$model, live$population$get_id(), 2L,
    include_in_msy = TRUE
  ), "one TRUE or FALSE per fleet")
  expect_error(get_reference_points(live$model, live$population$get_id(), 2L,
    include_in_msy = setNames(c(TRUE, FALSE), c("bad", "ids"))
  ), "fleet IDs")
  expect_error(get_reference_points(live$model, live$population$get_id(), 2L,
    include_in_msy = c(TRUE, NA)
  ), "one TRUE or FALSE per fleet")
  snapshot_file <- tempfile(fileext = ".rds")
  on.exit(unlink(snapshot_file), add = TRUE)
  saveRDS(excluded$inputs, snapshot_file)
  clear()
  expect_equal(calculate_reference_points(readRDS(snapshot_file)), excluded)
})

test_that("SPR benchmarks return independent equilibrium quantities and collapse", {
  #' @description Each target is evaluated at its own root, independently of MSY.
  inputs <- multifleet_reference_inputs()
  inputs$fleet_ids <- c(7L, 12L)
  inputs$fleets[[2]]$include_in_msy <- FALSE
  result <- calculate_reference_points(inputs, spr_targets = c(1, 0.4, 0.3, 0.05))
  expect_equal(result$spr$status, rep("converged", 4))
  expect_equal(result$spr$collapsed, c(FALSE, FALSE, FALSE, TRUE))
  expect_equal(result$spr$recruitment, c(1000, 1000 * 19 / 22, 1000 * 26 / 33, 0),
    tolerance = 1e-7
  )
  for (i in seq_len(nrow(result$spr))) {
    expected <- independent_multifleet_equilibrium(inputs, result$spr$fishing_mortality[i])
    for (field in setdiff(names(expected), "fleet_yield")) {
      expect_equal(result$spr[[field]][i], expected[[field]], tolerance = 1e-10)
    }
    expect_equal(result$spr$fleet_yield[[i]], expected$fleet_yield, tolerance = 1e-10)
    expect_equal(
      result$spr$fleet_yield[[i]],
      result$spr$recruitment[i] * result$spr$fleet_yield_per_recruit[[i]]
    )
  }
  spr_only <- calculate_reference_points(inputs, spr_targets = result$spr$target, msy = FALSE)
  expect_identical(spr_only$spr, result$spr)
  expect_null(spr_only$msy)
  table <- result$reference_points
  expect_equal(table$reference_point, c("MSY", "SPR_1", "SPR_0.4", "SPR_0.3", "SPR_0.05"))
  expect_equal(table$fleet_yield_7 + table$fleet_yield_12, table$total_yield)
  expect_equal(table$fleet_yield_7, table$objective_yield)
  expect_equal(table$achieved_spr[-1], result$spr$spr)
  expect_equal(table$total_biomass[1], result$msy$biomass)
  expect_true(is.na(table$residual[1]))
  expect_lte(result$msy$bracket_width, result$settings$tolerance)
  expect_gte(result$msy$intervals_refined, 2)
  expect_true(all(result$spr$lower_bound <= result$spr$fishing_mortality))
  expect_true(all(result$spr$upper_bound >= result$spr$fishing_mortality))
})

test_that("SPR-only outputs preserve missing recruitment and empty targets", {
  #' @description Missing equilibrium scales are NA, including fleet catches and collapse.
  inputs <- reference_point_test_inputs()
  inputs$recruitment <- NULL
  result <- calculate_reference_points(inputs, msy = FALSE)
  expect_true(all(is.na(result$spr$recruitment)))
  expect_true(all(is.na(result$spr$collapsed)))
  expect_true(all(is.na(result$spr$fleet_yield[[1]])))
  expect_gt(result$spr$total_yield_per_recruit, 0)
  expect_equal(nrow(calculate_reference_points(inputs, spr_targets = numeric(), msy = FALSE)$reference_points), 0)
  inputs$fleets <- list()
  no_fishing <- calculate_reference_points(inputs, msy = FALSE)
  expect_equal(no_fishing$spr$status, "not_bracketed")
  expect_length(no_fishing$spr$fleet_yield[[1]], 0)
  inputs <- reference_point_test_inputs()
  expect_equal(nrow(calculate_reference_points(inputs, spr_targets = numeric())$reference_points), 1)
  inputs$recruitment$type <- "unsupported"
  expect_true(is.na(calculate_reference_points(inputs, msy = FALSE)$spr$recruitment))
  inputs$recruitment$type <- "beverton_holt"
  inputs$recruitment$steepness <- NA_real_
  expect_error(calculate_reference_points(inputs, msy = FALSE), "Beverton-Holt")
  inputs$fleet_ids <- c(1, 1)
  expect_error(calculate_reference_points(inputs, msy = FALSE), "fleet_ids")
})

test_that("SPR uses benchmark biology without rebasing recruitment", {
  #' @description Changing the stock-recruit baseline changes equilibrium scale, not SPR roots.
  inputs <- multifleet_reference_inputs()
  original <- calculate_reference_points(inputs, spr_targets = c(0.4, 0.3), msy = FALSE)
  inputs$recruitment$phi0 <- inputs$recruitment$phi0 * 2
  changed <- calculate_reference_points(inputs, spr_targets = c(0.4, 0.3), msy = FALSE)
  expect_identical(changed$spr$fishing_mortality, original$spr$fishing_mortality)
  expect_true(all(changed$spr$recruitment < original$spr$recruitment))
  for (i in 1:2) {
    expected <- independent_multifleet_equilibrium(inputs, changed$spr$fishing_mortality[i])
    expect_equal(changed$spr$recruitment[i], expected$recruitment)
  }
})

test_that("SPR vectors obey reproductive and catch scale invariance", {
  #' @description Uniform positive reproductive scaling cancels in SPR; catch weights do not enter SPR.
  inputs <- multifleet_reference_inputs()
  targets <- c(0.8, 0.4, 0.3)
  original <- calculate_reference_points(inputs, targets, msy = FALSE)$spr$fishing_mortality
  for (field in c("weight", "maturity", "proportion_female")) {
    scaled <- inputs
    scaled[[field]] <- scaled[[field]] * 0.5
    expect_equal(calculate_reference_points(scaled, targets, msy = FALSE)$spr$fishing_mortality, original)
  }
  for (i in seq_along(inputs$fleets)) inputs$fleets[[i]]$weight <- inputs$fleets[[i]]$weight * 1000
  expect_equal(calculate_reference_points(inputs, targets, msy = FALSE)$spr$fishing_mortality, original)
})

test_that("solved benchmarks are stationary under independent annual transitions", {
  #' @description Recruitment, plus-group retention, and catches remain at the solved equilibrium.
  inputs <- multifleet_reference_inputs()
  result <- calculate_reference_points(inputs, spr_targets = c(0.4, 0.3))
  for (i in seq_len(nrow(result$reference_points))) {
    row <- result$reference_points[i, ]
    f <- row$fishing_mortality
    z <- inputs$natural_mortality + Reduce(`+`, lapply(inputs$fleets, function(v) f * v$share * v$selectivity))
    n <- row$recruitment * exp(-c(0, head(cumsum(z), -1)))
    n[length(n)] <- n[length(n)] / -expm1(-tail(z, 1))
    for (year in 1:20) {
      ssb <- sum(n * inputs$weight * inputs$maturity * inputs$proportion_female)
      sr <- inputs$recruitment
      recruitment <- 4 * sr$steepness * sr$rzero * ssb /
        (sr$rzero * sr$phi0 * (1 - sr$steepness) + (5 * sr$steepness - 1) * ssb)
      catch <- sum(vapply(inputs$fleets, function(v) {
        sum(n * f * v$share * v$selectivity * -expm1(-z) / z * v$weight)
      }, numeric(1)))
      expect_equal(ssb, row$spawning_biomass, tolerance = 1e-10)
      expect_equal(catch, row$total_yield, tolerance = 1e-10)
      next_n <- c(recruitment, head(n * exp(-z), -1))
      next_n[length(n)] <- next_n[length(n)] + tail(n * exp(-z), 1)
      expect_equal(next_n, n, tolerance = 1e-10)
      n <- next_n
    }
  }
})

test_that("live snapshots retain effective population fishing intensity", {
  #' @description The population multiplier cancels in shares but remains in comparable F coefficients.
  clear()
  on.exit(clear(), add = TRUE)
  live <- make_reference_point_test_model(multiplier = 3)
  result <- get_reference_points(live$model, live$population$get_id(), 2L)
  expect_equal(result$inputs$f_multiplier, 3)
  expect_equal(result$inputs$effective_fishing_mortality, 1.2)
  expect_equal(vapply(result$inputs$fleets, `[[`, numeric(1), "share"), c(0.25, 0.75))
  expect_equal(calculate_reference_points(result$inputs), result)
})

test_that("MSY refines multiple peaks and the upper boundary interval", {
  #' @description Independent dense scans detect both modes and boundary-adjacent optima.
  inputs <- list(
    natural_mortality = c(0.416356, 0.156127, 0.340233, 0.187815, 0.175327, 0.233294),
    weight = c(11.6065, 39.0565, 2.85594, 108.572, 1.5719, 15.695),
    maturity = c(0, 0, 1, 1, 1, 1), proportion_female = rep(0.5, 6),
    fleets = list(
      list(
        share = 0.5, selectivity = c(0.0237445, 0.0963198, 0.0577185, 10.53, 4.78878, 15.5413),
        weight = c(23.2008, 6.76598, 39.1049, 23.0764, 5.59064, 118.398)
      ),
      list(
        share = 0.5, selectivity = c(0.0416688, 0.0190299, 0.656334, 9.19992, 7.78927, 27.2963),
        weight = c(43.7937, 69.6794, 49.3288, 2.15177, 27.9686, 46.6037)
      )
    )
  )
  n <- exp(-c(0, head(cumsum(inputs$natural_mortality), -1)))
  n[6] <- n[6] / -expm1(-inputs$natural_mortality[6])
  inputs$recruitment <- list(
    type = "beverton_holt", rzero = 1000, steepness = 0.95,
    phi0 = sum(n * inputs$weight * inputs$maturity * inputs$proportion_female)
  )
  grid <- seq(0, 5, length.out = 2001)
  objective <- function(f) independent_multifleet_equilibrium(inputs, f)$objective_yield
  yields <- vapply(grid, objective, numeric(1))
  peaks <- which(diff(sign(diff(yields))) == -2) + 1L
  expect_gte(length(peaks), 2)
  maxima <- lapply(peaks, function(i) optimize(objective, grid[c(i - 1L, i + 1L)], maximum = TRUE))
  expected <- max(vapply(maxima, `[[`, numeric(1), "objective"))
  result <- calculate_reference_points(inputs)
  expect_equal(result$msy$status, "converged")
  expect_gte(result$msy$intervals_refined, 4)
  expect_equal(result$msy$objective_yield, expected, tolerance = 1e-7)
  inputs <- reference_point_test_inputs()
  for (bound in c(0.41, 0.372369089366)) {
    result <- calculate_reference_points(inputs,
      max_f = bound,
      grid_intervals = if (bound == 0.41) 2L else 100L
    )
    expect_equal(result$msy$status, "converged")
    expect_equal(result$msy$fishing_mortality, 0.37199709, tolerance = 1e-7)
    expect_equal(result$msy$objective_yield, 503.371401258, tolerance = 1e-9)
  }
})
