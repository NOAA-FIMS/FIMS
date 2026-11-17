# Plot a `FIMSFrame` object

Plot data availability for a `FIMSFrame` object using
[`stockplotr::plot_data_availability()`](https://rdrr.io/pkg/stockplotr/man/plot_data_availability.html).

## Usage

``` r
# S4 method for class 'FIMSFrame,missing'
plot(x, y, ...)
```

## Arguments

- x:

  A `FIMSFrame` object.

- y:

  Unused (inherited from R base).

- ...:

  Unused (inherited from R base).

## Value

The result of
[`stockplotr::plot_data_availability()`](https://rdrr.io/pkg/stockplotr/man/plot_data_availability.html).

## Examples

``` r
if (FALSE) { # \dontrun{
data("data_big", package = "FIMS")
data_4_model <- FIMSFrame(data_big)
plot(data_4_model)
} # }
```
