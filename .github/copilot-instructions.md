# GitHub Copilot Instructions for FIMS

These guidelines are meant to help Copilot generate suggestions that fit the structure and development practices used in FIMS.
Copilot should treat the existing codebase, especially the `tests/` directory as the main reference before suggesting any new patterns or structures.

## General Development Guidelines

- Always follow the existing C++ and R coding style used in the repo.
- Do not introduce any new structural patterns or large stylistic rewrites unless the issue specifically asks for them.
- Avoid adding any new dependencies without any discussion.
- Prefer clear and maintainable code over compact or overly clever and complicalted implementations.
- If any particular change significantly affects the behavior or structure then include a clear explanation on why, and try to avoid it unless it significantly improves the performance or if the current direction is wrong.

## Scope and Change Discipline

- Keep AI generated changes small and focused following requirements.
- Do not mix formatting only edits with functional changes.
- Avoid making refactors that are not directly related to the given issue.

## C++ Code and Model Structure

- Follow the existing initialization and configuration patterns used in model classes for example -> Population, fleet, integration helpers, etc.
- Do not restructure model setup logic unless explicitly required and mentioned.
- Maintain consistency between C++ logic and the Rcpp interface.
- Do not modify public interfaces without updating related tests and documentation.

## C++ Testing - Google Test

Before generating or modifying C++ tests, review the `tests/gtest/` directory, also

- Follow the naming convention `test_*.cpp`.
- Use `FIMS:::use_gtest_template()` when creating new test files.
- Ensure new test files are registered in `tests/gtest/CMakeLists.txt` using the same pattern as existing tests (`add_executable`, `target_link_libraries`,gtest_discover_tests`).
- Reuse existing fixture patterns and helper structures.
- Treat existing files in `tests/gtest/` as the reference instead of introducing new test styles.

## R Testing - testthat

Before generating or modifying R tests, review the `tests/testthat/` directory.

- Follow the naming convention `test-*.R`.
- Use `FIMS:::use_testthat_template()` when creating new test files.
- Reuse existing helper functions and integration helpers where possible.
- Store reusable test data in `tests/testthat/fixtures/` following current conventions.
- Treat existing test files as the reference instead of creating new patterns.

## Logic and Behavioral Changes

- Any change to logic or way of functioning should include the corresponding tests.
- If modifying JSON parsing, model configuration, or any integration logic, ensure that the tests reflect on the updated behavior.
- Avoid silent behavior changes without test updates.

## Documentation

- If functionality changes, then update relevant comments, Roxygen blocks, or other documentation as per the requirements.