# Run with an installed FIMS built from dev-extend-timing-multi-recruiment:
# Rscript inst/examples/recruitment-phases-without-wrappers.R
# Or source this file to inspect the module objects, obj, and report in R.
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

# A single fishing fleet, with annual biomass catch and lognormal error.
catch_data <- methods::new(FIMS::Catch, ny)
catch_data$catch_data[] <- c(500, 550, 600)
fleet <- methods::new(FIMS::Fleet)
fleet$n_years$set(ny)
fleet$n_ages$set(na)
fleet$log_Fmort[] <- rep(log(0.2), ny)
fleet$log_Fmort$set_estimation_types("fixed_effects")
fleet$log_q[1]$value <- 0
fleet$SetSelectivityID(selectivity$get_id())
fleet$SetObservedCatchDataID(catch_data$get_id())
catch_likelihood <- methods::new(FIMS::DlnormDistribution)
catch_likelihood$log_sd[] <- rep(log(0.2), ny)
catch_likelihood$set_observed_data(catch_data$get_id())
catch_likelihood$set_distribution_links("data", fleet$log_catch_expected$get_id())

population <- methods::new(FIMS::Population)
population$n_years$set(ny)
population$n_ages$set(na)
population$n_fleets$set(1L)
population$ages[] <- ages
population$log_M[] <- rep(log(0.2), ny * na)
population$log_init_naa[] <- log(c(1e6, 8e5, 6e5, 4e5, 5e5))
# The first element is the FULL first-year recruitment budget: 400,000 early
# and 600,000 late. Older initial age totals are split internally among phases.
population$SetRecruitmentID(recruitment$get_id())
population$SetGrowthID(growth$get_id())
population$SetMaturityID(maturity$get_id())
population$AddFleet(fleet$get_id())
model <- methods::new(FIMS::CatchAtAge)
model$AddPopulation(population$get_id())

FIMS::CreateTMBModel()
obj <- TMB::MakeADFun(
  data = list(), parameters = list(p = FIMS::get_fixed(), re = FIMS::get_random()),
  DLL = "FIMS", silent = TRUE
)

# Start with a forward evaluation. Change to TRUE to estimate annual fishing
# mortality against the illustrative catches, using nlminb directly.
optimize <- TRUE
parameters <- obj$par
if (optimize) {
  opt <- nlminb(parameters, obj$fn, obj$gr)
  stopifnot(opt$convergence == 0L)
  parameters <- opt$par
}
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
print(report$catch_expected)
cat("\nNegative log likelihood:", nll, "\n")
# Leave objects available for inspection. Call clear() when finished, after
# discarding obj; do not evaluate an old TMB objective after resetting FIMS.
