# TMB Interface

This directory contains the new TMB integration introduced on the `dev-TU-RTMB-Style` branch. These files are not present on the `main` branch and represent a new development path for building and loading TMB-based code in FIMS.

## What changed

- Added `TMB.h` to provide a unified include wrapper for TMB integration.
  - Supports both pre-compilation of the TMB library and user code compilation.
  - Redefines macros like `WITH_LIBTMB`, `TMB_PRECOMPILE`, `CSKIP`, `IF_TMB_PRECOMPILE`, and `TMB_EXTERN` to prevent conflicts between TMB library build and FIMS user code.
  - Includes `<TMB.hpp>` safely while suppressing compiler warnings for the TMB-related code path.

- Added `config.h` as the RTMB configuration header.
  - Configures Rcpp exception handling for TMB failures.
  - Enables `TMB_SAFEBOUNDS` and uses `TMBAD_FRAMEWORK` with 64-bit index type (`uint64_t`).
  - Ensures thread-safe R support when OpenMP is enabled.

- Added `init_tmb.hpp` for runtime initialization and registration.
  - Provides `R_init_FIMS` callback registration for the built shared object.
  - Includes optional initialization path for `FIMS_ONLOAD_INIT_TMB`.
  - Ensures TMB C-callables are registered if `TMB_CCALLABLES` is available.


## Translational units in `src`

- The `src` directory is responsible for building individual translational units that link together the FIMS core and TMB interface.
- By keeping TMB-related code in separate source files, the build avoids a single monolithic translation unit that would instantiate many template combinations at once.
- This is important because TMB and its autodiff types are heavily templated; a large translation unit can cause "template explosion" where the compiler has to generate and optimize many redundant instantiations.
- Splitting code into smaller translational units reduces compile-time memory pressure and keeps the generated object code manageable.
- It also improves incremental build performance because only changed source files need recompilation, rather than rebuilding a huge templated implementation all at once.

## How this changes development

- Developers now have a dedicated TMB interface layer inside `inst/include/interface/TMB`.
- The branch introduces a cleaner separation between TMB precompilation and normal user code builds.
- New configuration and initialization headers make it easier to control TMB runtime behavior from FIMS.
- The added files support safer TMB integration by using Rcpp-compatible exception handling and by enabling bounds checking.

