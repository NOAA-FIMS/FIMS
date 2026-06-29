# Setter for `Rcpp_RealVector`

In R, indexing starts at one. But, in C++ indexing starts at zero. These
functions do the translation for you so you can think in R terms.

## Usage

``` r
# S4 method for class 'Rcpp_RealVector,ANY,ANY,ANY'
x[i, j] <- value
```

## Arguments

- x:

  A numeric vector.

- i:

  An integer specifying the location in R speak, where indexing starts
  at one, of the vector that you wish to set.

- j:

  Not used with `Rcpp_RealVector` because it is a vector.

- value:

  The value you want to set the indexed location to.

## Value

For `[<-`, the index `i` of object `x` is set to `value`.
