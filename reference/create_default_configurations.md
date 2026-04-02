# Create a default FIMS configuration tibble

A FIMS configuration tibble specifies the high-level model
configuration, i.e., what parameter sets are needed, for a FIMS model.
This high-level information can then be passed to
[`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md),
which returns a tibble with all of the parameters required to run your
specified configuration. For example, logistic selectivity is the
default, and this function will specify logistic selectivity for every
fleet it finds in `data`. If you want double-normal selectivity you can
run this function first and then edit the tibble that is returned,
changing the entry in the `module_type` column before running
[`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md).
Thus, running this function is not strictly necessary to run a FIMS
model but it is helpful to see what parameter sets are necessary given
the data that you want to model. It automatically creates configuration
entries for data modules (e.g., landings, index, compositions) and,
depending on the model family, standard population dynamics modules
(recruitment, growth, maturity) and selectivity modules for fleets.

## Usage

``` r
create_default_configurations(data, model_family = c("catch_at_age"))
```

## Arguments

- data:

  A `FIMSFrame` object returned from running
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  on your long input data.

- model_family:

  A string specifying the model family. Defaults to `"catch_at_age"`.

## Value

A nested `tibble` with the following top-level columns is returned:

- `model_family`::

  The specified model family (e.g., "catch_at_age").

- `module_name`::

  The name of the FIMS module (e.g., "Data", "Selectivity",
  "Recruitment", "Growth", "Maturity"). These entries are always written
  in PascalCase to match the names used in the C++ code.

- `fleet_name`::

  The name of the fleet the module applies to. This will be `NA` for
  non-fleet-specific modules like "Recruitment".

- `data`::

  A list-column containing a `tibble` with detailed configurations.
  Unnesting this column reveals:

  `module_type`:

  :   The specific type of the module (e.g., "Logistic" for a
      "Selectivity" module). This column will always be written in
      PascalCase to match the names used in the C++ code.

  `distribution_type`:

  :   The type of distribution (e.g., "Data", "process"), where a
      process distribution can refer to a fixed effect or a random
      effect but it does not fit to data, e.g., recruitment deviations.

  `distribution`:

  :   The name of distribution (e.g., "Dlnorm", `Dmultinom`). The column
      will always be written in PascalCase to match the names used in
      the C++ code.

## Details

The function inspects the data to find unique combinations of fleet
names and data types. It then maps these to the appropriate FIMS module
names and joins them with a predefined template of default settings.
When the `model_family` is "catch_at_age", it also adds default
configurations for:

- **Selectivity:** A logistic selectivity module for each unique fleet.

- **Recruitment:** A Beverton–Holt recruitment module.

- **Growth:** An empirical weight-at-age (EWAA) growth module.

- **Maturity:** A logistic maturity module.

The final output is a nested tibble, which serves as a starting point
for building a complete FIMS model configuration. You can unnest the
rows to see the full configuration with
[`tidyr::unnest()`](https://tidyr.tidyverse.org/reference/unnest.html),
see the examples below for more details.

## See also

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)

- [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md)

## Examples

``` r
# Load the example dataset and create a FIMS data frame
data("data_big")
fims_frame <- FIMSFrame(data_big)

# Create the default model configuration tibble
default_configurations <- create_default_configurations(data = fims_frame)

# Unnest the data column to see detailed configurations
default_configurations_unnest <- default_configurations |>
  tidyr::unnest(cols = data) |>
  print()
#> # A tibble: 11 × 6
#>    model_family module_name fleet_name module_type  distribution_type
#>    <chr>        <chr>       <chr>      <chr>        <chr>            
#>  1 catch_at_age Data        fleet1     AgeComp      Data             
#>  2 catch_at_age Data        fleet1     Landings     Data             
#>  3 catch_at_age Data        fleet1     LengthComp   Data             
#>  4 catch_at_age Selectivity fleet1     Logistic     NA               
#>  5 catch_at_age Data        survey1    AgeComp      Data             
#>  6 catch_at_age Data        survey1    Index        Data             
#>  7 catch_at_age Data        survey1    LengthComp   Data             
#>  8 catch_at_age Selectivity survey1    Logistic     NA               
#>  9 catch_at_age Growth      NA         EWAA         NA               
#> 10 catch_at_age Maturity    NA         Logistic     NA               
#> 11 catch_at_age Recruitment NA         BevertonHolt process          
#> # ℹ 1 more variable: distribution <chr>

# Use dplyr::rows_update to modify the selectivity specified for fleet1
# from logistic (the default) to double logistic
configurations_double_logistic <- default_configurations_unnest |>
  dplyr::rows_update(
    tibble::tibble(
      module_name = "Selectivity",
      fleet_name = "fleet1",
      module_type = "DoubleLogistic"
    ),
    by = c("module_name", "fleet_name")
  ) |>
  print()
#> # A tibble: 11 × 6
#>    model_family module_name fleet_name module_type    distribution_type
#>    <chr>        <chr>       <chr>      <chr>          <chr>            
#>  1 catch_at_age Data        fleet1     AgeComp        Data             
#>  2 catch_at_age Data        fleet1     Landings       Data             
#>  3 catch_at_age Data        fleet1     LengthComp     Data             
#>  4 catch_at_age Selectivity fleet1     DoubleLogistic NA               
#>  5 catch_at_age Data        survey1    AgeComp        Data             
#>  6 catch_at_age Data        survey1    Index          Data             
#>  7 catch_at_age Data        survey1    LengthComp     Data             
#>  8 catch_at_age Selectivity survey1    Logistic       NA               
#>  9 catch_at_age Growth      NA         EWAA           NA               
#> 10 catch_at_age Maturity    NA         Logistic       NA               
#> 11 catch_at_age Recruitment NA         BevertonHolt   process          
#> # ℹ 1 more variable: distribution <chr>
```
