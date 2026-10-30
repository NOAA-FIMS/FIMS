test_that("subpopulation configuration validates and round trips", {
  on.exit(clear(), add = TRUE)
  population <- methods::new(Population)
  expect_length(population$GetSubpopulations(), 0)
  population$SetTwoSex(0.6, 0.4)
  groups <- population$GetSubpopulations()
  expect_equal(vapply(groups, `[[`, "", "name"), c("female", "male"))
  expect_equal(groups[[1]]$recruitment_fraction, 0.4)
  expect_equal(groups[[2]]$reproductive_contribution, 0)
  expect_error(population$SetTwoSex(-0.1, 0.5), "fractions")
  expect_identical(population$GetSubpopulations(), groups)
  bad <- groups
  bad[[2]]$name <- "female"
  expect_error(population$SetSubpopulations(bad), "unique")
  expect_error(population$SetSubpopulations(list(list(name = "female"))), "Missing")
  population$SetSubpopulations(list())
  expect_length(population$GetSubpopulations(), 0)
})

test_that("explicit groups run through R, TMB and output reshaping", {
  on.exit(clear(), add = TRUE)
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data = data)
  groups <- list(
    list(
      name = "female", initial_fraction = 0.5,
      recruitment_fraction = 0.5, reproductive_contribution = 1
    ),
    list(
      name = "male", initial_fraction = 0.5,
      recruitment_fraction = 0.5, reproductive_contribution = 0
    )
  )
  input <- initialize_fims(parameters, data, subpopulations = groups)
  fit <- fit_fims(input, optimize = FALSE)
  report <- get_report(fit)
  expect_true(all(is.finite(fit@obj$gr())))
  n <- report$numbers_at_age_by_subpopulation[[1]]
  pooled <- report$numbers_at_age[[1]]
  expect_length(n, 2 * length(pooled))
  expect_equal(n[seq_along(pooled)] + n[length(pooled) + seq_along(pooled)], pooled)
  estimates <- get_estimates(fit)
  expect_true("subpopulation_i" %in% names(estimates))
  expect_true(any(estimates$label == "numbers_at_age_by_subpopulation"))
})

test_that("group mortality, growth and maturity retain automatic derivatives", {
  on.exit(clear(), add = TRUE)
  population <- methods::new(Population)
  population$n_years$set(2)
  population$n_ages$set(3)
  population$n_fleets$set(0)
  population$ages[] <- 1:3
  population$SetTwoSex(0.6, 0.4)
  population$log_M[] <- log(c(rep(0.2, 6), rep(0.5, 6)))
  population$log_M$set_estimation_types("fixed_effects")
  population$log_init_naa[] <- log(c(100, 80, 60))
  growth <- methods::new(EWAAGrowth)
  growth$n_years$set(2)
  growth$ages[] <- 1:3
  growth$weights[] <- c(1, 2, 3)
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[] <- 2
  maturity$slope[] <- 1
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment$n_years$set(2)
  recruitment$log_rzero[] <- log(100)
  recruitment$logit_steep[] <- 0
  recruitment$log_r[] <- log(100)
  process <- methods::new(LogRRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(process$get_id())
  population$SetGrowthID(growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$SetRecruitmentID(recruitment$get_id())
  female_growth <- methods::new(VonBertalanffySchnuteGrowth)
  female_growth$n_ages$set(3)
  female_growth$mean_length_young[] <- 20
  female_growth$mean_length_old[] <- 60
  female_growth$growth_coefficient[] <- 0.2
  female_growth$reference_age_for_length_young[] <- 1
  female_growth$reference_age_for_length_old[] <- 3
  female_growth$length_weight_a[] <- 1e-5
  female_growth$length_weight_a$set_estimation_types("fixed_effects")
  female_growth$length_weight_b[] <- 3
  female_growth$length_at_age_sd_at_reference_ages[] <- c(2, 3)
  female_maturity <- methods::new(LogisticMaturity)
  female_maturity$inflection_point[] <- 1.5
  female_maturity$inflection_point$set_estimation_types("fixed_effects")
  female_maturity$slope[] <- 0.8
  groups <- population$GetSubpopulations()
  groups[[1]]$growth_id <- female_growth$get_id()
  groups[[1]]$maturity_id <- female_maturity$get_id()
  population$SetSubpopulations(groups)
  model <- methods::new(CatchAtAge)
  model$AddPopulation(population$get_id())
  CreateTMBModel()
  pars <- list(p = get_fixed(), re = get_random())
  obj <- TMB::MakeADFun(list(), pars, DLL = "FIMS", silent = TRUE, ADreport = TRUE)
  values <- obj$fn(obj$par)
  jac <- obj$gr(obj$par)
  h <- 1e-5
  upper <- lower <- obj$par
  upper[1] <- upper[1] + h
  lower[1] <- lower[1] - h
  finite_difference <- (obj$fn(upper) - obj$fn(lower)) / (2 * h)
  expect_true(all(is.finite(jac)))
  expect_gt(max(abs(jac[, 1])), 1)
  expect_gt(max(abs(jac[, 7])), 1)
  expect_equal(as.numeric(jac[, 1]), as.numeric(finite_difference), tolerance = 1e-6)
  columns <- c(
    grep("length_weight_a", names(get_parameter_names(obj$par))),
    grep("Maturity.*inflection_point", names(get_parameter_names(obj$par)))
  )
  expect_length(columns, 2)
  for (column in columns) {
    upper <- lower <- obj$par
    upper[column] <- upper[column] + h
    lower[column] <- lower[column] - h
    difference <- (obj$fn(upper) - obj$fn(lower)) / (2 * h)
    expect_gt(max(abs(jac[, column])), 1e-6)
    expect_equal(as.numeric(jac[, column]), as.numeric(difference), tolerance = 1e-6)
  }
})

test_that("Rcpp subpopulation module references round trip and validate IDs", {
  on.exit(clear(), add = TRUE)
  population <- methods::new(Population)
  population$SetTwoSex(0.5, 0.5)
  groups <- population$GetSubpopulations()
  groups[[1]]$growth_id <- 3L
  groups[[1]]$maturity_id <- 4L
  groups[[1]]$selectivity_ids <- c("5" = 6L)
  population$SetSubpopulations(groups)
  expect_identical(population$GetSubpopulations(), groups)
  bad <- groups
  bad[[1]]$growth_id <- NA_real_
  expect_error(population$SetSubpopulations(bad), "nonnegative integers")
  bad <- groups
  bad[[1]]$selectivity_ids <- c(6L)
  expect_error(population$SetSubpopulations(bad), "named by fleet")
  expect_identical(population$GetSubpopulations(), groups)
  bad <- groups
  bad[[1]]$growth_typo <- 3L
  expect_error(population$SetSubpopulations(bad), "Unknown or duplicate")
})

test_that("high-level biological overrides affect state and retain derivatives", {
  on.exit(clear(), add = TRUE)
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data = data)
  female_maturity <- parameters |>
    dplyr::filter(.data$module_name == "Maturity") |>
    dplyr::mutate(
      value = dplyr::if_else(.data$label == "inflection_point", 3, 0.7),
      estimation_type = "constant"
    )
  female_selectivity <- parameters |>
    dplyr::filter(.data$module_name == "Selectivity", .data$fleet == "fleet1") |>
    dplyr::mutate(
      value = dplyr::if_else(.data$label == "inflection_point", 2, 0.9),
      estimation_type = "fixed_effects"
    )
  ages <- get_ages(data)
  groups <- list(
    list(
      name = "female", initial_fraction = 0.5, recruitment_fraction = 0.5,
      reproductive_contribution = 1,
      growth = list(weights = 0.1 * ages^2),
      maturity = list(parameters = female_maturity),
      selectivity = list(fleet1 = list(parameters = female_selectivity))
    ),
    list(
      name = "male", initial_fraction = 0.5, recruitment_fraction = 0.5,
      reproductive_contribution = 0
    )
  )
  input <- initialize_fims(parameters, data, subpopulations = groups)
  fit <- fit_fims(input, optimize = FALSE)
  report <- get_report(fit)
  n <- report$numbers_at_age_by_subpopulation[[1]]
  w <- report$weight_at_age_by_subpopulation[[1]]
  mature <- report$proportion_mature_at_age_by_subpopulation[[1]]
  plane <- length(n) / 2
  female <- seq_len(plane)
  male <- plane + female
  expect_equal(w[female], rep(0.1 * ages^2, plane / length(ages)))
  expect_equal(mature[female], rep(plogis(0.7 * (ages - 3)), plane / length(ages)))
  expect_equal(report$biomass[[1]], rowSums(matrix(n[female] * w[female] + n[male] * w[male], ncol = length(ages), byrow = TRUE)))
  expect_equal(report$spawning_biomass[[1]], rowSums(matrix(n[female] * w[female] * mature[female], ncol = length(ages), byrow = TRUE)))
  expect_false(isTRUE(all.equal(n[length(ages) + 2], n[plane + length(ages) + 2])))
  obj <- get_obj(fit)
  gradient <- obj$gr(obj$par)
  expect_true(all(is.finite(gradient)))
  # The final Selectivity module is the female override built after shared modules.
  selection <- tail(grep("Selectivity.*inflection_point", names(get_parameter_names(obj$par))), 1)
  expect_length(selection, 1)
  upper <- lower <- obj$par
  h <- 1e-5
  upper[selection] <- upper[selection] + h
  lower[selection] <- lower[selection] - h
  difference <- (obj$fn(upper) - obj$fn(lower)) / (2 * h)
  expect_equal(as.numeric(gradient[selection]), as.numeric(difference), tolerance = 1e-4)
  expect_gt(abs(gradient[selection]), 1e-6)
  obj$fn(obj$par)
  expect_true(any(get_estimates(fit)$label == "weight_at_age_by_subpopulation"))
  groups[[1]]$selectivity <- list(unknown_fleet = list(parameters = female_selectivity))
  expect_error(initialize_fims(parameters, data, subpopulations = groups), "known fleets")
})
