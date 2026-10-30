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

test_that("group-specific mortality is differentiable through abundance", {
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
})
