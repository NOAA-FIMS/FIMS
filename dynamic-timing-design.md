# Dynamic Timing Design for FIMS

## Status

- **Document type:** Technical design proposal
- **Initial scope:** Irregularly timed fishery-fleet and survey observations
- **Long-term scope:** Continuous processes, discrete biological events, and observations at arbitrary times
- **Implementation strategy:** Deliver a narrow observation-timing feature first while preserving a path to a sparse event-based engine

## 1. Summary

FIMS currently stores population numbers at annual boundaries and evaluates fishery catches, fishery-dependent indices, survey indices, compositions, and biological quantities from those annual states. Consequently, point observations are effectively assumed to occur at the start of their model year, while catch observations are effectively assumed to cover the entire year.

This design adds exact within-year timing without introducing monthly or seasonal population arrays. The annual population state remains canonical. When an observation occurs within a year, FIMS analytically propagates the annual state to the observation time using the mortality applicable during that year:

\[
N_{a,y}(t) = N_{a,y}(0)\exp(-Z_{a,y}t), \qquad 0 \leq t \leq 1.
\]

Observation schedules are sparse and attached to fishery-fleet and survey data streams. A point schedule identifies the model year and within-year fraction of an observation. An interval schedule additionally identifies its start and end. This supports:

- different starting years for fishing fleets and surveys;
- different observation dates among fishing fleets and surveys;
- dates that vary by year;
- missing observation years;
- multiple observations within a year; and
- multiple data types collected on the same sampling occasion; and
- catch observations covering a full year or a defined within-year interval.

The first implementation changes fishery-fleet and survey observation predictions. Fishing mortality remains an annual continuous process unless a later phase explicitly adds process windows; recruitment and aging remain annual boundary operations, and annual population storage remains unchanged. An observation's time support does not, by itself, change when fishing mortality acts.

## 2. Goals

The design must:

1. Represent fishery-fleet and survey observation time independently of the model's annual storage layout.
2. Support sparse, irregular schedules and different start dates.
3. Preserve the current annual population dynamics when timing is not supplied or is zero.
4. Avoid year-by-season-by-age or year-by-day-by-age state arrays.
5. Keep automatic differentiation operations small and predictable.
6. Give observations at a common time a shared temporal identity.
7. Establish explicit time and propagation semantics that can support future timed processes.
8. Detect invalid or ambiguous schedules before objective-function evaluation.

## 3. Non-goals for the first implementation

The initial implementation will not provide:

- seasonal population states;
- arbitrary calendar libraries in the C++ population engine;
- within-year changes in mortality rates;
- instantaneous catches or removals as state-changing events;
- timed or multiple recruitment pulses;
- alternative aging dates;
- numerical ODE integration;
- state-triggered events;
- a general-purpose event scheduler; or
- persistent caching of parameter-dependent population states.

These features may be added later using the temporal concepts established here.

## 4. Terminology

### 4.1 Model epoch

The model epoch is the start of model year zero. Calendar dates supplied through R are converted relative to this epoch.

### 4.2 Model year

`model_year` is a zero-based integer index into annual model quantities. It is not a calendar year.

### 4.3 Within-year fraction

`fraction` is elapsed time since the start of a model year, expressed in model years:

- `0.0` is the start of the model year;
- `0.5` is halfway through the model year; and
- `1.0` is immediately before the next annual boundary operation.

The initial implementation treats annual rates as constant over this interval.

### 4.4 Observation stream

An observation stream is one time series of a particular data type, such as fishery catch, fishery-dependent index, survey index, age composition, or length composition.

### 4.5 Observation occasion

An observation occasion is a unique point sampling time. Multiple streams may refer to the same occasion when, for example, an index and an age composition were collected during the same survey or fishery sampling operation.

### 4.6 Observation interval

An observation interval is the time support over which an accumulated quantity was measured. Catch is the primary example. An annual catch record usually covers `[0, 1]`; a catch record for part of a year may cover a smaller interval. An interval is not necessarily the period during which the underlying fishing process was active.

### 4.7 Process schedule

A process schedule describes when a state-changing or continuously acting process applies. Fishing mortality schedules are process schedules.

### 4.8 Observation schedule

An observation schedule describes either when state is inspected or the interval over which an observed quantity is accumulated. It does not alter population state.

Process schedules and observation schedules must remain distinct.

## 5. Temporal semantics

### 5.1 Annual state

`numbers_at_age[a, y]` represents abundance at the start of model year `y`, after boundary operations assigned to that instant. The initial implementation retains the current FIMS ordering for recruitment, aging, and plus-group calculations.

### 5.2 Within-year state

For constant total mortality in year `y`, abundance at an observation fraction `t` is:

\[
N^{obs}_{a,y,t} = N_{a,y}\exp(-Z_{a,y}t).
\]

This is a transient value used to calculate an observation. It is not stored as an additional annual population state.

### 5.3 Expected index

The expected index in numbers at age for fleet or survey `f` is:

\[
I_{f,a,y,t} = q_{f,y}S_{f,a,y}N_{a,y}\exp(-Z_{a,y}t).
\]

Index weight at age, total index, age composition, and length composition must all be derived from the same timed abundance when they belong to the same observation occasion.

### 5.4 Expected catch over an interval

When `F` and `Z` are constant over an observation interval `[t_0,t_1]`, expected catch in numbers is:

\[
C_{f,a,y,[t_0,t_1]} =
\frac{F_{f,a,y}}{Z_{a,y}}
N_{a,y}\exp(-Z_{a,y}t_0)
\left[1-\exp\{-Z_{a,y}(t_1-t_0)\}\right].
\]

The existing annual Baranov catch equation is the special case `t_0 = 0` and `t_1 = 1`. This equation permits catch data with different coverage intervals without adding seasonal states.

The interval is the support of the catch observation. It must not silently be interpreted as the interval during which fishing mortality contributes to population dynamics. Supporting a fishery that is active only during part of the year requires piecewise mortality regimes and belongs to a later implementation phase.

### 5.5 Boundary convention

An observation at `fraction = 1.0` sees the state immediately before the next model year's aging and recruitment boundary operations. It must not be implemented by indexing the next annual state because that state may have different age-class and recruitment semantics.

An observation at `fraction = 0.0` must reproduce the existing beginning-of-year result.

### 5.6 Observation purity

Evaluating an observation must not modify population state. Adding, removing, or reordering observations must not change later annual population states.

## 6. Data model

### 6.1 Core time representation

The first implementation should use an exact year index and a fractional offset rather than a single floating-point timestamp:

```cpp
struct ModelTime {
  std::size_t model_year;
  double fraction;
};
```

Keeping the integer year separate prevents fragile floating-point year lookup and maps directly to the current annual arrays.

`ModelTime` is structural input and must not use the automatic-differentiation scalar type.

Point and interval support should be explicit:

```cpp
enum class ObservationSupport { kPoint, kInterval };

struct ModelInterval {
  ModelTime start;
  ModelTime end;
};
```

The initial catch implementation should require an interval to remain within one model year. Cross-year intervals can later be split into year-specific segments during schedule compilation.

### 6.2 Observation occasions

```cpp
using ObservationOccasionId = std::uint32_t;

struct ObservationOccasion {
  ObservationOccasionId id;
  ModelTime time;
};
```

An occasion may be referenced by one or more observation streams. Sharing an occasion guarantees that associated predictions use the same population state and timing convention.

### 6.3 Observation schedule

```cpp
struct ObservationScheduleEntry {
  std::size_t observation_index;
  ObservationSupport support;
  ObservationOccasionId occasion_id;  // Used for point observations.
  ModelInterval interval;              // Used for interval observations.
};

struct ObservationSchedule {
  std::vector<ObservationScheduleEntry> entries;
};
```

`observation_index` identifies the corresponding time element in the observed data object. The schedule therefore does not require leading values before a fishing fleet or survey starts, or placeholder entries for missing years.

### 6.4 Initial simplified representation

If introducing occasion identifiers is too large for the first pull request, the initial interface may store parallel vectors:

```cpp
struct ObservationSchedule {
  std::vector<std::size_t> model_year;
  std::vector<double> start_fraction;
  std::vector<double> end_fraction;
};
```

For point observations, `start_fraction` equals `end_fraction`. This representation is acceptable only if each entry maps unambiguously to an observed time step. The code should still centralize validation and conversion so it can migrate to typed point occasions and intervals without changing propagation mathematics.

### 6.5 Fleet role and capabilities

The current `Fleet` abstraction can represent both fisheries and surveys. Timing makes it important to distinguish their effects. The preferred design uses capabilities rather than relying on a near-zero fishing mortality:

```cpp
struct FleetCapabilities {
  bool contributes_to_mortality = false;
  bool predicts_catch = false;
  bool predicts_index = false;
};
```

Typical configurations are:

| Component | Mortality | Catch | Index |
|---|---:|---:|---:|
| Fishing fleet with CPUE | Yes | Yes | Yes |
| Fishing fleet without CPUE | Yes | Yes | No |
| Fishery-independent survey | No | No | Yes |

This prevents survey placeholder fishing mortality from contributing to the `Z` used to propagate survey abundance.

## 7. R-facing input design

### 7.1 Preferred tabular form

The user-facing representation should accept calendar years or dates and identify the observation stream:

```text
component  data_type  year  start_fraction  end_fraction  occasion
fleet1    catch      2003            0.00          1.00  fleet1_catch_2003
fleet1    cpue       2003            0.58          0.58  fleet1_sample_2003
fleet1    age_comp   2003            0.58          0.58  fleet1_sample_2003
survey1   index      2005            0.42          0.42  survey1_2005
survey1   age_comp   2005            0.42          0.42  survey1_2005
survey1   index      2006            0.45          0.45  survey1_2006
survey1   index      2008            0.39          0.39  survey1_2008
survey2   index      1999            0.71          0.71  survey2_1999
```

The R layer converts calendar year to `model_year` using:

\[
model\_year = calendar\_year - model\_start\_year.
\]

Equal start and end fractions identify a point observation. Unequal values identify an interval. The absence of a row means that no observation occurred in that year. It is not interpreted as an observation at an unknown time.

### 7.2 Calendar dates

The R layer may optionally accept an ISO date and derive the fraction. The conversion convention must be documented and tested, including leap years. One suitable convention is:

\[
fraction = \frac{day\_of\_year - 1}{days\_in\_year}.
\]

Under this convention January 1 maps to zero. Exact calendar conversion belongs in R; the C++ engine receives normalized `ModelTime` values.

Users should not be allowed to supply both a date and a fraction for the same record unless FIMS verifies that they agree.

### 7.3 Defaults

For a transition period, an omitted point-observation schedule may default to `fraction = 0.0`, while an omitted catch interval may default to `[0.0, 1.0]`. These defaults reproduce current behavior. Because FIMS is pre-1.0, the preferred final API should require timing support explicitly for newly constructed fishery-fleet and survey streams.

Defaults and scalar recycling, if offered, should be implemented in R rather than implicitly in C++.

## 8. C++ design

### 8.1 Stateless propagation primitive

The initial propagation operation should be a small pure function:

```cpp
template <typename Type>
Type PropagateSurvival(const Type& numbers,
                       const Type& mortality_rate,
                       double elapsed_years) {
  return numbers *
         fims_math::exp(-mortality_rate *
                        static_cast<Type>(elapsed_years));
}
```

An age-vector overload may be added if it simplifies callers, but the scalar operation should remain independently testable.

### 8.2 Prediction path

For each point observation occasion:

1. Resolve its `model_year` and `fraction`.
2. Obtain beginning-of-year `numbers_at_age` and annual `mortality_Z`.
3. Propagate abundance analytically to the occasion.
4. Apply observation-specific selectivity and catchability.
5. Calculate index numbers or weight.
6. Calculate any associated age or length composition from the same timed abundance.
7. Store the prediction at the observation's data index.

For each catch observation interval:

1. Resolve its model year, start fraction, and end fraction.
2. Obtain beginning-of-year abundance and annual `F` and `Z`.
3. Evaluate the interval Baranov equation directly.
4. Calculate catch numbers or weight and associated catch compositions.
5. Store the prediction at the observation's data index.

### 8.3 Separation from annual output arrays

Annual population outputs remain dense because population dynamics operate annually. Observation predictions may be sparse. Code must not assume that observation index `i` is identical to model year `i`.

Where current derived-quantity arrays require annual layouts, the first implementation may retain dense arrays internally and use schedule mappings to populate observed elements. New code should avoid strengthening this coupling.

### 8.4 Automatic differentiation

`model_year`, `fraction`, occasion identifiers, sorting, and grouping are fixed data. Only abundance, mortality, selectivity, catchability, and other model quantities use `Type`.

No parameter-dependent state should persist between objective-function evaluations. Any reuse of timed abundance must be local to one evaluation.

### 8.5 Grouping common times

The first implementation may evaluate the analytical expression separately for each stream because it is inexpensive and clear. The schedule representation must nevertheless permit later grouping by `ModelTime` or occasion identifier.

If profiling demonstrates a benefit, model initialization can compile an execution plan that groups observations sharing an occasion. The structure of that plan is fixed outside the AD tape; the timed state is computed inside each objective evaluation.

## 9. Validation and error handling

Validation must occur during R configuration or C++ model construction, not deep inside objective evaluation.

FIMS must reject:

- point or interval fractions outside `[0.0, 1.0]`;
- non-finite fractions;
- intervals whose end precedes their start;
- zero-width intervals for accumulated observations;
- cross-year intervals until schedule splitting is implemented;
- model years outside the population's modeled range;
- schedule and observation dimensions that do not agree;
- unknown component, stream, or occasion identifiers;
- one occasion identifier assigned conflicting times;
- ambiguous duplicate schedule entries;
- calendar dates outside the modeled period; and
- catch observations without a resolvable interval;
- index or composition observations without a resolvable point time when timing is required.

FIMS should allow:

- a component starting after the population model begins;
- a component ending before the population model ends;
- gaps in an observation series;
- dates that vary among years;
- different schedules for different components;
- multiple occasions in one model year; and
- multiple observation streams on one occasion.

Duplicate entries at one time are valid only when they refer to distinct observations or explicitly share an occasion. Accidental duplicates within one stream should be rejected.

## 10. Interaction with data types

### 10.1 Fishery and survey index data

Each index observation uses abundance propagated to its scheduled occasion. Catchability and selectivity are evaluated using their existing year-dependent behavior.

### 10.2 Age composition

An age composition sampled at a point in time must be calculated from timed selected abundance, whether it comes from a fishing fleet or a survey. If it shares an occasion with an index, both predictions use the same propagated abundance. A catch composition representing accumulated removals instead uses catch numbers over its associated interval.

### 10.3 Length composition

Point-sampled length composition is derived from timed selected abundance at age followed by the existing age-to-length conversion. Catch length composition is derived from interval catch at age followed by the same conversion.

### 10.4 Fishery catch and catch composition

The first milestone preserves full-year catch as the default but also permits an explicitly supplied within-year catch interval using the interval Baranov equation. Catch must not be treated as an instantaneous observation merely because a landing or reporting date is available.

Fishery catch observations, fishery-dependent sampling observations, survey observations, and fishing-mortality process schedules remain separate concepts.

### 10.5 Fishery-dependent indices

A CPUE index may have an observation schedule even though its fleet also contributes continuous fishing mortality. The observation time affects the CPUE prediction; it does not alter the annual period over which that fleet's fishing mortality acts.

## 11. Execution flow for the first milestone

```text
R model and data specification
          |
          v
Validate dates, schedules, roles, and dimensions
          |
          v
Convert calendar values to point or interval model time
          |
          v
Build annual population state and mortality as today
          |
          +-------------------------------+
          |                               |
          v                               v
For each catch interval       For each point occasion
          |                               |
          v                               v
Interval Baranov catch        N(t) = N(0) exp(-Z t)
          |                               |
          v                               v
Catch and catch-composition   Apply q and selectivity
predictions                               |
          |                               v
          |                  Index and sampled-composition
          |                  predictions
          |                               |
          +---------------+---------------+
                          |
                          v
                      Likelihood
```

## 12. Implementation plan

### Phase 0: Resolve fleet semantics

1. Add explicit fleet capabilities or an equivalent role mechanism.
2. Exclude non-mortality components from total fishing mortality.
3. Add regression tests proving that survey fleets do not affect `Z`.

### Phase 1: Time and schedule infrastructure

1. Add `ModelTime` and centralized validation.
2. Add observation schedule storage to the model-facing and Rcpp objects.
3. Expose schedule configuration through R.
4. Convert calendar year/date input to normalized model time.
5. Add tests for late starts, gaps, varying dates, and invalid schedules.

### Phase 2: Timed fleet and survey predictions

1. Add the analytical survival propagation primitive.
2. Use timed abundance for expected index numbers and weight.
3. Use the same timed abundance for fishery- and survey-sampled age compositions.
4. Propagate the timed prediction through age-to-length conversion.
5. Add interval Baranov predictions for catch and catch compositions.
6. Preserve existing behavior at point fraction zero and catch interval `[0, 1]`.

### Phase 3: Observation occasions

1. Allow fishery and survey index and composition streams to share an occasion identifier.
2. Verify conflicting occasion definitions during construction.
3. Optionally group common occasions in a compiled observation plan.
4. Benchmark run time and AD tape size before adding further optimization.

### Phase 4: General temporal engine

Begin this phase only when a state-changing within-year feature is approved.

1. Generalize scalar propagation to `Dynamics::Propagate(state, from, to)`.
2. Introduce a sparse ordered timeline.
3. Define events separately from observations.
4. Define explicit pre-event and post-event observation semantics.
5. Implement recruitment as the first state-changing event.
6. Add piecewise-constant dynamics regimes for within-year changes in mortality.

## 13. Test plan

### 13.1 Propagation unit tests

- Zero elapsed time returns the input abundance.
- One year returns `N * exp(-Z)`.
- A known fractional example matches an independently calculated value.
- Zero mortality leaves abundance unchanged.
- The semigroup property holds within numerical tolerance:

  \[
  \Phi(t_0,t_2) = \Phi(t_1,t_2)\circ\Phi(t_0,t_1).
  \]

### 13.2 Schedule unit tests

- A schedule may begin after model year zero.
- A schedule may omit internal years.
- Fishing fleets and surveys may have different dates in the same year.
- One fishing fleet or survey may have multiple observations within a year.
- Catch streams may begin late, omit years, and use different coverage intervals.
- Calendar dates convert correctly in leap and non-leap years.
- Invalid years, fractions, dimensions, and identifiers fail with informative messages.

### 13.3 Prediction tests

- Point fraction zero exactly reproduces current index predictions.
- Catch interval `[0, 1]` exactly reproduces current annual catch predictions.
- Partial-year catch matches the interval Baranov equation.
- Fraction one uses full-year survival without applying the next aging transition.
- Timed index numbers and weight match analytical expectations.
- Fishery and survey sampled age-composition proportions use timed abundance.
- Catch compositions use removals accumulated over their catch interval.
- Length compositions are derived from the timed age prediction.
- Associated streams sharing an occasion use identical timing.

### 13.4 Invariance tests

- Adding an observation does not change annual population state.
- Changing a point-observation date does not change population dynamics or catch predictions.
- Changing catch observation support changes its prediction but not the underlying fishing-mortality process schedule.
- Reordering observations does not change predictions or likelihood.
- Duplicate observations at one occasion do not cause extra state transitions.
- A survey that does not contribute to mortality does not change `Z`.

### 13.5 Integration tests

Construct a model with:

- a fishing fleet whose catch and sampled composition data begin in different years;
- a partial-year catch interval;
- one survey that begins late and skips a year;
- a second survey with a different start year and timing;
- fishery and survey indices with compositions sharing their respective occasions; and
- a fishery-dependent index whose observation schedule differs from its mortality schedule.

Compare all predictions to independent analytical calculations.

## 14. Performance criteria

The initial implementation should:

- leave persistent population storage asymptotically unchanged;
- add work proportional to the number of observations, not days or months;
- avoid creating estimated timing parameters;
- avoid parameter-dependent caches across objective evaluations; and
- preserve current performance when point observations occur at fraction zero and catch observations cover `[0, 1]`, within reasonable constant overhead.

Benchmarks should compare:

1. the current annual implementation;
2. timing enabled with all fractions at zero;
3. one unique observation time per fishery fleet and survey per year;
4. several fishery and survey streams sharing and not sharing occasions; and
5. full-year and partial-year catch intervals.

Measurements should include objective evaluation time, peak memory, and AD tape size where available.

## 15. Compatibility and migration

The safest migration path is:

1. Existing models without schedules temporarily receive beginning-of-year point schedules and full-year catch intervals.
2. FIMS emits a deprecation message encouraging explicit observation timing.
3. R helper functions support concise constant schedules and conversion from dates.
4. A later pre-1.0 release requires explicit point or interval support for fishery-fleet and survey observation streams.

No compatibility promise should be made for an intermediate fleet-level `index_time[n_years]` representation. The public contract should be sparse observation schedules, even if the first internal implementation temporarily uses dense arrays.

## 16. Alternatives considered

### 16.1 Fixed seasons

Rejected because storage and computation scale with the number of seasons even when no observations occur at most seasonal boundaries. Seasons also discretize dates unnecessarily.

### 16.2 Daily population states

Rejected because it introduces large arrays and computation unrelated to the number of observations.

### 16.3 One timing vector per fleet

Useful as a prototype but insufficient as the final design. It assumes annual alignment, encourages padding, handles missing years poorly, and cannot cleanly distinguish index, composition, and process schedules.

### 16.4 Full event engine before fleet and survey timing

Deferred because point observations and accumulated catch observations can both be evaluated analytically without mutable state events. Implementing event ordering and dynamics regimes now would substantially increase scope without improving the initial fleet and survey result.

### 16.5 Generic memoization cache

Deferred because cached states depend on estimated parameters. A fixed execution plan that groups common observation times is safer for TMB and should be considered only after profiling.

## 17. Future architecture

When FIMS adds within-year state-changing processes, the temporal model should distinguish four concepts:

```text
STATE        Population quantities at a particular time
FLOW         Continuous evolution between two times
EVENT        Instantaneous state transformation
OBSERVATION  Prediction or likelihood evaluation that does not change state
```

The sparse execution pattern is then:

```cpp
state = initial_state;
time = initial_time;

for (const TimelineNode& node : execution_plan.nodes) {
  state = dynamics.Propagate(state, time, node.time);
  EvaluatePreEventObservations(node, state);
  ApplyEvents(node, state);
  EvaluatePostEventObservations(node, state);
  time = node.time;
}
```

Known-time observations, known-time events, and future state-triggered events should remain separate concepts. Analytical propagation remains the default whenever the dynamics permit it; numerical integration is an optional future implementation, not the core model.

## 18. Open decisions

The implementation team should explicitly decide:

1. Whether fraction `1.0` is accepted or represented as the next year at `0.0` with a pre-boundary flag. This proposal recommends accepting `1.0` with pre-boundary semantics.
2. Whether timing is attached directly to data objects or stored in a separate observation registry. This proposal recommends a separate schedule linked to data indices.
3. Whether shared observation occasions are required in the first pull request or introduced immediately afterward.
4. How current dense derived-quantity reporting maps sparse observations to output.
5. Whether missing schedules initially receive legacy defaults or are rejected. This proposal recommends temporary defaults of zero for points and `[0, 1]` for catches, followed by explicit timing before version 1.0.
6. Whether fleet capabilities replace or supplement a fleet-role enumeration. This proposal recommends capabilities because fisheries can also produce indices.

## 19. Acceptance criteria for the first release

The first dynamic-timing release is complete when:

- users can specify a different point or interval observation schedule for every fishery-fleet and survey data stream;
- schedules can have different starts, ends, gaps, and within-year fractions;
- expected fishery and survey indices and their sampled compositions use analytically propagated abundance;
- full-year and partial-year catch observations use the appropriate Baranov interval equation;
- point observations at fraction zero and catch intervals `[0, 1]` reproduce existing results;
- observation timing does not implicitly change annual fishing-mortality process timing or population dynamics;
- surveys can be configured not to contribute to mortality;
- invalid schedules fail before optimization;
- C++ unit tests and R integration tests cover the required timing behavior; and
- benchmarks show cost scaling with observations rather than a fixed seasonal or daily grid.
