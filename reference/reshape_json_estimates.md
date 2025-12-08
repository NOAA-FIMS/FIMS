# Reshape JSON estimates

This function processes the finalized FIMS JSON output and reshapes the
parameter estimates into a structured tibble for easier analysis and
manipulation.

## Usage

``` r
reshape_json_estimates(model_output)
```

## Arguments

- model_output:

  A JSON object containing the finalized FIMS output as returned from
  `get_output()`, which is an internal function to each model family.

## Value

A tibble containing the reshaped parameter estimates.
