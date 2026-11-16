mortality_rw_fixture <- function(initial = "fixed") {
  raw <- subset(data_big, is.na(timing) | timing <= 4 | type == "weight_at_age")
  raw$timing[raw$type == "weight_at_age"] <- NA_integer_
  data <- FIMSFrame(unique(raw))
  parameters <- setup_default_parameters(data)
  rows <- which(parameters$module_name %in% "Population" & parameters$label %in% "log_M")
  ages <- length(get_ages(data))
  parameters$value[rows] <- rep(log(c(0.2, 0.23, 0.19, 0.22)), each = ages) +
    rep(seq(0, 0.1, length.out = ages), 4)
  parameters$estimation_type[rows] <- "random_effects"
  if (initial == "fixed") parameters$estimation_type[rows[seq_len(ages)]] <- "constant"
  list(data = data, parameters = parameters, rows = rows, ages = ages,
       spec = list(type = "rw1", sd = 0.1, estimation_type = "constant",
         initial = if (initial == "fixed") list(type = "fixed") else
           list(type = "normal", mean = log(0.2), sd = 0.4)))
}

test_that("interleaved processes follow ages through time", {
  on.exit(clear())
  density <- methods::new(GaussianProcessDistribution)
  density$set_random_walk("normal", -1.5, 0.4)
  density$set_series_count(2L)
  density$log_sd[] <- log(0.1)
  density$expected_values[] <- 0
  values <- c(-1.4, -1.8, -1.3, -1.7, -1.5, -1.6)
  density$observed_values[] <- values
  expected <- sum(dnorm(values[1:2], -1.5, 0.4, log = TRUE)) +
    sum(dnorm(values[3:6] - values[1:4], 0, 0.1, log = TRUE))
  expect_equal(density$evaluate(), expected, tolerance = 1e-10)
  density$set_random_walk("fixed", 0, 1)
  expect_equal(density$evaluate(), expected - sum(dnorm(values[1:2], -1.5, 0.4, log = TRUE)))
  expect_equal(density$simulate_from(2L, FALSE), rep(values[1:2], 3))
  set.seed(47)
  draws <- replicate(2500, density$simulate())
  increments <- draws[3:6, ] - draws[1:4, ]
  expect_lt(max(abs(diag(cov(t(increments))) - 0.01)), 0.001)
  expect_lt(abs(cov(increments[1, ], increments[2, ])), 0.001)
  density$set_series_count(4L)
  expect_error(density$evaluate(), "divide target length")
  expect_error(density$set_series_count(0L), "positive")
})

test_that("mortality process adds exactly age-specific annual increment densities", {
  on.exit(clear())
  for (rule in c("fixed", "normal")) {
    x <- mortality_rw_fixture(rule)
    base <- initialize_fims(x$parameters, x$data)
    obj <- TMB::MakeADFun(data = list(), parameters = base$parameters,
      random = NULL, DLL = "FIMS", silent = TRUE)
    baseline <- obj$fn(obj$par)
    initialized <- initialize_fims(x$parameters, x$data, mortality_process = x$spec)
    obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
      random = NULL, DLL = "FIMS", silent = TRUE)
    values <- matrix(x$parameters$value[x$rows], nrow = x$ages)
    expected <- sum(dnorm(values[, -1] - values[, -ncol(values)], 0, 0.1, log = TRUE))
    if (rule == "normal") expected <- expected + sum(dnorm(values[, 1], log(0.2), 0.4, log = TRUE))
    expect_equal(obj$fn(obj$par), baseline - expected, tolerance = 1e-8)
    expect_length(initialized$parameters$re, length(base$parameters$re))
    expect_false(is.null(attr(initialized, "gaussian_mortality")))
  }
})

test_that("mortality innovation SD has valid AD and fitted ownership", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  x$spec$estimation_type <- "fixed_effects"
  initialized <- initialize_fims(x$parameters, x$data, mortality_process = x$spec)
  names <- names(get_parameter_names(as.list(initialized$parameters$p)))
  index <- grep("gaussian_process.*log_sd", names)
  expect_length(index, 1L)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
    random = NULL, DLL = "FIMS", silent = TRUE)
  grid <- matrix(x$parameters$value[x$rows], nrow = x$ages)
  increments <- grid[, -1] - grid[, -ncol(grid)]
  sum_squares <- sum(increments^2)
  expect_equal(as.numeric(obj$gr(obj$par))[index],
    length(increments) - sum_squares / x$spec$sd^2, tolerance = 1e-9)
  expect_equal(obj$he(obj$par)[index, index],
    2 * sum_squares / x$spec$sd^2, tolerance = 1e-9)
  plus <- minus <- obj$par
  plus[index] <- plus[index] + 1e-5
  minus[index] <- minus[index] - 1e-5
  expect_equal(as.numeric(obj$gr(obj$par))[index],
    (obj$fn(plus) - obj$fn(minus)) / 2e-5, tolerance = 1e-5)
  expect_equal(obj$he(obj$par)[index, index],
    (as.numeric(obj$gr(plus))[index] - as.numeric(obj$gr(minus))[index]) / 2e-5,
    tolerance = 1e-5)
  fit <- suppressMessages(fit_fims(initialized, optimize = FALSE, get_sd = FALSE))
  rows <- get_estimates(fit)
  rows <- rows[rows$label %in% "log_sd" & rows$module_name %in% "Population", ]
  expect_equal(nrow(rows), 1L)
  expect_equal(rows$input, log(0.1), tolerance = 1e-5)
})

test_that("mortality validation preserves the active model", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  initialize_fims(x$parameters, x$data, mortality_process = x$spec)
  before <- get_random()
  bad <- x$parameters
  bad$timing[x$rows[1]] <- 99
  expect_error(initialize_fims(bad, x$data, mortality_process = x$spec), "age-fast")
  bad <- x$parameters
  bad$estimation_type[x$rows[2]] <- "random_effects"
  expect_error(initialize_fims(bad, x$data, mortality_process = x$spec), "constant first-year")
  bad <- x$parameters
  bad$distribution[x$rows[1]] <- "Dnorm"
  expect_error(initialize_fims(bad, x$data, mortality_process = x$spec), "only through")
  spec <- x$spec
  spec$sd <- -1
  expect_error(initialize_fims(x$parameters, x$data, mortality_process = spec), "positive")
  expect_identical(get_random(), before)
})

test_that("native fixed initialization protects every age anchor", {
  on.exit(clear())
  clear()
  # A minimal owning vector is sufficient to exercise the generic native guard.
  owner <- methods::new(Fleet)
  owner$n_years$set(6L)
  owner$n_ages$set(1L)
  owner$log_Fmort[] <- rep(-1.5, 6)
  owner$log_Fmort$set_estimation_types(rep("constant", 6))
  owner$log_Fmort[2]$estimation_type$set("random_effects")
  density <- methods::new(GaussianProcessDistribution)
  density$set_random_walk("fixed", 0, 1)
  density$set_series_count(2L)
  density$log_sd[] <- log(0.1)
  density$expected_values[] <- 0
  density$set_distribution_links("random_effects", owner$log_Fmort$get_id())
  expect_error(CreateTMBModel(), "cannot be registered")
})


test_that("age-year M enters cohort survival and both plus-group parents", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  initialized <- initialize_fims(x$parameters, x$data, mortality_process = x$spec)
  snapshot <- initialized$model$evaluate_recruitment_path(numeric(), "log_devs")
  n <- matrix(snapshot$population$numbers_at_age, ncol = x$ages, byrow = TRUE)
  z <- matrix(snapshot$population$mortality_Z, ncol = x$ages, byrow = TRUE)
  expect_equal(snapshot$population$mortality_M, exp(x$parameters$value[x$rows]))
  for (year in seq_len(nrow(n) - 1L)) {
    expect_equal(n[year + 1, 2:(x$ages - 1)],
      n[year, 1:(x$ages - 2)] * exp(-z[year, 1:(x$ages - 2)]))
    expect_equal(n[year + 1, x$ages],
      sum(n[year, (x$ages - 1):x$ages] * exp(-z[year, (x$ages - 1):x$ages])))
  }
  parameters <- x$parameters
  parameters$estimation_type[] <- "constant"
  parameters$estimation_type[x$rows[-seq_len(x$ages)]] <- "random_effects"
  rec <- parameters$module_name %in% "Recruitment"
  parameters$distribution[rec] <- NA_character_
  parameters$distribution_type[rec] <- NA_character_
  initialized <- initialize_fims(parameters, x$data, mortality_process = x$spec)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters,
    random = "re", DLL = "FIMS", silent = TRUE)
  expect_true(is.finite(obj$fn(obj$par)))
})

test_that("mortality simulation is reproducible and drives coherent survival", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  run <- function(noise) simulate_fims(x$parameters, x$data, list(type = "iid"),
    nsim = 2, seed = 648, process_mode = "fixed", observation_error = noise,
    mortality_process = x$spec, mortality_mode = "simulate")
  result <- run(FALSE)
  expect_identical(result, run(FALSE))
  noisy <- run(TRUE)
  expect_false(identical(result$replicates[[1]]$natural_mortality$log_M,
    result$replicates[[2]]$natural_mortality$log_M))
  for (i in 1:2) {
    draw <- result$replicates[[i]]
    expect_identical(draw$natural_mortality, noisy$replicates[[i]]$natural_mortality)
    expect_identical(draw$population, noisy$replicates[[i]]$population)
    expect_equal(draw$population$mortality_M, draw$natural_mortality$M)
    expect_identical(draw$natural_mortality$log_M[seq_len(x$ages)],
      x$parameters$value[x$rows[seq_len(x$ages)]])
    n <- matrix(draw$population$numbers_at_age, ncol = x$ages, byrow = TRUE)
    z <- matrix(draw$population$mortality_Z, ncol = x$ages, byrow = TRUE)
    for (year in seq_len(nrow(n) - 1L)) {
      expect_equal(n[year + 1, 2:(x$ages - 1)],
        n[year, 1:(x$ages - 2)] * exp(-z[year, 1:(x$ages - 2)]))
      expect_equal(n[year + 1, x$ages],
        sum(n[year, (x$ages - 1):x$ages] * exp(-z[year, (x$ages - 1):x$ages])))
    }
  }
  expect_false(result$configuration$mortality_initial_uncertainty)
})

test_that("mortality forecasts retain history and continue each age separately", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  result <- forecast_fims(x$parameters, x$data, list(type = "iid"), from_year = 3,
    seed = 93, process_mode = "fixed", observation_error = FALSE,
    mortality_process = x$spec, mortality_mode = "mean")
  draw <- result$replicates[[1]]
  values <- matrix(x$parameters$value[x$rows], nrow = x$ages)
  expected <- c(values[, 1:2], values[, 2], values[, 2])
  expect_equal(draw$natural_mortality$log_M, expected)
  expect_identical(draw$natural_mortality$conditioned,
    rep(c(TRUE, TRUE, FALSE, FALSE), each = x$ages))
  obs <- draw$observations
  expect_equal(obs$simulated[obs$timing < 3], obs$observed[obs$timing < 3])
  fixed <- simulate_fims(x$parameters, x$data, list(type = "iid"),
    process_mode = "fixed", observation_error = FALSE,
    mortality_process = x$spec, mortality_mode = "fixed")
  expect_equal(fixed$replicates[[1]]$natural_mortality$log_M, c(values))
})

test_that("proper mortality initialization and initial-year forecasts are explicit", {
  on.exit(clear())
  x <- mortality_rw_fixture("normal")
  run <- function(mode) forecast_fims(x$parameters, x$data, list(type = "iid"),
    from_year = 1, seed = 45, process_mode = "fixed", observation_error = FALSE,
    mortality_process = x$spec, mortality_mode = mode)
  drawn <- run("simulate")
  expect_true(drawn$configuration$mortality_initial_uncertainty)
  expect_false(any(drawn$replicates[[1]]$natural_mortality$conditioned))
  expect_false(identical(drawn$replicates[[1]]$natural_mortality$log_M[seq_len(x$ages)],
    x$parameters$value[x$rows[seq_len(x$ages)]]))
  mean <- run("mean")
  expect_equal(mean$replicates[[1]]$natural_mortality$log_M,
    rep(x$spec$initial$mean, length(x$rows)))
})

test_that("native mortality paths restore targets and reject invalid batches atomically", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  initialized <- initialize_fims(x$parameters, x$data, mortality_process = x$spec)
  before <- get_random()
  baseline <- initialized$model$evaluate_recruitment_path(numeric(), "log_devs")
  path <- x$parameters$value[x$rows] + log(2)
  changed <- initialized$model$evaluate_all_process_paths(numeric(), "log_devs", list(), path)
  expect_equal(changed$population$mortality_M, exp(path))
  expect_lt(changed$population$numbers_at_age[x$ages + 2],
    baseline$population$numbers_at_age[x$ages + 2])
  expect_identical(get_random(), before)
  expect_identical(initialized$model$evaluate_recruitment_path(numeric(), "log_devs"), baseline)
  evaluate <- function(m) initialized$model$evaluate_all_process_paths(numeric(), "log_devs",
    list(fleet1 = rep(log(0.3), 4)), m)
  expect_error(evaluate(path[-1]), "full age-year grid")
  path[2] <- NA_real_
  expect_error(evaluate(path), "finite")
  path[2] <- 1000
  expect_error(evaluate(path), "finite")
  expect_identical(get_random(), before)
  expect_identical(initialized$model$evaluate_recruitment_path(numeric(), "log_devs"), baseline)
})

test_that("joint mortality and fishing draws preserve fixed direct recruitment and RNG", {
  on.exit(clear())
  x <- mortality_rw_fixture()
  rec <- x$parameters$label %in% "log_devs"
  x$parameters$label[rec] <- "log_r"
  x$parameters$value[rec] <- x$parameters$value[x$parameters$label %in% "log_rzero"][1]
  f <- x$parameters$module_name %in% "Fleet" & x$parameters$fleet %in% "fleet1" &
    x$parameters$label %in% "log_Fmort"
  x$parameters$estimation_type[f] <- "random_effects"
  fishing <- list(fleet1 = list(type = "rw1", sd = 0.15, estimation_type = "constant",
    initial = list(type = "normal", mean = log(0.2), sd = 0.3)))
  set.seed(512)
  before <- .Random.seed
  result <- simulate_fims(x$parameters, x$data, list(type = "iid"), seed = 902,
    nsim = 2, process_mode = "fixed", observation_error = FALSE,
    fishing_process = fishing, fishing_mode = "simulate",
    mortality_process = x$spec, mortality_mode = "simulate")
  expect_identical(.Random.seed, before)
  for (draw in result$replicates) {
    expect_identical(draw$recruitment$value, x$parameters$value[rec])
    expect_equal(draw$recruitment$residual, draw$recruitment$value - draw$recruitment$log_mean)
    expect_equal(draw$population$mortality_M, draw$natural_mortality$M)
  }
  expect_false(identical(result$replicates[[1]]$fishing_mortality,
    result$replicates[[2]]$fishing_mortality))
  expect_false(identical(result$replicates[[1]]$recruitment$log_mean,
    result$replicates[[2]]$recruitment$log_mean))
})

test_that("failed mortality simulation restores RNG and leaves native targets unchanged", {
  on.exit(clear())
  x <- mortality_rw_fixture("normal")
  initialize_fims(x$parameters, x$data, mortality_process = x$spec)
  before <- get_random()
  set.seed(246)
  rng <- .Random.seed
  x$spec$initial$mean <- 1000
  expect_error(simulate_fims(x$parameters, x$data, list(type = "iid"), seed = 84,
    process_mode = "fixed", observation_error = FALSE,
    mortality_process = x$spec, mortality_mode = "mean"), "finite log-M")
  expect_identical(.Random.seed, rng)
  expect_identical(get_random(), before)
})
