# Observation timing benchmark

Measured September 18, 2026 on the same arm64 macOS host with Apple clang,
C++17, and `-O0 -g0` for both milestone 4 and milestone 5 package builds.
Run `observation-timing.R` against each installed library to repeat the comparison.

Both scenarios use three annual years, 12 age classes, parametric growth, and
24 additional dates shared by two survey fleets. Setup includes initialization
and the initial model report. Reporting time is the median of three calls;
objective-plus-gradient time is the median of three batches of 20 calls.

| Scenario | MS4 setup (s) | MS5 setup (s) | MS4 report (s) | MS5 report (s) | MS5 size rows |
|---|---:|---:|---:|---:|---:|
| Index only | 3.929 | 3.071 | 0.437 | 0.340 | 0 |
| Index and length, shared dates | 9.794 | 10.015 | 1.088 | 1.109 | 288 |

Index-only setup and reporting were about 22% faster in this run. The shared
length workload was approximately unchanged (about 2% slower, within the scale
of run-to-run variation). Setup is a single measurement and the processes ran
with other local verification work, so these are indicative timings, not a
statistical performance claim or a cross-platform guarantee.

Objective-plus-gradient calls were sub-millisecond in both builds; the timer
resolution does not support a strong comparison. Objective values agreed within
1e-10 for both scenarios. The deterministic work counts are the regression
criterion: index-only dates build no size rows, and two fleets sharing 24
length-sample dates build 24 * 12 = 288 biological size rows, rather than one
set per fleet. Products are released after each date; no daily grid is allocated.
