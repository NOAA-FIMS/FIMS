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

A data frame with 2808 observations of 8 variables:

- type:

  The type of input the row contains. Allowed types include age_comp,
  age_to_length_conversion, index, catch, length_comp, and
  weight_at_age.

- fleet:

  A character string providing the name of the information source that
  the input was collected from, e.g., `"Trawl fishery"` or
  `"age_zero_survey"`. Entries can be `NA` for information that is not
  fleet/vessel specific.

- age:

  An integer age. Entry can be `NA` if information pertains to multiple
  ages, e.g., total catch rather than catch of age-4 fish. In this
  dataset, ages start at one, therefore recruitment happens at age one
  not age zero.

- length:

  A numeric length. Entry can be `NA` if information doesn't pertain to
  length.

- timing:

  The timing, i.e., year the data was collected.

- observed:

  The observed measurement of interest.

- unit:

  A character string specifying the units of `observed`. Allowed units
  for each data type are as follows. `mt` is used for `index`, `catch`,
  and `weight_at_age` data. `number` or `proportion` are each viable
  units for the composition data, where the former is the preferred unit
  of measurement.

- uncertainty:

  A right-handed formula specifying the distributional assumptions for
  the entry in `observed`. See
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  for more information about this column.

## Source

[www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison](https://NOAA-FIMS.github.io/FIMS/reference/www.github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison)

## See also

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
