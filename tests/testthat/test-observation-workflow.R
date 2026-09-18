workflow_timing_data <- function() {
  d <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  d$timing <- ifelse(is.na(d$timing), NA_character_, sprintf("%04d", d$timing + 2026L))
  d <- d[!(d$fleet == "survey1" & d$type %in% c("age_comp", "length_comp") & d$timing %in% "2028"), ]
  idx <- which(d$fleet == "survey1" & d$type == "index")
  d$timing[idx] <- c("2027-12-31", "2028-01-01", "2029-12-31")
  extra <- d[idx[2], ]
  extra$timing <- "2028-07-01"
  bins <- d[d$fleet == "survey1" & d$type == "length_comp" & d$timing == "2027", ]
  bins$type <- "length_bin"
  bins$age <- bins$timing <- bins$uncertainty <- NA
  bins$observed <- NA_real_
  dplyr::bind_rows(d, extra, bins)
}

workflow_timing_parameters <- function(data) {
  p <- setup_default_parameters(data)
  p$estimation_type <- "constant"
  p$distribution[p$module_name == "Recruitment"] <- NA_character_
  p$distribution_type[p$module_name == "Recruitment"] <- NA_character_
  p$estimation_type[p$module_name == "Fleet" & p$fleet == "survey1" & p$label == "log_q"] <- "fixed_effects"
  p
}

test_that("dated fitting, uncertainty, diagnostics, and retrospective peels stay aligned", {
  #' @description A one-parameter fit verifies dated sample SEs, missing-data diagnostics, peels, and stable IDs after clear.
  d <- workflow_timing_data()
  f <- FIMSFrame(d)
  p <- workflow_timing_parameters(f)
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(p, f), number_of_loops = 1)
  expect_lt(get_max_gradient(fit), 1e-4)
  estimates <- get_estimates(fit)
  index <- estimates[estimates$fleet == "survey1" & estimates$label == "index_expected", ]
  expect_equal(nrow(index), 4L)
  expect_true(all(is.finite(index$uncertainty) & index$uncertainty > 0))
  q <- estimates[estimates$fleet == "survey1" & estimates$label == "log_q", ]
  # Only log(q) is estimated: the delta-method SE of each index is I * SE(log q).
  expect_equal(unname(index$uncertainty), index$estimated * unname(q$uncertainty), tolerance = 1e-5)
  diagnostic <- generics::augment(fit)
  expect_false(any(diagnostic$.truth == -999))
  expect_equal(diagnostic$.pred[diagnostic$fleet == "survey1" & diagnostic$label == "index_expected"], index$estimated)
  comps <- estimates[estimates$fleet == "survey1" & estimates$label == "agecomp_expected" & estimates$observed != -999, ]
  expect_equal(diagnostic$.pred[diagnostic$fleet == "survey1" & diagnostic$label == "agecomp_expected"], comps$estimated)
  expect_true(all(!is.na(diagnostic$observation_id)))
  expect_equal(diagnostic$observation_id[diagnostic$fleet == "survey1" & diagnostic$label == "index_expected"], index$observation_id)
  metrics <- get_fit_metrics(fit, group_by = c("fleet", "label"))
  expect_true(all(is.finite(metrics$.estimate)))
  before <- get_report(fit)
  clear()
  expect_equal(generics::augment(fit), diagnostic)

  # Peels retain annual catch and the model horizon; removed samples become missing.
  peeled <- run_modified_data_fims(1, d, p)
  remaining <- generics::augment(peeled)
  survey <- remaining[remaining$fleet == "survey1", ]
  expect_true(all(survey$timing <= 2028))
  expect_equal(remaining$observation_id[remaining$label == "catch_expected"], diagnostic$observation_id[diagnostic$label == "catch_expected"])
  expect_true(all(survey$observation_id %in% diagnostic$observation_id))
  expect_equal(length(get_report(peeled)$numbers_at_age[[1]]), length(before$numbers_at_age[[1]]))
  # Public wrapper also retains timing metadata on every peel.
  retro <- run_fims_retrospective(0:1, d, p, n_cores = 1)
  expect_equal(sort(unique(retro$estimates$retrospective_peel)), 0:1)
  retained <- retro$estimates[retro$estimates$retrospective_peel == 1 & retro$estimates$fleet == "survey1" & !is.na(retro$estimates$observed) & retro$estimates$observed != -999, ]
  expect_true(all(retained$timing <= 2028))
  for (bad in list(-1, 0.5, NA_real_, Inf, "1")) {
    expect_error(run_modified_data_fims(bad, d, p), "non-negative integer")
    expect_error(run_fims_retrospective(bad, d, p, n_cores = 1), "non-negative integer")
  }
})

test_that("year boundaries, sparse samples, ordering, and annual projections are invariant", {
  #' @description Shuffling samples or adding a terminal dated survey does not change annual projected states at fixed parameters.
  d <- workflow_timing_data()
  f <- FIMSFrame(d)
  p <- workflow_timing_parameters(f)
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(p, f), optimize = FALSE)
  baseline <- get_report(fit)
  coordinates <- get_observations(f)
  samples <- coordinates[coordinates$fleet == "survey1" & coordinates$type == "index", ]
  expect_equal(samples$year_fraction[1:2], c(364 / 365, 0))
  expect_equal(samples$day[2] - samples$day[1], 1L)
  nages <- get_n_ages(f)
  expected <- sum(baseline$index_weight_at_age[[2]][seq_len(nages)] * exp(-baseline$mortality_Z[[1]][seq_len(nages)] * 364 / 365))
  expect_equal(baseline$index_expected[[2]][1], expected, tolerance = 1e-10)
  expect_equal(baseline$index_expected[[2]][2], baseline$index_weight[[2]][2])
  clear()
  shuffled <- fit_fims(initialize_fims(p, FIMSFrame(d[nrow(d):1, ])), optimize = FALSE)
  expect_equal(get_observations(FIMSFrame(d[nrow(d):1, ])), coordinates)
  expect_equal(get_report(shuffled), baseline)
  clear()

  # Extend the annual horizon with a prescribed missing catch and constant F.
  projected <- d[d$type == "catch" & d$timing == "2029", ]
  projected$timing <- "2030"
  projected$observed <- -999
  weights <- d[d$type == "weight_at_age" & d$timing == "2030", ]
  weights$timing <- "2031"
  extended <- FIMSFrame(dplyr::bind_rows(d, projected, weights))
  ep <- workflow_timing_parameters(extended)
  projection <- fit_fims(initialize_fims(ep, extended), optimize = FALSE)
  annual_projection <- get_report(projection)
  expect_equal(head(annual_projection$numbers_at_age[[1]], length(baseline$numbers_at_age[[1]])), baseline$numbers_at_age[[1]])
  clear()
  future <- d[d$type == "index" & d$fleet == "survey1", ][1, ]
  future$timing <- "2030-07-01"
  future$observed <- -999
  future_data <- FIMSFrame(dplyr::bind_rows(d, projected, weights, future))
  future_fit <- fit_fims(initialize_fims(ep, future_data), optimize = FALSE)
  for (quantity in c("numbers_at_age", "catch_expected", "spawning_biomass")) {
    expect_equal(get_report(future_fit)[[quantity]], annual_projection[[quantity]])
  }
  expect_false(any(generics::augment(future_fit)$timing == 2030))
})
