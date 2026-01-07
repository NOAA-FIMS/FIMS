# Quick Reference: Adding a New C++ Module to FIMS

This is a quick reference checklist for adding a new module to FIMS. For detailed instructions, see [the comprehensive guide](../vignettes/adding-new-module.Rmd).

## Checklist

### 1. C++ Implementation

- [ ] Create base class (if new module category): `inst/include/[category]/functors/[category]_base.hpp`
- [ ] Create implementation class: `inst/include/[category]/functors/[name].hpp`
- [ ] Create module header: `inst/include/[category]/[category].hpp`
- [ ] Add Doxygen documentation to all classes and methods

### 2. Rcpp Interface

- [ ] Create Rcpp interface: `inst/include/interface/rcpp/rcpp_objects/rcpp_[category].hpp`
- [ ] Implement base interface class with ID management
- [ ] Implement specific interface class with:
  - [ ] Constructor that registers with `live_objects`
  - [ ] `evaluate()` method for R testing
  - [ ] `add_to_fims_tmb_internal<Type>()` for TMB integration
  - [ ] `add_to_fims_tmb()` that instantiates all TMB types

### 3. Module Registration (src/)

- [ ] Add include to `src/fims_modules.hpp`
- [ ] Add `Rcpp::class_<>()` definition in `RCPP_MODULE(fims)` block
- [ ] Export fields with `.field()`
- [ ] Export methods with `.method()`
- [ ] Add descriptive strings for all fields and methods

### 4. R Integration

- [ ] Add `@export` to `R/FIMS-package.R` for new class
- [ ] Run `devtools::document()` to update NAMESPACE
- [ ] Add initialization function to `R/initialize_modules.R`
- [ ] Integrate initialization into `initialize_fims()` workflow

### 5. Testing

- [ ] Add C++ Google tests: `tests/gtest/test_[category]_[name].cpp`
- [ ] Add R testthat tests: `tests/testthat/test-rcpp-[category].R`
- [ ] Test basic functionality (construction, parameter setting)
- [ ] Test evaluation methods
- [ ] Test ID generation and management
- [ ] Run `setup_and_run_gtest()` (C++ tests)
- [ ] Run `devtools::test()` (R tests)

### 6. Documentation

- [ ] Add roxygen2 comments to R functions
- [ ] Add Doxygen comments to C++ code
- [ ] Update or create vignettes if adding major functionality
- [ ] Update `NEWS.md` to document the new feature

### 7. Code Quality

- [ ] Run `styler::style_pkg()` for R code
- [ ] Run clang-format for C++ code
- [ ] Run spell check: `spelling::spell_check_package()`
- [ ] Run `devtools::check()` and fix any issues
- [ ] Verify code coverage with `covr::report()`

## Naming Conventions Quick Reference

### C++ (Google Style Guide)
- **Classes**: `PascalCase` (e.g., `LogisticSelectivity`)
- **Methods**: `PascalCase` (e.g., `GetId`, `SetParameter`)
- **Variables**: `snake_case` (e.g., `inflection_point`)
- **Namespaces**: `snake_case` (e.g., `fims_popdy`)
- **Files**: `snake_case.hpp` (e.g., `logistic.hpp`)
- **Header guards**: `FIMS_[PATH]_[FILE]_HPP`

### R (tidyverse Style Guide)
- **Functions**: `snake_case` (e.g., `initialize_selectivity`)
- **Classes**: `PascalCase` (e.g., `LogisticSelectivity`)
- **Variables**: `snake_case` (e.g., `fleet_name`)

### Module-Specific
- **Base classes**: Suffix with `Base` (e.g., `SelectivityBase`)
- **Interface classes**: Suffix with `Interface` (e.g., `LogisticSelectivityInterface`)

## File Structure Reference

```
inst/include/
  [category]/                  # e.g., population_dynamics
    functors/
      [category]_base.hpp      # Base class (if new category)
      [name].hpp               # Specific implementation
    [category].hpp             # Umbrella header
  interface/rcpp/rcpp_objects/
    rcpp_[category].hpp        # Rcpp interface

src/
  fims_modules.hpp             # Rcpp module registration
  
R/
  initialize_modules.R         # R initialization functions
  FIMS-package.R              # Package documentation and exports

tests/
  gtest/
    test_[category]_[name].cpp  # C++ unit tests
  testthat/
    test-rcpp-[category].R      # R unit tests
```

## Common Patterns

### Parameter Declaration (C++)
```cpp
fims::Vector<Type> my_parameter;  // Use Vector for time-varying support
```

### ID Management (Interface)
```cpp
// In class definition
static uint32_t id_g;  // Global counter
uint32_t id;           // Instance ID

// In constructor
this->id = MyInterface::id_g++;
```

### Parameter Registration (TMB)
```cpp
// Fixed effects
if (this->param[i].estimation_type_m.get() == "fixed_effects") {
  info->RegisterParameter(module->param[i]);
  info->RegisterParameterName(param_name);
}

// Random effects
if (this->param[i].estimation_type_m.get() == "random_effects") {
  info->RegisterRandomEffect(module->param[i]);
  info->RegisterRandomEffectName(param_name);
}
```

## Need Help?

- Full guide: [vignettes/adding-new-module.Rmd](vignettes/adding-new-module.Rmd)
- Example modules:
  - Selectivity: `inst/include/population_dynamics/selectivity/`
  - Maturity: `inst/include/population_dynamics/maturity/`
  - Recruitment: `inst/include/population_dynamics/recruitment/`
- FIMS Discussions: https://github.com/orgs/NOAA-FIMS/discussions
