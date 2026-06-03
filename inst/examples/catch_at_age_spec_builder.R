# Clean R CatchAtAge specification builder example.
#
# This example uses the new R specification layer. Users describe the model in
# terms of populations, fleets, observations, and distributions. The builder
# creates and links the lower-level Rcpp objects internally.

library(FIMS)

# Step 1: Describe the model in R.
#
# Parameter roles make estimation intent explicit:
# - estimate(...) is estimated as a fixed effect
# - constant(...) is held fixed
# - random(...) is estimated as a random effect with a distribution
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
      a50 = constant(3),
      slope = constant(1.2)
    ),
    recruitment = beverton_holt(
      log_rzero = estimate(log(1000)),
      steepness = constant(0.75),
      deviations = random(normal(sd = 0.4))
    )
  ),
  fleets = list(
    fleet(
      name = "fishery",
      selectivity = logistic_selectivity(
        a50 = estimate(3),
        slope = constant(1.1)
      ),
      fishing_mortality = estimate(rep(log(0.1), 3))
    )
  ),
  observations = list(
    # Observations describe the data and its likelihood distribution.
    observe_landings(
      fleet = "fishery",
      data = c(100, 105, 110),
      distribution = lognormal(sd = 0.2)
    )
  )
)

# Step 2: Initialize the spec.
#
# initialize_fims() builds and links the lower-level FIMS objects internally.
initialized <- initialize_fims(spec)
built <- initialized[["built"]]

# Step 3: Inspect likelihood terms or pass the initialized object to fit_fims().
likelihood_terms <- get_likelihood_terms(built$model)
fit <- fit_fims(initialized, optimize = FALSE)

# Step 4: Check that the spec built the expected likelihood-term path.
stopifnot(built$model$UsesLikelihoodTerms())
stopifnot(nrow(likelihood_terms) > 0L)
stopifnot(any(likelihood_terms$type == "data"))
stopifnot(methods::is(fit, "FIMSFit"))

print(likelihood_terms)

clear()
