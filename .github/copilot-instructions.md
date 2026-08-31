# GitHub Copilot Instructions for FIMS

These guidelines are meant to help Copilot generate suggestions that fit the structure and development practices used in FIMS.  
Copilot should treat the existing codebase, especially the `tests/` directory, as the main reference before suggesting any new patterns or structures.

Before suggesting or modifying code, Copilot must read and follow the [FIMS AI-Assisted Development Guidelines](../ai_guidelines.md). Those project-wide requirements govern scientific correctness, semantics, architecture, numerical and derivative integrity, documentation, performance, and validation. If this file and the project-wide guidelines differ, follow the stricter requirement and surface any semantic conflict that cannot be resolved from authoritative FIMS sources.

## General Development Guidelines

- Always follow the existing C++ and R coding style used in the repo.
  - For R: follow the [tidyverse style guide](https://style.tidyverse.org). Use tidyverse-style syntax instead of base R, and use the pipe operator (`|>`) to connect code.
  - For C++: follow conventions consistent with the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
  - For variables: Use full, descriptive names whenever possible.
  - For R object fields: Use `[[ ]]` instead of `$` when extracting fields from an object.
- Do not introduce new structural patterns or large stylistic rewrites unless the issue specifically asks for them.
- Avoid adding new dependencies without discussion.
- Prefer clear and maintainable code over overly clever or complicated ones.
- If a change significantly affects behavior or structure, include a clear explanation and avoid it unless it meaningfully improves the performance or correctness.

## Scope and Change Discipline

- Keep AI-generated changes small and focused on the issue requirements.
- Do not mix formatting-only edits with functional changes.
- Avoid refactors that are not directly related to the given issue.
- Do not invent FIMS APIs, classes, fields, function signatures, or behavior. Search the repository and verify them before use.
- Before implementation, identify the requested behavior, the nearest existing analogue, the affected contracts and invariants, and the relevant tests.

## Scientific and Semantic Integrity

- Preserve the scientific and statistical meaning of existing code. Compilation and plausible output are not sufficient evidence of correctness.
- Keep established fisheries terminology and mathematical relationships recognizable across the R API, native interface, C++ interface, domain model, and mathematical implementation.
- Preserve units, scales, dimensions, indexing conventions, parameter transformations, fixed/random-effect status, and initialization behavior.
- Do not silently normalize, clamp, reorder, convert, deduplicate, or otherwise reinterpret data.
- Use routine engineering judgment for implementation details, but do not guess when uncertainty could change scientific meaning, statistical formulation, numerical behavior, model structure, or public API behavior. Surface that uncertainty for maintainer direction.
- If code, tests, documentation, and scientific formulations conflict, identify the conflict instead of silently choosing one source.

## C++ Code and Model Structure

- Follow the existing initialization and configuration patterns used in model classes, for example: `Population`, `Fleet`, integration helpers, etc.
- Do not restructure model setup logic unless explicitly required.
- Maintain consistency between C++ logic and the Rcpp interface.
- Do not modify public interfaces without updating related tests and documentation.
- Preserve automatic-differentiation compatibility in inference-path code, including scalar types, differentiability, branch behavior, AD graph size, gradients, and Hessians where relevant.
- Do not replace a numerically stable expression merely because another expression is algebraically equivalent. Consider overflow, underflow, cancellation, boundary behavior, conditioning, and NaN/Inf propagation.
- In performance-sensitive paths, consider algorithmic complexity, copies, allocations, recomputation, memory layout, and AD graph growth before applying low-level optimizations. Benchmark changes that reduce readability or add complexity.

## C++ Testing – Google Test

Before generating or modifying C++ tests, review the `tests/gtest/` directory.

- New test files should follow the pattern:  
  `tests/gtest/test_FileName_ClassName_FunctionName.cpp`.
- When creating new C++ tests, first suggest using  
  `FIMS:::use_gtest_template(name = "FileName_ClassName_FunctionName")`  
  which generates the test file and registers it in `tests/gtest/CMakeLists.txt`.
- Do not prioritize manual creation of test files or manual editing of `tests/gtest/CMakeLists.txt` unless necessary.
- If registering tests manually, ensure they are linked to both `gtest_main` and `fims_test`, and use `gtest_discover_tests()`.
- Reuse existing fixture patterns and helper structures.
- Treat existing files in `tests/gtest/` as the reference instead of introducing new test styles.

## R Testing – testthat

Before generating or modifying R tests, review the `tests/testthat/` directory.

- New test files should follow the pattern:  
  `tests/testthat/test-function_name.R`.
- When creating new R tests, first suggest using  
  `FIMS:::use_testthat_template("function_name")`.
- Reuse existing helper functions and integration helpers where possible.
- Store reusable test data in `tests/testthat/fixtures/` following the current conventions.
- Treat existing test files as the reference instead of creating any new patterns.

## Logic and Behavioral Changes

- Any change to logic or behavior should include corresponding tests.
- If modifying JSON parsing, model configuration, or integration logic, ensure tests reflect the updated behavior.
- Avoid silent behavior changes without corresponding test updates.
- Prefer tests of scientific relationships, known analytical results, invariants, boundary behavior, numerical stability, interface contracts, and cross-language consistency over tests coupled only to implementation details.
- For inference-path changes, validate values and gradients, and validate Hessians and parameter-boundary behavior where relevant.

## Documentation

- If functionality changes, update the relevant comments, Roxygen blocks, or other documentation as per the requirements.
- Document scientific meaning, assumptions, units and scales, invariants, numerical considerations, ownership or lifetime constraints, and non-obvious architectural or performance decisions where relevant.
- Explain why non-obvious code exists; do not add comments that merely restate the code.
