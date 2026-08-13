# Set up default growth parameters

This function creates default growth parameters for a Fisheries
Integrated Modeling System (FIMS) model. It generates a tibble with
fields for module name, module type, label, value, and estimation type.

## Usage

``` r
setup_default_Growth(module_type = c("EWAA"))
```

## Arguments

- module_type:

  A character string specifying the type of growth module. The default
  is `"EWAA"`.

## Value

A tibble containing default growth parameters. See
[`setup_default_parameters`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)
for full column descriptions.

## See also

- [`setup_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_default_parameters.md)

## Examples

``` r
if (FALSE) { # \dontrun{
# Set up default growth parameters
default_growth_parameters <- setup_default_Growth()
} # }
```
