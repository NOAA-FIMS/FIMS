test_that("TMB map names omit fixed parameters and collapse shared levels", {
  parameter_names <- c("alpha", "beta", "gamma", "delta")
  map <- factor(c(1, 1, NA, 2))

  expect_equal(
    FIMS:::tmb_mapped_parameter_names(parameter_names, map),
    c("alpha", "delta")
  )
})

test_that("TMB map names preserve all names without a map", {
  parameter_names <- c("alpha", "beta")
  expect_equal(
    FIMS:::tmb_mapped_parameter_names(parameter_names),
    parameter_names
  )
})

test_that("TMB map names validate the map", {
  expect_error(
    FIMS:::tmb_mapped_parameter_names(c("alpha", "beta"), c(1, 2)),
    "must be a factor"
  )
  expect_error(
    FIMS:::tmb_mapped_parameter_names(c("alpha", "beta"), factor(1)),
    "one entry per FIMS parameter"
  )
})

test_that("TMB mapped values remain aligned in FIMS output", {
  clear()
  on.exit(clear(), add = TRUE)

  selectivities <- lapply(seq_len(2), function(index) {
    selectivity <- methods::new(LogisticSelectivity)
    selectivity$inflection_point[1]$value <- 10 + index
    selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
    selectivity$slope[1]$value <- 0.1 * index
    selectivity$slope[1]$estimation_type$set("fixed_effects")
    selectivity
  })
  CreateTMBModel()

  initial_parameters <- get_fixed()
  map <- factor(c(1, 1, NA, 2))
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(p = initial_parameters, re = get_random()),
    map = list(p = map),
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  full_names <- names(get_parameter_names(as.list(initial_parameters)))
  names(obj$par) <- FIMS:::tmb_mapped_parameter_names(full_names, map)
  opt <- list(par = unname(obj$par) + c(0.5, 1))
  obj$env$last.par.best <- opt$par
  expanded_parameters <- obj$env$parList(par = obj$env$last.par.best)$p
  FIMS:::restore_tmb_parameters(obj)
  estimates <- FIMS:::reshape_tmb_estimates(
    obj = obj,
    opt = opt,
    parameter_names = names(obj$par)
  )

  expect_length(obj$par, 2L)
  expect_identical(names(obj$par), full_names[c(1, 4)])
  expect_equal(get_fixed(), expanded_parameters)
  expect_equal(
    get_fixed(),
    c(opt$par[1], opt$par[1], initial_parameters[3], opt$par[2])
  )
  expect_equal(estimates$initial, unname(obj$par))
  expect_equal(estimates$estimate, unname(opt$par))

  TMB::FreeADFun(obj)
  rm(obj)
})

for (random_map in list(factor(c(1, NA, 1, 2)), factor(rep(NA, 4)))) {
  test_that(paste("best joint parameters restore", nlevels(random_map), "random levels"), {
    clear()
    on.exit(clear(), add = TRUE)

    selectivities <- lapply(seq_len(4), function(index) {
      selectivity <- methods::new(LogisticSelectivity)
      selectivity$inflection_point[1]$value <- 10 + index
      selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
      selectivity$slope[1]$value <- 0.1 * index
      selectivity$slope[1]$estimation_type$set("random_effects")
      selectivity
    })
    CreateTMBModel()
    initial_fixed <- get_fixed()
    initial_random <- get_random()

    # Cover shared levels, fixed entries, and a fully mapped-off random vector.
    obj <- TMB::MakeADFun(
      data = list(),
      parameters = list(p = initial_fixed, re = initial_random),
      map = list(p = factor(c(1, 1, NA, 2)), re = random_map),
      random = "re",
      DLL = "FIMS",
      silent = TRUE
    )
    # Distinct best and current vectors catch accidental use of last.par.
    best <- obj$env$last.par.best
    best[obj$env$lfixed()] <- c(20, 30)
    best[obj$env$lrandom()] <- seq_len(sum(obj$env$lrandom())) + 0.5
    obj$env$last.par.best <- best
    obj$env$last.par <- best + 10

    expect_no_warning(FIMS:::restore_tmb_parameters(obj))
    expect_equal(get_fixed(), c(20, 20, initial_fixed[3], 30))
    expected_random <- if (all(is.na(random_map))) {
      initial_random
    } else {
      c(1.5, initial_random[2], 1.5, 2.5)
    }
    expect_equal(get_random(), expected_random)

    fit <- FIMSFit(
      input = list(
        parameters = list(p = initial_fixed, re = initial_random),
        map = list(p = factor(c(1, 1, NA, 2)), re = random_map),
        model = list(get_output = function() "{}")
      ),
      obj = obj
    )
    expect_equal(
      unname(fit@number_of_parameters),
      c(2, nlevels(random_map))
    )
    rm(obj, fit)
    gc()
  })
}
