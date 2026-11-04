# Full random-effects fit profile

This exploratory profile uses `data_big`, default parameter setup, and a complete
`fit_fims()` call with default optimizer restarts and uncertainty reports.
The FIMS source includes the composition-validation optimization based on
`efb6c32e`. No TMB changes are involved.

The measured run has 49 fixed and 29 random parameters. Optimizer code is zero,
the maximum absolute gradient is 0.000390, and the reported Hessian is positive
definite. The objective is 3231.28735372779.

| Measurement | Result |
| --- | ---: |
| Complete fit call | 17.90 s |
| Optimization, as recorded by FIMS | 8.95 s |
| Uncertainty reporting, as recorded by FIMS | 8.87 s |
| Extract estimates and ordinary reports | 0.66 s |
| Whole-process wall time | 19.91 s |
| Whole-process peak RSS | 931.3 MiB |

R CPU sampling attributes approximately 52% of fit samples to `TMB::sdreport`.
The largest self-time category is `.Call`; R profiling cannot identify the
individual C++ routines within it. These are measurements of one complete run,
not an optimization comparison. CPU profiling adds overhead. Peak RSS includes
startup, preparation, fitting, and output extraction; it cannot attribute the
memory peak to uncertainty reporting alone.

The initial profiling run produced matching fitted parameters, gradients,
reports, and estimates within `1e-10`, but the macOS system timer failed to read
resource statistics under the sandbox. The measured rerun uses `wait4`, matching
the method used by the other benchmarks.

The next useful investigation is phase-specific memory measurement and testing
FIMS's existing selective uncertainty reporting on this complete Laplace fit.
This can distinguish unnecessary derived-quantity uncertainty work from costs
that remain necessary for the requested outputs, without modifying TMB.

```sh
python3 tests/benchmarks/random-effects-profile/run.py \
  /path/to/FIMS-library /tmp/new-full-fit-profile
```

The runner limits BLAS/OpenMP threads to one, records process peak RSS, and saves
R CPU profiles, fit metrics, and numerical outputs. Raw files are under
`/private/tmp/fims-validation/full-fit-measured`. See [metrics](results/metrics.csv),
[CPU profile](results/fit-cpu.csv), and [process measurements](results/process.csv).
