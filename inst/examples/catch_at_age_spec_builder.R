# Clean R CatchAtAge specification builder example.
#
# This example uses the new R specification layer. Users describe the model in
# terms of populations, fleets, observations, and distributions. The builder
# creates and links the lower-level Rcpp objects internally.

library(FIMS)

spec <- catch_at_age(
  population = population(
    name = "stock",
    ages = 1:4,
    years = 2020:2022,
    growth = ewaa_growth(
      ages = 1:4,
      weights = c(0.2, 0.5, 0.9, 1.1)
    ),
    maturity = logistic_maturity(
      a50 = 3,
      slope = 1.2
    ),
    recruitment = beverton_holt(
      log_rzero = fixed_effect(log(1000)),
      steepness = fixed_effect(0.75)
    )
  ),
  fleets = list(
    fleet(
      name = "fishery",
      selectivity = logistic_selectivity(
        a50 = 3,
        slope = 1.1
      ),
      fishing_mortality = fixed_effect(rep(log(0.1), 3))
    )
  ),
  observations = list(
    observe_landings(
      fleet = "fishery",
      data = c(100, 105, 110),
      distribution = lognormal(sd = 0.2)
    )
  )
)

built <- build_fims(spec)

likelihood_terms <- get_likelihood_terms(built$model)

stopifnot(built$model$UsesLikelihoodTerms())
stopifnot(nrow(likelihood_terms) > 0L)
stopifnot(any(likelihood_terms$type == "data"))

print(likelihood_terms)

clear()
