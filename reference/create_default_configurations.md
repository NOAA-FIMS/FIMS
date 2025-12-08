# Create a default FIMS configuration tibble

This function generates a default configuration tibble for a Fisheries
Integrated Modeling System (FIMS) model based on the data input. It
automatically creates configuration entries for data modules (e.g.,
landings, index, compositions) and, depending on the model family,
standard population dynamics modules (recruitment, growth, maturity) and
selectivity modules for fleets.

## Usage

``` r
create_default_configurations(data, model_family = c("catch_at_age"))
```

## Arguments

- data:

  An S4 object of class `FIMSFrame`. FIMS input data.

- model_family:

  A string specifying the model family. Defaults to `"catch_at_age"`.

## Value

A `tibble` with default model configurations. The tibble has a nested
structure with the following top-level columns.

- `model_family`::

  The specified model family (e.g., "catch_at_age").

- `module_name`::

  The name of the FIMS module (e.g., "Data", "Selectivity",
  "Recruitment", "Growth", "Maturity").

- `fleet_name`::

  The name of the fleet the module applies to. This will be `NA` for
  non-fleet-specific modules like "Recruitment".

- `data`::

  A list-column containing a `tibble` with detailed configurations.
  Unnesting this column reveals:

  `module_type`:

  :   The specific type of the module (e.g., "Logistic" for a
      "Selectivity" module).

  `distribution_link`:

  :   The component the distribution module links to.

  `distribution_type`:

  :   The type of distribution (e.g., "Data", "process").

  `distribution`:

  :   The name of distribution (e.g., "Dlnorm", `Dmultinom`).

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
for building a complete FIMS model configuration.

## Examples

``` r
# Load the example dataset and create a FIMS data frame
data("data1")
fims_frame <- FIMSFrame(data1)

# Create the default model configuration tibble
default_configurations <- create_default_configurations(data = fims_frame)

# Unnest the data column to see detailed configurations
default_configurations_unnest <- default_configurations |>
  tidyr::unnest(cols = data) |>
  print()
#> # A tibble: 11 × 7
#>    model_family module_name fleet_name module_type  distribution_link
#>    <chr>        <chr>       <chr>      <chr>        <chr>            
#>  1 catch_at_age Data        fleet1     AgeComp      AgeComp          
#>  2 catch_at_age Data        fleet1     Landings     Landings         
#>  3 catch_at_age Data        fleet1     LengthComp   LengthComp       
#>  4 catch_at_age Selectivity fleet1     Logistic     NA               
#>  5 catch_at_age Data        survey1    AgeComp      AgeComp          
#>  6 catch_at_age Data        survey1    Index        Index            
#>  7 catch_at_age Data        survey1    LengthComp   LengthComp       
#>  8 catch_at_age Selectivity survey1    Logistic     NA               
#>  9 catch_at_age Growth      NA         EWAA         NA               
#> 10 catch_at_age Maturity    NA         Logistic     NA               
#> 11 catch_at_age Recruitment NA         BevertonHolt log_devs         
#> # ℹ 2 more variables: distribution_type <chr>, distribution <chr>

# Model fleet1 with double logistic selectivity
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
#> # A tibble: 11 × 7
#>    model_family module_name fleet_name module_type    distribution_link
#>    <chr>        <chr>       <chr>      <chr>          <chr>            
#>  1 catch_at_age Data        fleet1     AgeComp        AgeComp          
#>  2 catch_at_age Data        fleet1     Landings       Landings         
#>  3 catch_at_age Data        fleet1     LengthComp     LengthComp       
#>  4 catch_at_age Selectivity fleet1     DoubleLogistic NA               
#>  5 catch_at_age Data        survey1    AgeComp        AgeComp          
#>  6 catch_at_age Data        survey1    Index          Index            
#>  7 catch_at_age Data        survey1    LengthComp     LengthComp       
#>  8 catch_at_age Selectivity survey1    Logistic       NA               
#>  9 catch_at_age Growth      NA         EWAA           NA               
#> 10 catch_at_age Maturity    NA         Logistic       NA               
#> 11 catch_at_age Recruitment NA         BevertonHolt   log_devs         
#> # ℹ 2 more variables: distribution_type <chr>, distribution <chr>
```
