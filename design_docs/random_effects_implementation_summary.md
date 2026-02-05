# Random Effects Interface: Implementation Summary and Recommendations

## Executive Summary

This document provides a proposed design for implementing a user-friendly random effects interface in FIMS that:

1. **Follows existing FIMS conventions** using tibble-based workflows
2. **Supports three types of random effects** (parametric, semi-parametric, non-parametric)
3. **Provides helper functions** for common use cases
4. **Enables advanced customization** for power users
5. **Includes validation and error checking** to catch specification errors early

## Key Design Documents

1. **Main Design Document**: `design_docs/random_effects_interface_design.md`
   - Comprehensive specification of the interface
   - Details on parameter tibble structure
   - All helper function specifications
   - Usage examples for each random effect type

2. **Implementation**: `design_docs/random_effects_helpers.R`
   - Core helper functions implemented
   - Ready for integration with FIMS

3. **Examples**: `design_docs/random_effects_examples.R`
   - Practical examples covering all use cases
   - Comparison of different structures
   - Sensitivity analysis examples

## Enhanced Parameter Tibble Structure

### New Columns Added for Random Effects

```r
# Core columns (existing)
estimation_type           # "constant", "fixed_effects", "random_effects"

# New random effects columns
re_specification_type     # "parametric", "semi_parametric", "non_parametric"
re_structure              # "iid", "AR1", "RW", "RW2"
re_operation              # "additive", "multiplicative" (semi-parametric only)
re_base_function          # Base function name (semi-parametric only)

# Hyperparameter columns
re_log_sigma              # Log SD value
re_log_sigma_est_type     # Estimation type for log_sigma
re_rho                    # Correlation parameter (AR1)
re_rho_est_type           # Estimation type for rho
```

**Benefits:**
- All random effects specifications visible in one tibble
- Easy to inspect, modify, and validate
- Consistent with existing FIMS workflow
- Supports programmatic model building

## Helper Functions

### Core Functions

1. **`add_random_effect()`** - Generic function for adding random effects
   - Most flexible, supports all options
   - Use for advanced or custom specifications

2. **`remove_random_effect()`** - Remove random effects from parameters
   - Reverts to fixed effects
   - Useful for model comparison

3. **`validate_random_effects()`** - Validate specifications
   - Checks for completeness and consistency
   - Run before `initialize_fims()`
   - Provides informative error messages

4. **`summary_re()`** - Summarize all random effects in model
   - Quick overview of RE specifications
   - Useful for documentation and checking

### Convenience Functions

1. **`add_iid_random_effect()`** - Add IID random effects
   - Simplest structure
   - Single `sigma` parameter

2. **`add_ar1_random_effect()`** - Add AR1 random effects
   - First-order autoregressive
   - Parameters: `sigma`, `rho`
   - Most common for time series

3. **`add_rw_random_effect()`** - Add random walk random effects
   - First or second order
   - Good for smooth trends
   - Parameter: `sigma`, `order`

**Benefits:**
- Convenience functions cover 90% of use cases
- Simple, memorable function names
- Sensible defaults reduce user burden
- Generic function available for 10% edge cases

## Three Types of Random Effects

### 1. Parametric Random Effects

**Use when:** Function parameters vary over time

**Example:** Time-varying selectivity inflection point
```r
params |>
  add_ar1_random_effect(
    module_name = "Selectivity",
    fleet_name = "survey1",
    label = "inflection_point",
    specification_type = "parametric",
    sigma = 0.2,
    rho = 0.6
  )
```

**Mathematical form:**
- Selectivity: $S(a,t) = \frac{1}{1 + e^{-slope \cdot (a - \mu(t))}}$
- Random effect: $\mu(t) \sim AR1(\rho, \sigma^2)$

### 2. Semi-Parametric Random Effects

**Use when:** Base function plus random deviations

**Example:** Logistic selectivity with multiplicative random effects
```r
params |>
  add_random_effect(
    module_name = "Selectivity",
    fleet_name = "fleet1",
    label = "re_deviations",
    specification_type = "semi_parametric",
    structure = "iid",
    operation = "multiplicative",
    base_function = "logistic"
  )
```

**Mathematical form:**
- Base: $S_{base}(a) = \frac{1}{1 + e^{-slope \cdot (a - \mu)}}$
- Realized: $S(a,t) = S_{base}(a) \times e^{\varepsilon(t)}$
- Random effect: $\varepsilon(t) \sim IID(0, \sigma^2)$

### 3. Non-Parametric Random Effects

**Use when:** Pure time-varying process (most common)

**Example:** Recruitment deviations
```r
params |>
  add_ar1_random_effect(
    module_name = "Recruitment",
    label = "log_r",
    specification_type = "non_parametric",
    sigma = 0.4,
    rho = 0.5
  )
```

**Mathematical form:**
- Recruitment: $R(t) = R_0 \times e^{\varepsilon(t)}$
- Random effect: $\varepsilon(t) = \rho \varepsilon(t-1) + \nu(t)$
- Innovation: $\nu(t) \sim N(0, \sigma^2)$

## Correlation Structures

| Structure | Description | Parameters | Use Case |
|-----------|-------------|------------|----------|
| **IID** | Independent | `sigma` | No temporal correlation expected |
| **AR1** | Autoregressive order 1 | `sigma`, `rho` | Moderate temporal correlation |
| **RW** | Random walk | `sigma` | Smooth trends, high correlation |
| **RW2** | 2nd-order random walk | `sigma` | Very smooth trends |

**Choosing a structure:**
- Start simple (IID)
- Use AR1 if correlation expected
- Use RW for smooth temporal trends
- Compare models using AIC/BIC

## Integration with initialize_fims()

The `initialize_fims()` function needs modifications to:

1. **Detect random effects:** Filter `estimation_type == "random_effects"`
2. **Route by type:** Branch based on `re_specification_type`
3. **Set up hyperparameters:** Register as parameters to estimate
4. **Apply correlation structure:** Use TMB density functions

**Pseudo-code:**
```r
initialize_fims <- function(parameters, data) {
  # Existing validation
  validate_random_effects(parameters)
  
  # Extract random effects
  re_params <- parameters |>
    filter(estimation_type == "random_effects")
  
  # Initialize modules (existing code)
  # ...
  
  # Add random effects setup
  for (each re_param) {
    if (re_specification_type == "parametric") {
      # Set up time-varying parameter
      # Register as random effects
      # Apply correlation structure
    } else if (re_specification_type == "semi_parametric") {
      # Set up base function
      # Add deviation process
      # Apply operation (additive/multiplicative)
    } else if (re_specification_type == "non_parametric") {
      # Set up pure RE process
      # Apply correlation structure
    }
    
    # Register hyperparameters
    # ...
  }
  
  # Existing code
  # ...
}
```

## Implementation Roadmap

### Phase 1: Foundation (2-3 weeks)
- [ ] Extend parameter tibble structure with RE columns
- [ ] Implement `add_random_effect()` base function
- [ ] Implement `validate_random_effects()`
- [ ] Add unit tests for helper functions
- [ ] Update documentation

**Deliverable:** Basic RE infrastructure in place

### Phase 2: Non-Parametric Support (2-3 weeks)
- [ ] Implement IID structure in C++/TMB
- [ ] Implement AR1 structure in C++/TMB
- [ ] Update `initialize_fims()` for non-parametric RE
- [ ] Add integration tests
- [ ] Create examples and vignette

**Deliverable:** Recruitment deviations with IID/AR1 working

### Phase 3: Convenience Functions (1-2 weeks)
- [ ] Implement `add_iid_random_effect()`
- [ ] Implement `add_ar1_random_effect()`
- [ ] Implement `add_rw_random_effect()`
- [ ] Implement `remove_random_effect()`
- [ ] Implement `summary_re()`
- [ ] Add examples for each function

**Deliverable:** User-friendly interface complete

### Phase 4: Additional Structures (2-3 weeks)
- [ ] Implement RW structure in C++/TMB
- [ ] Implement RW2 structure in C++/TMB
- [ ] Add integration tests
- [ ] Create comparison examples

**Deliverable:** Full suite of correlation structures

### Phase 5: Parametric Support (3-4 weeks)
- [ ] Design C++ interface for parametric RE
- [ ] Implement time-varying parameters
- [ ] Update selectivity, mortality, etc. for time variation
- [ ] Add integration tests
- [ ] Create examples and vignette section

**Deliverable:** Time-varying parameters working

### Phase 6: Semi-Parametric Support (3-4 weeks)
- [ ] Design interface for semi-parametric RE
- [ ] Implement additive operations
- [ ] Implement multiplicative operations
- [ ] Add integration tests
- [ ] Create advanced examples

**Deliverable:** Semi-parametric RE working

### Phase 7: Polish and Documentation (2 weeks)
- [ ] Comprehensive vignette: "Random Effects in FIMS"
- [ ] Update all function documentation
- [ ] Create comparison tables
- [ ] Performance benchmarking
- [ ] Create tutorial video/workshop materials

**Deliverable:** Publication-ready feature

**Total estimated time:** 15-21 weeks (~4-5 months)

## Testing Strategy

### Unit Tests
- Each helper function
- Input validation
- Error handling
- Edge cases

### Integration Tests
- Each RE type with each structure
- Multiple REs in one model
- Interaction with fixed effects
- Convergence tests

### Comparison Tests
- Against known solutions (e.g., TMB examples)
- Against other stock assessment models (SS3, ASAP)
- Simulation-estimation studies

### Performance Tests
- Scaling with number of REs
- Different structures (IID vs AR1 vs RW)
- Large models

## Documentation Needs

### Function Documentation
- Roxygen2 for all exported functions
- Examples for each function
- Links between related functions

### Vignette: "Random Effects in FIMS"
Sections:
1. Introduction to random effects
2. Three types of random effects
3. Choosing correlation structures
4. Recruitment deviations (basic example)
5. Time-varying selectivity (parametric example)
6. Semi-parametric models (advanced example)
7. Model comparison and selection
8. Troubleshooting and FAQ

### Reference Material
- Table of correlation structures
- Mathematical notation guide
- Comparison with other software
- Performance considerations

## Advantages of This Design

1. **Consistency:** Follows existing FIMS tibble-based workflow
2. **Clarity:** All specifications explicit and visible
3. **Flexibility:** Supports simple and complex use cases
4. **Safety:** Validation catches errors early
5. **Extensibility:** Easy to add new structures/types
6. **Readability:** Code is self-documenting
7. **Reproducibility:** All settings in one place

## Potential Challenges and Solutions

### Challenge 1: C++ Implementation Complexity
**Solution:** 
- Start with non-parametric RE (already partially implemented)
- Use TMB's built-in density functions (AR1, GMRF)
- Phase implementation starting with simple cases

### Challenge 2: User Confusion About Types
**Solution:**
- Clear documentation with examples
- Convenience functions for common cases
- Validation with helpful error messages
- Visual diagrams and decision trees

### Challenge 3: Performance with Many REs
**Solution:**
- Use sparse precision matrices (TMB)
- Parallel evaluation where possible
- Profile and optimize critical paths
- Provide performance guidelines in docs

### Challenge 4: Model Convergence Issues
**Solution:**
- Good initial values (helper functions)
- Phase estimation (estimate some params first)
- Informative priors on hyperparameters
- Diagnostic tools and troubleshooting guide

## Recommendations

### Immediate Next Steps

1. **Review and refine design** with FIMS team
   - Get feedback on tibble structure
   - Confirm helper function API
   - Prioritize implementation phases

2. **Create prototype** for Phase 1
   - Implement basic tibble structure
   - Create `add_random_effect()` skeleton
   - Test integration with existing code

3. **Pilot implementation** for recruitment deviations
   - Implement IID structure first (simplest)
   - Get one complete example working
   - Use as template for other structures

4. **Iterate based on user feedback**
   - Share prototype with beta users
   - Gather feedback on usability
   - Refine before full implementation

### Long-Term Considerations

- **Spatial random effects:** Extension to spatial correlation
- **Multivariate REs:** Multiple correlated RE processes
- **Hierarchical models:** Stock-specific deviations from meta-population
- **Penalized splines:** Alternative to parametric functions
- **MCMC support:** Full Bayesian inference with REs

## Conclusion

This design provides a comprehensive, user-friendly interface for random effects in FIMS that:
- Maintains consistency with existing FIMS conventions
- Supports the full range of random effects specifications
- Balances simplicity for common cases with flexibility for advanced use
- Includes validation and error checking
- Is extensible for future needs

The phased implementation plan allows for incremental development and testing, ensuring each component works correctly before building on it.

## Questions for Discussion

1. Any columns to add/remove in the tibble structure?
2. Are the helper function names clear and memorable?
3. Should we prioritize any particular random effect type first?
4. What level of C++ expertise is available for implementation?
5. Are there any existing implementations we should leverage/compare to?
6. What timeline is realistic for the team?
7. Who are the target users and what are their skill levels?

## Contact

For questions or suggestions about this design, please contact the FIMS development team.

---
Document created by co-pilot: February 2026
Last updated: February 2026
