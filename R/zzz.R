# Note
#
# Placing `Rcpp::loadModule(module = "fims", what = TRUE)` inside the
# `.onLoad` function has been observed to fix R session crashes when running
# `devtools::load_all()` followed by `devtools::test()` in the package
# development workflow.
#
# `Rcpp::loadModule()` is called to make all objects from C++ module
# "fims" accessible in R.
# `loadNamespace` calls `.onLoad()` before sealing the namespace and processing
# exports.
# This might provide `pkgload` (used by `devtools::load_all()` and
# `devtools::test()`) a more consistent state to manage the dynamic library
# during its unload/reload cycles in a development environment (Source: Gemini).
#
# There is no direct or explicit documentation stating that `Rcpp::loadModule()`
# must be called in `.onLoad()`. The `Rcpp` documentation mentions that for
# R 2.15.1 and later (e.g., from
# `https://github.com/RcppCore/Rcpp/blob/29b3b78df547e55dbcceb1c5e81a978f441dd58b/inst/skeleton/zzz.R`)
# `loadModule()` does not strictly need to be in `.onLoad()`
# because it "triggers a load action" internally. However, packages like
# `stan-dev/rstan` and several others do call `Rcpp::loadModule()` within
# their `.onLoad()` functions. See examples from
# https://github.com/search?q=Rcpp%3A%3AloadModule%28+zzz.R&type=code.
.onLoad <- function(libname, pkgname) {
  Rcpp::loadModule(module = "fims", what = TRUE)
}

.onUnload <- function(libpath) {
  library.dynam.unload("FIMS", libpath)
}

# Methods for Rcpp
#' Setter for `Rcpp_ParameterVector`
#'
#' In R, indexing starts at one. But, in C++ indexing starts at zero. These
#' functions do the translation for you so you can think in R terms.
#'
#' @param x A numeric vector.
#' @param i An integer specifying the location in R speak, where indexing
#'   starts at one, of the vector that you wish to set.
#' @param j Not used with `Rcpp_ParameterVector` because it is a vector.
#' @param value The value you want to set the indexed location to.
#' @return
#' For `[<-`, the index `i` of object `x` is set to `value`.
#' @keywords internal
#' @rdname Rcpp_ParameterVector
methods::setMethod(
  f = "[<-",
  signature = signature(
    x = "Rcpp_ParameterVector"
  ),
  definition = function(x, i, j, value) {
    x$set(i - 1, value) # R uses 1-based indexing, C++ uses 0-based indexing
    return(x) # Return the modified object
  }
)

#' Get information from Rcpp_ParameterVector
#'
#' In R, indexing starts at one. But, in C++ indexing starts at zero. This
#' function does the translation for you so you can think in R terms.
#'
#' @param x A numeric vector.
#' @param i An integer specifying the location in R speak, where indexing
#'   starts at one, of the vector that you wish to get information from.
#' @return
#' For `[`, the index `i` of object `x` is returned.
#' @keywords internal
#' @rdname Rcpp_ParameterVector
methods::setMethod(
  f = "[",
  signature = signature(x = "Rcpp_ParameterVector", i = "numeric"),
  definition = function(x, i) {
    return(x$get(i - 1))
  }
)

#' Get the length of an Rcpp_ParameterVector
#'
#' @param x A numeric vector.
#' @return
#' For `length()`, the length of object `x` is returned as an integer.
#' @keywords internal
#' @rdname Rcpp_ParameterVector
methods::setMethod(
  f = "length",
  signature = signature(x = "Rcpp_ParameterVector"),
  definition = function(x) {
    return(x$size())
  }
)

#' Get the sum of all entries in an Rcpp_ParameterVector
#'
#' @param x A numeric vector.
#' @return
#' For `sum()`, the sum of object `x` is returned as a numeric value.
#' @keywords internal
#' @rdname Rcpp_ParameterVector
methods::setMethod(
  f = "sum",
  signature = signature(x = "Rcpp_ParameterVector"),
  definition = function(x) {
    ret <- methods::new(Parameter)
    tmp <- 0.0
    for (i in 1:x$size()) {
      tmp <- tmp + x[i]$value
    }
    ret$value <- tmp
    return(ret)
  }
)

#' Get the dimensions of an Rcpp_ParameterVector
#'
#' @param x A numeric vector.
#' @return
#' For `dim()`, the dimensions of object `x` is returned as a single integer
#' because there is only one dimension to return for a vector.
#' @keywords internal
#' @rdname Rcpp_ParameterVector
methods::setMethod(
  f = "dim",
  signature = signature(x = "Rcpp_ParameterVector"),
  definition = function(x) {
    return(x$size())
  }
)

#' Sets methods for operators under the S4 Generic Group, Ops
#'
#' Ops include Arith (`+`, `-`, `*`, `^`, `%%`, `%/%`, and `/`);
#' Compare (`==`, `>`, `<`, `!=`, `<=`, and `>=`); and
#' Logic (`&`, `|`).
#'
#' @param e1,e2 An Rcpp_Parameter or Rcpp_ParameterVector class object or a
#'   numeric vector or value.
#' @return
#' A numeric or logical value(s) depending on the generic and the length of
#' the input values.
#' @keywords internal
#' @export
#' @rdname Rcpp_Math
methods::setMethod(
  "Ops",
  signature(e1 = "Rcpp_Parameter", e2 = "Rcpp_Parameter"),
  function(e1, e2) {
    ret <- methods::new(Parameter)
    ret$value <- methods::callGeneric(e1$value, e2$value)
  }
)

#' @rdname Rcpp_Math
methods::setMethod(
  "Ops",
  signature(e1 = "Rcpp_Parameter", e2 = "numeric"),
  function(e1, e2) {
    if (length(e2) != 1) {
      stop("Call to operator Ops, value not scalar")
    }
    ret <- methods::new(Parameter)
    ret$value <- methods::callGeneric(e1$value, e2)
  }
)

#' @rdname Rcpp_Math
methods::setMethod(
  "Ops", signature(e1 = "numeric", e2 = "Rcpp_Parameter"),
  function(e1, e2) {
    if (length(e1) != 1) {
      stop("Call to operator Ops, value not scalar")
    }
    ret <- methods::new(Parameter)
    ret$value <- methods::callGeneric(e1, e2$value)
  }
)

#' @rdname Rcpp_Math
methods::setMethod(
  "Ops",
  signature(e1 = "Rcpp_ParameterVector", e2 = "Rcpp_ParameterVector"),
  function(e1, e2) {
    if (e1$size() != e2$size()) {
      stop("Call to operator Ops, vectors not equal length")
    }
    ret <- methods::new(ParameterVector, e1$size())
    for (i in 1:e1$size()) {
      ret[i]$value <- methods::callGeneric(e1[i]$value, e2[i]$value)
    }
    return(ret)
  }
)

#' @rdname Rcpp_Math
methods::setMethod(
  "Ops",
  signature(e1 = "Rcpp_ParameterVector", e2 = "numeric"),
  function(e1, e2) {
    if (e1$size() != length(e2)) {
      if (length(e2) == 1) {
        ret <- methods::new(ParameterVector, e1$size())
        for (i in 1:e1$size()) {
          ret[i]$value <- methods::callGeneric(e1[i]$value, e2)
        }
        return(ret)
      }
      stop("Call to Ops, vectors not equal length")
    }
    ret <- methods::new(ParameterVector, e1$size())
    for (i in 1:e1$size()) {
      ret[i]$value <- methods::callGeneric(e1[i]$value, e2[i])
    }
    return(ret)
  }
)

#' @rdname Rcpp_Math
methods::setMethod(
  "Ops",
  signature(e1 = "numeric", e2 = "Rcpp_ParameterVector"),
  function(e1, e2) {
    if (length(e1) != e2$size()) {
      if (length(e1) == 1) {
        ret <- methods::new(ParameterVector, e2$size())
        for (i in 1:e2$size()) {
          ret[i]$value <- methods::callGeneric(e1, e2[i]$value)
        }
        return(ret)
      }
      stop("Call to operator, vectors not equal length")
    }
    ret <- methods::new(ParameterVector, e2$size())
    for (i in 1:e2$size()) {
      ret[i]$value <- methods::callGeneric(e1[i], e2[i]$value)
    }
    return(ret)
  }
)

#' Sets methods for math functions for Rcpp_ParameterVector
#'
#' Methods of mathematical functions include trigonometry functions, `abs`,
#' `sign`, `sqrt`, `ceiling`, `floor`, `trunc`, `cummax`, `cumprod`, `cumsum`,
#' `log`, `log10`, `log2`, `log1p`, `exp`, `expm1`, `gamma`, `lgamma`,
#' `digamma`, and `trigamma`.
#'
#' @param x An Rcpp_ParameterVector class object.
#' @return
#' A vector of numeric values.
#' @keywords internal
#' @export
#' @rdname Rcpp_Math
methods::setMethod(
  "Math",
  signature(x = "Rcpp_ParameterVector"),
  function(x) {
    xx <- methods::new(ParameterVector, x$size())
    for (i in 1:x$size()) {
      xx[i]$value <- methods::callGeneric(x[i]$value)
    }
    return(xx)
  }
)

#' Set methods for summary functions with an Rcpp_ParameterVector
#'
#' Methods of summary functions include `max`, `min`, `range`, `prod`, `sum`,
#' `any`, and `all`.
#'
#' @param x An Rcpp_ParameterVector class object.
#' @return
#' `Summary` returns a single or two numeric or logical values.
#' @export
#' @keywords internal
#' @rdname Rcpp_ParameterVector
methods::setMethod(
  "Summary",
  signature(x = "Rcpp_ParameterVector"),
  function(x) {
    xx <- methods::new(ParameterVector, x$size())
    for (i in 1:x$size()) {
      xx[i]$value <- methods::callGeneric(x[i]$value)
    }
    return(xx)
  }
)

# TODO: update zzz.R to set methods for RealVector that handle indexing
# translation (e.g., R uses 1-based indexing, C++ uses 0-based indexing).
