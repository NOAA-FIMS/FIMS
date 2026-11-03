# Recruitment dependencies and exact deterministic plus-group lifting

This is an investigation of the **actual FIMS objective**, not a replacement
assessment model or a production fitter. The scripts generate and install an
instrumented copy of FIMS outside the working tree. No diagnostic controls are
added to the production model.

## What causes the density?

There are two long dependency paths in the current catch-at-age model:

1. Surviving members of the plus group retain contributions from earlier cohorts.
2. In the deviation coordinates, recruitment depends on earlier spawning biomass,
   which itself depends on earlier recruitment deviations.

The factorial experiments independently disable each path and compare deviations
with annual log-recruitment coordinates. **Disabling a path changes the model and
is only a diagnostic ablation.** All scenarios retain the catch, index, age and
length composition likelihoods. Fixed effects are held at their default values.
The plus-group ablation removes its carry-over survival term; it retains the
oldest age bin and the original starting settings.

Results confirm that neither changing to annual log recruitment alone nor
removing only the plus group from the deviation formulation makes the full
Hessian sparse. In the annual-log-recruitment formulation, removing the plus
group produces a banded Hessian with half-bandwidth 12 for the 12-age example.
Keeping both dependency paths produces a dense recruitment Hessian.

The existing [RE sparsity vignette](../../../vignettes/RD-re-sparsity.Rmd)
uses AR1/SAM examples. Its principle motivates this study, but directly estimating
recruitment does not remove the deterministic plus-group history in this model.
TMB's [sparsity documentation](https://kaskr.github.io/adcomp/Sparsity.html)
also explains why dependencies through observations and eliminated states matter.

## A model-preserving route

First use annual log recruitment `r_t` instead of innovations `u_t`:

```
u_t = r_t - log(mu(SB_(t-1), theta))
```

For fixed `theta`, this is a triangular change of coordinates with diagonal one:
the recruitment mean only depends on earlier years. The Jacobian determinant is
one. The joint density is unchanged after expressing each innovation density in
terms of `r`. At the conditional mode, its Hessian determinant and leading Laplace
approximation also agree. `equivalence.R` checks objective/report equivalence at
two points and the mode value, gradients and Hessian log-determinant.

Next retain scaled plus-group abundances `z_t = N_(A,t) / R0` as **deterministic
auxiliary states**, with exact constraints:

```
c_t(r, z, theta) = z_t - survival_(A,t-1) * z_(t-1)
                        - incoming_cohort_t(r, theta) / R0 = 0
```

The initial state is fixed conditionally on `theta`. Early incoming cohorts come
from initial numbers at age; later incoming cohorts depend on one earlier annual
recruitment coordinate. The current cohort transitions remain exact. The
conditional likelihood can now be evaluated locally from recent recruitment and
the retained plus-group states. No survival noise or history truncation is added.

Write the joint negative log density as `f(r,z)` and the constraint Jacobian as
`J = [C_r, C_z]`. The state block `C_z` is unit lower triangular. At feasible states,
compute adjoint multipliers and a Lagrangian Hessian:

```
lambda = -solve(t(C_z), gradient_z(f))
W      = Hessian_(r,z)(f + t(lambda) * c)   # lambda held fixed in this derivative
K      = [ W       t(J) ]
         [ J         0 ]
```

`K` is sparse and indefinite. Eliminating the auxiliary-state and multiplier
blocks recovers the original dense recruitment Hessian `H`. In particular,

```
T = [ I ; -solve(C_z, C_r) ]
H = t(T) * W * T
det(K) = (-1)^n_states * det(C_z)^2 * det(H)
```

Since `det(C_z) = 1`, `log(abs(det(K))) = log(det(H))` at a positive-definite
conditional mode. A solve of `K` with right-hand side `[b, 0, 0]` recovers
`solve(H, b)` in the recruitment block. The prototype verifies these identities,
including the curvature contributed by the constraint multipliers. Merely using
the unconstrained likelihood Hessian would omit that curvature.

The dense tangent matrix `T` is formed **only for validation** at small sizes,
never in the timed sparse path. Constraints are exact, rather than tiny-variance
Gaussian penalties. The Laplace normalization still uses the original number of
recruitment random effects, not the larger augmented dimension.

## Measurements and limits

Measured warm Hessian evaluation plus fresh factorization:

| Years | Dense formulation (ms) | Exact lifting (ms) | Component speedup |
| ---: | ---: | ---: | ---: |
| 30 | 3.33 | 2.46 | 1.35x |
| 60 | 14.00 | 5.78 | 2.42x |
| 120 | 56.56 | 12.31 | 4.59x |
| 240 | 213.92 | 23.38 | 9.15x |

At 240 years, the stored lower-triangle Hessian entries decrease from 28,680
to 5,853, although the constrained solve uses a larger augmented matrix.
All 32 dependency ablations converged with positive-definite conditional
Hessians. Exact-lifting checks at all four sizes agree on objective values,
reports, gradients, constraint Jacobians, solves and log-determinants within
`1e-5`; observed errors are below `3e-12`. Explicit dense Hessian reconstruction
was also checked through 120 years (relative error below `7e-16`).
The coordinate-change mode log-determinants agree within `7e-12`.
With instrumentation disabled, the diagnostic build matches the production
reference objective, mode, Hessian and reports at tolerance `1e-10`.

See [results/ablation.csv](results/ablation.csv),
[results/lifted.csv](results/lifted.csv), and the figures:

- [Dependency ablations](results/ablation-sparsity.svg)
- [Exact lifting sparsity](results/exact-lifting-sparsity.svg)
- [Component scaling](results/component-scaling.svg)

The scaling workloads repeat the observed 30-year `data_big` series to 60, 120
and 240 years, retaining 12 ages. These are synthetic computational workloads,
not additional observations for a scientific assessment. Each case runs in a
fresh process with BLAS/OpenMP thread limits of one.

Warm derivative evaluations alternate a small perturbation of one input, to
avoid counting reuse of an unchanged evaluation as computation. Timings use CPU
time and the median of three adaptively sized batches. Factorization timings
clear cached factors and include fresh ordering/factorization. They compare
Cholesky of the dense recruitment Hessian with sparse LU of the augmented system.

**The timed component is Hessian evaluation plus factorization, not a complete
Laplace evaluation or model fit.** It excludes state/adjoint construction,
constraint assembly, solver iterations, differentiation with respect to fixed
effects, uncertainty reporting and graph recording. In this proof of concept,
both the dense and lifted comparison use annual-log-recruitment coordinates,
with the original plus group and stock-recruitment feedback retained. The
adjoints are data constants when recording the Lagrangian tape. A production
implementation must make them reusable tape inputs rather than retape at each
iteration. Fixed effects are held constant here; outer Laplace gradients have
not been implemented or benchmarked.

Matrix entry counts are not measurements of total peak memory. The lifted system
has more rows; at small sizes its LU factors can require more storage and time
than dense-Hessian Cholesky. The derivative graph can still be substantially
cheaper. End-to-end memory and speed require a completed solver implementation.

The experiments cover one population and recruitment-only random effects, with
all modeled recruitment years random. Additional random effects, partial maps,
multiple populations, shared recruitment objects and alternative growth or
likelihood families need explicit coverage before production use.

## Recommended implementation sequence

1. Separate catch-at-age state propagation, local likelihood contributions and
   state reporting. Provide explicit annual recruitment/plus-state inputs to
   the local evaluator, rather than mutating registered parameter vectors as the
   isolated diagnostic does. Use typed storage for these states.
2. Implement and validate the annual-log-recruitment coordinate option against
   the current deviations model, including mappings and outer gradients. Do
   not assume this step alone resolves plus-group density.
3. Add an experimental constrained inner solver: propagate feasible states,
   compute adjoints, assemble sparse curvature, solve the augmented Newton
   system, and use a line search/damping while maintaining feasibility. Keep
   adjoints as tape inputs so the local derivative graph can be reused.
4. Implement the corresponding log-determinant and implicit derivatives for
   the outer Laplace objective, including derivatives of the adjoints and
   states. Handle the indefinite system with an appropriate sparse solver.
   Do not hand unconstrained auxiliary states to TMB's ordinary random-effect
   integration: that would be a different model.
5. Validate marginal objectives, fixed-effect gradients, conditional modes,
   covariance/reported uncertainty and failure handling. Benchmark full fits
   and peak memory across years, ages, fleets and supported random-effect
   structures. Keep the existing solver as the reference/fallback until then.

## Reproduce

Requires the package's R dependencies, `svglite` and `ragg` for the figures,
a working compiler, and Python 3.12+.
Run from the repository root. Choose a new scratch directory each time.

```sh
study=$(mktemp -d "${TMPDIR:-/tmp}/fims-re-dependencies.XXXXXX")
python3 tests/benchmarks/recruitment-dependencies/prepare_diagnostic.py . "$study/source"
mkdir -p "$study/library"
R CMD INSTALL --library="$study/library" "$study/source"
python3 tests/benchmarks/recruitment-dependencies/run_experiments.py \
  "$study/library" "$study/results"
R_LIBS="$study/library" Rscript tests/benchmarks/recruitment-dependencies/equivalence.R \
  "$study/equivalence.rds"
Rscript tests/benchmarks/recruitment-dependencies/summarize.R \
  "$study/results" tests/benchmarks/recruitment-dependencies/results
```

The recorded run used Apple clang with the macOS SDK (the same compiler setup
as the preceding suite benchmark), R 4.4.3, TMB 1.9.20 and an Apple M3 Pro.
The diagnostic library has four environment controls used by the scripts:
`FIMS_DIAG_CENTERED`, `FIMS_DIAG_NO_PLUS`, `FIMS_DIAG_NO_FEEDBACK`, and
`FIMS_DIAG_LIFT_PLUS`. They exist only in the generated source copy. **Do not use
that installed copy for assessment inference.**
