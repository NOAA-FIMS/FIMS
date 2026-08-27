# Modeled Years Vector

FIMS populations now store the calendar years represented by annual vector
indices. This provides a direct mapping between an annual index and its actual
model year, similar to the existing mapping between age indices and the
population `ages` vector.

## Indexing contract

For a population with `n_years` modeled years:

- `years` has length `n_years`.
- `years[i]` identifies the calendar year represented by annual index `i`.
- A year-by-age vector remains flattened in year-major order:

  ```text
  index = year_index * n_ages + age_index
  ```

- Vectors with `n_years + 1` entries contain an additional terminal state. The
  extra entry corresponds to the year after `years[n_years - 1]`; it is not
  included in the population's `years` vector.

## Using a `FIMSFrame`

`get_years()` returns the complete sequence of modeled calendar years:

```r
fims_data <- FIMSFrame(data_big)

get_start_year(fims_data)
get_end_year(fims_data)
get_years(fims_data)

stopifnot(length(get_years(fims_data)) == get_n_years(fims_data))
```

When modules are initialized through the standard FIMS workflow, the
population year vector is populated automatically:

```r
parameters <- setup_default_parameters(data = fims_data)
initialized <- initialize_fims(parameters = parameters, data = fims_data)

# The population created internally uses get_years(fims_data).
```

## Using the population interface directly

When constructing a population directly, supply one year for each annual
index:

```r
population <- methods::new(Population)
population$n_years$set(3)
population$years[] <- 2001:2003

population$years[]
#> [1] 2001 2002 2003
```

### Omitted years

For backward compatibility, an empty year vector is allowed. During model
creation, FIMS fills it with sequential indices from `1` through `n_years` and
records a warning:

```r
population <- methods::new(Population)
population$n_years$set(3)

# After the population is transferred into the model:
population$years[]
#> [1] 1 2 3
```

These fallback values are indices, not inferred calendar years. Supply
calendar years explicitly whenever they are known.

A non-empty vector whose length differs from `n_years` is an error:

```r
population$n_years$set(3)
population$years[] <- c(2001, 2002)

# Model creation fails because length(years) != n_years.
```

An empty vector remains valid when `n_years` is zero.

## JSON output

The finalized year vector is serialized as population-level metadata:

```json
{
  "module_name": "Population",
  "population": "NA",
  "module_id": 1,
  "years": [2001.0000000000, 2002.0000000000, 2003.0000000000],
  "recruitment_id": 1,
  "growth_id": 1,
  "maturity_id": 1
}
```

Annual parameters and derived quantities continue to use dimensionality names
such as `n_years`, `n_years-1`, and `n_years+1`. Consumers can use the
population `years` metadata to translate internal year-dimension indices into
the `timing` values returned by `get_estimates()`.

For example, given `years = c(2001, 2002, 2003)`:

| Dimension | Internal indices | `timing` values |
|---|---:|---:|
| `n_years` | 1, 2, 3 | 2001, 2002, 2003 |
| `n_years-1` | 2, 3 | 2002, 2003 |
| `n_years+1` | 1, 2, 3, 4 | 2001, 2002, 2003, 2004 |
