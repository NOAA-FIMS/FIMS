# Distributions not available in the stats package

Family objects provide a convenient way to specify the details of the
models used by functions such as
[`stats::glm()`](https://rdrr.io/r/stats/glm.html). These functions
within this package are not available within the stats package but are
designed in a similar manner.

## Usage

``` r
lognormal(link = "log")

multinomial(link = "logit")
```

## Arguments

- link:

  A string specifying the model link function. For example, `"identity"`
  or `"log"` are appropriate names for the
  [`stats::gaussian()`](https://rdrr.io/r/stats/family.html)
  distribution. `"log"` and `"logit"` are the defaults for the lognormal
  and the multinomial, respectively.

## Value

An object of class `family` (which has a concise print method). This
particular family has a truncated length compared to other distributions
in [`stats::family()`](https://rdrr.io/r/stats/family.html).

- family:

  character: the family name.

- link:

  character: the link name.

## See also

- [`stats::family()`](https://rdrr.io/r/stats/family.html)

- [`stats::gaussian()`](https://rdrr.io/r/stats/family.html)

- [`stats::glm()`](https://rdrr.io/r/stats/glm.html)

- [`stats::power()`](https://rdrr.io/r/stats/power.html)

- [`stats::make.link()`](https://rdrr.io/r/stats/make.link.html)

## Examples

``` r
a_family <- multinomial()
a_family[["family"]]
#> [1] "multinomial"
a_family[["link"]]
#> [1] "logit"
```
