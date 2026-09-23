# Run with an installed FIMS built from dev-extend-timing-multi-recruiment:
# Rscript inst/examples/recruitment-phases-without-wrappers.R
# Or source this file to inspect the module objects, obj, and report in R.
# Walkthrough: vignette("fims-recruitment-direct", package = "FIMS").
# This uses exposed Rcpp modules and TMB directly: no FIMSFrame,
# setup_default_parameters(), setup_recruitment_schedule(), initialize_fims(),
# fit_fims(), or get_report(). The data below are illustrative, not a stock fit.
library(FIMS)
# Use explicit namespace references so class lookup does not depend on the
# search path. These are the exposed C++ classes, not model setup wrappers.
FIMS::clear() # Resets the global FIMS registry; use a fresh R session for this example.

years <- 2027:2029
ages <- 1:5
ny <- length(years)
na <- length(ages)

# Illustrative observations: annual fishery totals and April 1 survey samples.
# Biomass uses the same units as growth-derived weight; the survey index is
# relative biomass (q scales it). Composition entries are COUNTS, not proportions.
annual_data <- data.frame(year = years, catch = c(500, 550, 600), log_sd = 0.2)
annual_age_counts <- rbind(
  c(10, 25, 45, 50, 70),
  c(12, 28, 43, 52, 65),
  c(15, 30, 45, 50, 60)
)
survey_data <- data.frame(
  year = years, date = as.Date(paste0(years, "-04-01")),
  index = c(220, 230, 240), log_sd = 0.25
)
survey_age_counts <- rbind(
  c(10, 25, 30, 35, 50),
  c(12, 28, 32, 35, 43),
  c(15, 30, 35, 32, 38)
)
colnames(annual_age_counts) <- colnames(survey_age_counts) <- ages
rownames(annual_age_counts) <- years
rownames(survey_age_counts) <- as.character(survey_data$date)
stopifnot(nrow(annual_age_counts) == ny, nrow(survey_age_counts) == ny,
          all(annual_age_counts >= 0), all(survey_age_counts >= 0))

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
  rep(c(0.4, 0.6), ny), phase
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
fleet$log_Fmort[] <- rep(log(0.2), ny)
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
survey$log_q[1]$value <- log(0.03)
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

# Fit annual fishing mortality and survey catchability using nlminb directly.
# Set FALSE for a forward evaluation at the initial parameter values.
optimize <- TRUE
parameters <- obj$par
max_gc <- 1000
if (optimize) {
  opt <- nlminb(parameters, obj$fn, obj$gr)
  stopifnot(opt$convergence == 0L)
  parameters <- opt$par
  max_gc <- max(abs(obj$gr(opt$par)))
}
max_gc <- max(abs(obj$gr(parameters)))
nll <- obj$fn(parameters)
report <- obj$report(parameters)
stopifnot(is.finite(nll), all(is.finite(obj$gr(parameters))))

# Report coordinates are ONE-based (unlike schedule inputs); columns are unnamed.
events <- as.data.frame(report$recruitment_events)
names(events) <- c(
  "year_i", "phase_i", "year_fraction", "entry_age",
  "recruits", "unfished_recruits"
)
events$year <- c(years, max(years) + 1L)[events$year_i]
events$phase <- c("early", "late")[events$phase_i]
cohorts <- as.data.frame(report$recruitment_cohorts)
names(cohorts) <- c(
  "year_i", "phase_i", "age_i", "biological_age",
  "available_from", "numbers", "unfished_numbers"
)
# The extra terminal year reports January only, using the final year's phase
# pattern and mean recruitment; later terminal-year pulses are not inserted.
# Cohort ages/numbers describe their available_from date, not a shared survey
# date. The plus group uses a representative biological age for each phase.
cat("\nRecruitment events:\n")
print(events)
cat("\nInitial-year cohorts:\n")
print(subset(cohorts, year_i == 1))
cat("\nAnnual catch predictions:\n")
print(data.frame(annual_data, predicted = as.numeric(report$catch_expected[[1]])))
cat("\nApril survey index predictions:\n")
survey_predictions <- data.frame(survey_data,
  predicted = as.numeric(report$index_expected[[2]]))
print(survey_predictions)
annual_age_predictions <- matrix(report$agecomp_proportion[[1]], ny, na, byrow = TRUE)
survey_age_predictions <- matrix(report$agecomp_proportion[[2]], ny, na, byrow = TRUE)
cat("\nApril survey age probabilities (rows are dates):\n")
rownames(survey_age_predictions) <- as.character(survey_data$date)
colnames(survey_age_predictions) <- ages
print(survey_age_predictions)

# Independent R calculations make each likelihood link inspectable. FIMS uses
# log(prediction) as meanlog, with no lognormal mean-bias adjustment.
nll_by_component <- c(
  annual_catch = -sum(dlnorm(annual_data$catch,
    log(report$catch_expected[[1]]), annual_data$log_sd, log = TRUE)),
  annual_age = -sum(vapply(seq_len(ny), function(i)
    dmultinom(annual_age_counts[i, ], prob = annual_age_predictions[i, ], log = TRUE), numeric(1))),
  april_index = -sum(dlnorm(survey_data$index,
    log(survey_predictions$predicted), survey_data$log_sd, log = TRUE)),
  april_age = -sum(vapply(seq_len(ny), function(i)
    dmultinom(survey_age_counts[i, ], prob = survey_age_predictions[i, ], log = TRUE), numeric(1)))
)
stopifnot(isTRUE(all.equal(sum(nll_by_component), nll, tolerance = 1e-7)),
          all(abs(rowSums(survey_age_predictions) - 1) < 1e-8))
cat("\nNegative log likelihood by component:\n")
print(nll_by_component)
cat("\nNegative log likelihood:", nll, "\n")
cat("Maximum gradient component:", max_gc, "\n")
# Leave objects available for inspection. Call clear() when finished, after
# discarding obj; do not evaluate an old TMB objective after resetting FIMS.
