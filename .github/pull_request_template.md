Thanks for opening a pull request (PR) to FIMS! Please describe the changes under the headings below. Please also go through the checklist below to ensure the appropriate steps have been taken during development. For smaller changes, feel free to skip steps that aren't relevant, but at least describe the feature under the "what is the feature".

# What is the feature?
A brief title of this change.

# How have you implemented the solution?
Describe the changes.

# Does it impact any other area of the project?
Please describe which features are impacted by this change.

# Does this change impact the model input or output?*
Please describe any way this change impacts the model input or output.

# How to test this change
Please include a test file and/or Github workflow.

# Developer pre-PR Checklist

Please do these steps locally for big changes. Note GitHub Actions does all of these checks automatically when pushing to the repository. Please see [code development section in the contributor guide](https://noaa-fims.github.io/collaborative_workflow/contributor-guidelines.html#code-development) for details.

- [ ] Run [cmake build and ctest locally](https://noaa-fims.github.io/collaborative_workflow/testing.html#c-unit-testing-and-benchmarking) and make sure the C++ tests pass
- [ ] Run `devtools::document()` locally and push changes to the remote feature branch
- [ ] Run `styler::style_pkg()` locally and push changes to remote feature branch. If there are many changes, please do this in a separate commit.
- [ ] Run `devtools::check()` locally and make sure the package can be compiled and R tests pass. If there are failing tests, run `devtools::test(filter = "file_name")` (where "test-file_name.R" is the testthat file containing failing tests) and edit code/tests to troubleshoot tests.
- [ ] Before opening the PR, make sure all the github actions are passing on the remote feature branch.
- [ ] Make sure this PR has an informative title.
