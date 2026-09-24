# Helpers for vignette("fims-recruitment-recovery", package = "FIMS").
# Direct Rcpp/TMB construction; no FIMS model-setup wrappers are used.
# Source this file, then call run_recruitment_recovery(). Results are returned
# as ordinary R objects. Use a fresh session: each model resets FIMS's registry.

# Keep numerical uncertainty failures separate from optimizer failures.
recovery_uncertainty <- function(par, fn, gr) {
  tryCatch({
    hessian <- optimHess(par, fn, gr)
    if (!all(is.finite(hessian))) stop("Non-finite Hessian")
    covariance <- chol2inv(chol(hessian))
    se <- sqrt(diag(covariance))
    if (any(!is.finite(se)) || any(se <= 0)) stop("Invalid standard errors")
    list(log_se = se, hessian_ok = TRUE, uncertainty_message = "")
  }, error = function(e) list(log_se = rep(NA_real_, length(par)),
    hessian_ok = FALSE, uncertainty_message = conditionMessage(e)))
}

recovery_usable <- function(f) {
  isTRUE(f$convergence == 0L) && is.finite(f$nll) &&
    is.finite(f$gradient) && f$gradient < 1e-3 && isTRUE(f$hessian_ok) &&
    all(is.finite(f$estimate) & f$estimate > 0) &&
    all(is.finite(f$log_se) & f$log_se > 0) &&
    all(is.finite(exp(log(f$estimate) + qnorm(0.975) * f$log_se))) &&
    all(exp(log(f$estimate) - qnorm(0.975) * f$log_se) > 0)
}

# Independent observation likelihood, including fractional multinomial counts.
recovery_nll <- function(observations, prediction) {
  composition <- function(counts, probabilities) {
    stopifnot(all(is.finite(probabilities)), all(probabilities >= 0),
              max(abs(rowSums(probabilities) - 1)) < 1e-8)
    terms <- counts * log(probabilities)
    terms[counts == 0] <- 0
    sum(lgamma(rowSums(counts) + 1) - rowSums(lgamma(counts + 1)) +
          rowSums(terms))
  }
  -(sum(dlnorm(observations$annual$catch, log(prediction$catch),
               observations$annual$log_sd, log = TRUE)) +
    sum(dlnorm(observations$survey$index, log(prediction$index),
               observations$survey$log_sd, log = TRUE)) +
    composition(observations$annual_age, prediction$annual_age) +
    composition(observations$survey_age, prediction$survey_age))
}

recovery_model <- function(observations, start, january_only = FALSE,
                           fit = TRUE) {
  FIMS::clear()
  obj <- NULL
  on.exit({
    if (!is.null(obj)) TMB::FreeADFun(obj)
    FIMS::clear()
  }, add = TRUE)
  years <- 2027:2029
  ages <- 1:5
  ny <- length(years)
  na <- length(ages)

  annual_data <- observations$annual
  annual_age_counts <- observations$annual_age
  survey_data <- observations$survey
  survey_age_counts <- observations$survey_age

  # One annual recruitment total, allocated 40% in January and 60% in July.
  # C++ year coordinates are ZERO-based. Fractions use actual calendar year length
  # (2028 is a leap year); do not approximate July 1 as exactly half a year.
  dates <- as.Date(paste0(rep(years, each = 2), rep(c("-01-01", "-07-01"), ny)))
  year_start <- as.Date(paste0(rep(years, each = 2), "-01-01"))
  year_end <- as.Date(paste0(rep(years + 1L, each = 2), "-01-01"))
  phase <- rep(c("early", "late"), ny)
  year_fraction <- as.numeric(dates - year_start) / as.numeric(year_end - year_start)

  recruitment <- methods::new(FIMS::BevertonHoltRecruitment)
  process <- methods::new(FIMS::LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(process$get_id())
  recruitment$n_years$set(ny)
  recruitment$log_rzero[1]$value <- log(1e6)
  h <- 0.75
  recruitment$logit_steep[1]$value <- log((h - 0.2) / (1 - h))
  recruitment$log_devs[] <- rep(0, ny - 1L)
  recruitment$SetRecruitmentSchedule(
    rep(seq_len(ny) - 1L, each = 2), year_fraction,
    rep(if (january_only) c(1, 0) else c(0.4, 0.6), ny), phase
  )
  recruitment$SetRecruitmentEntryAges(rep(1, length(dates)))
  # Both enter at biological age 1 on their own date. The July cohort is younger
  # than the January cohort when observed together. All parameters default to
  # constant unless explicitly marked below.

  growth <- methods::new(FIMS::VonBertalanffySchnuteGrowth)
  growth$n_ages$set(na)
  growth$mean_length_young[1]$value <- 275
  growth$mean_length_old[1]$value <- 725
  growth$reference_age_for_length_young[1]$value <- min(ages)
  growth$reference_age_for_length_old[1]$value <- max(ages)
  growth$growth_coefficient[1]$value <- 0.18
  growth$length_weight_a[1]$value <- 2.5e-11
  growth$length_weight_b[1]$value <- 3
  growth$length_at_age_sd_at_reference_ages[] <- c(28, 73)

  maturity <- methods::new(FIMS::LogisticMaturity)
  maturity$inflection_point[1]$value <- 3
  maturity$slope[1]$value <- 1
  selectivity <- methods::new(FIMS::LogisticSelectivity)
  selectivity$inflection_point[1]$value <- 2.5
  selectivity$slope[1]$value <- 1

  # Annual fishing fleet: lognormal biomass catch and multinomial age counts.
  catch_data <- methods::new(FIMS::Catch, ny)
  catch_data$catch_data[] <- annual_data$catch
  annual_age_data <- methods::new(FIMS::AgeComp, ny, na)
  # Flatten sample-by-age matrices in row-major order for C++.
  annual_age_data$age_comp_data[] <- c(t(annual_age_counts))
  fleet <- methods::new(FIMS::Fleet)
  fleet$n_years$set(ny)
  fleet$n_ages$set(na)
  fleet$log_Fmort[] <- log(start[1:3])
  fleet$log_Fmort$set_estimation_types("fixed_effects")
  fleet$log_q[1]$value <- 0
  fleet$SetSelectivityID(selectivity$get_id())
  fleet$SetObservedCatchDataID(catch_data$get_id())
  fleet$SetObservedAgeCompDataID(annual_age_data$get_id())
  catch_likelihood <- methods::new(FIMS::DlnormDistribution)
  # log_sd stores log(SD on the log-observation scale), not log(CV).
  catch_likelihood$log_sd[] <- log(annual_data$log_sd)
  catch_likelihood$set_observed_data(catch_data$get_id())
  catch_likelihood$set_distribution_links("data", fleet$log_catch_expected$get_id())

  annual_age_likelihood <- methods::new(FIMS::DmultinomDistribution)
  annual_age_likelihood$set_observed_data(annual_age_data$get_id())
  annual_age_likelihood$set_distribution_links("data", fleet$agecomp_proportion$get_id())

  # April survey: a separate non-extractive fleet with its own selectivity and q.
  # No annual integration for these samples: predictions are evaluated on April 1.
  survey_index_data <- methods::new(FIMS::Index, ny)
  survey_index_data$index_data[] <- survey_data$index
  survey_age_data <- methods::new(FIMS::AgeComp, ny, na)
  survey_age_data$age_comp_data[] <- c(t(survey_age_counts))
  survey_selectivity <- methods::new(FIMS::LogisticSelectivity)
  survey_selectivity$inflection_point[1]$value <- 2
  survey_selectivity$slope[1]$value <- 1
  survey <- methods::new(FIMS::Fleet)
  survey$n_years$set(ny)
  survey$n_ages$set(na)
  survey$log_Fmort[] <- rep(-200, ny) # Fixed, effectively zero survey mortality.
  survey$log_q[1]$value <- log(start[4])
  survey$log_q[1]$estimation_type$set("fixed_effects")
  survey$SetSelectivityID(survey_selectivity$get_id())
  survey$SetObservedIndexDataID(survey_index_data$get_id())
  survey$SetObservedAgeCompDataID(survey_age_data$get_id())

  survey_start <- as.Date(paste0(years, "-01-01"))
  survey_end <- as.Date(paste0(years + 1L, "-01-01"))
  survey_fraction <- as.numeric(survey_data$date - survey_start) /
    as.numeric(survey_end - survey_start)
  # days are absolute days since 1970-01-01, NOT day-of-year. Year and sample
  # coordinates are zero-based. Each stream's rows must match its data row order.
  for (stream in c("index", "age_comp")) {
    survey$SetObservationTiming(
      stream, seq_len(ny) - 1L, as.integer(survey_data$date), seq_len(ny) - 1L,
      paste0("survey1_", stream, "_", survey_data$date),
      as.character(survey_data$date), survey_fraction
    )
  }
  survey_index_likelihood <- methods::new(FIMS::DlnormDistribution)
  survey_index_likelihood$log_sd[] <- log(survey_data$log_sd)
  survey_index_likelihood$set_observed_data(survey_index_data$get_id())
  survey_index_likelihood$set_distribution_links("data", survey$log_index_expected$get_id())
  survey_age_likelihood <- methods::new(FIMS::DmultinomDistribution)
  survey_age_likelihood$set_observed_data(survey_age_data$get_id())
  survey_age_likelihood$set_distribution_links("data", survey$agecomp_proportion$get_id())
  # Recruitment deviations remain fixed at zero, so no recruitment-process
  # likelihood is needed here. Estimating random deviations requires a process
  # distribution as well as registering those deviations as random effects.

  population <- methods::new(FIMS::Population)
  population$n_years$set(ny)
  population$n_ages$set(na)
  population$n_fleets$set(2L)
  population$ages[] <- ages
  population$log_M[] <- rep(log(0.2), ny * na)
  population$log_init_naa[] <- log(c(1e6, 8e5, 6e5, 4e5, 5e5))
  # The first element is the FULL first-year recruitment budget: 400,000 early
  # and 600,000 late. Older initial age totals are split internally among phases.
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fleet$get_id())
  population$AddFleet(survey$get_id())
  model <- methods::new(FIMS::CatchAtAge)
  model$AddPopulation(population$get_id())

  FIMS::CreateTMBModel()
  obj <- TMB::MakeADFun(
    data = list(), parameters = list(p = FIMS::get_fixed(), re = FIMS::get_random()),
    DLL = "FIMS", silent = TRUE
  )

  # Three fishing mortalities followed by one survey catchability are the only
  # estimated parameters. The explicit parameter order is checked here.
  stopifnot(length(obj$par) == 4L,
            isTRUE(all.equal(unname(obj$par), log(unname(start)))))
  par <- obj$par
  opt <- list(convergence = NA_integer_, message = "forward evaluation")
  if (fit) {
    opt <- nlminb(par, obj$fn, obj$gr,
                  control = list(iter.max = 2000, eval.max = 4000))
    for (i in seq_len(3L)) {
      if (opt$convergence == 0L && max(abs(obj$gr(opt$par))) < 1e-4) break
      opt <- nlminb(opt$par, obj$fn, obj$gr,
                    control = list(iter.max = 2000, eval.max = 4000))
    }
    par <- opt$par
  }
  nll <- obj$fn(par)
  gradient <- max(abs(obj$gr(par)))
  report <- obj$report(par)
  uncertainty <- list(log_se = rep(NA_real_, length(par)),
    hessian_ok = FALSE, uncertainty_message = "Uncertainty not evaluated")
  if (fit && is.finite(nll) && all(is.finite(par))) {
    uncertainty <- recovery_uncertainty(par, obj$fn, obj$gr)
  }
  list(estimate = exp(par), log_se = uncertainty$log_se, nll = nll,
       gradient = gradient, convergence = opt$convergence,
       message = if (is.null(opt$message)) "" else opt$message,
       hessian_ok = uncertainty$hessian_ok,
       uncertainty_message = uncertainty$uncertainty_message,
       catch = as.numeric(report$catch_expected[[1]]),
       index = as.numeric(report$index_expected[[2]]),
       annual_age = matrix(report$agecomp_proportion[[1]], ny, na, byrow = TRUE),
       survey_age = matrix(report$agecomp_proportion[[2]], ny, na, byrow = TRUE))
}

# At zero noise, lognormal observations equal medians, and compositions use
# fractional expected counts. FIMS supports these counts; base R dmultinom
# rounds them, so it should not be used to verify this deterministic objective.
recovery_observations <- function(prediction = NULL, noisy = FALSE) {
  x <- list(
    annual = data.frame(year = 2027:2029, catch = rep(1000, 3), log_sd = 0.2),
    survey = data.frame(year = 2027:2029,
      date = as.Date(paste0(2027:2029, "-04-01")),
      index = rep(200, 3), log_sd = 0.25),
    annual_age = matrix(40, 3, 5), survey_age = matrix(30, 3, 5)
  )
  if (is.null(prediction)) return(x)
  if (noisy) {
    x$annual$catch <- rlnorm(3, log(prediction$catch), x$annual$log_sd)
    x$survey$index <- rlnorm(3, log(prediction$index), x$survey$log_sd)
    x$annual_age <- t(vapply(seq_len(3), function(i)
      as.numeric(rmultinom(1, 200, prediction$annual_age[i, ])), numeric(5)))
    x$survey_age <- t(vapply(seq_len(3), function(i)
      as.numeric(rmultinom(1, 150, prediction$survey_age[i, ])), numeric(5)))
  } else {
    x$annual$catch <- prediction$catch
    x$survey$index <- prediction$index
    x$annual_age <- 200 * prediction$annual_age
    x$survey_age <- 150 * prediction$survey_age
  }
  x
}

run_recruitment_recovery <- function(n_replicates = 50L, seed = 20260924L) {
  stopifnot(length(n_replicates) == 1L, is.finite(n_replicates),
            n_replicates >= 2, n_replicates == as.integer(n_replicates))
  had_seed <- exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE)
  if (had_seed) old_seed <- get(".Random.seed", envir = .GlobalEnv)
  on.exit({
    if (had_seed) assign(".Random.seed", old_seed, envir = .GlobalEnv)
    else if (exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE))
      rm(".Random.seed", envir = .GlobalEnv)
  }, add = TRUE)
  set.seed(seed)
  truth <- c(F_2027 = 0.12, F_2028 = 0.20, F_2029 = 0.28, q = 0.03)
  prediction <- recovery_model(recovery_observations(), truth, fit = FALSE)
  exact <- recovery_observations(prediction)
  starts <- rbind(low = truth * 0.5, high = truth * 1.8,
                  mixed = truth * c(1.8, 0.6, 1.5, 0.4))
  exact_fits <- lapply(seq_len(nrow(starts)), function(i)
    recovery_model(exact, starts[i, ]))
  exact_recovery <- do.call(rbind, lapply(seq_along(exact_fits), function(i) {
    f <- exact_fits[[i]]
    data.frame(start = rownames(starts)[i], parameter = names(truth),
      truth = unname(truth), estimate = unname(f$estimate),
      relative_error = unname(f$estimate / truth - 1),
      gradient = f$gradient, convergence = f$convergence)
  }))
  stopifnot(all(exact_recovery$convergence == 0),
            all(exact_recovery$gradient < 1e-3),
            max(abs(exact_recovery$relative_error)) < 1e-3)

  # Same exact observations, all allocation in January. A zero-weight July
  # phase retains the phase evaluator for an apples-to-apples timing comparison.
  january <- recovery_model(exact, starts[1, ], january_only = TRUE)
  timing_comparison <- data.frame(
    model = c("January/July", "January only"),
    nll = c(exact_fits[[1]]$nll, january$nll),
    gradient = c(exact_fits[[1]]$gradient, january$gradient),
    convergence = c(exact_fits[[1]]$convergence, january$convergence),
    usable = c(recovery_usable(exact_fits[[1]]), recovery_usable(january))
  )
  if (!all(timing_comparison$usable))
    stop("Timing comparison failed fit diagnostics; do not interpret its likelihoods")
  # Check likelihood differences away from the optimum as well as at the truth.
  check_fits <- lapply(seq_len(nrow(starts)), function(i)
    recovery_model(exact, starts[i, ], fit = FALSE))
  likelihood_check <- data.frame(start = rownames(starts),
    tmb = vapply(check_fits, function(f) f$nll, numeric(1)),
    independent = vapply(check_fits, function(f) recovery_nll(exact, f), numeric(1)))
  likelihood_check$delta_error <- (likelihood_check$tmb - exact_fits[[1]]$nll) -
    (likelihood_check$independent - recovery_nll(exact, exact_fits[[1]]))
  stopifnot(max(abs(likelihood_check$delta_error)) < 1e-6)
  timing_parameters <- data.frame(parameter = names(truth), truth = unname(truth),
    phased = unname(exact_fits[[1]]$estimate), january_only = unname(january$estimate))

  # Record failures rather than silently dropping inconvenient replicates.
  draws <- list()
  failures <- list()
  for (i in seq_len(n_replicates)) {
    data <- recovery_observations(prediction, noisy = TRUE)
    for (mode in c("phased", "january_only")) {
      f <- tryCatch(recovery_model(data, starts[(i - 1L) %% 3L + 1L, ],
                                   january_only = mode == "january_only"),
                    error = function(e) e)
      if (inherits(f, "error")) {
        failures[[length(failures) + 1L]] <- data.frame(
          replicate = i, model = mode, message = conditionMessage(f))
        next
      }
      usable <- recovery_usable(f)
      lower <- exp(log(f$estimate) - qnorm(0.975) * f$log_se)
      upper <- exp(log(f$estimate) + qnorm(0.975) * f$log_se)
      draws[[length(draws) + 1L]] <- data.frame(
        replicate = i, model = mode, parameter = names(truth),
        truth = unname(truth), estimate = unname(f$estimate),
        lower = unname(lower), upper = unname(upper),
        covered = unname(lower <= truth & truth <= upper),
        convergence = f$convergence, gradient = f$gradient,
        hessian_ok = f$hessian_ok, usable = usable, nll = f$nll,
        message = f$message, uncertainty_message = f$uncertainty_message)
    }
  }
  draws <- if (length(draws)) do.call(rbind, draws) else data.frame(
    replicate = integer(), model = character(), parameter = character(),
    truth = numeric(), estimate = numeric(), lower = numeric(), upper = numeric(),
    covered = logical(), convergence = integer(), gradient = numeric(),
    hessian_ok = logical(), usable = logical(), nll = numeric(),
    message = character(), uncertainty_message = character())
  failures <- if (length(failures)) do.call(rbind, failures) else
    data.frame(replicate = integer(), model = character(), message = character())
  summary <- do.call(rbind, lapply(c("phased", "january_only"), function(mode)
    do.call(rbind, lapply(names(truth), function(parameter) {
      rows <- draws[draws$model == mode & draws$parameter == parameter & draws$usable, ]
      n <- nrow(rows)
      data.frame(model = mode, parameter = parameter,
        attempted = n_replicates, usable = n,
        relative_bias = if (n) mean(rows$estimate / rows$truth - 1) else NA_real_,
        relative_rmse = if (n) sqrt(mean((rows$estimate / rows$truth - 1)^2)) else NA_real_,
        coverage = if (n) mean(rows$covered) else NA_real_,
        coverage_mcse = if (n) sqrt(mean(rows$covered) * (1 - mean(rows$covered)) / n) else NA_real_)
    }))))
  list(seed = seed, n_replicates = n_replicates, session_info = sessionInfo(),
       likelihood_check = likelihood_check,
       truth = truth, exact_recovery = exact_recovery,
       timing_comparison = timing_comparison, timing_parameters = timing_parameters,
       draws = draws, failures = failures, summary = summary)
}
