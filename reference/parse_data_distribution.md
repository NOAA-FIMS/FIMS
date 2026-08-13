# Parse a distribution formula into structured components

Parse one-sided formula-like distribution specifications and return
their distribution families and parameter expressions as an aligned
list.

## Usage

``` r
parse_data_distribution(expr)
```

## Arguments

- expr:

  One or more distribution specifications provided as a character vector
  (for example, `"~ gaussian(mean = x, sd = 0.1)"`), an expression (for
  example, `expression(~ gaussian(mean = x, sd = 0.1))`), a language
  call produced by [`quote()`](https://rdrr.io/r/base/substitute.html),
  or an R formula object.

## Value

A column-oriented list. Every component has the same length as `expr`,
and every parameter supported by
[fims_distributions](https://NOAA-FIMS.github.io/FIMS/reference/fims_distributions.md)
is present. This includes:

- `family`: a character vector containing the distribution family names.

- `link`: a character vector containing the central parameter field
  names.

- One list component for each supported distribution parameter.

Parameters that do not apply to a non-missing specification are
represented by `NA`. Missing specifications use `NA_character_` for the
family and link, `NA` for central parameters, and `1` for ancillary
parameters. These neutral ancillary values prevent missing observations
from introducing `NA` into vectorized likelihood calculations.

## Details

The parser accepts inputs that represent a formula containing a
distribution call on the right-hand side, such as
`~ gaussian(mean = x, sd = 0.1)`.

Input handling and validation proceed in stages:

- Character input is converted with
  [`base::str2lang()`](https://rdrr.io/r/base/parse.html).

- Elements of [`expression()`](https://rdrr.io/r/base/expression.html)
  input are extracted and parsed as language objects.

- A language call or formula object is treated as a scalar input.

- Any other input type throws an error.

After conversion, the object must be a call whose head is `~`. Only
one-sided formulas (`~ dist(...)`) are supported.

The distribution call is decomposed into:

- `family`: the function name used for the distribution, as a character
  scalar (for example, `"gaussian"`, `"multinomial"`).

- `parameters`: a list of unevaluated parameter expressions from the
  distribution call. Named arguments preserve their names, while
  positional arguments remain unnamed.

Parameter values are returned as language objects rather than evaluated
numeric vectors. This preserves references to symbols in the calling
environment (for example, `catch_expected`) so evaluation can occur
later in a context where those objects exist.

## Examples

``` r
FIMS:::parse_data_distribution(
  "~ dmultinom(prob = age_comp_expected, size = 300)"
)
#> $family
#> [1] "dmultinom"
#> 
#> $link
#> [1] "age_comp_expected"
#> 
#> $mean
#> $mean[[1]]
#> [1] NA
#> 
#> 
#> $meanlog
#> $meanlog[[1]]
#> [1] NA
#> 
#> 
#> $prob
#> $prob[[1]]
#> age_comp_expected
#> 
#> 
#> $sd
#> $sd[[1]]
#> [1] NA
#> 
#> 
#> $sdlog
#> $sdlog[[1]]
#> [1] NA
#> 
#> 
#> $size
#> $size[[1]]
#> [1] 300
#> 
#> 
FIMS:::parse_data_distribution(
  quote(~ dnorm(mean = 0, sd = 1))
)
#> $family
#> [1] "dnorm"
#> 
#> $link
#> [1] "0"
#> 
#> $mean
#> $mean[[1]]
#> [1] 0
#> 
#> 
#> $meanlog
#> $meanlog[[1]]
#> [1] NA
#> 
#> 
#> $prob
#> $prob[[1]]
#> [1] NA
#> 
#> 
#> $sd
#> $sd[[1]]
#> [1] 1
#> 
#> 
#> $sdlog
#> $sdlog[[1]]
#> [1] NA
#> 
#> 
#> $size
#> $size[[1]]
#> [1] NA
#> 
#> 
FIMS:::parse_data_distribution(
  expression(~ dlnorm(meanlog = mu, sdlog = 0.1))
)
#> $family
#> [1] "dlnorm"
#> 
#> $link
#> [1] "mu"
#> 
#> $mean
#> $mean[[1]]
#> [1] NA
#> 
#> 
#> $meanlog
#> $meanlog[[1]]
#> mu
#> 
#> 
#> $prob
#> $prob[[1]]
#> [1] NA
#> 
#> 
#> $sd
#> $sd[[1]]
#> [1] NA
#> 
#> 
#> $sdlog
#> $sdlog[[1]]
#> [1] 0.1
#> 
#> 
#> $size
#> $size[[1]]
#> [1] NA
#> 
#> 
```
