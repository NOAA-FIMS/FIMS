dm_simulation_fixture <- function() {
  raw <- subset(data_big, is.na(timing) | timing <= 3 | type == "weight_at_age")
  raw$timing[raw$type == "weight_at_age"] <- NA_integer_
  raw$uncertainty[raw$type == "age_comp"] <- "~ dmultinom(prob = agecomp_proportion, size = 40)"
  raw$uncertainty[raw$type == "length_comp"] <- "~ dmultinom(prob = lengthcomp_proportion, size = 60)"
  data <- FIMSFrame(unique(raw))
  parameters <- suppressMessages(setup_default_parameters(data))
  parameters$estimation_type[parameters$label %in% c("log_M", "log_Fmort")] <- "random_effects"
  list(data = data, parameters = parameters)
}

test_that("DM draws satisfy independent analytical moments and multinomial limit", {
  size <- 40; probability <- c(.2, .3, .5); n <- 16000L
  set.seed(592)
  for (kappa in c(4, 1e8)) {
    draws <- replicate(n, FIMS:::simulation_dm_counts(size, probability, kappa))
    center <- draws - size * probability
    variance <- size * probability * (1 - probability) * (size + kappa) / (1 + kappa)
    expect_true(all(abs(rowMeans(draws) - size * probability) < 6 * sqrt(variance / n)))
    squared <- center^2
    expect_true(all(abs(rowMeans(squared) - variance) < 6 * apply(squared, 1, sd) / sqrt(n)))
    product <- center[1, ] * center[2, ]
    covariance <- -size * probability[1] * probability[2] * (size + kappa) / (1 + kappa)
    expect_lt(abs(mean(product) - covariance), 6 * sd(product) / sqrt(n))
    expect_equal(colSums(draws), rep(size, n))
    expect_true(all(draws >= 0 & draws == floor(draws)))
  }
})

test_that("DM sampling preserves zeros and handles small shapes without pseudocounts", {
  set.seed(33)
  draws <- replicate(100, FIMS:::simulation_dm_counts(30, c(0, .2, .8), 1e-10))
  expect_true(all(draws[1, ] == 0))
  expect_true(all(is.finite(draws)))
  expect_equal(colSums(draws), rep(30, 100))
  expect_equal(FIMS:::simulation_dm_counts(0, c(0, .2, .8), 4), c(0, 0, 0))
  expect_equal(FIMS:::simulation_dm_counts(30, c(0, 1, 0), 4), c(0, 30, 0))
  expect_error(FIMS:::simulation_dm_counts(3.5, c(.5, .5), 4), "integer size")
  expect_error(FIMS:::simulation_dm_counts(3, c(.5, .5), Inf), "positive finite")
  expect_error(FIMS:::simulation_dm_counts(3, c(1e-323, .5, .5), .001), "representable")
})

test_that("DM observation rows respect counts, declared missing designs and conditioning", {
  design <- data.frame(fleet = "A", type = "age_comp", timing = 1L, unit = "number",
    observed = c(2, 3, 5), family = "dirichlet_multinomial", link = "agecomp_proportion",
    position = 1:3, model_bin = TRUE, sd = NA_real_, sample_size = 40, concentration = 4)
  snapshot <- list(fleets = list(A = list(agecomp_proportion = c(.2, .3, .5))))
  draw <- FIMS:::simulation_observations(snapshot, design, 1, TRUE)
  expect_equal(sum(draw$simulated), 10)
  expect_equal(draw$observation_mean, c(2, 3, 5))
  design$observed[1] <- NA_real_
  draw <- FIMS:::simulation_observations(snapshot, design, 1, TRUE)
  expect_equal(sum(draw$simulated), 40)
  expect_equal(draw$observation_mean, c(8, 12, 20))
  history <- FIMS:::simulation_observations(snapshot, design, 2, TRUE)
  expect_identical(history$simulated, design$observed)
  expect_true(all(history$status == "conditioned"))
  design$sample_size[] <- NA_real_
  missing <- FIMS:::simulation_observations(snapshot, design, 1, TRUE)
  expect_true(all(is.na(missing$simulated)))
  expect_true(all(missing$status == "missing_uncertainty"))
  design$unit[] <- "proportion"
  mean <- FIMS:::simulation_observations(snapshot, design, 1, FALSE)
  expect_equal(mean$simulated, c(.2, .3, .5))
})

test_that("DM public simulation preserves process paths, seeded RNG and forecasts", {
  on.exit(clear())
  x <- dm_simulation_fixture()
  spec <- list(fleet1 = list(age_comp = list(type = "dirichlet_multinomial", concentration = 5,
    estimation_type = "fixed_effects"), length_comp = list(type = "dirichlet_multinomial", concentration = 7)))
  walk <- list(type = "rw1", sd = .05, initial = list(type = "normal", mean = log(.2), sd = .1))
  run <- function(composition = NULL, noise = TRUE) simulate_fims(x$parameters, x$data,
    list(type = "iid"), nsim = 2, seed = 721, composition_likelihood = composition,
    observation_error = noise, fishing_process = list(fleet1 = walk), mortality_process = walk)
  initialize_fims(x$parameters, x$data, recruitment_process = list(type = "iid"),
    fishing_process = list(fleet1 = walk), mortality_process = walk, composition_likelihood = spec)
  before_random <- as.numeric(get_random()); before_fixed <- as.numeric(get_fixed())
  set.seed(431); before <- .Random.seed
  dm <- run(spec)
  expect_identical(.Random.seed, before)
  expect_equal(as.numeric(get_random()), before_random)
  expect_equal(as.numeric(get_fixed()), before_fixed)
  expect_identical(dm, run(spec))
  multinomial <- run()
  noiseless <- run(spec, FALSE)
  plain_mean <- run(NULL, FALSE)
  expect_equal(dm$configuration$composition_likelihood$fleet1$age_comp$concentration, 5)
  expect_false(dm$configuration$parameter_uncertainty)
  for (i in 1:2) {
    for (component in c("recruitment", "population", "fishing_mortality", "natural_mortality")) {
      expect_identical(dm$replicates[[i]][[component]], multinomial$replicates[[i]][[component]])
      expect_identical(dm$replicates[[i]][[component]], noiseless$replicates[[i]][[component]])
    }
    expect_equal(noiseless$replicates[[i]]$observations$simulated,
      plain_mean$replicates[[i]]$observations$simulated)
    obs <- dm$replicates[[i]]$observations
    expect_true(all(obs$concentration[obs$fleet == "fleet1" & obs$type == "age_comp"] == 5))
    expect_true(all(obs$concentration[obs$fleet == "fleet1" & obs$type == "length_comp"] == 7))
    expect_true(all(obs$family[obs$fleet == "fleet1" & obs$type == "age_comp"] == "dirichlet_multinomial"))
    expect_true(all(obs$family[obs$fleet == "survey1" & obs$type == "age_comp"] == "dmultinom"))
    comp <- subset(obs, type %in% c("age_comp", "length_comp"))
    expect_equal(aggregate(simulated ~ fleet + type + timing, comp, sum)$simulated, rep(1, 12))
  }
  expect_identical(simulate_fims(x$parameters, x$data, list(type = "iid"), seed = 17),
    simulate_fims(x$parameters, x$data, list(type = "iid"), seed = 17, composition_likelihood = NULL))
  forecast <- forecast_fims(x$parameters, x$data, list(type = "iid"), from_year = 3,
    process_mode = "fixed", seed = 721, composition_likelihood = spec)
  obs <- forecast$replicates[[1]]$observations
  expect_identical(obs$simulated[obs$timing < 3], obs$observed[obs$timing < 3])
  expect_true(all(obs$status[obs$timing == 3] == "simulated"))
})
