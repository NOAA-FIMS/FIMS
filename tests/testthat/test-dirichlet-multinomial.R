dm_density <- function(counts, probability, concentration, rows = 1L) {
  density <- methods::new(DmultinomDistribution)
  density$set_dirichlet_multinomial()
  density$dims$resize(2)
  density$dims$set(0, rows)
  density$dims$set(1, length(counts) / rows)
  density$observed_values[] <- counts
  density$expected_values[] <- probability
  density$log_concentration[] <- log(concentration)
  density
}

dm_fixture <- function() {
  raw <- subset(data_big, is.na(timing) | timing <= 4 | type == "weight_at_age")
  raw$timing[raw$type == "weight_at_age"] <- NA_integer_
  raw$uncertainty[raw$type == "age_comp"] <- "~ dmultinom(prob = agecomp_proportion, size = 60)"
  data <- FIMSFrame(unique(raw))
  parameters <- setup_default_parameters(data)
  parameters$estimation_type[] <- "constant"
  rec <- parameters$module_name %in% "Recruitment"
  parameters$distribution[rec] <- NA_character_
  parameters$distribution_type[rec] <- NA_character_
  list(data = data, parameters = parameters,
    spec = list(fleet1 = list(age_comp = list(type = "dirichlet_multinomial", concentration = 20))))
}

test_that("DM mass agrees with beta-binomial, normalizes and approaches multinomial", {
  on.exit(clear())
  density <- dm_density(c(2, 6), c(.3, .7), 4)
  expect_equal(density$evaluate(), lchoose(8, 2) + lbeta(3.2, 8.8) - lbeta(1.2, 2.8), tolerance = 1e-12)
  probabilities <- numeric()
  for (a in 0:5) for (b in 0:(5 - a)) {
    density <- dm_density(c(a, b, 5 - a - b), c(.2, .3, .5), 8)
    probabilities <- c(probabilities, exp(density$evaluate()))
  }
  expect_equal(sum(probabilities), 1, tolerance = 1e-12)
  density <- dm_density(c(1, 2, 2), c(.2, .3, .5), 1e7)
  expect_equal(density$evaluate(), dmultinom(c(1, 2, 2), prob = c(.2, .3, .5), log = TRUE), tolerance = 1e-5)
  density <- dm_density(c(2, 3, 0), c(.4, .6, 0), 10)
  expect_equal(density$evaluate(), lchoose(5, 2) + lbeta(6, 9) - lbeta(4, 6))
  density$observed_values[] <- c(1, 3, 1)
  expect_identical(density$evaluate(), -Inf)
  density$observed_values[] <- c(0, 0, 0)
  expect_equal(density$evaluate(), 0)
  density$observed_values[] <- c(-1, 3, 3)
  expect_error(density$evaluate(), "nonnegative")
  density$observed_values[] <- c(1, 2, 2)
  density$expected_values[] <- c(.2, .3, .4)
  expect_error(density$evaluate(), "sum to one")
  density$dims$set(1, -1)
  expect_error(density$evaluate(), "positive finite integers")
})

test_that("DM composition override changes exactly the requested data likelihood", {
  on.exit(clear())
  x <- dm_fixture()
  initialized <- initialize_fims(x$parameters, x$data)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters, random = NULL, DLL = "FIMS", silent = TRUE)
  old <- obj$fn(obj$par)
  snapshot <- initialized$model$evaluate_recruitment_path(numeric(), "log_devs")
  probability <- matrix(snapshot$fleets$fleet1$agecomp_proportion, nrow = 4, byrow = TRUE)
  raw <- get_data(x$data)
  observed <- raw[raw$fleet %in% "fleet1" & raw$type %in% "age_comp", ]
  observed <- observed[order(observed$timing, observed$age), ]
  counts <- matrix(observed$observed * 60, nrow = 4, byrow = TRUE)
  dm <- sum(vapply(1:4, function(i) {
    n <- sum(counts[i, ])
    lgamma(n + 1) - sum(lgamma(counts[i, ] + 1)) + lgamma(20) - lgamma(20 + n) +
      sum(lgamma(20 * probability[i, ] + counts[i, ]) - lgamma(20 * probability[i, ]))
  }, numeric(1)))
  multinomial <- sum(vapply(1:4, function(i)
    lgamma(sum(counts[i, ]) + 1) - sum(lgamma(counts[i, ] + 1)) + sum(counts[i, ] * log(probability[i, ])), numeric(1)))
  initialized <- initialize_fims(x$parameters, x$data, composition_likelihood = x$spec)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters, random = NULL, DLL = "FIMS", silent = TRUE)
  expect_equal(obj$fn(obj$par), old + multinomial - dm, tolerance = 1e-8)
  expect_length(initialized$parameters$p, 0)
  legacy <- attr(initialized, "composition_distributions")$length_comp$fleet1
  expect_error(legacy$set_dirichlet_multinomial(), "before model creation")
})

test_that("DM concentration and biological probability derivatives and reporting work", {
  on.exit(clear())
  x <- dm_fixture()
  f <- x$parameters$module_name %in% "Fleet" & x$parameters$fleet %in% "fleet1" & x$parameters$label %in% "log_Fmort"
  x$parameters$estimation_type[f] <- "fixed_effects"
  x$spec$fleet1$age_comp$estimation_type <- "fixed_effects"
  initialized <- initialize_fims(x$parameters, x$data, composition_likelihood = x$spec)
  parameter_names <- names(get_parameter_names(as.list(initialized$parameters$p)))
  concentration <- grep("dirichlet_multinomial.*log_concentration", parameter_names)
  expect_length(concentration, 1)
  obj <- TMB::MakeADFun(data = list(), parameters = initialized$parameters, random = NULL, DLL = "FIMS", silent = TRUE)
  gradient <- as.numeric(obj$gr(obj$par))
  for (index in seq_along(obj$par)) {
    plus <- minus <- obj$par
    plus[index] <- plus[index] + 1e-5
    minus[index] <- minus[index] - 1e-5
    expect_equal(gradient[index], (obj$fn(plus) - obj$fn(minus)) / 2e-5, tolerance = 1e-4)
  }
  plus <- minus <- obj$par
  plus[concentration] <- plus[concentration] + 1e-4
  minus[concentration] <- minus[concentration] - 1e-4
  expect_equal(obj$he(obj$par)[concentration, concentration],
    (as.numeric(obj$gr(plus))[concentration] - as.numeric(obj$gr(minus))[concentration]) / 2e-4, tolerance = 1e-4)
  fit <- suppressMessages(fit_fims(initialized, optimize = FALSE, get_sd = FALSE))
  estimates <- get_estimates(fit)
  rows <- estimates[estimates$label %in% "log_concentration", ]
  expect_equal(nrow(rows), 1)
  expect_identical(rows$fleet, "fleet1")
  expect_identical(rows$composition_type, "age_comp")
  expect_equal(rows$input, log(20), tolerance = 1e-5)
})

test_that("DM configuration rejects invalid overrides before clearing model state", {
  on.exit(clear())
  x <- dm_fixture()
  initialize_fims(x$parameters, x$data)
  before <- get_fixed()
  bad <- x$spec
  bad$fleet1$age_comp$concentration <- -1
  expect_error(initialize_fims(x$parameters, x$data, composition_likelihood = bad), "positive concentration")
  bad <- x$spec
  bad$fleet1$age_comp$estimation_type <- "random_effects"
  expect_error(initialize_fims(x$parameters, x$data, composition_likelihood = bad), "constant or fixed_effects")
  names(bad) <- "missing_fleet"
  expect_error(initialize_fims(x$parameters, x$data, composition_likelihood = bad), "must exist")
  bad <- x$spec
  bad$fleet1$age_comp$estimation_type <- factor("constant")
  expect_error(initialize_fims(x$parameters, x$data, composition_likelihood = bad), "constant or fixed_effects")
  bad$fleet1$age_comp <- NULL
  expect_error(initialize_fims(x$parameters, x$data, composition_likelihood = bad), "age_comp and/or")
  expect_identical(get_fixed(), before)
})

test_that("DM concentration is recovered from independently sampled compositions", {
  on.exit(clear())
  set.seed(531)
  p <- c(.2, .3, .5)
  counts <- replicate(250, {
    gamma <- rgamma(3, 20 * p)
    as.numeric(rmultinom(1, 50, gamma / sum(gamma)))
  })
  density <- dm_density(as.numeric(counts), rep(p, 250), 10, rows = 250)
  opt <- nlminb(log(10), function(log_kappa) {density$log_concentration[] <- log_kappa; -density$evaluate()},
    lower = log(.1), upper = log(1e5))
  expect_equal(opt$convergence, 0)
  expect_lt(abs(log(exp(opt$par) / 20)), .35)
})
