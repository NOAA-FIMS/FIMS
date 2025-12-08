# Setter for `Rcpp_ParameterVector`

In R, indexing starts at one. But, in C++ indexing starts at zero. These
functions do the translation for you so you can think in R terms.

In R, indexing starts at one. But, in C++ indexing starts at zero. This
function does the translation for you so you can think in R terms.

Methods of summary functions include `max`, `min`, `range`, `prod`,
`sum`, `any`, and `all`.

## Usage

``` r
# S4 method for class 'Rcpp_ParameterVector,ANY,ANY,ANY'
x[i, j] <- value

# S4 method for class 'Rcpp_ParameterVector,numeric,ANY,ANY'
x[i]

# S4 method for class 'Rcpp_ParameterVector'
length(x)

# S4 method for class 'Rcpp_ParameterVector'
sum(x)

# S4 method for class 'Rcpp_ParameterVector'
dim(x)

# S4 method for class 'Rcpp_ParameterVector'
Summary(x)
```

## Arguments

- x:

  An Rcpp_ParameterVector class object.

- i:

  An integer specifying the location in R speak, where indexing starts
  at one, of the vector that you wish to get information from.

- j:

  Not used with `Rcpp_ParameterVector` because it is a vector.

- value:

  The value you want to set the indexed location to.

## Value

For `[<-`, the index `i` of object `x` is set to `value`.

For `[`, the index `i` of object `x` is returned.

For [`length()`](https://rdrr.io/r/base/length.html), the length of
object `x` is returned as an integer.

For [`sum()`](https://rdrr.io/r/base/sum.html), the sum of object `x` is
returned as a numeric value.

For [`dim()`](https://rdrr.io/r/base/dim.html), the dimensions of object
`x` is returned as a single integer because there is only one dimension
to return for a vector.

`Summary` returns a single or two numeric or logical values.
