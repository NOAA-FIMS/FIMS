public_dm_benchmark <- new.env(parent = asNamespace("FIMS"))
for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R",
  "mode-handoff.R", "dm-validation.R", "dm-profiles.R", "public-dm.R"))
  sys.source(testthat::test_path("..", "integration", "process-recovery", file), public_dm_benchmark)

test_that("public sampler round trips retain bin identities, integer counts and truth", {
  on.exit(clear())
  e <- public_dm_benchmark
  fixture <- e$public_dm_fixture(3L)
  set.seed(81); before <- .Random.seed
  simulation <- e$public_dm_generate(fixture, 91L, 200)
  expect_identical(.Random.seed, before)
  model <- e$public_dm_roundtrip(simulation)
  obs <- simulation$replicates[[1]]$observations
  raw <- get_data(model$data); idx <- raw$type %in% c("catch", "index", "age_comp", "length_comp")
  expect_identical(raw$observed[idx], obs$simulated)
  expect_identical(model$truth$M, simulation$replicates[[1]]$natural_mortality$log_M)
  other <- e$public_dm_generate(fixture, 91L, 2000)
  expect_identical(e$public_dm_roundtrip(other)$truth, model$truth)
  scalar <- obs$type %in% c("catch", "index")
  expect_identical(obs$simulated[scalar], other$replicates[[1]]$observations$simulated[scalar])
  damaged <- simulation
  damaged$replicates[[1]]$observations$age[1] <- -123
  expect_error(e$public_dm_roundtrip(damaged))
})

test_that("estimated-known contrasts use only paired eligible datasets", {
  e <- public_dm_benchmark
  m <- expand.grid(replicate = 1:3, case = c("known_dm", "estimated_dm"), stringsAsFactors = FALSE)
  m$scenario <- "strong"; m$process <- "M"; m$warm <- TRUE
  m$eligible <- c(TRUE, TRUE, FALSE, TRUE, FALSE, TRUE); m$regular <- m$eligible
  m$coverage <- c(.5, .8, NA, .9, NA, .7); m$rmse <- 1-m$coverage
  m$bias <- 0; m$mean_width <- 1
  r <- m; r$optimizer_valid <- r$eligible; r$boundary <- FALSE
  r$status <- ifelse(r$eligible, "consistent_local_report", "report_error")
  p <- m; p$parameter <- "M"; p$truth <- .05; p$value <- log(.05); p$estimate <- .05
  p$covered <- TRUE; p$boundary <- FALSE
  x <- e$public_dm_summaries(list(metrics=m,reports=r,points=p))
  paired <- subset(x$paired_summary, process=="M" & metric=="coverage")
  expect_equal(paired$n, 1)
  expect_equal(paired$mean_difference, .4)
  expect_true(is.na(paired$mcse))
  expect_true(all(x$fit_summary$planned==3))
})

test_that("all failed public recovery fits keep planned rows", {
  e <- new.env(parent = asNamespace("FIMS"))
  for (file in c("dm-validation.R","public-dm.R"))
    sys.source(testthat::test_path("..","integration","process-recovery",file),e)
  e$public_dm_fixture <- function(...) list(sigma=c(R=.2,F=.15,M=.05))
  e$public_dm_generate <- function(...) list(replicates=list(list(observations=data.frame(type="catch",simulated=1))))
  e$public_dm_roundtrip <- function(...) list(data=data.frame(),truth=list())
  e$get_data <- identity
  e$dm_composition_spec <- function(...) NULL
  e$dm_profile_controls <- function() list(original=list(),tight=list())
  e$dm_fit_snapshot <- function(...) stop("deliberate failure")
  x <- e$run_public_dm_recovery(1,3)
  expect_equal(nrow(x$tables$attempts),16)
  expect_equal(nrow(x$tables$reports),4)
  expect_equal(nrow(x$tables$metrics),12)
  expect_true(all(x$tables$reports$status=="report_error"))
  expect_false(any(x$tables$reports$eligible))
})
