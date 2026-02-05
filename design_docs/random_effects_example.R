# Random Effects Interface Examples
# This file demonstrates how to use the random effects helper functions
# for different types of random effects in FIMS

library(FIMS)
library(dplyr)
library(tidyr)

# Setup: Load data and create base configurations ----
data("data1")
data_4_model <- FIMSFrame(data1)

# Create default configurations and parameters
configs <- create_default_configurations(data = data_4_model)
params_base <- create_default_parameters(configurations = configs, data = data_4_model)

# =============================================================================
# Example 1: Non-parametric Random Effects (Recruitment Deviations with AR1)
# =============================================================================

# This is the most common use case: recruitment deviations with temporal
# correlation structure

params_ex1 <- params_base |>
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,          # Innovation SD
    rho = 0.5,            # AR1 correlation
    estimate_sigma = TRUE,
    estimate_rho = FALSE   # Often fixed based on prior knowledge
  )

# View the random effects
summary_re(params_ex1)

# Validate
validate_random_effects(params_ex1)

# Fit the model
# fit_ex1 <- params_ex1 |>
#   initialize_fims(data = data_4_model) |>
#   fit_fims(optimize = TRUE)


# =============================================================================
# Example 2: Parametric Random Effects (Time-varying Selectivity)
# =============================================================================

# Time-varying selectivity inflection point with random walk structure
# This allows the selectivity curve to shift smoothly over time

params_ex2 <- params_base |>
  add_rw_random_effect(
    module_name = "Selectivity",
    fleet_name = "survey1",
    label = "inflection_point",
    specification_type = "parametric",
    sigma = 0.1,          # Small SD for smooth changes
    order = 1,            # First-order random walk
    estimate_sigma = TRUE
  )

# View summary
summary_re(params_ex2)

# Validate
validate_random_effects(params_ex2)

# Alternative: Use AR1 structure instead of RW
params_ex2_ar1 <- params_base |>
  add_ar1_random_effect(
    module_name = "Selectivity",
    fleet_name = "survey1",
    label = "inflection_point",
    specification_type = "parametric",
    sigma = 0.2,
    rho = 0.7,            # High correlation = smooth changes
    estimate_sigma = TRUE,
    estimate_rho = TRUE
  )


# =============================================================================
# Example 3: Multiple Random Effects
# =============================================================================

# Add random effects to both recruitment and selectivity

params_ex3 <- params_base |>
  # Add AR1 recruitment deviations
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    rho = 0.5,
    estimate_sigma = TRUE,
    estimate_rho = FALSE
  ) |>
  # Add time-varying selectivity inflection point
  add_rw_random_effect(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    label = "inflection_point",
    specification_type = "parametric",
    sigma = 0.15,
    order = 1,
    estimate_sigma = TRUE
  ) |>
  # Add time-varying selectivity slope
  add_rw_random_effect(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    label = "slope",
    specification_type = "parametric",
    sigma = 0.05,
    order = 2,            # Second-order for smoother changes
    estimate_sigma = FALSE
  )

# View summary
summary_re(params_ex3)

# Validate all random effects
validate_random_effects(params_ex3)


# =============================================================================
# Example 4: Semi-parametric Random Effects (Advanced)
# =============================================================================

# Semi-parametric approach: base selectivity function with multiplicative
# random deviations. This is more advanced and requires adding a new
# parameter row for the deviations.

# First, create the base parameters
params_ex4_base <- params_base |>
  tidyr::unnest(cols = data)

# Add a new row for the random deviations
params_ex4_with_deviations <- params_ex4_base |>
  tibble::add_row(
    model_family = "catch_at_age",
    module_name = "Selectivity",
    fleet_name = "fleet1",
    module_type = "Logistic",
    label = "re_deviations",
    time = 1:30,
    value = rep(0, 30),              # Start at 0 (no deviation)
    estimation_type = "random_effects"
  )

# Now add the random effect specification
params_ex4 <- params_ex4_with_deviations |>
  add_random_effect(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    label = "re_deviations",
    specification_type = "semi_parametric",
    structure = "iid",                  # Independent deviations
    operation = "multiplicative",       # Multiply base selectivity
    base_function = "logistic",
    hyperparameters = list(
      log_sigma = list(
        value = log(0.1),
        estimation_type = "fixed_effects"
      )
    )
  )

# Re-nest if needed
params_ex4_nested <- params_ex4 |>
  tidyr::nest(.by = c(model_family, module_name, fleet_name))

# Validate
validate_random_effects(params_ex4_nested)


# =============================================================================
# Example 5: Comparison of Structures (IID vs AR1 vs RW)
# =============================================================================

# IID structure (independent over time)
params_iid <- params_base |>
  add_iid_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    estimate_sigma = TRUE
  )

# AR1 structure (first-order autoregressive)
params_ar1 <- params_base |>
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    rho = 0.6,
    estimate_sigma = TRUE,
    estimate_rho = TRUE
  )

# RW structure (random walk)
params_rw <- params_base |>
  add_rw_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    order = 1,
    estimate_sigma = TRUE
  )

# Compare AICs or other model selection criteria
# fit_iid <- params_iid |> initialize_fims(data = data_4_model) |> fit_fims()
# fit_ar1 <- params_ar1 |> initialize_fims(data = data_4_model) |> fit_fims()
# fit_rw <- params_rw |> initialize_fims(data = data_4_model) |> fit_fims()


# =============================================================================
# Example 6: Modifying Random Effects
# =============================================================================

# Start with a model with random effects
params_ex6 <- params_base |>
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    rho = 0.5,
    estimate_sigma = TRUE,
    estimate_rho = FALSE
  )

# Remove random effects (revert to fixed effects)
params_ex6_no_re <- params_ex6 |>
  remove_random_effect(
    module_name = "Recruitment",
    label = "log_devs"
  )

# Add them back with different settings
params_ex6_modified <- params_ex6_no_re |>
  add_iid_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.3,              # Different sigma
    estimate_sigma = FALSE     # Now fixed
  )


# =============================================================================
# Example 7: Manual Specification Using add_random_effect()
# =============================================================================

# For more control, use the base add_random_effect() function directly

params_ex7 <- params_base |>
  add_random_effect(
    module_name = "Selectivity",
    fleet_name = "survey1",
    label = "inflection_point",
    specification_type = "parametric",
    structure = "AR1",
    hyperparameters = list(
      log_sigma = list(
        value = log(0.15),
        estimation_type = "fixed_effects"
      ),
      rho = list(
        value = 0.8,
        estimation_type = "constant"
      )
    )
  )


# =============================================================================
# Example 8: Inspecting Random Effects Specifications
# =============================================================================

# Create a model with multiple random effects
params_complex <- params_base |>
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    rho = 0.5,
    estimate_sigma = TRUE,
    estimate_rho = TRUE
  ) |>
  add_rw_random_effect(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    label = "inflection_point",
    specification_type = "parametric",
    sigma = 0.15,
    order = 1,
    estimate_sigma = TRUE
  )

# Get summary of all random effects
re_summary <- summary_re(params_complex)
print(re_summary)

# View full details by unnesting and filtering
params_complex |>
  tidyr::unnest(cols = data) |>
  dplyr::filter(estimation_type == "random_effects") |>
  dplyr::select(
    module_name, fleet_name, label,
    re_specification_type, re_structure,
    re_log_sigma, re_log_sigma_est_type,
    re_rho, re_rho_est_type
  ) |>
  print()


# =============================================================================
# Example 9: Sensitivity to Hyperparameter Values
# =============================================================================

# Test different sigma values for recruitment deviations

sigmas <- c(0.2, 0.4, 0.6, 0.8)
fits_sensitivity <- list()

for (i in seq_along(sigmas)) {
  params_sens <- params_base |>
    add_iid_random_effect(
      module_name = "Recruitment",
      label = "log_devs",
      specification_type = "non_parametric",
      sigma = sigmas[i],
      estimate_sigma = FALSE  # Fixed at specified value
    )
  
  # fits_sensitivity[[i]] <- params_sens |>
  #   initialize_fims(data = data_4_model) |>
  #   fit_fims(optimize = TRUE)
}

# Compare negative log-likelihoods or other metrics


# =============================================================================
# Notes on Usage
# =============================================================================

# 1. Validation: Always run validate_random_effects() before fitting
#    to catch specification errors early.

# 2. Estimation types for hyperparameters:
#    - "constant": Fixed at the specified value (not estimated)
#    - "fixed_effects": Estimated as a fixed effect parameter
#    - "random_effects": Estimated as a random effect (rare for hyperparameters)

# 3. Choosing structures:
#    - IID: No temporal correlation (simplest)
#    - AR1: First-order autoregressive (moderate correlation)
#    - RW: Random walk (high correlation, smooth trends)
#    - RW2: Second-order random walk (very smooth trends)

# 4. Specification types:
#    - "parametric": Random effects on existing function parameters
#    - "semi_parametric": Base function + random deviations
#    - "non_parametric": Pure time-varying process (e.g., recruitment devs)

# 5. Performance considerations:
#    - More random effects = longer computation time
#    - AR1 and RW structures are more computationally intensive than IID
#    - Estimating hyperparameters increases computation time

# 6. Model comparison:
#    - Use AIC, BIC, or cross-validation to compare structures
#    - Check convergence for all models
#    - Examine residual patterns to assess adequacy
