# Class constructors for `FIMSFrame` and associated child classes

All constructor functions take a single input and build an object
specific to the needs of each model type within FIMS. `FIMSFrame` is the
parent class. Future, associated child classes will have the additional
slots needed for different types of models.

## Usage

``` r
FIMSFrame(data)
```

## Arguments

- data:

  A `data.frame` that contains the necessary columns to construct a
  `FIMSFrame-class` object. Currently, those columns are type, fleet,
  age, length, timing, observed, unit, and uncertainty. See the
  `data_big` object in FIMS, e.g., `data(data_big, package = "FIMS")`.

## Value

An object of the S4 class `FIMSFrame` class, or one of its child
classes, is validated and then returned. All objects will at a minimum
have a slot called `data` to store the input data frame. Additional
slots are dependent on the child class. Use
[`methods::showClass()`](https://rdrr.io/r/methods/RClassUtils.html) to
see all available slots.

## Details

### data

The input data are both sorted (see the section below on sorting) and
expanded to include -999 observations for all missing rows before
returning them in the data slot.

#### Ages

Currently, ages must be integers, i.e., FIMS cannot accommodate numeric
ages like age 1.5 but we hope that this is something that we will be
able to accommodate in the future. Additionally, the first age in your
data set will be the age at which fish recruit to the population. So, if
you do not have age-0 age compositions and age two is the first age you
have in your composition data, then recruitment will happen at age two.
Furthermore, you must have composition entries for every integer between
your first and last age in the model. For example, you cannot bin fish
into bins that span multiple years, you must have age-2, age-3, and
age-4 not just age-2 and age-4 fish in your composition data.

#### Uncertainty

Uncertainty information for your data contains information for fitting
the model and for creating bootstrapped data sets. Right-handed formulas
are used to specify the distribution your data is assumed to follow, the
derived quantity in the model that the data is assumed to represent, and
parameters associated with the assumed distribution. For example, catch
data can be distributed using a normal or lognormal distribution and if
you use a normal distribution the observation is assumed to represent
`catch_expected` but if you use a lognormal distribution the observation
is assumed to represent `log_catch_expected`. Therefore, for the former
you would specify `"~dnorm(mean = catch_expected, sd = 0.01)"` and for
the later you would specify
`"~dlnorm(meanlog = log_catch_expected, sdlog = 0.01)"`. Where, the
input value for sd or sdlog is the standard deviation of the mean or
standard deviation of the log of the mean, respectively. The
distribution names and argument names match the names used in R. Use
[`args()`](https://rdrr.io/r/base/args.html), e.g., `args(dnorm)`, to
get information on the argument names that are required for your
distribution. And, use
[`?distributions`](https://rdrr.io/r/stats/Distributions.html) to get a
list of distributions that have density functions in base R. To get
information on available derived quantities to set the central tendency
of the distribution to, you can create a fleet object and inspect the
available names, e.g., `names(methods::new(Fleet))`, or you can navigate
to the Public Attributes section of the doxygen documentation for Fleet
by running
[`?Fleet`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md).

#### Sorting

It is important that the order of the rows in the data are correct but
it is not expected that the user will do this. Instead, the returned
data are sorted using
[`dplyr::arrange()`](https://dplyr.tidyverse.org/reference/arrange.html)
before placing them in the data slot. Data are first sorted by data
type, placing all `weight_at_age` data next to other `weight_at_age`
data and all catch data next to catch data. Thus, `age_comp` data will
come first because their type is "age" and "a" is first in the alphabet.
All other types will follow according to their order in the alphabet.
Next, within each type, data are organized by fleet. So, `age_comp`
information for fleet1 will come before survey1. Next, all data within
type and fleet are arranged by timing, e.g., by year. That is the end of
the sorting for time series data like catch and indices. The biological
data are further sorted by bin. Thus, `age_comp` information will be
arranged as follows:

|          |        |        |     |          |
|----------|--------|--------|-----|----------|
| type     | name   | timing | age | observed |
| age_comp | fleet1 | 2022   | 1   | 0.3      |
| age_comp | fleet1 | 2022   | 2   | 0.7      |
| age_comp | fleet1 | 2023   | 1   | 0.5      |

`length_comp` data are sorted the same way but by length bin instead of
by age bin. It becomes more complicated for the
`age_to_length_conversion` data, which are sorted by type, name, timing,
age, and then length. So, a full set of length, e.g., length 10, length
20, length 30, etc., is placed together for a given age. After that age,
another entire set of length information will be provided for that next
age. Once the year is complete for a given fleet then the next year will
begin.

## See also

[`plot,FIMSFrame,missing-method()`](https://NOAA-FIMS.github.io/FIMS/reference/plot.md)
for plotting `FIMSFrame` objects.
