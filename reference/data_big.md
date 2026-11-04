# FIMS input data frame

A data frame containing the information needed to run an age-structured
stock assessment model in FIMS. This data was generated using the
`ASSAMC` package written for the [model comparison
project](https://github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison).
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
  age_to_length_conversion, ageing_error, index, catch, length_bin,
  length_comp, and weight_at_age.

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
  length. For `length_comp`, `length_bin`, and
  `age_to_length_conversion` rows, this is the lower edge of the length
  bin. A bin holds lengths from its lower edge up to the next lower
  edge. When growth is estimated in FIMS, the first bin also holds all
  smaller fish and the last bin holds all larger fish. With fixed
  `age_to_length_conversion` rows, those rows set what each bin holds.
  In this dataset, the bins are 50 mm wide, so `length = 250` is the bin
  from 250 to 300 mm, and the conversion puts all fish under 50 mm in
  the first bin and all fish of 1100 mm or more in the last.

- timing:

  The timing, i.e., year the data was collected.

- observed:

  The observed measurement of interest.

- unit:

  A character string specifying the units of `observed`. Allowed units
  for each data type are as follows. `mt` or `number` are used for
  `index` and `catch` data, with 1 unit per fleet for each type. `mt` is
  used for `weight_at_age` data. `number` or `proportion` are each
  viable units for the composition data, where the former is the
  preferred unit of measurement. `proportion` is also used for
  `ageing_error` data, the probability that a fish of a given true age
  is read as the age in `age`.

- uncertainty:

  A character string with a right-handed formula specifying the
  distributional assumptions for the entry in `observed`. See
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  for more information about this column. For `ageing_error` rows, the
  string is the true age, e.g., `"5"`.

## Source

<https://github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison>

## See also

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
