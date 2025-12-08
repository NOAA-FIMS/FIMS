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
  `FIMSFrame-class` object. Currently, those columns are type, name,
  age, length, timing, value, unit, and uncertainty. See the data1
  object in FIMS, e.g., `data(data1, package = "FIMS")`.

## Value

An object of the S4 class `FIMSFrame` class, or one of its child
classes, is validated and then returned. All objects will at a minimum
have a slot called `data` to store the input data frame. Additional
slots are dependent on the child class. Use
[`methods::showClass()`](https://rdrr.io/r/methods/RClassUtils.html) to
see all available slots.

## Details

### data

The input data are both sorted and expanded before returning them in the
data slot.

#### Sorting

It is important that the order of the rows in the data are correct but
it is not expected that the user will do this. Instead, the returned
data are sorted using
[`dplyr::arrange()`](https://dplyr.tidyverse.org/reference/arrange.html)
before placing them in the data slot. Data are first sorted by data
type, placing all weight-at-age data next to other weight-at-age data
and all landings data next to landings data. Thus, age-composition data
will come first because their type is "age" and "a" is first in the
alphabet. All other types will follow according to their order in the
alphabet. Next, within each type, data are organized by fleet. So,
age-composition information for fleet1 will come before survey1. Next,
all data within type and fleet are arranged by timing, e.g., by year.
That is the end of the sorting for time series data like landings and
indices. The biological data are further sorted by bin. Thus,
age-composition information will be arranged as follows:

|      |        |        |     |       |
|------|--------|--------|-----|-------|
| type | name   | timing | age | value |
| age  | fleet1 | 2022   | 1   | 0.3   |
| age  | fleet1 | 2022   | 2   | 0.7   |
| age  | fleet1 | 2023   | 1   | 0.5   |

Length composition-data are sorted the same way but by length bin
instead of by age bin. It becomes more complicated for the
age-to-length-conversion data, which are sorted by type, name, timing,
age, and then length. So, a full set of length, e.g., length 10, length
20, length 30, etc., is placed together for a given age. After that age,
another entire set of length information will be provided for that next
age. Once the year is complete for a given fleet then the next year will
begin.
