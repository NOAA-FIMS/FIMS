# Parameter Mapping Implementation

## Purpose

FIMS parameter mapping is implemented through a TMB factor map supplied to
`TMB::MakeADFun(map = ...)`. FIMS does not maintain a separate native mapping
system, so TMB is the single source of truth for fixing and sharing estimated
parameters.

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

## Files changed

| File | Responsibility |
| --- | --- |
| `R/fimsfit.R` | Reduces names with the TMB map and expands optimized values before `set_fixed()` |
| `R/reshape_output.R` | Keeps estimate rows aligned with TMB's reduced indexing |
| `tests/testthat/test-tmb-map-names.R` | Tests TMB name reduction and `MakeADFun()` indexing |

## Test coverage

The TMB tests cover:

- unchanged names when no map is supplied;
- omission of `NA` entries;
- collapsing repeated factor levels;
- rejection of non-factor maps;
- rejection of incorrectly sized maps;
- agreement between mapped names and the vector produced by
  `TMB::MakeADFun()`.

The normal R test suite should run these cases against the TMB-backed build.
