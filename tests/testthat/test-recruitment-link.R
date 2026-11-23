# Low-level Rcpp prototype; wrapper-based multi-population inputs are future work.
make_linked_recruitment_example <- function(reverse = FALSE, fraction = 0.4, initialize = TRUE) {
  recruitment <- methods::new(BevertonHoltRecruitment)
  process <- methods::new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(process$get_id())
  recruitment$n_years$set(3L)
  recruitment$log_rzero[1]$value <- log(1000)
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  recruitment$logit_steep[1]$value <- log((0.75 - 0.2) / (1 - 0.75))
  recruitment$log_devs[] <- c(0, 0)

  make_population <- function(fraction, mortality, contribution) {
    maturity <- methods::new(ConstantMaturity)
    maturity$SetValue(contribution)
    growth <- methods::new(EWAAGrowth)
    growth$n_years$set(3L)
    growth$ages[] <- 1:3
    growth$weights[] <- rep(2, 3)
    population <- methods::new(Population)
    population$n_years$set(3L)
    population$n_ages$set(3L)
    population$n_fleets$set(0L)
    population$ages[] <- 1:3
    population$log_M[] <- rep(log(mortality), 9)
    population$log_f_multiplier[] <- rep(0, 3)
    population$log_init_naa[] <- log(fraction * 1000) - c(
      0, mortality, 2 * mortality + log(1 - exp(-mortality))
    )
    population$log_init_naa$set_estimation_types("constant")
    population$SetGrowthID(growth$get_id())
    population$SetMaturityID(maturity$get_id())
    population$SetRecruitmentID(recruitment$get_id())
    population
  }
  female <- make_population(fraction, 0.2, 1)
  male <- make_population(1 - fraction, 0.3, 0)
  model <- methods::new(CatchAtAge)
  ids <- c(female$get_id(), male$get_id())
  for (id in if (reverse) rev(ids) else ids) model$AddPopulation(id)
  model$LinkRecruitment(ids, c(fraction, 1 - fraction))
  obj <- NULL
  if (initialize) {
    CreateTMBModel()
    obj <- TMB::MakeADFun(
      data = list(), parameters = list(p = get_fixed(), re = get_random()),
      DLL = "FIMS", silent = TRUE
    )
  }
  list(obj = obj, model = model, ids = ids, female = female, male = male)

}

test_that("linked recruitment works through the Rcpp and TMB interfaces", {
  on.exit(clear())
  clear()
  example <- make_linked_recruitment_example()
  report <- example$obj$report()
  #' @description Shared recruitment is registered once and conserves recruits in fished and unfished states.
  expect_length(get_fixed(), 1)
  expect_true(is.finite(example$obj$fn(example$obj$par)))
  expect_true(all(is.finite(example$obj$gr(example$obj$par))))
  expect_equal(report$expected_recruitment[[1]], rep(400, 4))
  expect_equal(report$expected_recruitment[[2]], rep(600, 4))
  expect_equal(report$spawning_biomass[[2]], rep(0, 4))
  expect_equal(report$spawning_biomass_ratio[[1]], rep(1, 4))
  expect_equal(report$spawning_biomass_ratio[[2]], rep(1, 4))
  # A unit covariance for log(R0) makes the reported standard error equal to
  # the absolute derivative. Check the full shared recruitment path against FD.
  sdr <- TMB::sdreport(example$obj, hessian.fixed = matrix(1))
  reported <- summary(sdr, "report")
  recruitment_se <- reported[
    rownames(reported) == "expected_recruitment", "Std. Error"
  ]
  step <- 1e-5
  upper <- unlist(example$obj$report(example$obj$par + step)$expected_recruitment)
  lower <- unlist(example$obj$report(example$obj$par - step)$expected_recruitment)
  #' @description Automatic derivatives through shared recruitment agree with finite differences.
  expect_equal(unname(recruitment_se), abs((upper - lower) / (2 * step)), tolerance = 1e-6)
  configuration <- jsonlite::fromJSON(example$model$get_output(), simplifyVector = FALSE)
  expect_length(configuration$recruitment_links, 1)
  expect_equal(configuration$recruitment_links[[1]]$allocations[[1]]$fraction, 0.4)
})

test_that("constant maturity supports exact zero and rejects invalid values", {
  on.exit(clear())
  clear()
  maturity <- methods::new(ConstantMaturity)
  #' @description A fixed spawning contribution has exact endpoints and validates its range.
  expect_equal(maturity$evaluate(5), 0)
  maturity$SetValue(1)
  expect_equal(maturity$evaluate(5), 1)
  for (value in c(-0.1, 1.1, NA_real_, Inf)) {
    expect_error(maturity$SetValue(value), "finite and in")
  }
})

test_that("recruitment links reject invalid allocations at the R boundary", {
  on.exit(clear())
  clear()
  model <- methods::new(CatchAtAge)
  a <- methods::new(Population)
  b <- methods::new(Population)
  model$AddPopulation(a$get_id())
  model$AddPopulation(b$get_id())
  ids <- c(a$get_id(), b$get_id())
  #' @description Invalid and overlapping links are rejected before model creation.
  expect_error(model$LinkRecruitment(ids, 0.5), "equal lengths")
  expect_error(model$LinkRecruitment(ids, c(0.4, 0.4)), "sum to one")
  expect_error(model$LinkRecruitment(c(ids[1], ids[1]), c(0.5, 0.5)), "Duplicate")
  expect_error(model$LinkRecruitment(ids, c(NA_real_, 1)), "finite")
  expect_error(model$LinkRecruitment(c(ids[1], 99999L), c(0.5, 0.5)), "Add each")
  model$LinkRecruitment(ids, c(0.4, 0.6))
  expect_error(model$LinkRecruitment(ids, c(0.4, 0.6)), "multiple")
})

make_pooled_observation_example <- function(use_catch = TRUE, shared = FALSE, annual_q = FALSE) {
  example <- make_linked_recruitment_example(initialize = FALSE)
  make_fleet <- function(population = NULL, male = FALSE) {
    fleet <- methods::new(Fleet)
    fleet$n_years$set(3L)
    fleet$n_ages$set(3L)
    fleet$n_lengths$set(2L)
    fleet$lengths[] <- c(10, 20)
    fleet$log_Fmort[] <- rep(0, 3)
    fleet$age_to_length_conversion$resize(0)
    fleet$observed_catch_units$set("weight")
    fleet$observed_index_units$set("number")
    if (!is.null(population)) {
      selectivity <- methods::new(LogisticSelectivity)
      selectivity$inflection_point[1]$value <- if (male) 2 else 1
      selectivity$slope[1]$value <- 1
      fleet$SetSelectivityID(selectivity$get_id())
      fleet$log_Fmort[] <- rep(log(if (male) 0.35 else 0.2), 3)
      fleet$log_q[1]$value <- log(if (male) 0.4 else 0.7)
      fleet$SetRequiresAgeLengthMapping(TRUE)
      fleet$age_to_length_conversion[] <- if (male) c(0, 1, 0, 1, 0, 1) else c(1, 0, 0.5, 0.5, 0, 1)
      if (male) fleet$weight_at_age[] <- rep(4, 9)
      population$AddFleet(fleet$get_id())
      population$n_fleets$set(1L)
    }
    fleet
  }
  female <- make_fleet(example$female)
  male <- make_fleet(example$male, TRUE)
  target <- make_fleet()
  if (shared) {
    female$log_Fmort$set_estimation_types("fixed_effects")
    if (annual_q) female$log_q[] <- log(c(0.6, 0.7, 0.8))
    female$log_q$set_estimation_types("fixed_effects")
    example$model$ShareFishingMortality(female$get_id(), male$get_id())
    example$model$ShareCatchability(female$get_id(), male$get_id())
  }
  catch <- methods::new(Catch, 3L)
  catch$catch_data[] <- rep(1000, 3)
  index <- methods::new(Index, 3L)
  index$index_data[] <- rep(2000, 3)
  ages <- methods::new(AgeComp, 3L, 3L)
  ages$age_comp_data[] <- rep(c(10, 20, 30), 3)
  lengths <- methods::new(LengthComp, 3L, 2L)
  lengths$length_comp_data[] <- rep(c(12, 18), 3)
  target$SetObservedCatchDataID(catch$get_id())
  target$SetObservedIndexDataID(index$get_id())
  target$SetObservedAgeCompDataID(ages$get_id())
  target$SetObservedLengthCompDataID(lengths$get_id())
  catch_density <- methods::new(DnormDistribution)
  catch_density$log_sd[1]$value <- log(100)
  catch_density$set_observed_data(catch$get_id())
  catch_density$set_distribution_links("data", target$catch_expected$get_id())
  index_density <- methods::new(DnormDistribution)
  index_density$log_sd[1]$value <- log(100)
  index_density$set_observed_data(index$get_id())
  index_density$set_distribution_links("data", target$index_expected$get_id())
  for (inputs in list(
    list(data = ages, bins = 3L, variable = target$agecomp_proportion),
    list(data = lengths, bins = 2L, variable = target$lengthcomp_proportion)
  )) {
    density <- methods::new(DmultinomDistribution)
    density$dims$resize(2)
    density$dims$set(0, 3L)
    density$dims$set(1, inputs$bins)
    density$set_observed_data(inputs$data$get_id())
    density$set_distribution_links("data", inputs$variable$get_id())
  }
  example$model$LinkFleetObservations(target$get_id(), c(female$get_id(), male$get_id()), use_catch)
  CreateTMBModel()
  example$obj <- TMB::MakeADFun(
    data = list(), parameters = list(p = get_fixed(), re = get_random()), DLL = "FIMS", silent = TRUE
  )
  example$target <- target
  example$component_fleets <- list(female, male)
  example
}

test_that("pooled observations are combined before one likelihood per dataset", {
  on.exit(clear())
  clear()
  example <- make_pooled_observation_example()
  report <- example$obj$report()
  # Fleets are reported in ID order: female, male, observation target.
  #' @description Target predictions sum sex-specific counts and weights, retaining each source's length mapping.
  for (key in c("catch_expected", "index_expected", "catch_numbers_at_age", "catch_weight_at_age",
                "index_numbers_at_age", "index_weight_at_age", "catch_numbers_at_length", "index_numbers_at_length")) {
    expect_equal(report[[key]][[3]], report[[key]][[1]] + report[[key]][[2]])
  }
  age_counts <- matrix(report$catch_numbers_at_age[[3]], ncol = 3, byrow = TRUE)
  length_counts <- matrix(report$catch_numbers_at_length[[3]], ncol = 2, byrow = TRUE)
  age_prob <- age_counts / rowSums(age_counts)
  length_prob <- length_counts / rowSums(length_counts)
  expect_equal(report$agecomp_expected[[3]], as.vector(t(60 * age_prob)))
  expect_equal(report$lengthcomp_expected[[3]], as.vector(t(30 * length_prob)))
  #' @description Each pooled dataset contributes exactly one density component, matching direct R likelihoods.
  expect_length(report$nll_components, 4)
  expected <- -sum(dnorm(rep(1000, 3), report$catch_expected[[3]], 100, log = TRUE)) -
    sum(dnorm(rep(2000, 3), report$index_expected[[3]], 100, log = TRUE))
  for (y in 1:3) expected <- expected - dmultinom(c(10, 20, 30), prob = age_prob[y, ], log = TRUE) -
    dmultinom(c(12, 18), prob = length_prob[y, ], log = TRUE)
  expect_equal(sum(report$nll_components), expected)
  step <- 1e-5
  finite_difference <- (example$obj$fn(example$obj$par + step) - example$obj$fn(example$obj$par - step)) / (2 * step)
  #' @description Derivatives propagate through population dynamics and pooled observation likelihoods.
  expect_equal(as.numeric(example$obj$gr(example$obj$par)), finite_difference, tolerance = 1e-5)
  config <- jsonlite::fromJSON(example$model$get_output(), simplifyVector = FALSE)
  expect_length(config$fleet_observation_links, 1)
  expect_length(config$fleets, 3)
  estimates <- FIMS:::reshape_json_estimates(example$model$get_output())
  expect_s3_class(estimates, "tbl_df")
  expect_equal(sum(estimates$module_name == "Population" & estimates$label == "numbers_at_age"), 24)
})

test_that("survey compositions pool index counts rather than catch counts", {
  on.exit(clear())
  clear()
  example <- make_pooled_observation_example(FALSE)
  report <- example$obj$report()
  #' @description Survey length compositions normalize the sum of mapped component index counts.
  counts <- matrix(report$index_numbers_at_length[[3]], ncol = 2, byrow = TRUE)
  expect_equal(report$lengthcomp_proportion[[3]], as.vector(t(counts / rowSums(counts))))
  counts <- matrix(report$index_numbers_at_age[[3]], ncol = 3, byrow = TRUE)
  expect_equal(report$agecomp_proportion[[3]], as.vector(t(counts / rowSums(counts))))
})


test_that("shared F and q register one owner and preserve automatic derivatives", {
  on.exit(clear())
  clear()
  example <- make_pooled_observation_example(shared = TRUE)
  #' @description Only R0, three owner F values, and one owner q value are registered.
  expect_length(get_fixed(), 5)
  report <- example$obj$report()
  configuration <- jsonlite::fromJSON(example$model$get_output(), simplifyVector = FALSE)
  expect_length(configuration$fleet_parameter_links, 2)
  for (label in c("log_Fmort", "log_q")) {
    extract <- function(fleet) {
      parameters <- Filter(function(x) x$name == label, fleet$parameters)
      unlist(lapply(parameters[[1]]$values, function(x) x$estimated_value))
    }
    expect_length(extract(configuration$fleets[[1]]), if (label == "log_Fmort") 3 else 1)
    expect_equal(extract(configuration$fleets[[1]]), extract(configuration$fleets[[2]]))
  }
  step <- 1e-5
  numerical <- vapply(seq_along(example$obj$par), function(i) {
    increment <- rep(0, length(example$obj$par))
    increment[i] <- step
    (example$obj$fn(example$obj$par + increment) - example$obj$fn(example$obj$par - increment)) / (2 * step)
  }, numeric(1))
  #' @description Derivatives of the pooled likelihood include contributions from both sexes to the same owner parameters.
  expect_equal(as.numeric(example$obj$gr(example$obj$par)), numerical, tolerance = 1e-5)
  expect_true(all(is.finite(unlist(report$catch_expected))))
})

test_that("annual shared catchability uses the owner's complete year vector", {
  on.exit(clear())
  clear()
  example <- make_pooled_observation_example(shared = TRUE, annual_q = TRUE)
  #' @description Three annual q parameters are registered once alongside three annual F values and R0.
  expect_length(get_fixed(), 7)
  report <- example$obj$report()
  # Re-evaluation at the same parameters must not accumulate pooled predictions.
  expect_equal(example$obj$report()$index_expected, report$index_expected)
  expect_true(all(is.finite(as.numeric(example$obj$gr(example$obj$par)))))
})

test_that("shared fleet parameters reject ambiguous ownership", {
  on.exit(clear())
  clear()
  model <- methods::new(CatchAtAge)
  owner <- methods::new(Fleet)
  follower <- methods::new(Fleet)
  third <- methods::new(Fleet)
  #' @description Unknown owners, self-links, duplicate owners, chains, and independently estimated followers are rejected.
  expect_error(model$ShareFishingMortality(99999L, follower$get_id()), "not defined")
  expect_error(model$ShareFishingMortality(owner$get_id(), owner$get_id()), "distinct")
  follower$log_Fmort$set_estimation_types("fixed_effects")
  expect_error(model$ShareFishingMortality(owner$get_id(), follower$get_id()), "estimate only the owner")
  follower$log_Fmort$set_estimation_types("constant")
  model$ShareFishingMortality(owner$get_id(), follower$get_id())
  expect_error(model$ShareFishingMortality(third$get_id(), follower$get_id()), "only one owner")
  expect_error(model$ShareFishingMortality(follower$get_id(), third$get_id()), "Chained or cyclic")
  # Separate parameters may have different owners.
  expect_silent(model$ShareCatchability(follower$get_id(), owner$get_id()))
})
