information_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R", "information.R"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", file),
    envir = information_benchmark)

test_that("observation designs isolate precision and sample-size changes", {
  design <- information_benchmark$information_design()
  expect_identical(design$design, c("baseline", "survey", "compositions", "both"))
  expect_equal(design$index_sd, c(.2, .05, .2, .05))
  expect_equal(design$composition_size, c(200, 200, 2000, 2000))
  fixture <- information_benchmark$recovery_fixture(4)
  bad <- design
  bad$composition_size[2] <- 200.5
  expect_error(information_benchmark$information_operating_models(fixture, 91, 92, bad))
})

test_that("paired designs preserve truth, catch noise and nested compositions", {
  on.exit(clear())
  fixture <- information_benchmark$recovery_fixture(4)
  set.seed(817)
  rng <- .Random.seed
  paired <- information_benchmark$information_operating_models(fixture, 91, 92)
  expect_identical(.Random.seed, rng)
  data <- lapply(paired$models, function(x) get_data(x$data))
  for (model in paired$models) expect_identical(model$truth, paired$truth)
  catch <- data$baseline$type %in% "catch"
  expect_identical(data$baseline$observed[catch], data$both$observed[catch])
  index <- data$baseline$type %in% "index"
  expected <- paired$predictions$location[paired$predictions$type %in% "index"]
  expect_equal((log(data$baseline$observed[index]) - expected) / .2,
    (log(data$survey$observed[index]) - expected) / .05)
  expect_identical(data$baseline$observed[index], data$compositions$observed[index])
  expect_identical(data$survey$observed[index], data$both$observed[index])
  comp <- data$baseline$type %in% c("age_comp", "length_comp")
  expect_identical(data$baseline$observed[comp], data$survey$observed[comp])
  expect_identical(data$compositions$observed[comp], data$both$observed[comp])
  small <- round(data$baseline$observed[comp] * 200)
  large <- round(data$both$observed[comp] * 2000)
  expect_true(all(large >= small))
  groups <- interaction(data$baseline$fleet[comp], data$baseline$type[comp], data$baseline$timing[comp], drop = TRUE)
  expect_true(all(tapply(small, groups, sum) == 200))
  expect_true(all(tapply(large, groups, sum) == 2000))
  expect_true(all(grepl("sdlog = 0.05", data$survey$uncertainty[index], fixed = TRUE)))
  expect_true(all(grepl("size = 2000", data$both$uncertainty[comp], fixed = TRUE)))
  again <- information_benchmark$information_operating_models(fixture, 91, 92)
  expect_identical(lapply(again$models, function(x) get_data(x$data)), data)
})

test_that("information profiles have a separate reference for every dataset", {
  attempts <- data.frame(years = 18, replicate = 1, seed = 91, initialization = "fixed",
    design = c("baseline", "baseline", "both", "both"), profile_sd = c(NA, .05, NA, .05),
    start_sd = .1, error = NA_character_, objective = c(10, 11, 100, 100.2),
    convergence = 0, max_projected_gradient = 0, max_inner_gradient = 0,
    M_boundary = FALSE, M_sd = .05, M_rmse = .1)
  result <- information_benchmark$information_summaries(attempts)
  expect_equal(result$best$delta_nll[result$best$design == "baseline" & !is.na(result$best$profile_sd)], 1)
  expect_equal(result$best$delta_nll[result$best$design == "both" & !is.na(result$best$profile_sd)], .2)
  expect_true(all(result$summary$valid_fits == 1))
  failed <- attempts[c(1, 3), setdiff(names(attempts), c("M_boundary", "M_sd", "M_rmse"))]
  failed$error <- "failed"
  failed$objective <- NA_real_
  result <- information_benchmark$information_summaries(failed)
  expect_equal(sum(result$summary$planned_datasets), 2)
  expect_equal(sum(result$summary$valid_fits), 0)
  expect_true(all(is.na(result$summary$mean_M_sd)))
})

test_that("information benchmark refits all paired designs", {
  skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true", "Set RUN_SLOW_TESTS=true for information integration.")
  on.exit(clear())
  result <- information_benchmark$run_information_benchmark(years = 4, replicates = 1,
    starts = .1, initializations = "fixed", profile_designs = character())
  expect_equal(nrow(result$tables$attempts), 4)
  expect_true(all(is.na(result$tables$attempts$error)))
  expect_true(all(result$tables$attempts$valid))
  expect_equal(sum(result$tables$summary$planned_datasets), 4)
  expect_length(result$operating_models[[1]]$data, 4)
})
