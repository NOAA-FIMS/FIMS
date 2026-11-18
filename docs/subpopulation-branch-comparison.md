# Subpopulation branch scope

This branch retains the developed `subpopulation.hpp` as inactive scaffolding.
The previous working implementation remains in the parent branch history.

Population storage, module resolution, catch-at-age dynamics, parameter
registration, reporting, and R/Rcpp entry points use their pre-integration pooled
implementation (revision `d7608eb`). Runtime subpopulation examples and integration
tests have been removed. Standalone header tests remain.

See `inst/include/population_dynamics/population/README.md` for the configuration
contract and possible future control functors. Activation is deferred.
