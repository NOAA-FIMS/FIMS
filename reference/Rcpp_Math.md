# Sets methods for operators under the S4 Generic Group, Ops

Ops include Arith (`+`, `-`, `*`, `^`, `%%`, `%/%`, and `/`); Compare
(`==`, `>`, `<`, `!=`, `<=`, and `>=`); and Logic (`&`, `|`).

Methods of mathematical functions include trigonometry functions, `abs`,
`sign`, `sqrt`, `ceiling`, `floor`, `trunc`, `cummax`, `cumprod`,
`cumsum`, `log`, `log10`, `log2`, `log1p`, `exp`, `expm1`, `gamma`,
`lgamma`, `digamma`, and `trigamma`.

## Usage

``` r
# S4 method for class 'Rcpp_Variable,Rcpp_Variable'
Ops(e1, e2)

# S4 method for class 'Rcpp_Variable,numeric'
Ops(e1, e2)

# S4 method for class 'numeric,Rcpp_Variable'
Ops(e1, e2)

# S4 method for class 'Rcpp_VariableVector,Rcpp_VariableVector'
Ops(e1, e2)

# S4 method for class 'Rcpp_VariableVector,numeric'
Ops(e1, e2)

# S4 method for class 'Rcpp_RealVector,numeric'
Ops(e1, e2)

# S4 method for class 'numeric,Rcpp_VariableVector'
Ops(e1, e2)

# S4 method for class 'Rcpp_VariableVector'
Math(x)
```

## Arguments

- e1, e2:

  An Rcpp_Variable or Rcpp_VariableVector class object or a numeric
  vector or value.

- x:

  An Rcpp_VariableVector class object.

## Value

A numeric or logical value(s) depending on the generic and the length of
the input values.

A vector of numeric values.
