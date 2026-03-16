# FIMS input data frame

A data frame containing the information needed to run an age-structured
stock assessment model in FIMS. This data was generated using the
`ASSAMC` package written for the [model comparison
project](https://NOAA-FIMS.github.io/FIMS/reference/www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison).
The source code to make this dataset is in `data-raw/data_big.R` which
is on GitHub but will not be in your local installation because it is in
the .Rbuildignore file.

## Usage

``` r
data_big
```

## Format

A data frame with 19092 observations of 8 variables:

- type:

  The type of input the row contains. Allowed types include age_comp,
  age_to_length_conversion, index, landings, length_comp, and
  weight_at_age.

- name:

  A character string providing the name of the information source that
  the input was collected from, e.g., `"Trawl fishery"`.

- age:

  An integer age. Entry can be `NA` if information pertains to multiple
  ages, e.g., total landings rather than landings of age-4 fish.

- length:

  A numeric length. Entry can be `NA` if information doesn't pertain to
  length.

- timing:

  The timing, i.e., year the data was collected.

- value:

  The measurement of interest.

- unit:

  A character string specifying the units of `value`. Allowed units for
  each data type are as follows. `mt` is used for `index`, `landings`,
  and `weight_at_age` data. `number` or `proportion` are each viable
  units for the composition data, where the former is the preferred unit
  of measurement.

- uncertainty:

  A real value providing a measurement of uncertainty for value. For
  landings and indices of abundance this should be the standard
  deviation of the logged observations if you are using the lognormal
  distribution to fit your data. For composition data it will be your
  input sample size.

## Source

[www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison](https://NOAA-FIMS.github.io/FIMS/reference/www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison)
