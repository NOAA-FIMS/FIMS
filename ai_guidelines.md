# FIMS AI-Assisted Development Guidelines

## Purpose

These guidelines define expectations for AI-assisted code generation and modification within the Fisheries Integrated Modeling System (FIMS). AI-generated code must look, behave, and integrate as though it were written by a knowledgeable FIMS developer.

The purpose is not merely to enforce formatting or programming style. These guidelines preserve scientific meaning, architectural consistency, human maintainability, numerical integrity, computational performance, and semantic traceability throughout FIMS.

> **AI has considerable freedom over how FIMS code is implemented. It has very little freedom over what FIMS code means.**

---

## 1. Foundational Principles

### 1.1 Scientific correctness comes first

Generated code must faithfully represent the intended mathematical, statistical, and fisheries-science formulation.

Correct compilation and apparently reasonable output are not sufficient evidence of correctness. When code participates in statistical inference, numerical and derivative behavior are part of the implementation contract.

### 1.2 Preserve semantics

The intended meaning, behavior, invariants, and contracts of FIMS software must be preserved.

AI-generated changes must not alter behavior merely because an alternative implementation appears cleaner, faster, more generic, or more conventional.

### 1.3 Document generated code

AI-generated code must be appropriately documented. Documentation should preserve knowledge future developers need to understand the scientific and engineering intent.

Documentation should emphasize:

- purpose and intent;

- mathematical or statistical meaning;

- assumptions;

- invariants;

- units and scales where relevant;

- parameter constraints;

- ownership or lifetime expectations where relevant;

- architectural decisions;

- numerical considerations;

- non-obvious behavior; and

- reasons for implementation choices that a future developer might otherwise be tempted to "fix."

Comments should not merely translate obvious code into English.

### 1.4 Code for human readability

Code is maintained by humans. Prefer straightforward, explicit implementations that a FIMS developer can understand, debug, validate, and extend.

Readable does not mean verbose. Avoid excessive comments, unnecessary helper layers, artificially long identifiers, and abstractions that increase cognitive load without meaningful benefit.

### 1.5 Optimize computational performance

Within the constraints of correctness, semantic integrity, and maintainability, implementations should seek good computational performance.

Performance includes more than execution time:

- runtime;

- memory consumption;

- allocation frequency;

- copying;

- cache behavior;

- automatic-differentiation graph size;

- compilation cost;

- binary size where relevant; and

- parallel scalability.

Prefer improvements to algorithms, data structures, memory behavior, and unnecessary computation over readability-reducing micro-optimizations.

### 1.6 Understand before modifying

AI should not treat code generation as the first step of a development task. Existing FIMS behavior, architecture, semantics, and tests should be understood before implementation begins.

---

## 2. FIMS Domain Semantics

### 2.1 Prefer domain semantics over generic programming conventions

FIMS is scientific software. Established fisheries and statistical terminology should be preserved throughout the implementation.

Concepts such as recruitment, selectivity, maturity, growth, fleet, population, fishing mortality, natural mortality, abundance, and likelihood should not acquire unrelated names merely because an implementation crosses architectural layers.

Established mathematical notation such as `F`, `M`, `Z`, or `N` may improve readability when its meaning is clear from scientific context and appropriately documented.

### 2.2 Scientific intent should remain visible

Minimize the conceptual distance between the scientific formulation and its implementation.

If the intended relationship is:

```text

Z = F + M

```

the implementation should, where practical, make that relationship recognizable rather than burying it inside generic machinery.

### 2.3 Preserve units, scales, and identities

Generated code must not silently mix or reinterpret quantities with different meanings.

Particular care should be taken to distinguish:

- age from year;

- index from identifier;

- position from value;

- natural scale from transformed scale;

- rates from probabilities;

- fixed effects from random effects;

- estimated parameters from fixed inputs; and

- quantities with different physical or temporal units.

### 2.4 Preserve invariants

If a FIMS object or calculation requires an invariant, generated code must maintain it.

Examples include positivity, boundedness, normalization, dimensional correspondence, ordering, valid indexing, ownership relationships, and mathematical constraints.

### 2.5 Avoid hidden semantic transformations

Do not silently normalize, clamp, reorder, convert, deduplicate, transform, or otherwise reinterpret data unless that behavior is explicitly part of the contract.

Mutation should be apparent from an interface. Functions that modify state should not masquerade as pure computations.

---

## 3. Semantic Traceability

### 3.1 Preserve semantic identity across boundaries

> **Semantic continuity shall be maintained across abstraction, module, and language boundaries.**

When a FIMS concept crosses an interface boundary, its domain identity should remain recognizable.

For example, if the public R API contains:

```r

create_logistic_selectivity()

```

and the implementation crosses a `.Call` boundary into native code, the native entry point and subsequent C++ implementation should retain an obvious semantic relationship to logistic selectivity.

A desirable conceptual path might resemble:

```text

R

create_logistic_selectivity()

        ↓

.Call()

        ↓

Native interface

create_logistic_selectivity_native()

        ↓

C++ construction/interface

CreateLogisticSelectivity()

        ↓

Domain implementation

LogisticSelectivity

```

The exact spelling may follow conventions of each language, but the conceptual name should remain intact.

Avoid semantic chains such as:

```text

create_logistic_selectivity()

        ↓

.Call("_fims_obj_14")

        ↓

make_sel()

        ↓

Factory::Create(3)

        ↓

ModelComponent<double>

```

when those choices prevent a developer from tracing the domain concept.

### 3.2 Implementation boundaries are not semantic boundaries

R, C++, JSON, serialization, native interfaces, templates, and other implementation mechanisms should not cause domain vocabulary to disappear.

Conceptually, FIMS should preserve meaning through:

```text

User vocabulary

      ↓

Public API vocabulary

      ↓

Interface vocabulary

      ↓

Native API vocabulary

      ↓

Domain-model vocabulary

      ↓

Mathematical vocabulary

```

Representations need not be mechanically identical, but their semantic mapping should be obvious.

### 3.3 Do not rename concepts merely to satisfy local conventions

Language-specific syntax may change while conceptual identity remains constant. For example, `create_logistic_selectivity` in R and `CreateLogisticSelectivity` in C++ can clearly express the same concept.

General programming convention must not override FIMS domain semantics.

### 3.4 Prefer explicit semantic identifiers

Where practical, prefer meaningful domain identifiers over arbitrary numeric codes, generic dispatch identifiers, or unrelated names.

For example:

```json

{

  "type": "logistic_selectivity"

}

```

is generally more semantically transparent than an unexplained numeric type code.

When generic machinery or numeric identifiers are required internally, their semantic mapping must be explicit and documented.

---

## 4. Architecture and Abstraction

### 4.1 Extend FIMS rather than casually redesigning it

AI-generated code should follow established FIMS architectural patterns unless the task explicitly requires architectural change.

> **Prefer architectural consistency over locally optimal design.**

A locally elegant implementation that introduces a second competing architectural pattern can make FIMS globally harder to understand and maintain.

### 4.2 Pattern discovery precedes pattern creation

Before introducing a new implementation pattern, find and examine the closest existing FIMS analogue.

For example:

- new selectivity implementations should examine existing selectivity implementations;

- new distributions should examine established distribution patterns;

- new R/native constructors should examine analogous constructors; and

- new model components should examine related model components.

> **Pattern discovery must precede pattern creation.**

### 4.3 Abstraction must earn its existence

Do not introduce factories, registries, managers, adapters, builders, helper layers, generic templates, or other abstractions simply because they are possible.

Introduce abstraction when it meaningfully:

- removes important duplication;

- enforces an architectural invariant;

- enables required extensibility;

- improves semantic consistency; or

- materially improves maintainability or performance.

Prefer several clear implementations of genuinely different scientific concepts over a highly generic abstraction that obscures those concepts.

### 4.4 Prefer semantic proximity

Related concepts should remain structurally and conceptually close enough that developers can readily trace their relationships.

The conceptual distance between the public API and scientific implementation should be minimized where practical.

### 4.5 Make invalid states difficult to represent

Use types and interfaces to enforce important invariants where doing so improves correctness and clarity without excessive abstraction.

---

## 5. Numerical and Statistical Integrity

### 5.1 Numerical stability is part of correctness

Algebraically equivalent formulations are not necessarily computationally equivalent.

AI must not simplify or replace numerical expressions solely because they are algebraically equivalent.

Consider, where relevant:

- overflow;

- underflow;

- catastrophic cancellation;

- parameter boundaries;

- log-space formulations;

- NaN and Inf propagation;

- conditioning;

- derivative stability; and

- Hessian behavior.

### 5.2 Differentiability is part of the contract

When FIMS code participates in automatic differentiation or statistical inference, value correctness alone is insufficient.

Generated code must consider:

- whether operations participate in the AD graph;

- compatibility with required scalar/AD types;

- differentiability;

- branch behavior;

- unnecessary AD graph growth;

- gradient behavior; and

- Hessian behavior where relevant.

An implementation that calculates the expected objective value but produces incorrect derivatives is incorrect FIMS code.

### 5.3 Mathematical formulations should remain traceable

Likelihoods, selectivity functions, recruitment models, growth functions, mortality calculations, reference points, and other mathematical components should remain recognizable relative to their documented scientific formulation whenever practical.

---

## 6. Performance Engineering

### 6.1 Performance is multidimensional

Do not optimize runtime while ignoring potentially unacceptable changes in memory use, AD graph size, compilation cost, or scalability.

### 6.2 Optimize architecture before micro-operations

Prefer improvements such as:

```text

better algorithmic complexity

        ↓

appropriate dense/sparse representation

        ↓

less copying

        ↓

fewer allocations

        ↓

less recomputation

        ↓

smaller AD graphs

        ↓

better memory locality

        ↓

measured low-level optimization

```

before sacrificing readability for minor expression-level optimizations.

### 6.3 Avoid unnecessary copies and allocations

Particularly in performance-sensitive C++ paths, consider references, views, move semantics, reusable workspaces, contiguous storage, and allocation behavior.

### 6.4 Avoid hidden expensive operations

Computational cost should be reasonably apparent from an interface.

Seemingly trivial getters or accessors should not unexpectedly perform major recomputation, allocate large objects, rebuild model structures, or mutate state without a clear architectural reason.

### 6.5 Benchmark meaningful optimizations

When an optimization reduces readability or increases complexity, there should be evidence that the performance gain matters.

Do not obscure scientific intent for an assumed micro-optimization.

---

## 7. Documentation Guidelines

### 7.1 Document why, not merely what

Useful comments explain:

- why an operation exists;

- what scientific relationship it represents;

- assumptions being made;

- important units or scales;

- numerical considerations;

- invariants;

- non-obvious performance choices; and

- why an apparently simpler alternative is intentionally not used.

Avoid:

```cpp

// Add F and M.

Type Z = F + M;

```

Prefer:

```cpp

// Total instantaneous mortality. Survival over one model

// time step is S = exp(-Z), where Z = F + M.

const Type Z = F + M;

const Type survival = exp(-Z);

```

### 7.2 Document decisions likely to be revisited

If a future developer might reasonably interpret an unusual implementation as a mistake, document why it exists.

This is particularly important for numerically stable formulations, performance-sensitive code, AD-specific implementation choices, compatibility constraints, and intentionally non-obvious architecture.

---

## 8. Pre-Coding Protocol for AI-Assisted FIMS Development

Code generation is not the first step. Understanding is.

### Step 1 — Establish requested behavior

Determine:

- scientific purpose;

- expected inputs;

- expected outputs;

- intended behavior; and

- criteria for correctness.

Do not invent missing scientific or behavioral requirements.

### Step 2 — Find the nearest existing analogue

Search FIMS for the closest existing implementation and understand the pattern already in use before creating a new one.

### Step 3 — Trace the semantic path

Identify how the concept travels through relevant layers:

```text

R API

    ↓

Native .Call interface

    ↓

C++ interface

    ↓

FIMS model component

    ↓

Mathematical implementation

```

If the requested change crosses multiple layers, understand all affected layers before modifying them.

### Step 4 — Identify contracts and invariants

Determine relevant:

- parameter dimensions;

- parameter transformations;

- valid ranges;

- indexing conventions;

- ownership/lifetime requirements;

- component relationships;

- AD compatibility;

- R-facing behavior;

- mathematical formulation;

- units;

- initialization behavior; and

- error behavior.

### Step 5 — Identify the mathematical/statistical formulation

Establish the scientific equation or statistical formulation represented by the implementation where applicable.

### Step 6 — Determine AD implications

Ask, where relevant:

- Does this operation enter the AD graph?

- Which scalar/AD types must it support?

- Is the operation differentiable?

- Does it introduce unnecessary graph nodes?

- What happens at parameter boundaries?

- Could branching alter derivative behavior?

- Are gradients or Hessians affected?

### Step 7 — Assess computational consequences

Before choosing the implementation, consider what scales with years, ages, fleets, populations, regions, projection length, and other model dimensions.

Also consider what is copied, allocated, recomputed, retained, and placed on the AD graph.

### Step 8 — Examine existing tests

Determine what existing tests reveal about:

- current behavior;

- scientific relationships;

- edge cases;

- derivative behavior;

- R/native interfaces; and

- regression expectations.

Existing tests are evidence of intended behavior, not proof that untested behavior is irrelevant.

### Step 9 — Define the smallest coherent change

Implement the requested capability with the smallest architectural change that cleanly satisfies the requirement.

This does not necessarily mean the fewest lines of code. It means avoiding unrelated redesign and unnecessary expansion of scope.

### Step 10 — Implement, document, and validate

Only after completing the preceding analysis should code be generated or modified.

```text

UNDERSTAND

    ↓

DISCOVER

    ↓

TRACE

    ↓

IDENTIFY CONTRACTS

    ↓

VERIFY MATHEMATICS

    ↓

ASSESS AD BEHAVIOR

    ↓

ASSESS PERFORMANCE

    ↓

EXAMINE TESTS

    ↓

DEFINE MINIMAL CHANGE

    ↓

IMPLEMENT

    ↓

DOCUMENT

    ↓

VALIDATE

```

---

## 9. Semantic vs. Implementation Uncertainty

AI-assisted FIMS development must distinguish between implementation uncertainty and semantic uncertainty.

### 9.1 Implementation uncertainty

Implementation uncertainty concerns **how** an already-understood requirement should be implemented.

When intended behavior, scientific meaning, and software contracts are clear, AI should generally resolve routine implementation decisions using existing FIMS conventions and sound engineering judgment.

Examples include:

- selecting a clear local variable name;

- deciding whether a local value should be `const`;

- extracting a small helper function;

- eliminating an unnecessary copy;

- following an established testing pattern; or

- selecting between genuinely equivalent implementations when semantics and numerical behavior are preserved.

> **Resolve implementation uncertainty using established FIMS patterns; do not unnecessarily interrupt the developer for routine engineering decisions.**

### 9.2 Semantic uncertainty

Semantic uncertainty concerns **what the software is supposed to mean or do**.

AI must not resolve semantic uncertainty by assumption.

Examples include uncertainty about:

- whether a parameter represents `F`, `M`, or `Z`;

- whether an index represents age or year;

- whether selectivity should be normalized;

- whether a parameter is estimated or fixed;

- whether a quantity is on the natural or log scale;

- whether an operation should participate in AD;

- whether a random effect should become fixed;

- whether an API change is intended to alter existing behavior; or

- whether a different likelihood formulation is intended to represent the same statistical model.

> **Never guess across a semantic boundary.**

When an unresolved decision could alter scientific meaning, statistical formulation, numerical behavior, public API behavior, model structure, or interpretation of FIMS output, establish intended semantics from authoritative FIMS code, documentation, tests, scientific references, or explicit developer direction before proceeding.

### 9.3 Conflicting semantic evidence

Existing implementation is evidence of intent, but it is not automatically authoritative.

If documentation, tests, scientific formulations, and implementation disagree:

> **Conflicting semantic evidence must be surfaced, not silently reconciled.**

The AI should identify the conflict and seek resolution rather than arbitrarily selecting one interpretation.

### 9.4 Suggested evidence hierarchy

The following is a useful guide rather than an absolute ranking:

```text

Explicit developer requirement

        ↓

Established scientific formulation

        ↓

Documented FIMS interface/contract

        ↓

Consistent existing FIMS architecture

        ↓

Tests and validation cases

        ↓

Current implementation

        ↓

General programming convention

        ↓

AI preference

```

General programming conventions do not override FIMS semantics.

AI preference has no authority to redefine scientific meaning.

### 9.5 Unfamiliarity is not ambiguity

The AI must distinguish between code that is genuinely ambiguous and code that is merely unfamiliar.

Established fisheries notation, FIMS-specific architectural patterns, or domain conventions should be investigated before being labeled unclear or redesigned.

### 9.6 Local autonomy, global restraint

> **Exercise local implementation autonomy while maintaining global semantic restraint.**

```text

Should this local variable be const?

    → AI can decide.

Can this unnecessary copy be avoided?

    → AI can decide.

Should the existing FIMS construction pattern be followed?

    → Usually yes.

Should selectivity be normalized?

    → Stop if semantics are unclear.

Is this parameter log-scale?

    → Stop if semantics are unclear.

Should this random effect become fixed?

    → Stop.

Can the R-facing behavior change?

    → Stop unless explicitly requested or established.

Can an algebraically equivalent likelihood expression replace

an existing numerically stable formulation?

    → Not without establishing statistical, numerical, and

      derivative equivalence.

```

---

## 10. Testing and Validation

### 10.1 Test scientific behavior, not merely implementation details

AI-generated tests should preferentially verify meaningful contracts rather than simply reproducing values that the implementation itself just assigned.

Tests should, where appropriate, verify:

- scientific relationships;

- known analytical solutions;

- invariants;

- boundary behavior;

- edge cases;

- numerical stability;

- derivative correctness;

- interface contracts;

- cross-language consistency; and

- regression behavior.

> **Tests should make incorrect scientific implementations difficult to pass.**

### 10.2 Validate derivatives when relevant

For inference-path code, validation may need to include:

```text

value

  ↓

gradient

  ↓

Hessian, where relevant

  ↓

boundary behavior

```

Correct objective values do not compensate for incorrect derivative behavior.

---

## 11. AI-Specific Behavioral Rules

### 11.1 Never fabricate architectural knowledge

Do not invent FIMS APIs, classes, interfaces, fields, function signatures, or expected behavior when they can be established by examining the codebase.

Expected workflow:

```text

Search → Understand → Verify → Implement

```

not:

```text

Assume → Generate → Hope tests catch it

```

### 11.2 Do not silently redesign interfaces

A feature implementation is not implicit permission to redesign the surrounding API or architecture.

### 11.3 Do not silently change behavior

Do not change parameter interpretation, initialization, transformations, normalization, boundary handling, error behavior, likelihood formulation, derivative behavior, or public-facing semantics without explicit justification and validation.

### 11.4 Stop rather than guess at semantic boundaries

Do not ask the developer about every routine engineering choice. However, if a decision requires choosing between materially different scientific, statistical, architectural, or API semantics and the intended behavior cannot be established, surface the ambiguity before implementation.

---

## 12. Summary Doctrine

FIMS AI-assisted development should preserve a visible chain from scientific intent to user-facing behavior:

```text

Scientific meaning

        ↓

Mathematical/statistical formulation

        ↓

Domain model

        ↓

C++ implementation

        ↓

Native interface

        ↓

R API

        ↓

User-facing behavior

```

At each layer, preserve:

```text

Meaning

  ↓

Correctness

  ↓

Traceability

  ↓

Readability

  ↓

Maintainability

  ↓

Numerical integrity

  ↓

Performance

  ↓

Testability

```

> **AI-generated code must look, behave, and integrate as though it were written by a knowledgeable FIMS developer.**

> **AI has considerable freedom over how FIMS code is implemented. It has very little freedom over what FIMS code means.**

