# Timing semantics for FIMS

Status: milestones 1–5 implemented, including dated observation mappings,
within-year mortality, fractional-age biology, and workflow validation.
Updated: 2026-09-18. Implementation notes below record each milestone;
staged limitations in milestones 2 and 3 were resolved in later milestones.

This specification translates the September 15 summary in `FIMS Notes
Timing.docx` into concrete defaults for development. The notes require irregular
observation times, mortality adjustments, time-aware growth and maturity, date
defaults, and duplicate-observation errors. Calendar conventions and biological
policies below are proposed decisions, not previously agreed meeting outcomes.

## 1. Scope and delivery boundary

The first timing release will support fixed, day-resolution survey dates,
including multiple samples from one fleet within a year. Survey indices and
survey age/length compositions will use abundance and applicable biological
products at their observation times. Users will continue to supply annual catch
and annual fishery compositions. Model population states, mortality parameters,
cohort advancement, recruitment, and stock-recruit calculations remain annual.

The first implementation slice is irregular survey indices with analytic
mortality. Time-aware survey compositions, growth, size conversion, and maturity
complete the release; they are not optional omissions from that release.

The timing infrastructure must distinguish observation points, intervals, and
state transitions. It need not execute arbitrary state transitions yet. Timed
catch, dated fishery compositions, user-selected spawning/recruitment dates,
multiple recruitment pulses, movement, seasonal selectivity/mortality,
estimated dates, and subdaily timestamps are later work. No daily or monthly
population-state array or numerical ODE solver is required.

## 2. Input and normalized time

Accept integer model years, R `Date` values, and unambiguous ISO strings
`YYYY`, `YYYY-MM`, or `YYYY-MM-DD` through the R normalization layer. Numeric
input continues to mean an integer year (including existing relative years such
as 1 through 30), never an R date serial or decimal year. Calendar input uses
the Gregorian calendar, with supported years 1 through 9999. Relative-year
input uses the same year numbering convention; users must explicitly relabel
relative years before combining them with modern calendar dates.

For point observations, a missing month means January; a missing day means the
first day of that month. There is no implicit midpoint. Preserve input precision
(`year`, `month`, or `day`) so defaulted dates can be identified in reports.
Reject ambiguous locale strings, fractional numeric years, impossible dates,
timestamps, and missing timing on observation records. Existing untimed metadata
rows are not observation records and may retain missing timing.

Normalize each date to a strongly typed integer day relative to the model epoch
(January 1 of the first modeled year). Use this day coordinate for ordering and
equality; assign a compact `time_id` for indexing. Do not group by rounded
floating-point fractions. Retain the calendar year and a zero-based model-year
index separately; neither is an observation index.

For a date in year y, define:

```text
D(y) = number of calendar days in year y (365 or 366)
d    = elapsed whole days since January 1 of y
tau  = d / D(y)
```

January 1 has `tau = 0`. December 31 has `tau = (D(y) - 1) / D(y)`.
The following January 1 is the next year's boundary, not a second encoding of
an observation in year y. A date denotes the start of that calendar day.
Durations crossing January 1 are split at year boundaries and converted using
each year's denominator. Every complete calendar year therefore has duration
one in model-year units. Annual mortality and growth coefficients keep their
per-model-year units; they do not become rates per 365 physical days.

The modeled horizon consists of complete years, including years without data.
An observation on the next January 1 requires that next modeled year. A terminal
state may be reported there without extending the observation horizon. Derive
horizon bounds from observation support, not composition bins or untimed
metadata; an annual interval's exclusive end does not add an observation year.

## 3. Point observations and interval observations

Timing normalization depends on observation support, not just the input's
appearance. The following are the first-release rules:

| Observation | Support | Meaning of year-only input |
| --- | --- | --- |
| Survey index | Point | January 1 snapshot |
| Survey age/length composition | Point | January 1 sample |
| Catch total | Interval | Entire calendar year |
| Fishery age/length composition | Interval | Composition of annual catch |

Intervals are half-open `[start, end)`. Annual catch for 2027 covers
`[2027-01-01, 2028-01-01)`. A date default must never turn this total into an
instantaneous removal. In the first release, reject month/day input for annual
catch and fishery compositions with an error explaining that only annual
support is implemented. Reserve explicit interval start/end fields in the
internal observation representation for future partial-year support.

Record the prediction basis explicitly (survey population sample or fishery
catch sample). Do not infer it from dates. For legacy inputs, resolve the current
fleet convention during normalization: compositions in a fleet with linked
catch use the fishery basis; other compositions use the survey basis. Preserve
that resolved basis downstream. A future public override can be designed
separately; mixed sampling bases must not be guessed silently.

## 4. Observation identity, partitions, and duplicates

A sample key consists of population, partition, fleet, observation type,
prediction basis, and normalized support (point date or interval endpoints).
For currently unpartitioned data, use an explicit pooled partition. This is an
identity convention, not an implementation of sex/area population dynamics;
unsupported partition configurations must fail rather than be pooled silently.

An index or catch sample contains one observed value. A composition sample
contains its complete vector of bins. Assign one `observation_id` per sample,
with bin coordinates stored separately. Distinct types on the same date are
valid; so are distinct fleets or supported partitions on that date. Distinct
dates in one year are valid for survey samples.

Reject a second scalar value with the same sample key. Within a composition,
reject repeated bin coordinates, and retain existing bin-completeness checks.
Different bins of one sample are not duplicate observations. If source sample
IDs are available, reject multiple source samples with the same sample key;
without IDs, rows sharing the key define one composition vector. Two disjoint
partial source samples cannot be detected as replicates without that metadata.
Do not average, sum, or otherwise combine replicate samples automatically.

Errors must identify the fleet, partition, type, normalized support, and affected
input rows. An arbitrary new observation ID must not bypass duplicate checks.

## 5. Population dynamics and annual boundary ordering

Within a year, natural and fishing mortality remain constant at age. Propagate
the January 1 state to an observation time using:

```text
Z[a,y]       = M[a,y] + sum_f F_at_age[f,a,y]
N[a,y,tau]   = N[a,y,0] * exp(-Z[a,y] * tau)
I_numbers   = q * sum_a(selectivity[a] * N[a,y,tau])
I_weight    = q * sum_a(selectivity[a] * N[a,y,tau] * W[a,y,tau])
```

Reuse FIMS's existing fleet mortality, selectivity, and population multiplier
definitions. Only contributing fisheries enter total fishing mortality. An
M-only decrement is appropriate only when total fishing mortality is zero.
Mortality rates themselves are not decremented. A survey reads state without
changing it. Selectivity and catchability retain their existing annual
parameter interpretation in this release.

At each January 1 after the initial year, the conceptual order is:

1. Complete the previous year's mortality and annual catch accounting.
2. Advance surviving cohorts and accumulate the oldest plus group.
3. Insert recruitment into the youngest modeled age class, using the existing
   stock-recruit relationship and lag.
4. Establish the new year's rates and biological parameter context.
5. Calculate start-of-year biological summaries and evaluate January 1 surveys.

This defines January 1 observations as post-transition observations. In the
first year, the initial abundance vector already contains the youngest class;
do not add a second recruitment pulse. Recruitment enters the youngest modeled
age, which is not necessarily biological age zero.

Annual catch numbers retain the Baranov calculation over duration one. Annual
catch weights and fishery compositions retain the current annual biological
product convention; introducing survey-time growth does not silently introduce
time-integrated catch weight. Annual spawning biomass and unfished reference
quantities remain January 1 quantities. Evaluating maturity at survey dates
does not change spawning time or the stock-recruit relationship.

Reserve explicit pre-transition and post-transition sides for future events.
Only post-transition observations are exposed initially. When same-time events
are introduced, their ordering must be specified by process phase; container
insertion order must never determine biological behavior.

## 6. Growth, maturity, and composition semantics

Keep age-class labels fixed within a year. For parametric growth and age-based
maturity, the initial representative biological age is the configured age label
and its observation-time value is `age + tau`. Integer age classes advance only
at the annual boundary. For the plus group, use `plus_group_age + tau` as an
explicit representative-age approximation; this does not resolve the mixture
of older cohorts within that group.

Evaluate parametric growth at that fractional age, including the provider's
length spread and weight products. Evaluate age-based maturity at fractional
age; any future size-based maturity must use the matching size distribution.
Do not scale growth coefficients or maturity probabilities by `tau`. Providers
must state whether they support continuous age or annual lookup.

Empirical annual weight-at-age and fixed age-to-length matrices are constant
within each year by explicit policy. Do not interpolate between years or look
up fractional ages in an integer-age table. Their resolved annual lookup policy
must be inspectable in model metadata. Parametric growth-derived size products
and age-to-length conversions must instead be evaluated at each requested date.

Survey age compositions use selected abundance at their sample date. Survey
length compositions use that abundance and the matching date's age-to-length
mapping. An index and a composition need not share a date. Biological products
can be shared across requests only when population, partition, time, provider,
and relevant configuration agree; fleet-specific bin mappings remain distinct.

## 7. Execution, differentiation, and reporting contract

Compile fixed observation times, required annual boundaries, and product
requests into a sorted execution plan during model setup. Retain the mapping
from input samples/bins to prediction positions. The plan's structure depends
only on fixed configuration, never on estimated parameters or observed values.
Dates are data, not estimable parameters, in this release.

Keep annual population storage. Evaluate intermediate abundance only at required
times and share it among compatible requests at that time. Compute expensive
growth/size products only when requested. Rebuild parameter-dependent products
for each objective evaluation (or use an explicitly validated evaluation-local
cache). Do not reuse stale AD values across objective evaluations or tapes.
Audit existing growth `needs_update_` caching when wiring this path.

Preserve annual output dimensions for annual quantities. Observation predictions
must use sample dimensions, with bin dimensions where appropriate, and report
observation ID, date or interval, input precision, year, fleet, partition, type,
and prediction basis. Sorting input must not alter prediction/data alignment.
Likelihood construction, uncertainty reporting, diagnostics, retrospective
truncation, and projections must consume these explicit mappings.

## 8. Worked examples and acceptance cases

| Input or scenario | Expected interpretation/result |
| --- | --- |
| Survey timing `2027` | `2027-01-01`, year precision, `tau = 0` |
| Survey timing `"2027-07"` | `2027-07-01`, month precision, `tau = 181/365` |
| Survey timing `"2028-07-01"` | Day precision, `tau = 182/366` |
| Survey timing `"2028-02-29"` | Valid leap day, `tau = 59/366` |
| Survey timing `"2027-02-29"` | Invalid-date error |
| Survey timing `2027.5` | Fractional-year error; never interpreted as July |
| Catch timing `2027` | Annual interval, duration one |
| Catch timing `"2027-07-01"` | Unsupported catch support error |
| Same fleet index in March and September | Two samples and two predictions |
| Same fleet index twice on September 1 | Duplicate-sample error |
| Ages 1, 2, 3 in one survey sample | One composition, three bins |
| Same age bin twice in that sample | Duplicate-bin error |
| Two fleets surveying the same population/date | Separate predictions; shared compatible state calculation |

For a one-age population with `N0 = 1000`, `M = 0.2`, total `F = 0.3`,
selectivity one, and `q = 1`, the July 1, 2027 numbers index is
`1000 * exp(-0.5 * 181/365)`, approximately `780.403`. January 1 gives 1000;
immediately before the next annual transition, abundance is approximately
606.531. Using M alone would produce the wrong July prediction.

The December 31 to January 1 interval is `1/365` model years in 2027 and
`1/366` in 2028. December 31 is not the annual endpoint. A January 1 survey in
the following year sees the aged survivors and that year's recruitment.

With all other inputs fixed, adding a survey must not change later abundance,
annual catch, or annual spawning biomass. Refitting after adding observed data
may change parameter estimates and therefore those quantities. Propagating
`0 -> 0.37 -> 1` must equal `0 -> 1` under constant mortality, within numerical
tolerance. Zero mortality must preserve abundance. Same-time compatible requests
must not multiply the number of population propagations.

## 9. Implementation map and milestone handoff

| Existing location | Required follow-up |
| --- | --- |
| `R/fimsframe.R` | Replace integer-only observation timing validation; normalize support, check sample identity, preserve sparse observations and complete bins. |
| `R/initialize_modules.R` | Allocate indices/compositions by sample count rather than `get_n_years(data)`; pass time and observation mappings. |
| `inst/include/interface/rcpp/rcpp_objects/` | Carry fixed timing metadata and sample dimensions into backend objects. |
| `inst/include/models/functors/catch_at_age.hpp` | Separate annual dynamics/catch from point predictions; update composition loops, likelihood inputs, and reporting. |
| `inst/include/population_dynamics/growth/` | Extend product requests beyond annual indexing; validate fractional-age behavior and cache lifetime. |
| `inst/include/population_dynamics/size/` and `age_to_length_conversion/` | Prepare requested-time size products and fleet mappings. |
| `inst/include/population_dynamics/maturity/` | Distinguish fractional-age evaluation from annual parameter lookup. |
| `R/run_modified_fims.R` and reporting helpers | Truncate by modeled year/support, preserve observation IDs and dates, and retain annual projection boundaries. |
| `tests/testthat/` and `tests/gtest/` | Add normalization, mapping, analytic-value, biological-product, AD-gradient, and invariance checks. |

Milestone 1 delivers this specification and its acceptance cases. The decisions
to review most closely are the calendar-year rate convention, post-transition
January 1 observations, annual empirical-product lookup, representative plus
group age, and deferral of dated fishery compositions. These defaults make the
next milestone actionable; they are not claims of scientific-team approval.

Milestone 2 should begin with an end-to-end fixture containing one annual fishery
and one survey fleet sampled twice in a year. Establish sample identity,
normalization, dimensions, and report mappings before adding mortality timing
in milestone 3. Annual reference results remain regression targets for unchanged
configurations, even though pre-1.0 API changes are permitted. Defer a general
event engine until timed catch or recruitment actually requires state changes.


## Milestone 2 implementation notes

The implementation normalizes dates in `R/observation_timing.R` and exposes
`get_observations()`. Each fleet has separate index, age-composition, and
length-composition sample mappings. Rcpp copies fixed year/day/time coordinates
and stable sample IDs into the backend. Prediction dimensions use samples;
annual process and diagnostic arrays retain annual dimensions. JSON output
contains backend sample coordinates, and `get_estimates()` and augmentation add
calendar support and precision metadata retained with the fitted input.

This is an infrastructure stage: point predictions still read their January 1
annual state. Initialization explicitly warns for non-January dates, and
reported point predictions carry `prediction_timing = "annual_state_mapping"`.
The next milestone adds analytic mortality propagation. Biological products
remain annual until milestone 4.

For compatibility with existing data/likelihood handling, wholly missing years
in an existing stream retain annual `-999` placeholders. Actual within-year
samples remain distinct, with no regular within-year grid or additional
January 1 placeholder in a year that already has a sample. Removing annual
placeholders can be considered separately from enabling irregular sample counts.
Normalized `get_data()` output contains both an integer year (`timing`) and a
Date (`date`); changing its year requires updating both to avoid stale metadata.


## Milestone 3 implementation notes

Dated point predictions now apply `exp(-Z * year_fraction)` to annual selected
abundance (or selected biomass for weight indices). `Z` includes natural and
fishing mortality. Fractions use elapsed Gregorian days divided by 365 or 366.
Age compositions are normalized after survival; length compositions map surviving
selected abundance through the annual age-to-length conversion before normalization.
Annual catch, spawning, recruitment, and population states are unchanged.

One survival vector per distinct non-January-1 date is reused across fleets and
observation streams within each evaluation. The local cache is discarded before
the next evaluation; it never retains parameter-dependent values across AD calls.
The current dated path explicitly supports one population, matching R input
validation. January 1 samples take the legacy annual path exactly.

Point estimates carry `prediction_timing = "mortality_adjusted_annual_biology"`.
Growth, empirical weights, and age-to-length products remain annual in this
milestone; fractional-age biology is milestone 4. Initialization warns about
this remaining limitation. Tests compare index and composition predictions with
independent analytic calculations, finite-difference gradients, and annual states.


## Milestone 4 implementation notes

The von Bertalanffy–Schnute adapter now accepts fractional ages. A separate
continuous-age product request returns mean length, length spread (interpolation
or delta method), and biological mean weight using the current evaluation's
parameters. It does not change the prepared annual products. The youngest-age
ramp and oldest-plus-group representative-age approximation use the existing
growth formula at `age + year_fraction`.

Dated biomass indices use biological weight at that age. Dated length
compositions map the matching population size distribution to each fleet's own
observation bins using the same normal tails, probability floor, and rebinning
rules as the annual path. Population biological products are shared by date
within each evaluation; fleet mappings remain distinct. Age-based selectivity
retains its existing annual age-class convention.

Logistic maturity advances biological age while retaining the model year's
parameter lookup. Dated maturity is diagnostic; annual spawning and recruitment
are unchanged. Empirical weights and fixed age-to-length matrices remain
constant within the year, with no interpolation or fractional table lookup.

`get_input(fit)`'s `observation_timing` attribute and observation estimate rows
include `weight_timing`, `length_mapping_timing`, and `maturity_timing` policies.
These describe provider behavior: `fractional_age`, `annual_lookup`, or
`fixed_annual`. Point estimates now carry `prediction_timing = "observation_date"`;
catch intervals retain `annual_interval`. Annual-supported observations continue
to use annual predictions regardless of the provider capability.

For non-January-1 dates, `get_report(fit)$observation_biology` contains one row
per shared date and age class, with named columns `day`, `year_i`, `age`,
`year_fraction`, `mean_length`, `sd_length`, `mean_weight`, and
`proportion_mature`. Days are relative to the model epoch and `year_i` is
one-based. Empirical growth has no length distribution, so its length and SD
columns use the existing missing-value sentinel `-999`. Reports of annual-only
models retain their original fields. The milestone 3 limitation warning is
removed because dated biology now follows the provider's explicit policy.

Validation includes analytic predictions on two survey bin layouts, shared-date
products, leap-year fractions, oldest-age behavior, empirical/fixed-policy
regressions, AD versus finite differences, and annual report/objective/gradient
comparisons against the pre-timing baseline.


## Milestone 5 implementation notes

Fixed non-January-1 requests are compiled into a sorted date plan during model
initialization. Each evaluation prepares one date at a time, shares biological
products across that date's fleet/stream requests, and releases the products
before advancing to the next date. Biological size distributions are requested
only at dates with length samples. `get_report(fit)$observation_work` exposes
`dates`, `size_rows`, and `requests` counters for reproducible performance tests.

End-to-end tests cover a fitted catchability parameter and its dated prediction
standard errors, independent composition dates, sparse observations, stable IDs
after clearing backend state, shuffled input, December 31/January 1 boundaries,
retrospective wrappers, and annual projections with missing future surveys.
Retrospective peels retain catch and the annual model horizon by existing policy;
undated bin configuration is now explicitly preserved. Peel counts must be finite
non-negative integers.

Diagnostics now exclude -999 missing observations. `augment()` uses reported
natural-scale predictions (the `estimated` column for observation quantities)
instead of likelihood-specific `expected` inputs: lognormal means are logged and
multinomial expectations are probabilities. `get_estimates()` retains both
columns with their existing meanings; reported standard errors match `estimated`.

The executed worked example is `vignettes/fims-observation-timing.Rmd`.
The reproducible benchmark and measured comparison are in
`inst/benchmarks/observation-timing.R` and `observation-timing-results.md`.

## Extension: multiple recruitment phases

Status: phase-aware dynamics implemented on `dev-extend-timing-multi-recruiment`.
The investigation and staged implementation notes below are historical; the
current implemented conventions are summarized at the end of this document.
Confirmed scope: split one annual recruitment total across fixed dates.
Other semantics below remain proposals, not implemented capabilities or approved
biological assumptions. The default remains one January 1 recruitment event.

### Current implementation constraints

`CatchAtAge::CalculateRecruitment()` computes the annual recruitment process
from previous-year spawning biomass and `CalculateSBPR0()`, then writes directly
into the youngest January 1 abundance cell. The initial year instead uses
`log_init_naa`; the terminal report year uses mean recruitment without a process
deviation. Both `LogDevs` and `LogR` use annual process indices.

`CalculateNumbersAA()` advances survivors after a full year of mortality.
`CalculateCatchNumbersAA()` applies a full-year Baranov calculation to January 1
abundance. `EvaluateTimedObservations()` runs after annual population evaluation
and propagates January 1 selected abundance without adding fish. Consequently,
adding dates only to the observation plan cannot implement recruitment pulses.
`CalculateSBPR0()` and unfished abundance also assume January 1 recruitment and
must be reviewed alongside the fished dynamics.

### Confirmed first scope

Use fixed phase dates and fixed nonnegative fractions of one annual recruitment
total, with fractions summing to one in each modeled year. Preserve the annual
stock–recruit relationship and its existing lag and process parameterization;
calculate its total once, then allocate `R[y,k] = R[y] * fraction[y,k]`.
Independent phase-level deviations, estimated dates, seasonal mortality,
multiple spawning events, and continuous recruitment are later extensions.
Splitting recruitment does not imply splitting spawning or the likelihood for
the annual recruitment process.

A proposed separate population-level configuration table has columns
`population`, `phase`, `date`, and `fraction`. Recruitment is a state-changing
process, so these rows should not be inserted into the observation/likelihood
table. Use explicit ISO dates initially; a later convenience helper can expand
recurring month/day schedules. Require complete schedules for modeled years,
stable phase identifiers, unique dates per population, valid calendar dates,
finite nonnegative fractions, and a sum of one within a documented tolerance.
Reject unspecified leap-day substitutions. Compile fixed schedules outside the
AD tape; rebuild whenever input schedules change.

### Event calculations

For constant rates over a segment of duration `dt` in calendar-year units:

```
N_end[a] = N_start[a] * exp(-Z[a] * dt)
C[f,a]   = N_start[a] * F[f,a] * (1 - exp(-Z[a] * dt)) / Z[a]
```

Use a numerically stable implementation and the continuous limit at `Z = 0`.
Accumulate catch over segments, then add recruits at each recruitment boundary.
Survey dates only read the state and must not change annual catch or dynamics.
Use a merged schedule of annual boundaries, recruitment events, and survey
requests, with deterministic same-day ordering: finish survival/catch, age
cohorts at annual boundaries, add recruitment, calculate annual summaries when
applicable, then evaluate surveys. A January 1 event occurs once, not as both
an end-of-year and a start-of-year insertion. December 31 recruitment experiences
one remaining calendar day of mortality before the next annual boundary.

For fixed age-class rates, an independent analytic check is:

```
N[a,t] = N_initial[a] * exp(-Z[a] * t)
       + sum over pulses in class a with tau[k] <= t:
           R[k] * exp(-Z[a] * (t - tau[k]))
```

The corresponding contribution of pulse k to annual catch numbers is
`R[k] * F[f,a]/Z[a] * (1 - exp(-Z[a] * (1 - tau[k])))`.
A phase must contribute neither abundance nor catch before its event.

### Biological decisions to resolve before implementation

1. **Entry age versus birth time.** Recruitment means entry into the modeled
   population and is not necessarily birth. An annual-age-class interpretation
   assigns entrants the same `age + year_fraction` biology as their class.
   A distinct-cohort interpretation assigns an age at entry and tracks elapsed
   age from the event. These are different models; dates alone do not identify
   which is appropriate.
2. **Cohort retention.** If phases have distinct biological ages, retain phase
   state across annual boundaries and apply growth/maturity to each phase before
   aggregation. Annual class labels cannot recover that information after
   phases are pooled. Specify plus-group representative ages and initial phase
   composition explicitly. A bounded phase-by-age representation is possible
   with a fixed recurring phase structure, but arbitrary new phases each year
   require a cohort-retention policy.
3. **Initial year.** Existing youngest initial abundance already includes
   recruitment. Do not also add a full new annual total. Either define it as
   the first year's recruitment budget and allocate it across phases (an
   explicit new interpretation in pulse mode), or retain initial abundance and
   supply a separate first-year recruitment budget. Older initial classes need
   phase allocations if phase-specific biology is enabled.
4. **Reference quantities.** Recalculate unfished survival and spawning biomass
   per recruit using the same recruitment schedule and biology. Keeping the old
   `phi_0` while changing entry times would make stock–recruit calibration
   inconsistent. A repeating reference schedule and reference-year day-count
   convention must be specified for year-varying schedules and leap years.
5. **Catch biology.** Annual catch numbers must include partial-year exposure.
   Choose and document whether catch weight and fishery length compositions
   retain annual biological lookup or use an explicit within-year integration
   rule. Survey frequency must never determine the catch integration grid.
6. **Terminal reporting.** The extra January 1 report should contain only
   recruitment scheduled for that boundary, not the whole next year's total.
   Preserve the existing mean-only terminal recruitment policy, with separate
   reporting of the annual budget and realized phase additions.

### Suggested implementation milestones

1. Resolve the biological choices above; add schedule normalization, validation,
   and a single-January-1 default, with no change to model predictions.
2. Separate annual recruitment calculation from abundance insertion. Implement
   event survival, recruitment additions, annual aging, and catch accumulation
   together, initially verified with fixed rates and biological products.
3. Implement the chosen phase/cohort biology, initial conditions, unfished
   reference quantities, and terminal boundary semantics before exposing the
   new mode for assessment use.
4. Connect survey predictions, annual fishery compositions, R configuration,
   Rcpp validation, and reports. Report annual recruitment totals and per-phase
   additions separately from January 1 abundance.
5. Validate gradients, fitting, retrospective/projection workflows, invariance
   to extra survey dates, and performance; add a two-phase worked vignette.

Acceptance cases include exact legacy recovery for a single January 1 event;
zero-size phases; two pulses checked against analytic abundance and catch;
surveys immediately before/on/after a pulse; December 31/January 1 and leap-year
boundaries; no first-year double counting; plus-group accumulation; equilibrium
consistency between fished and unfished paths at zero fishing; both annual
recruitment process types; and finite-difference checks of recruitment, mortality,
and biological parameters. Schedule row ordering and added survey-only dates
must not change population states or annual catch.

### First implementation increment (historical)

`setup_recruitment_schedule()` now prepares and validates population-level
schedules, with one January 1 phase per modeled year by default. The helper
returns calendar coordinates and fractions without modifying observations,
parameters, or population dynamics. It requires a consistent set of phase IDs
across years and excludes the extra terminal reporting year. Non-default
schedules are configuration only until the event-dynamics milestone is complete.
Biological age semantics remain an explicit decision before that milestone.

### Backend preparation increment (historical)

`initialize_fims(..., recruitment_schedule = NULL)` now resolves the schedule
before clearing backend state. It accepts the legacy-compatible single January 1
phase, retains the schedule on the fitted input, and explicitly rejects other
schedules. The Rcpp recruitment interface validates fixed coordinates and copies
them into both scalar and AD recruitment objects. A shared C++ validator checks
ordering, coverage, phase identity, and annual allocation totals; a separate
guard prevents unsupported events from entering the current annual evaluator.

`CalculateAnnualRecruitment()` computes the annual process result without writing
abundance. The existing `CalculateRecruitment()` inserts that budget and reports
it at the annual boundary. First-year initialization, terminal mean recruitment,
and the stock–recruit/process likelihood remain unchanged. The event-survival and
partial-year catch implementation is still outstanding; this increment must not
be described as support for multi-phase model fitting.

### Phase-aware dynamics implementation

Confirmed biological choice: distinct biological ages for each phase, shared
biological parameters, annual age-class reporting. `entry_age` is optional and
defaults to the youngest modeled age; it must be constant by phase across years.
Phase IDs are sorted to produce stable one-based `phase_i` values in R and C++.

The new evaluator retains separate cohorts, applies mortality only after entry,
and advances annual age bins independently of biological entry-time offsets.
Same-day surveys are post-recruitment. Catch numbers use exact partial-year
Baranov exposure. Catch weight and fishery length compositions use eight-point
Gauss-Legendre quadrature per exposure interval, normalized to exact catch
numbers. Survey dates never define quadrature nodes. Mortality and selectivity
remain annual age-class rates. Empirical weights and fixed length mappings
retain their existing annual policy.

The youngest initial abundance is the first annual recruitment budget. Older
initial totals are split using phase-specific unfished survivorship under the
first schedule year and first-year natural mortality. Unfished spawning biomass
per recruit uses that reference schedule, its calendar fractions, and the same
phase biological ages. This is an explicit reference-year convention for
changing schedules and leap years. Later unfished trajectories follow actual
annual schedules and need not remain at the initial reference equilibrium.

The plus group preserves a separate representative age for each phase/cohort
entry-time offset, rather than tracking exact individual ages inside it. The
terminal January 1 report repeats the last modeled schedule's January 1 fraction
of mean recruitment; later events in the unmodeled terminal year are not added.
Annual recruitment budgets are distinct from January 1 youngest abundance.

`recruitment_events` reports per-phase allocations. `recruitment_cohorts` reports
cohort abundance and biological age at its `available_from` time in each year,
not at a common survey date. These replace `observation_biology` in phase mode.
Annual growth-product reports remain reference-age provider outputs, not
phase-weighted population averages. The worked recruitment-phases vignette
explains these conventions and the quadrature approximation.

A single January 1 phase with fraction one and the default entry age continues
to use the original annual evaluator. Explicit schedules are passed through
retrospective refits; likelihood profiles inherit the fitted schedule. Projection
inputs must supply a schedule for every extended modeled year.
