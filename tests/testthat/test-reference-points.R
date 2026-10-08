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

make_reference_point_test_model <- function() {
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
