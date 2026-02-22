# GitHub Copilot Instructions for FIMS

These guidelines are meant to help Copilot generate suggestions that fit the structure and development practices used in FIMS.  
Copilot should treat the existing codebase, especially the `tests/` directory, as the main reference before suggesting any new patterns or structures.

## General Development Guidelines

- Always follow the existing C++ and R coding style used in the repo.
- Do not introduce new structural patterns or large stylistic rewrites unless the issue specifically asks for them.
- Avoid adding new dependencies without discussion.
- Prefer clear and maintainable code over overly clever or complicated ones.
- If a change significantly affects behavior or structure, include a clear explanation and avoid it unless it meaningfully improves the performance or correctness.

## Scope and Change Discipline

- Keep AI generated changes small and focused on the issue requirements.
- Do not mix formatting only edits with functional changes.
- Avoid refactors that are not directly related to the given issue.

## C++ Code and Model Structure

- Follow the existing initialization and configuration patterns used in model classes, for example : `Population`, `Fleet`, integration helpers, etc.
- Do not restructure model setup logic unless explicitly required.
- Maintain consistency between C++ logic and the Rcpp interface.
- Do not modify public interfaces without updating related tests and documentation.

## C++ Testing – Google Test

Before generating or modifying C++ tests, review the `tests/gtest/` directory.

- New test files should follow the pattern:  
  `tests/gtest/test_FileName_ClassName_FunctionName.cpp`
- When creating new C++ tests, first suggest using  
  `FIMS:::use_gtest_template(name = "FileName_ClassName_FunctionName")`  
  which generates the test file and registers it in `CMakeLists.txt`.
- Do not prioritize manual creation of test files or manual editing of `CMakeLists.txt` unless necessary.
- If registering tests manually, ensure they are linked to both `gtest_main` and `fims_test`, and use `gtest_discover_tests()`.
- Reuse existing fixture patterns and helper structures.
- Treat existing files in `tests/gtest/` as the reference instead of introducing new test styles.

## R Testing – testthat

Before generating or modifying R tests, review the `tests/testthat/` directory.

- New test files should follow the pattern:  
  `tests/testthat/test-function_name.R`
- When creating new R tests, first suggest using  
  `FIMS:::use_testthat_template("function_name")`.
- Reuse existing helper functions and integration helpers where possible.
- Store reusable test data in `tests/testthat/fixtures/` following the current conventions.
- Treat existing test files as the reference instead of creating any new patterns.

## Logic and Behavioral Changes

- Any change to logic or behavior should include corresponding tests.
- If modifying JSON parsing, model configuration, or integration logic, ensure tests reflect the updated behavior.
- Avoid silent behavior changes without corresponding test updates.

## Documentation

- If functionality changes, update the relevant comments, Roxygen blocks, or other documentation as per the requirements.