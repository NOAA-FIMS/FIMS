# Get a vector of data to be passed to a FIMS module from a FIMSFrame object

There is an accessor function for each data type needed to run a FIMS
model. A FIMS model accepts vectors of data and thus each of the `m_*()`
functions, where the star can be replaced with the data type separated
by underscores, e.g., weight_at_age. These accessor functions are the
preferred way to pass data to a FIMS module because the data will have
the appropriate indexing.

## Usage

``` r
m_landings(x, fleet_name)

# S4 method for class 'FIMSFrame'
m_landings(x, fleet_name)

# S4 method for class 'data.frame'
m_landings(x, fleet_name)

m_index(x, fleet_name)

# S4 method for class 'FIMSFrame'
m_index(x, fleet_name)

# S4 method for class 'data.frame'
m_index(x, fleet_name)

m_agecomp(x, fleet_name)

# S4 method for class 'FIMSFrame'
m_agecomp(x, fleet_name)

# S4 method for class 'data.frame'
m_agecomp(x, fleet_name)

m_lengthcomp(x, fleet_name)

# S4 method for class 'FIMSFrame'
m_lengthcomp(x, fleet_name)

# S4 method for class 'data.frame'
m_lengthcomp(x, fleet_name)

m_weight_at_age(x)

# S4 method for class 'FIMSFrame'
m_weight_at_age(x)

# S4 method for class 'data.frame'
m_weight_at_age(x)

m_age_to_length_conversion(x, fleet_name)

# S4 method for class 'FIMSFrame'
m_age_to_length_conversion(x, fleet_name)

# S4 method for class 'data.frame'
m_age_to_length_conversion(x, fleet_name)
```

## Arguments

- x:

  An object returned from
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md).

- fleet_name:

  A string, or vector of strings, specifying the name of the fleet(s) of
  interest that you want landings data for. The strings must exactly
  match strings in the column `"name"` of `get_data(x)`.

## Value

All of the `m_*()` functions return vectors of data. Currently, the
order of the data is the same order as the data frame because no
arranging is done in
[`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
and the function just extracts the appropriate column.

## Details

Age-to-length-conversion data, i.e., the proportion of age "a" that are
length "l", are used to convert lengths (input data) to ages (modeled)
as a way to fit length data without estimating growth.
