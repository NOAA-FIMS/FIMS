# Native and Rcpp Interface Comparison

## Interface design

The two scripts configure and optimize the same FIMS model, but they construct
it through different interfaces:

- `tmb_call.R` uses the native `.Call` interface. Model components are created
  with functions such as `fleet_create()` and `population_create()`, and the
  underlying FIMS state is accessed directly with registered native routines.
- `tmb_Rcpp.R` uses the Rcpp module interface. Model components are represented
  by R reference objects created with `methods::new()`, and their fields and
  methods are populated individually before `CreateTMBModel()` is called.

Both scripts pass the resulting fixed and random parameters to
`TMB::MakeADFun()` and optimize the objective with `stats::nlminb()`.

## Optimization results

| Metric | Native | Rcpp | Rcpp difference |
|---|---:|---:|---:|
| Final objective | 3191.954 | 3191.954 | No displayed difference |
| Convergence code | 0 | 0 | Same |
| Convergence message | Relative convergence (4) | Relative convergence (4) | Same |
| Iterations | 288 | 294 | +6 (+2.1%) |
| Function evaluations | 407 | 430 | +23 (+5.7%) |
| Gradient evaluations | 289 | 295 | +6 (+2.1%) |

The interfaces reach the same displayed objective and successful convergence
status. The Rcpp run takes slightly more optimizer iterations and evaluations,
which suggests a small difference in the model or parameter values presented to
the optimizer, despite the equivalent final objective.

## Runtime and memory

| Metric | Native | Rcpp | Rcpp difference |
|---|---:|---:|---:|
| Real time | 10.77 s | 30.86 s | +20.09 s (2.87x) |
| User CPU time | 10.33 s | 30.32 s | +19.99 s (2.94x) |
| System CPU time | 0.35 s | 0.46 s | +0.11 s (1.31x) |
| Maximum resident set size | 476,135,424 B | 478,396,416 B | +2,260,992 B (+0.5%) |
| Peak memory footprint | 313,837,696 B | 340,674,624 B | +26,836,928 B (+8.6%) |
| Instructions retired | 127,527,999,320 | 438,770,328,675 | 3.44x |
| CPU cycles elapsed | 42,906,643,990 | 117,984,772,040 | 2.75x |

In this run, the native interface is about 2.9 times faster in wall-clock time
and user CPU time. Maximum resident memory is nearly identical, although the
reported peak memory footprint is about 8.6% higher for Rcpp. The much larger
instruction and cycle counts show that the Rcpp run performs substantially more
CPU work; the modest increase in optimizer evaluations explains only part of
the runtime difference.

## Interpretation

The benchmark indicates that both paths produce equivalent optimization results,
while the native path is substantially faster for this case. The measurements
do not by themselves isolate the cost of the interface. They include model
construction, TMB objective evaluation, optimization, R startup, and any
one-time initialization performed by either path. The difference in optimizer
evaluation counts also means this is not a comparison of an identical number of
objective calls.

For a more reliable performance conclusion, run each script multiple times in
fresh R processes, verify that their initial parameter vectors and objectives
are identical at full precision, and report medians and variability. Separately
timing model construction and repeated objective/gradient evaluations would help
identify whether the extra Rcpp cost occurs during setup or inside optimization.
