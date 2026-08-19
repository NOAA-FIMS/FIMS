# Parameter Mapping Implementation

## Purpose

FIMS supports two different parameter-mapping mechanisms:

1. A TMB factor map supplied to `TMB::MakeADFun(map = ...)`.
2. An optional native FIMS `map_to()` implementation for backends that need
   parameter aliasing inside the FIMS model.

The normal TMB build uses the TMB map. Native `map_to()` is compiled and
exposed only when `FIMS_ENABLE_NATIVE_MAP_TO` is defined at compile time.

This separation prevents the native mapping implementation from changing the
parameter vector before TMB applies its own mapping rules.

## Why TMB maps require special name handling

`get_parameter_names()` returns names for the full FIMS fixed-effect vector.
A TMB map can produce a shorter optimization vector because:

- `NA` map entries are held fixed and omitted from `obj$par`.
- Entries with the same factor level share one optimized value.
- TMB returns one parameter per active factor level.

Consequently, assigning all names from `get_parameter_names()` directly to
`obj$par` can produce an indexing mismatch.

For example:

```r
full_names <- c("alpha", "beta", "gamma", "delta")
map <- factor(c(1, 1, NA, 2))
```

The resulting TMB optimization vector has two elements:

| Full parameter | Map entry | TMB behavior |
| --- | ---: | --- |
| `alpha` | 1 | Optimized as the representative of level 1 |
| `beta` | 1 | Shares the level 1 value |
| `gamma` | `NA` | Held fixed and omitted |
| `delta` | 2 | Optimized as level 2 |

The mapped names are therefore `c("alpha", "delta")`.

## TMB implementation

### Reducing the names

The internal `tmb_mapped_parameter_names()` helper in `R/fimsfit.R` applies
the same reduction to the FIMS names that TMB applies to the parameter vector.

```r
tmb_mapped_parameter_names <- function(parameter_names, map = NULL) {
  if (is.null(map)) {
    return(parameter_names)
  }
  if (!is.factor(map)) {
    cli::cli_abort("A TMB parameter map must be a factor.")
  }
  if (length(parameter_names) != length(map)) {
    cli::cli_abort(
      "A TMB parameter map must have one entry per FIMS parameter."
    )
  }

  keep <- !is.na(map)
  active_map <- droplevels(map[keep])
  active_names <- parameter_names[keep]

  vapply(levels(active_map), function(level) {
    active_names[as.character(active_map) == level][[1L]]
  }, character(1L), USE.NAMES = FALSE)
}
```

The first FIMS name associated with each factor level is used as its canonical
name. The helper validates that the map is a factor and that it indexes the
full FIMS parameter vector.

`FIMSFit()` obtains the names from the original input vector rather than from
the already-reduced `obj$par`:

```r
full_parameter_names <- names(get_parameter_names(
  as.list(input[["parameters"]][["p"]])
))

parameter_names <- tmb_mapped_parameter_names(
  full_parameter_names,
  input[["map"]][["p"]]
)

names(obj[["par"]]) <- parameter_names
```

An additional length check stops construction if the number of mapped names
does not equal `length(obj$par)`.

### Expanding optimized values back into FIMS

An optimizer returns TMB's reduced parameter vector. Passing that vector
directly to `set_fixed()` would incorrectly index the full FIMS fixed-effect
vector. `fit_fims()` now asks TMB to expand it first:

```r
full_parameters <- obj[["env"]]$parList(opt[["par"]])[["p"]]
FIMS::set_fixed(full_parameters)
```

`parList()` restores parameters omitted with `NA` and duplicates values for
parameters that share a factor level. Without a TMB map, this operation leaves
the parameter layout unchanged.

### Reshaping estimates

`reshape_tmb_estimates()` now uses `obj$par` for initial fixed-effect values
and `opt$par` for optimized values. These vectors have the same reduced
indexing as the mapped parameter names.

This avoids mixing the full vector in `obj$env$parameters$p` with the reduced
rows returned by TMB.

## Using a TMB map with FIMS

Add the map to the initialized FIMS input before calling `fit_fims()`:

```r
input <- initialize_fims(parameters, data)

# Begin with one independent level per fixed effect.
tmb_map <- seq_along(input$parameters$p)

# Share the first two parameters and hold the third parameter fixed.
tmb_map[2] <- tmb_map[1]
tmb_map[3] <- NA_integer_

input$map <- list(
  p = factor(tmb_map)
)

fit <- fit_fims(input)
```

For a programmatically constructed map, it is useful to verify its size before
fitting:

```r
stopifnot(length(input$map$p) == length(input$parameters$p))
```

TMB map semantics are:

- different factor levels: independently optimized parameters;
- repeated factor levels: shared parameter value;
- `NA`: fixed at the supplied initial value.

## Optional native `map_to()` implementation

Native mapping is guarded everywhere by:

```cpp
#ifdef FIMS_ENABLE_NATIVE_MAP_TO
// Native mapping implementation
#endif
```

The guarded implementation includes:

- mapping storage and validation in
  `inst/include/common/information.hpp`;
- mapping preparation during `CreateTMBModel()` in
  `inst/include/interface/rcpp/rcpp_interface.hpp`;
- copying source values to mapped targets during objective evaluation in
  `src/FIMS.cpp`;
- Rcpp registration in `src/rcpp_functions.cpp`.

When enabled, native `map_to(target_id, source_id)`:

1. Validates that the target and source are different.
2. Requires both `VariableVector` objects to have the same length.
3. Rejects mappings between fixed and random effects.
4. Rejects multiple sources for one target.
5. Rejects chained mappings.
6. Removes the target from the independently estimated parameter vector.
7. Copies the source value into the target during objective evaluation.

### Enabling native mapping

For a backend build that supports native mapping, define the macro while
installing FIMS. For example:

```sh
PKG_CPPFLAGS="-DFIMS_ENABLE_NATIVE_MAP_TO" R CMD INSTALL .
```

A backend-specific build system can define the same macro in its Makevars or
compiler configuration.

During package loading, `R/zzz.R` checks whether the compiled Rcpp module
contains `map_to`. If present, it dynamically exports the function:

```r
if (exists("map_to", envir = asNamespace(pkgname), inherits = FALSE)) {
  namespaceExport(asNamespace(pkgname), "map_to")
}
```

Therefore:

- a normal TMB build does not contain or expose `map_to()`;
- a build with `FIMS_ENABLE_NATIVE_MAP_TO` contains and exposes `map_to()`.

## Files changed

| File | Responsibility |
| --- | --- |
| `R/fimsfit.R` | Reduces names with the TMB map and expands optimized values before `set_fixed()` |
| `R/reshape_output.R` | Keeps estimate rows aligned with TMB's reduced indexing |
| `R/zzz.R` | Exports native `map_to()` only when compiled into the Rcpp module |
| `inst/include/common/information.hpp` | Stores, validates, and applies native mappings when enabled |
| `inst/include/interface/rcpp/rcpp_interface.hpp` | Prepares, clears, and exposes native mappings when enabled |
| `src/FIMS.cpp` | Applies native aliases during objective evaluation when enabled |
| `src/rcpp_functions.cpp` | Conditionally registers `map_to()` with Rcpp |
| `tests/testthat/test-tmb-map-names.R` | Tests TMB name reduction and `MakeADFun()` indexing |
| `tests/testthat/test-map-to.R` | Tests native mapping when that capability is compiled in |

## Test coverage

The TMB tests cover:

- unchanged names when no map is supplied;
- omission of `NA` entries;
- collapsing repeated factor levels;
- rejection of non-factor maps;
- rejection of incorrectly sized maps;
- agreement between mapped names and the vector produced by
  `TMB::MakeADFun()`.

The native tests automatically skip when `map_to()` was not compiled into the
package. When enabled, they cover fixed-effect aliases, random-effect aliases,
self-mapping, and mismatched vector lengths.

Both macro-disabled and macro-enabled C++ paths should be compiled in CI. The
normal R test suite should run against the TMB-only build, with an additional
backend job compiled using `FIMS_ENABLE_NATIVE_MAP_TO` for the native tests.
