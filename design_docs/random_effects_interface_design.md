# Random Effects Interface Design for FIMS

## Overview

This document outlines the design for a user-friendly interface to specify and manage random effects in FIMS. The design follows the existing tibble-based workflow and extends it to accommodate the three types of random effects specifications.

## 1. Enhanced Parameter Tibble Structure

### Current Structure
The parameter tibble currently has these key columns:
- `model_family`, `module_name`, `module_type`, `fleet_name`
- `label`, `age`, `length`, `time`
- `value`, `estimation_type`
- `distribution_type`, `distribution`

### Proposed Additional Columns for Random Effects

```r
# Core random effects columns
estimation_type       # "constant", "fixed_effects", "random_effects" (existing)
re_specification_type # "parametric", "semi_parametric", "non_parametric"
re_structure          # "iid", "AR1", "RW", "RW2", "GMRF", etc.
re_operation          # "additive", "multiplicative" (for semi-parametric)
re_base_function      # The base function for parametric/semi-parametric (e.g., "logistic", "beverton_holt")

# Hyperparameters (flexible approach using list-column or separate columns)
re_hyperparameters    # A list-column containing hyperparameter specs
                      # Each element is a list with: name, value, estimation_type, min, max

# Alternative: Separate hyperparameter columns (more explicit)
re_log_sigma          # Log-scale SD for the random effect
re_log_sigma_est_type # Estimation type for log_sigma
re_rho                # Correlation parameter (for AR1, etc.)
re_rho_est_type       # Estimation type for rho
re_tau                # Precision parameter (alternative to sigma)
re_tau_est_type       # Estimation type for tau
```

## 2. Three Types of Random Effects Specifications

### 2.1 Parametric Random Effects

**Definition**: Random effects applied directly to time-varying parameters within a functional form.

**Example**: Time-varying selectivity inflection point in a logistic curve
- The logistic function has parameters: inflection_point(t), slope
- We put random effects on inflection_point over time

**Tibble specification**:
```r
tibble(
  module_name = "Selectivity",
  fleet_name = "survey1",
  module_type = "Logistic",
  label = "inflection_point",
  time = 1:30,
  value = rep(2.0, 30),               # Initial values
  estimation_type = "random_effects",
  re_specification_type = "parametric",
  re_structure = "AR1",               # Temporal correlation structure
  re_log_sigma = log(0.2),           # Hyperparameter: SD
  re_log_sigma_est_type = "fixed_effects",
  re_rho = 0.5,                       # Hyperparameter: AR1 correlation
  re_rho_est_type = "fixed_effects"
)
```

### 2.2 Semi-Parametric Random Effects

**Definition**: A base functional form is used, then modified by adding or multiplying random effects.

**Example**: Logistic selectivity with multiplicative random deviations
- Base selectivity: logistic(age; inflection_point, slope)
- Realized selectivity: logistic(age; inflection_point, slope) × exp(ε(t))
- where ε(t) ~ N(0, σ²) with some temporal structure

**Tibble specification**:
```r
# First, define the base selectivity parameters (fixed or estimated)
tibble(
  module_name = "Selectivity",
  fleet_name = "fleet1",
  module_type = "Logistic",
  label = c("inflection_point", "slope"),
  value = c(3.0, 1.5),
  estimation_type = "fixed_effects"
)

# Then, add a row for the random effect process
tibble(
  module_name = "Selectivity",
  fleet_name = "fleet1",
  module_type = "Logistic",
  label = "re_deviations",           # Special label for the RE process
  time = 1:30,
  value = rep(0, 30),                 # Deviations (typically start at 0)
  estimation_type = "random_effects",
  re_specification_type = "semi_parametric",
  re_base_function = "logistic",      # What function is being modified
  re_operation = "multiplicative",    # How the RE is applied
  re_structure = "iid",
  re_log_sigma = log(0.1),
  re_log_sigma_est_type = "fixed_effects"
)
```

### 2.3 Non-Parametric Random Effects

**Definition**: No functional form; the process itself is time-varying with a specified correlation structure.

**Example**: Recruitment deviations with AR1 structure
- R(t) = R₀ × exp(ε(t))
- where ε(t) follows AR1: ε(t) = ρε(t-1) + ν(t), ν(t) ~ N(0, σ²)

**Tibble specification**:
```r
tibble(
  module_name = "Recruitment",
  label = "log_devs",
  time = 2:30,                        # Recruitment deviations
  value = rep(0, 29),                 # Initial values
  estimation_type = "random_effects",
  re_specification_type = "non_parametric",
  re_structure = "AR1",
  re_log_sigma = log(0.4),
  re_log_sigma_est_type = "fixed_effects",
  re_rho = 0.6,
  re_rho_est_type = "constant"       # Could also be "fixed_effects"
)
```

## 3. Helper Functions

### 3.1 Core Helper Functions

#### `add_random_effect()`
Add random effects to existing parameters.

```r
#' Add Random Effects to Parameters
#'
#' @param parameters A tibble of model parameters
#' @param module_name Character. Name of the module (e.g., "Selectivity", "Recruitment")
#' @param fleet_name Character. Name of fleet (if applicable, otherwise NA)
#' @param label Character. Parameter name (e.g., "inflection_point", "log_devs")
#' @param specification_type Character. One of "parametric", "semi_parametric", "non_parametric"
#' @param structure Character. Correlation structure ("iid", "AR1", "RW", "RW2", etc.)
#' @param hyperparameters List. Named list of hyperparameters with their settings
#' @param operation Character. For semi-parametric only: "additive" or "multiplicative"
#' @param base_function Character. For semi-parametric only: base function name
#' @return Updated parameters tibble with random effects specifications
#' @export
add_random_effect <- function(parameters,
                              module_name,
                              fleet_name = NA_character_,
                              label,
                              specification_type = c("parametric", "semi_parametric", "non_parametric"),
                              structure = c("iid", "AR1", "RW", "RW2"),
                              hyperparameters = list(
                                log_sigma = list(value = 0, estimation_type = "fixed_effects"),
                                rho = list(value = 0, estimation_type = "constant")
                              ),
                              operation = c("multiplicative", "additive"),
                              base_function = NULL) {
  
  specification_type <- rlang::arg_match(specification_type)
  structure <- rlang::arg_match(structure)
  operation <- rlang::arg_match(operation)
  
  # Input validation
  # ... (check that parameters exist, valid combinations, etc.)
  
  # Update the parameters tibble
  parameters <- parameters |>
    dplyr::mutate(
      estimation_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        "random_effects",
        estimation_type
      ),
      re_specification_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        specification_type,
        re_specification_type
      ),
      re_structure = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        structure,
        re_structure
      )
      # ... set other RE-specific columns
    )
  
  return(parameters)
}
```

#### `remove_random_effect()`
Remove random effects from parameters (revert to fixed effects).

```r
#' Remove Random Effects from Parameters
#'
#' @param parameters A tibble of model parameters
#' @param module_name Character. Name of the module
#' @param fleet_name Character. Name of fleet (if applicable)
#' @param label Character. Parameter name
#' @return Updated parameters tibble with random effects removed
#' @export
remove_random_effect <- function(parameters,
                                 module_name,
                                 fleet_name = NA_character_,
                                 label) {
  parameters |>
    dplyr::mutate(
      estimation_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label &
          estimation_type == "random_effects",
        "fixed_effects",
        estimation_type
      ),
      # Clear RE-specific columns
      re_specification_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        NA_character_,
        re_specification_type
      )
      # ... clear other RE columns
    )
}
```

#### `set_re_hyperparameter()`
Set or update random effects hyperparameters.

```r
#' Set Random Effects Hyperparameter
#'
#' @param parameters A tibble of model parameters
#' @param module_name Character. Name of the module
#' @param fleet_name Character. Name of fleet (if applicable)
#' @param label Character. Parameter name
#' @param hyperparameter Character. Hyperparameter name ("log_sigma", "rho", etc.)
#' @param value Numeric. Value for the hyperparameter
#' @param estimation_type Character. "constant", "fixed_effects", or "random_effects"
#' @return Updated parameters tibble
#' @export
set_re_hyperparameter <- function(parameters,
                                   module_name,
                                   fleet_name = NA_character_,
                                   label,
                                   hyperparameter = c("log_sigma", "rho", "tau"),
                                   value,
                                   estimation_type = c("constant", "fixed_effects", "random_effects")) {
  
  hyperparameter <- rlang::arg_match(hyperparameter)
  estimation_type <- rlang::arg_match(estimation_type)
  
  # Create column names
  value_col <- paste0("re_", hyperparameter)
  est_type_col <- paste0("re_", hyperparameter, "_est_type")
  
  # Update using dplyr
  # ... (similar pattern to add_random_effect)
}
```

#### `validate_random_effects()`
Validate that random effects specifications are complete and consistent.

```r
#' Validate Random Effects Specifications
#'
#' @param parameters A tibble of model parameters
#' @return Invisible TRUE if valid, otherwise throws informative errors
#' @export
validate_random_effects <- function(parameters) {
  
  re_params <- parameters |>
    dplyr::filter(estimation_type == "random_effects")
  
  if (nrow(re_params) == 0) return(invisible(TRUE))
  
  # Check 1: All RE parameters have specification_type
  missing_spec <- re_params |>
    dplyr::filter(is.na(re_specification_type))
  
  if (nrow(missing_spec) > 0) {
    cli::cli_abort(c(
      "Random effects parameters must have a specification type.",
      i = "Missing for: {unique(missing_spec$label)}"
    ))
  }
  
  # Check 2: All RE parameters have structure
  # Check 3: Semi-parametric has operation and base_function
  # Check 4: Required hyperparameters are present
  # ... (additional validation logic)
  
  cli::cli_inform(c(
    v = "Random effects specifications validated successfully.",
    i = "Found {nrow(re_params)} random effects parameter(s)."
  ))
  
  invisible(TRUE)
}
```

### 3.2 Convenience Functions

#### `add_iid_random_effect()`
Shortcut for adding IID random effects.

```r
#' Add IID Random Effect
#'
#' @inheritParams add_random_effect
#' @param sigma Numeric. Standard deviation (on natural scale)
#' @return Updated parameters tibble
#' @export
add_iid_random_effect <- function(parameters,
                                  module_name,
                                  fleet_name = NA_character_,
                                  label,
                                  specification_type = "parametric",
                                  sigma = 0.2,
                                  estimate_sigma = TRUE) {
  add_random_effect(
    parameters = parameters,
    module_name = module_name,
    fleet_name = fleet_name,
    label = label,
    specification_type = specification_type,
    structure = "iid",
    hyperparameters = list(
      log_sigma = list(
        value = log(sigma),
        estimation_type = if (estimate_sigma) "fixed_effects" else "constant"
      )
    )
  )
}
```

#### `add_ar1_random_effect()`
Shortcut for adding AR1 random effects.

```r
#' Add AR1 Random Effect
#'
#' @inheritParams add_random_effect
#' @param sigma Numeric. Standard deviation (innovation SD)
#' @param rho Numeric. AR1 correlation parameter
#' @param estimate_rho Logical. Should rho be estimated?
#' @return Updated parameters tibble
#' @export
add_ar1_random_effect <- function(parameters,
                                  module_name,
                                  fleet_name = NA_character_,
                                  label,
                                  specification_type = "non_parametric",
                                  sigma = 0.4,
                                  rho = 0.5,
                                  estimate_sigma = TRUE,
                                  estimate_rho = FALSE) {
  add_random_effect(
    parameters = parameters,
    module_name = module_name,
    fleet_name = fleet_name,
    label = label,
    specification_type = specification_type,
    structure = "AR1",
    hyperparameters = list(
      log_sigma = list(
        value = log(sigma),
        estimation_type = if (estimate_sigma) "fixed_effects" else "constant"
      ),
      rho = list(
        value = rho,
        estimation_type = if (estimate_rho) "fixed_effects" else "constant"
      )
    )
  )
}
```

#### `add_rw_random_effect()`
Shortcut for adding random walk random effects.

```r
#' Add Random Walk Random Effect
#'
#' @inheritParams add_random_effect
#' @param sigma Numeric. Standard deviation of the random walk increments
#' @param order Integer. Order of random walk (1 for RW, 2 for RW2)
#' @return Updated parameters tibble
#' @export
add_rw_random_effect <- function(parameters,
                                 module_name,
                                 fleet_name = NA_character_,
                                 label,
                                 specification_type = "parametric",
                                 sigma = 0.1,
                                 order = 1,
                                 estimate_sigma = TRUE) {
  structure <- if (order == 1) "RW" else "RW2"
  
  add_random_effect(
    parameters = parameters,
    module_name = module_name,
    fleet_name = fleet_name,
    label = label,
    specification_type = specification_type,
    structure = structure,
    hyperparameters = list(
      log_sigma = list(
        value = log(sigma),
        estimation_type = if (estimate_sigma) "fixed_effects" else "constant"
      )
    )
  )
}
```

## 4. Integration with `initialize_fims()`

The `initialize_fims()` function needs to be updated to recognize and process the new random effects columns. Key changes:

1. **Detect RE specifications**: Check for parameters with `estimation_type == "random_effects"`
2. **Route to appropriate C++ interfaces**: Based on `re_specification_type`
3. **Set up hyperparameters**: Register hyperparameters as parameters to estimate
4. **Apply correlation structure**: Use TMB's density functions (e.g., `SCALE(AR1(rho), sigma)`)

```r
# Pseudo-code for initialize_fims() modifications
initialize_fims <- function(parameters, data) {
  # ... existing validation ...
  
  # New: Validate random effects
  validate_random_effects(parameters)
  
  # New: Extract random effects parameters
  re_params <- parameters |>
    dplyr::filter(estimation_type == "random_effects")
  
  # New: Group by specification type
  parametric_re <- re_params |> dplyr::filter(re_specification_type == "parametric")
  semi_parametric_re <- re_params |> dplyr::filter(re_specification_type == "semi_parametric")
  non_parametric_re <- re_params |> dplyr::filter(re_specification_type == "non_parametric")
  
  # ... existing module initialization ...
  
  # New: Set up random effects for each type
  # (Details depend on C++ implementation)
  for (param in parametric_re) {
    # Set parameter values
    # Register as random effects
    # Set up correlation structure (AR1, RW, etc.)
    # Register hyperparameters
  }
  
  # ... similar for semi_parametric_re and non_parametric_re ...
  
  # ... rest of existing code ...
}
```

## 5. Example Workflows

### Example 1: Time-varying selectivity (Parametric)

```r
library(FIMS)

# Load data
data("data1")
data_4_model <- FIMSFrame(data1)

# Create configurations and parameters
configs <- create_default_configurations(data = data_4_model)
params <- create_default_parameters(configurations = configs, data = data_4_model)

# Add parametric random effects to survey selectivity inflection point
params <- params |>
  add_ar1_random_effect(
    module_name = "Selectivity",
    fleet_name = "survey1",
    label = "inflection_point",
    specification_type = "parametric",
    sigma = 0.2,
    rho = 0.6,
    estimate_sigma = TRUE,
    estimate_rho = TRUE
  )

# Fit the model
fit <- params |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
```

### Example 2: Recruitment deviations with AR1 (Non-parametric)

```r
# Create parameters
params <- create_default_parameters(configurations = configs, data = data_4_model)

# Add AR1 structure to recruitment deviations
params <- params |>
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_devs",
    specification_type = "non_parametric",
    sigma = 0.4,
    rho = 0.5,
    estimate_sigma = TRUE,
    estimate_rho = FALSE
  )

# Fit the model
fit <- params |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
```

### Example 3: Semi-parametric selectivity

```r
# Create parameters with fixed base selectivity
params <- create_default_parameters(configurations = configs, data = data_4_model)

# Convert to unnested to add a new row
params_unnested <- params |>
  tidyr::unnest(cols = data)

# Add semi-parametric random deviations
params_with_re <- params_unnested |>
  tibble::add_row(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    module_type = "Logistic",
    label = "re_deviations",
    time = 1:30,
    value = rep(0, 30)
  ) |>
  add_random_effect(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    label = "re_deviations",
    specification_type = "semi_parametric",
    structure = "iid",
    operation = "multiplicative",
    base_function = "logistic",
    hyperparameters = list(
      log_sigma = list(value = log(0.1), estimation_type = "fixed_effects")
    )
  )

# Re-nest if needed
params_nested <- params_with_re |>
  tidyr::nest(.by = c(model_family, module_name, fleet_name))

# Fit the model
fit <- params_nested |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
```

## 6. Benefits of This Design

1. **Consistent with existing workflow**: Uses tibbles and tidy functions
2. **Explicit and clear**: All random effects specifications in one place
3. **Flexible**: Supports three types of random effects
4. **Modular**: Helper functions for common cases, extensible for advanced users
5. **Validatable**: Can check for completeness and consistency before model fitting
6. **Readable**: Parameter table shows full model specification
7. **Reproducible**: All settings stored in documented tibble format

## 7. Future Extensions

- Support for multivariate random effects (e.g., spatially correlated)
- Support for crossed random effects (e.g., age × year)
- Integration with penalized splines
- Custom correlation structures
- Random effects on derived quantities
- Hierarchical random effects (e.g., stock-specific deviation from meta-population)

## 8. Implementation Priorities

### Phase 1 (Core functionality)
1. Extend parameter tibble structure with RE columns
2. Implement `add_random_effect()` and validation
3. Update `initialize_fims()` for basic RE support
4. Support non-parametric RE with IID and AR1 structures

### Phase 2 (Convenience)
1. Implement convenience functions (`add_iid_random_effect()`, etc.)
2. Support for RW and RW2 structures
3. Improved error messages and validation

### Phase 3 (Advanced)
1. Parametric random effects
2. Semi-parametric random effects
3. More complex correlation structures
4. Multivariate random effects

## 9. Documentation Needs

- Vignette: "Working with Random Effects in FIMS"
- Examples for each type of random effect
- Guidance on choosing correlation structures
- Interpretation of hyperparameters
- Comparison with other modeling frameworks (e.g., TMB, ADMB, Stan)
