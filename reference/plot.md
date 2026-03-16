# Plot a `FIMSFrame` object

Use
[`ggplot2::geom_point()`](https://ggplot2.tidyverse.org/reference/geom_point.html)
to plot the information stored in the data slot of the `FIMSFrame`
class.

## Usage

``` r
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

A ggplot2 object is returned that uses
[`stockplotr::theme_noaa()`](https://noaa-fisheries-integrated-toolbox.r-universe.dev/stockplotr/reference/theme_noaa.html).
There will be one panel per input type with fleet-specific information
denoted using colors.

## Examples

``` r
if (FALSE) { # \dontrun{
data("data_big", package = "FIMS")
data_4_model <- FIMSFrame(data_big)
plot(data_4_model)
} # }
```
