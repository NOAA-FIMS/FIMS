# Subpopulation scaffold (inactive)

`subpopulation.hpp` defines a standalone, templated biological-group configuration.
It is development work only: `Population`, catch-at-age execution, parameter
registration, reporting, and the R/Rcpp interfaces do not use it.
Existing models continue to use pooled population dynamics.

Each group has a name, its own initial log abundance, a recruitment allocation,
a reproductive contribution, and optional growth, maturity, and fleet selectivity
IDs. These IDs are configuration placeholders; no modules are resolved here.
`ValidateSubpopulations()` checks names, allocation ranges and conservation,
and a positive recruitment-weighted reproductive contribution.
`MakeTwoSexSubpopulations()` constructs female/male configurations with separate
initial abundance vectors. `SubpopulationLayout` describes contiguous
[group, year, age] storage, with age varying fastest; it allocates no model state.
Initial abundance dimensions and module IDs will need model-context validation
before integration. An empty configuration is allowed.

## Possible future controls

A future integration could use small controls for recruitment allocation,
movement, and sex transitions. Existing recruitment modules would calculate
total recruits; an allocation control would distribute them among groups.
An identity movement control would retain each group's abundance, with future
transition controls conserving abundance unless explicit losses are modeled.
Controls should preserve the model's scalar `Type` for automatic differentiation.
The model would define operation order and own state updates.

No control interface or execution hooks are introduced in this scaffold.
Activation requires a separate implementation and review, including pooled-model
regression checks, parameter registration, reporting, and interface tests.
