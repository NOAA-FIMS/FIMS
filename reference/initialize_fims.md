# Initialize FIMS modules

Initializes multiple modules within the Fisheries Integrated Modeling
System (FIMS), including fleet, recruitment, growth, maturity, and
population modules. This function iterates over the provided fleets,
setting up necessary sub-modules such as selectivity, index, and age
composition. It also sets up distribution models for fishery index and
age-composition data.

## Usage

``` r
initialize_fims(parameters, data)
```

## Arguments

- parameters:

  A tibble. Contains parameters and modules required for initialization.

- data:

  An S4 object. FIMS input data.

## Value

A list containing parameters for the initialized FIMS modules, ready for
use in TMB modeling.
