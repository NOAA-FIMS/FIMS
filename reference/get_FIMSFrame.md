# Get a slot in a FIMSFrame object

There is an accessor function for each slot in the S4 class `FIMSFrame`,
where the function is named `get_*()` and the star can be replaced with
the slot name, e.g., `get_data()`. These accessor functions are the
preferred way to access objects stored in the available slots.

## Usage

``` r
get_data(x)

# S4 method for class 'FIMSFrame'
get_data(x)

# S4 method for class 'data.frame'
get_data(x)

get_fleets(x)

# S4 method for class 'FIMSFrame'
get_fleets(x)

# S4 method for class 'data.frame'
get_fleets(x)

get_n_fleets(x)

# S4 method for class 'FIMSFrame'
get_n_fleets(x)

# S4 method for class 'data.frame'
get_n_fleets(x)

get_n_years(x)

# S4 method for class 'FIMSFrame'
get_n_years(x)

# S4 method for class 'data.frame'
get_n_years(x)

get_start_year(x)

# S4 method for class 'FIMSFrame'
get_start_year(x)

# S4 method for class 'data.frame'
get_start_year(x)

get_end_year(x)

# S4 method for class 'FIMSFrame'
get_end_year(x)

# S4 method for class 'data.frame'
get_end_year(x)

get_ages(x)

# S4 method for class 'FIMSFrame'
get_ages(x)

# S4 method for class 'data.frame'
get_ages(x)

get_n_ages(x)

# S4 method for class 'FIMSFrame'
get_n_ages(x)

# S4 method for class 'data.frame'
get_n_ages(x)

get_lengths(x)

# S4 method for class 'FIMSFrame'
get_lengths(x)

# S4 method for class 'data.frame'
get_lengths(x)

get_n_lengths(x)

# S4 method for class 'FIMSFrame'
get_n_lengths(x)

# S4 method for class 'data.frame'
get_n_lengths(x)
```

## Arguments

- x:

  An object returned from
  [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md).

## Value

`get_data()` returns a data frame of the class `tbl_df` containing data
for a FIMS model in a long format. The tibble will potentially have the
following columns depending if it fits to ages and lengths or just one
of them: type, name, age, length, timing, value, unit, and uncertainty.

`get_fleets()` returns a vector of strings containing the fleet names.

`get_n_fleets()` returns an integer specifying the number of fleets in
the model, where fleets is inclusive of both fishing fleets and survey
vessels.

`get_n_years()` returns an integer specifying the number of years in the
model.

`get_start_year()` returns an integer specifying the start year of the
model.

`get_end_year()` returns an integer specifying the end year of the
model.

`get_ages()` returns a vector of age bins used in the model.

`get_n_ages()` returns an integer specifying the number of age bins used
in the model.

`get_lengths()` returns a vector of length bins used in the model.

`get_n_lengths()` returns an integer specifying the number of length
bins used in the model.
