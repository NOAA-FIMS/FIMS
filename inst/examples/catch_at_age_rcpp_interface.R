# Direct Rcpp CatchAtAge interface example.
#
# This script builds a small CatchAtAge model by creating and linking Rcpp
# module objects directly. It avoids the higher-level R initialization helpers
# so developers can see the object graph that the helper layer normally builds.

library(FIMS)

clear()

set_parameter_values <- function(parameter_vector, values,
                                 estimation_type = "constant") {
  parameter_vector$resize(length(values))
  for (i in seq_along(values)) {
    parameter_vector[i]$value <- values[i]
    parameter_vector[i]$estimation_type$set(estimation_type)
  }
}

set_real_values <- function(real_vector, values) {
  real_vector$resize(length(values))
  for (i in seq_along(values)) {
    real_vector$set(i - 1L, values[i])
  }
}

n_years <- 3L
n_ages <- 4L
ages <- seq_len(n_ages)

landings <- methods::new(Landings, n_years)
set_real_values(landings$landings_data, c(120, 130, 125))

index <- methods::new(Index, n_years)
set_real_values(index$index_data, c(95, 88, 82))

age_comp <- methods::new(AgeComp, n_years, n_ages)
set_real_values(
  age_comp$age_comp_data,
  c(
    10, 25, 35, 30,
    12, 24, 34, 29,
    13, 23, 33, 28
  )
)

recruitment <- methods::new(BevertonHoltRecruitment)
recruitment$n_years$set(n_years)
set_parameter_values(recruitment$logit_steep, 0.25)
set_parameter_values(recruitment$log_rzero, log(1000))
set_parameter_values(recruitment$log_devs, rep(0, n_years - 1L))

growth <- methods::new(EWAAGrowth)
growth$n_years$set(n_years)
set_real_values(growth$ages, ages)
set_real_values(growth$weights, rep(c(0.25, 0.55, 0.9, 1.15), n_years + 1L))

maturity <- methods::new(LogisticMaturity)
set_parameter_values(maturity$inflection_point, 2.5)
set_parameter_values(maturity$slope, 1.1)

selectivity <- methods::new(LogisticSelectivity)
set_parameter_values(selectivity$inflection_point, 2.0, "fixed_effects")
set_parameter_values(selectivity$slope, 1.3, "fixed_effects")

fleet <- methods::new(Fleet)
fleet$SetName("fishery")
fleet$n_years$set(n_years)
fleet$n_ages$set(n_ages)
fleet$n_lengths$set(0L)
set_parameter_values(fleet$log_Fmort, log(c(0.12, 0.14, 0.13)), "fixed_effects")
set_parameter_values(fleet$log_q, log(0.8), "fixed_effects")
fleet$SetSelectivityID(selectivity$get_id())
fleet$SetObservedLandingsDataID(landings$get_id())
fleet$SetObservedIndexDataID(index$get_id())
fleet$SetObservedAgeCompDataID(age_comp$get_id())

landings_distribution <- methods::new(DlnormDistribution)
set_parameter_values(landings_distribution$log_sd, rep(log(0.2), n_years))
landings_distribution$set_observed_data(fleet$GetObservedLandingsDataID())
landings_distribution$set_distribution_links(
  "data",
  fleet$log_landings_expected$get_id()
)

index_distribution <- methods::new(DlnormDistribution)
set_parameter_values(index_distribution$log_sd, rep(log(0.15), n_years))
index_distribution$set_observed_data(fleet$GetObservedIndexDataID())
index_distribution$set_distribution_links(
  "data",
  fleet$log_index_expected$get_id()
)

age_comp_distribution <- methods::new(DmultinomDistribution)
age_comp_distribution$set_observed_data(fleet$GetObservedAgeCompDataID())
age_comp_distribution$set_distribution_links(
  "data",
  fleet$agecomp_proportion$get_id()
)

population <- methods::new(Population)
population$SetName("toy_population")
population$n_years$set(n_years)
population$n_ages$set(n_ages)
population$n_fleets$set(1L)
population$n_lengths$set(0L)
set_real_values(population$ages, ages)
set_parameter_values(population$log_M, rep(log(0.2), n_years * n_ages))
set_parameter_values(population$log_f_multiplier, rep(0, n_years))
set_parameter_values(population$spawning_biomass_ratio, rep(1, n_years + 1L))
set_parameter_values(
  population$log_init_naa,
  log(c(1000, 650, 380, 180)),
  "fixed_effects"
)
population$SetRecruitmentID(recruitment$get_id())
population$SetGrowthID(growth$get_id())
population$SetMaturityID(maturity$get_id())
population$AddFleet(fleet$get_id())

model <- methods::new(CatchAtAge)
model$AddPopulation(population$get_id())
model$UseLikelihoodTerms(TRUE)

CreateTMBModel()

parameters <- list(
  p = get_fixed(),
  re = get_random()
)

likelihood_terms <- data.frame(
  name = model$LikelihoodTermNames(),
  type = model$LikelihoodTermTypes(),
  source_id = model$LikelihoodTermSourceIds(),
  stringsAsFactors = FALSE
)
uses_terms_directly <- model$UsesLikelihoodTerms()
likelihood_term_count <- model$LikelihoodTermCount()

stopifnot(uses_terms_directly)
stopifnot(likelihood_term_count == nrow(likelihood_terms))
stopifnot(any(likelihood_terms$type == "data"))
stopifnot(is.list(parameters))

print(likelihood_terms)

clear()
