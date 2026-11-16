# A small assessment retains all ages/bins and shares biological weights across
# years, including the extra terminal report year.
process_simulation_fixture <- function(target = "log_devs") {
  raw <- subset(data_big, is.na(timing) | timing <= 6 | type == "weight_at_age")
  raw$timing[raw$type == "weight_at_age"] <- NA_integer_
  data <- FIMSFrame(unique(raw))
  parameters <- setup_default_parameters(data)
  idx <- parameters$label %in% "log_devs"
  parameters$value[idx] <- c(0.15, -0.1, 0.25, 0.1, -0.2)
  if (target == "log_r") {
    parameters$label[idx] <- "log_r"
    parameters$value[idx] <- parameters$value[parameters$label %in% "log_rzero"][1] +
      parameters$value[idx]
  }
  list(parameters = parameters, data = data)
}

run_process_simulation <- function(fixture, ...) {
  simulate_fims(fixture$parameters, fixture$data,
                 list(type = "ar1", rho = 0.6), ...)
}

test_that("recruitment paths change states and observations use the same paths", {
  on.exit(clear())
  x <- process_simulation_fixture()
  #' @description Identical seeds give identical trajectories, with observation noise isolated from process draws.
  both <- run_process_simulation(x, nsim = 3, seed = 671)
  states <- run_process_simulation(x, nsim = 3, seed = 671, observation_error = FALSE)
  again <- run_process_simulation(x, nsim = 3, seed = 671)
  expect_identical(both, again)
  for (i in 1:3) {
    expect_identical(both$replicates[[i]]$recruitment, states$replicates[[i]]$recruitment)
    expect_identical(both$replicates[[i]]$population, states$replicates[[i]]$population)
    obs <- both$replicates[[i]]$observations
    for (fleet in unique(obs$fleet)) {
      for (type in c("catch", "index")) {
        rows <- obs[obs$fleet == fleet & obs$type == type, ]
        name <- paste0(type, "_expected")
        expect_equal(rows$expected,
                      both$replicates[[i]]$fleets[[fleet]][[name]][rows$timing])
      }
    }
    comps <- subset(obs, type %in% c("age_comp", "length_comp") & status == "simulated")
    sums <- aggregate(simulated ~ fleet + timing + type, comps, sum)
    expect_equal(sums$simulated, rep(1, nrow(sums)))
  }
  expect_false(identical(both$replicates[[1]]$population$numbers_at_age,
                          both$replicates[[2]]$population$numbers_at_age))
  expect_equal(both$configuration$observation_from_year, get_start_year(x$data))
  expect_true(all(both$replicates[[1]]$observations$status == "simulated"))
  expect_false(both$configuration$parameter_uncertainty)
  expect_false(both$configuration$initial_state_uncertainty)
})

test_that("conditional forecast preserves history and propagates AR1 residuals", {
  on.exit(clear())
  x <- process_simulation_fixture()
  #' @description Zero-innovation continuation retains historical states and decays future residuals by rho.
  result <- forecast_fims(x$parameters, x$data, list(type = "ar1", rho = 0.6),
                           from_year = 5, process_mode = "mean",
                           observation_error = FALSE, nsim = 2, seed = 12)
  rec <- result$replicates[[1]]$recruitment
  expect_equal(rec$residual, c(0.15, -0.1, 0.25, 0.15, 0.09))
  expect_identical(result$replicates[[1]], result$replicates[[2]])
  obs <- result$replicates[[1]]$observations
  historical <- obs$timing < 5
  expect_identical(obs$simulated[historical], obs$observed[historical])
  baseline <- run_process_simulation(x, process_mode = "fixed", observation_error = FALSE)
  a <- result$replicates[[1]]$population$numbers_at_age
  b <- baseline$replicates[[1]]$population$numbers_at_age
  n_ages <- length(get_ages(x$data))
  expect_equal(a[seq_len(4 * n_ages)], b[seq_len(4 * n_ages)])
  expect_false(identical(a, b))
})

test_that("direct log recruitment means respond to simulated spawning biomass", {
  on.exit(clear())
  x <- process_simulation_fixture("log_r")
  #' @description Direct log-R states equal dynamic biological means plus residuals, including the changed stock feedback.
  simulated <- run_process_simulation(x, nsim = 2, seed = 52, observation_error = FALSE)
  baseline <- run_process_simulation(x, process_mode = "fixed", observation_error = FALSE)
  for (replicate in simulated$replicates) {
    expect_equal(replicate$recruitment$value,
                  replicate$recruitment$log_mean + replicate$recruitment$residual)
    expect_false(identical(replicate$recruitment$log_mean,
                            baseline$replicates[[1]]$recruitment$log_mean))
  }
})

test_that("native path evaluation restores parameters and deterministic predictions", {
  on.exit(clear())
  x <- process_simulation_fixture("log_r")
  #' @description Evaluating a path and rejecting a malformed path preserve registered target values and baseline predictions.
  initialized <- initialize_fims(x$parameters, x$data,
                                  recruitment_process = list(type = "ar1", rho = 0.6))
  before <- get_random()
  baseline <- initialized$model$evaluate_recruitment_path(numeric(), "log_r")
  changed <- initialized$model$evaluate_recruitment_path(rep(0.5, 5), "log_r")
  expect_false(identical(changed$population, baseline$population))
  expect_identical(get_random(), before)
  expect_identical(initialized$model$evaluate_recruitment_path(numeric(), "log_r"), baseline)
  expect_error(initialized$model$evaluate_recruitment_path(1:2, "log_r"), "cover all")
  expect_error(initialized$model$evaluate_recruitment_path(rep(NA_real_, 5), "log_r"), "finite")
  expect_identical(get_random(), before)
})

test_that("conditional kernel uses innovation variance after a fixed prefix", {
  on.exit(clear())
  #' @description A supplied history is retained and the next simulated residual has conditional mean rho*u and innovation variance.
  density <- methods::new(GaussianProcessDistribution)
  density$set_process("ar1")
  density$observed_values[] <- c(0.4, 99)
  density$expected_values[] <- 0
  density$log_sd[] <- log(0.2)
  density$unconstrained_rho[] <- atanh(0.6)
  set.seed(9)
  samples <- replicate(3000, density$simulate_from(1L, TRUE))
  expect_true(all(samples[1, ] == 0.4))
  expect_equal(mean(samples[2, ]), 0.24, tolerance = 0.015)
  expect_lt(abs(var(samples[2, ]) - 0.04), 0.004)
  expect_equal(density$simulate_from(1L, FALSE), c(0.4, 0.24))
  expect_error(density$simulate_from(3L, TRUE), "out of range")
})

test_that("simulation seed is scoped and invalid configuration fails clearly", {
  on.exit(clear())
  x <- process_simulation_fixture()
  #' @description Explicit seeds restore caller RNG state and invalid horizons or replicate counts are rejected.
  set.seed(472)
  before <- .Random.seed
  run_process_simulation(x, seed = 3, observation_error = FALSE)
  expect_identical(.Random.seed, before)
  expect_error(run_process_simulation(x, nsim = 0), "positive integer")
  expect_error(run_process_simulation(x, from_year = 7), "modeled recruitment")
  expect_error(run_process_simulation(x, seed = NA), "nonnegative integer")
  rm(".Random.seed", envir = .GlobalEnv)
  on.exit(assign(".Random.seed", before, envir = .GlobalEnv), add = TRUE)
  run_process_simulation(x, seed = 3, observation_error = FALSE)
  expect_false(exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE))
})

test_that("sampled trajectories obey cohort survival and the plus-group equation", {
  on.exit(clear())
  x <- process_simulation_fixture()
  #' @description Annual numbers at age obey survival including both plus-group parents under sampled recruitment.
  result <- run_process_simulation(x, seed = 46, observation_error = FALSE)
  population <- result$replicates[[1]]$population
  n <- matrix(population$numbers_at_age, ncol = length(get_ages(x$data)), byrow = TRUE)
  z <- matrix(population$mortality_Z, ncol = ncol(n), byrow = TRUE)
  for (year in 2:nrow(n)) {
    previous <- n[year - 1, ] * exp(-z[year - 1, ])
    expect_equal(n[year, 2:(ncol(n) - 1)], previous[1:(ncol(n) - 2)])
    expect_equal(n[year, ncol(n)], sum(tail(previous, 2)))
  }
  expect_equal(n[2:6, 1], exp(result$replicates[[1]]$recruitment$log_recruitment))
})

test_that("count compositions preserve sample totals and observation-only draws keep states", {
  on.exit(clear())
  x <- process_simulation_fixture()
  raw <- get_data(x$data)
  idx <- raw$type %in% c("age_comp", "length_comp")
  raw$observed[idx] <- round(raw$observed[idx] * 200)
  raw$unit[idx] <- "number"
  x$data <- FIMSFrame(raw)
  #' @description Count draws preserve per-fleet/year/type sample sizes; observation draws vary with fixed latent states.
  result <- run_process_simulation(x, nsim = 2, seed = 721, process_mode = "fixed")
  expect_identical(result$replicates[[1]]$population, result$replicates[[2]]$population)
  expect_false(identical(result$replicates[[1]]$observations$simulated,
                          result$replicates[[2]]$observations$simulated))
  obs <- subset(result$replicates[[1]]$observations,
                 type %in% c("age_comp", "length_comp"))
  totals <- aggregate(cbind(simulated, observed) ~ fleet + timing + type, obs, sum)
  expect_equal(totals$simulated, totals$observed)
})

test_that("absent sampling errors are labeled rather than invented", {
  on.exit(clear())
  x <- process_simulation_fixture()
  raw <- get_data(x$data)
  raw$observed[raw$timing >= 5 & raw$type %in% c("catch", "index", "age_comp", "length_comp")] <- -999
  raw$uncertainty[raw$timing >= 5 & raw$type %in% c("catch", "index", "age_comp", "length_comp")] <- NA_character_
  x$data <- FIMSFrame(raw)
  #' @description Forward predictions are available without inventing future observation SDs or sample sizes.
  result <- run_process_simulation(x, from_year = 5, seed = 2)
  obs <- subset(result$replicates[[1]]$observations, timing >= 5)
  expect_true(all(obs$status == "missing_uncertainty"))
  expect_true(all(is.na(obs$simulated)))
  expect_true(all(is.finite(obs$expected)))
  deterministic <- run_process_simulation(x, from_year = 5, seed = 2,
                                            observation_error = FALSE)
  expected <- subset(deterministic$replicates[[1]]$observations, timing >= 5)
  expect_true(all(expected$status == "expected"))
  expect_true(all(is.finite(expected$simulated)))
})
