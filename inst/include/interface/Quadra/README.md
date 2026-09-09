# Quadra

Quadra is an experimental mixed-effects inference framework focused on:

- reverse-mode automatic differentiation
- Laplace approximation
- sparse Hessian methods
- implicit differentiation
- scalable scientific inference workflows

The project is oriented toward computational statistics, state-space models,
and large-scale scientific modeling applications such as fisheries stock
assessment.

Current development emphasizes:

- transparent numerical workflows
- reproducible benchmarking
- sparse linear algebra performance
- modular inference architecture
- computational profiling and scaling analysis

---

# Core Capabilities

Current implemented capabilities include:

- reverse-mode automatic differentiation
- exact and approximate gradients
- Laplace approximation
- random effects optimization
- sparse Hessian construction
- sparse factorization reuse
- implicit derivative calculations
- profiled derived quantity uncertainty
- delta-method utilities
- benchmark and scaling infrastructure
- benchmark normalization and plotting
- CI benchmark workflows

---

# Repository Structure

```text
core/
    autodiff/
    laplace/
    inference/
    model/
    optimizer/

examples/
    simple/
    big/

benchmarks/
    analysis/
    comparisons/
    outputs/
    normalized/

tests/
```

---

# Building

## Requirements

Recommended:

- C++17 compiler
- Eigen
- LBFGSpp
- GNU Make

Optional:

- R
- TMB

## Build benchmarks and examples

```bash
make
```

---

# Running Tests

Run all tests:

```bash
make test
```

Run selected tests:

```bash
make test-laplace-implicit-workspace
make test-laplace-profiled-derived-report
make test-random-intercept
```

---

# Running Examples

## Simple random intercept example

```bash
./examples/simple/random_intercept_model
```

## Catch-at-age Laplace example

```bash
./examples/big/catch_at_age_laplace
```

---

# Benchmarking

## Random intercept scaling

```bash
make benchmark-random-intercept
```

## State-space scaling

```bash
make benchmark-state-space
```

## Quadra vs TMB comparison scaffold

FIMS projection scaling benchmark:

```bash
Rscript benchmarks/compare-tmb-quadra-projections.R \
  50 0,5,10,25 100
```

The arguments are samples per horizon, comma-separated projection horizons,
and warmed replays per sample. The benchmark compares matched TMB and Quadra
joint-objective replay, verifies objective parity, and reports Quadra AD-graph
vertices and tracked reserved memory. Model construction, gradients,
optimization, Laplace profiling, and reporting are excluded from both timing
paths. A `quadra_speed_ratio` greater than one means Quadra is faster.

Catch-at-age benchmark without projection years:

```bash
Rscript benchmarks/compare-tmb-quadra-without-wrappers.R 100
```

---

# Benchmark Analysis

Normalize benchmark outputs:

```bash
make benchmark-normalize-all
```

Generate scaling plots:

```bash
make benchmark-plot-random-intercept
```

---

# Continuous Integration

GitHub Actions workflows currently provide:

- contract compilation checks
- benchmark execution
- RSS logging
- benchmark normalization
- benchmark artifact uploads
- scaling plot generation

---

# Current V1 Focus

The current V1 effort is focused on:

- stable mixed-effects workflows
- reusable sparse factorization infrastructure
- implicit differentiation utilities
- scalable state-space inference
- reproducible benchmark infrastructure
- comparative inference benchmarking

---

# Project Status

Quadra is currently experimental and under active development.

Interfaces, APIs, benchmark structure, and numerical methods may evolve rapidly
during the V1 development cycle.

<!-- QUADRA_BENCHMARK_STUDIES_START -->

## Benchmark studies

- The FIMS projection benchmark is maintained at
  `benchmarks/compare-tmb-quadra-projections.R` and uses the package's current
  catch-at-age projection fixture.

<!-- QUADRA_BENCHMARK_STUDIES_END -->

<!-- QUADRA_BENCHMARKS_START -->

## Performance Highlights

Quadra is being developed around a simple idea: **model structure should be used directly by the inference engine**.

For latent-state models with diagonal, tridiagonal, or banded random-effects Hessians, Quadra can avoid generic sparse rebuilds and dispatch to specialized Newton and log-determinant routines.

The benchmarks below use a state-space surplus production model with a tridiagonal latent-state Hessian. They are intended to document scaling behavior and runtime characteristics of the current optimized Quadra path.

### State-Space LaplaceEvaluator Scaling

Warm-start `LaplaceEvaluator` runtime for increasing numbers of latent states:

| Latent states | Warm-start runtime |
|--------------:|-------------------:|
| 1,000 | 0.091 ms |
| 2,000 | 0.138 ms |
| 5,000 | 0.319 ms |
| 10,000 | 0.636 ms |
| 20,000 | 1.278 ms |
| 100,000 | 7.464 ms |

Observed scaling is approximately linear in the number of latent states. In the 10,000 to 20,000 latent-state range, runtime roughly doubled, consistent with the expected `O(n)` tridiagonal path.

### Memory Scaling

Large latent-state memory benchmark:

| Latent states | Warm-start runtime | Peak RSS |
|--------------:|-------------------:|---------:|
| 20,000 | 1.554 ms | 40.2 MB |
| 100,000 | 7.464 ms | 164.6 MB |

The 100,000 latent-state benchmark completed with peak resident memory of approximately 165 MB.

### Quadra vs TMB Fixed-Theta Runtime

Clean fixed-theta comparison using the same state-space surplus production objective values:

| Latent states | TMB fixed-theta | Quadra `LaplaceEvaluator` warm-start | Relative speed |
|--------------:|----------------:|-------------------------------------:|---------------:|
| 25 | 0.100 ms | 0.003800 ms | 26x |
| 50 | 0.600 ms | 0.007519 ms | 80x |
| 100 | 2.150 ms | 0.009671 ms | 222x |
| 250 | 14.800 ms | 0.029000 ms | 510x |
| 500 | 68.950 ms | 0.047725 ms | 1,445x |
| 1,000 | 343.950 ms | 0.075504 ms | 4,555x |

The objective values matched across the benchmarked problem sizes.

These results should be interpreted carefully: the comparison reflects the current benchmark setup and a model where Quadra can exploit known tridiagonal latent-state structure. The strongest conclusion is that the structure-aware Quadra path scales linearly and avoids the large overheads associated with generic sparse latent-state evaluation.

### Current Runtime Architecture

The optimized path currently includes:

- structure-aware Hessian analysis
- native tridiagonal Hessian-value construction
- tridiagonal Newton solves for latent states
- persistent random-effect state
- warm-start optimization
- direct structured-value updates
- specialized tridiagonal log-determinant evaluation
- `LaplaceEvaluator` as a reusable runtime shell

### Reproducing the Benchmarks

Representative benchmark commands:

```bash
./run_quadra_vs_tmb_runtime_suite.sh 20 25,50,100,250,500,1000

./run_state_space_laplace_scaling_benchmark.sh \
  20 1000,2000,5000,10000,20000

./run_state_space_laplace_memory_benchmark.sh \
  10 10000,20000,50000,100000
```

Benchmark outputs are written under `benchmarks/`.

<!-- QUADRA_BENCHMARKS_END -->
