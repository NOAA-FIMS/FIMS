# Contributing to the NOAA Fisheries Integrated Modeling System

External contributions and feedback are important to the development and future maintenance of the Fisheries Integrated Modeling System (FIMS) and are welcome. This section provides contributing guidelines and workflows for developers of FIMS and the ecosystem surrounding FIMS. All contributors, both internal and external, are required to abide by the [Code of Conduct](https://github.com/NOAA-FIMS/.github/blob/main/CODE_OF_CONDUCT.md).

## TLDR --- I just have a question

Please use [GitHub Discussions](https://github.com/orgs/NOAA-FIMS/discussions) if you have a question. There is a special category set up for [Q&A](https://github.com/orgs/NOAA-FIMS/discussions/categories/q-a). But, you should also check out the other categories to see if your question has already been answered.

## Found a :bug:

Please use [GitHub Issues](https://github.com/NOAA-FIMS/FIMS/issues/new?template=bug_report.yml) to submit a :bug: report. If you have an idea for a well-defined enhancement to FIMS, please use GitHub to submit a [Feature request](https://github.com/NOAA-FIMS/FIMS/issues/new?template=feature_request.yml). Before submitting either of these tickets, please search the [existing issues](https://github.com/NOAA-FIMS/FIMS/issues?q=is%3Aissue) to see if something identical or similar has already been submitted. Once created, Issues are automatically tagged with the `status: triage_needed` tag and placed on the [Issue Triage Board](https://github.com/orgs/NOAA-FIMS/projects/21) where they will be labeled, given an assignee, and given a milestone by those in charge of the triage process.

## Contributing code

### Feature branches

Once an issue is approved and prioritized for work, a feature branch off of the dev branch, e.g., `dev-discards`, can be created to address the issue. Feature branches can be made within the repository or on a fork of the repository. When making a fork, be sure to fork all branches not just the default branch because feature branches should only be created off of main if they are a hotfix for the current release, e.g., `fix-pointer`. There are no hard and fast rules for branch names but we tend to use dashes to separate words. Often while working on this new feature branch, there will be updates to the dev branch. These updates must be brought in using `git rebase`. Branches that have merge commits will not be accepted into the code base. Please see the [GitHub documentation for rebasing](https://docs.github.com/en/get-started/using-git/about-git-rebase) for more information.

Along the development process it is important to add tests and ensure that the code is well documented. Below are the steps to follow to ensure that the code is ready for a pull request:
* Build the doxygen-generated C++ documentation using `cmake --build build`, where the resulting html files will be in build/html. This can also be done using `setup_and_run_gtest()` in R.
* Implement the suite of Google tests using `cmake --build build` and `ctest --test-dir build`. This can also be done using `setup_and_run_gtest()` in R. If there are failing tests, run `ctest --test-dir --rerun-failed --output-on-failure` to re-run the failed tests verbosely.
* Run clang format to format the C++ code using TODO: document how to run clang-format.
* Run clang tidy to check C++ for common mistakes using TODO: document how to run clang-tidy.
* Spell check the package using `spelling::spell_check_files(list.files(c("R", "tests", file.path("inst", "include")), recursive = TRUE, full.names = TRUE, pattern = "\\.cpp|\\.hpp|\\.md|\\.R$|\\.Rmd|\\.txt"), ignore = spelling::get_wordlist())`. Remove all the .md files in the vignettes directory and then spell check the package using `spelling::spell_check_package(pkg = ".", use_wordlist = TRUE)`.
* Build the roxygen documentation using `devtools::document()`. See the [r-lib](https://roxygen2.r-lib.org/) for all available tags and best practices.
* Implement the suite of testthat tests using `devtools::test()`.
* Run `styler::style_pkg()` to style R code.
* Run `devtools::check()` to ensure the package can be compiled and R tests pass. If there are failing tests, run `devtools::test(filter = "file_name")` (where "test-file_name.R" is the testthat file containing failing tests) and edit code/tests to troubleshoot tests. During development, run `devtools::build()` locally to build the package more frequently and faster.
* Build the package down site using `pkgdown::build_site()`.
* Run the code coverage report using `covr::report()` to ensure that all of the code is covered, with the goal of maintaining 80 percent code coverage.

### Pull requests

Once the work is complete, a pull request should be created to bring the changes into the appropriate branch. Pull requests from forks will be reviewed under the same guidelines as pull requests from internal branches. Review of the pull request will be both automated and manual, where all pull requests must pass automated tests and syntax checks before being merged in addition to being reviewed by at least one developer. [Code review](https://github.com/features/code-review) ensures health and continuous improvement of the FIMS codebase, while simultaneously helping FIMS developers become familiar with the codebase. [Google's code review guide](https://google.github.io/eng-practices/review/) provides a useful set of guidelines for both reviewers and code authors. Each PR is accompanied by a [checklist of major considerations for code reviews](https://github.com/NOAA-FIMS/FIMS/blob/main/.github/workflows/pr-checklist.yml) to guide reviewers through the PR. Additional guidance can be found at [Conventional Comments](https://conventionalcomments.org/) for formatting/structuring your reviewer comments. Navigate to the [Perforce Blog](https://www.perforce.com/blog/qac/9-best-practices-for-code-review) for nine best practices for code review and use the [FIMS Style Guide](#style-guide) to settle any style arguments.

Versioning follows [semantic versioning](https://semver.org/) to establish the major, minor, and patch number for a release. Often, multiple pull requests are included in a single release, which will be initiated by a member of the FIMS Implementation Team. Release notes for each version can be found in the [release notes](https://github.com/NOAA-FIMS/FIMS/releases).

## Non-code contributions

Not all contributions are code! Writing documentation, teaching, and other activities are also key parts of contributing to software that are much appreciated. For more information about open source contributions, see [the Open Source Guide's How to Contribute page](https://opensource.guide/how-to-contribute/). A good place to start is by looking for open issues with the [good first issue label](https://github.com/NOAA-FIMS/FIMS/contribute). These issues are generally small and well-defined, making them a good starting point for new contributors. We encourage collaborators to feel welcome to contribute in any way they can, and we will do our best to incorporate these contributions into the FIMS projects. We thank you in advance for being part of the team.

## Style guide

Style guides are used to ensure the code is consistent, easy to use (e.g., read, share, and verify) and ultimately easier to write. We use the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) for C++ code and the [tidyverse style guide](https://style.tidyverse.org/) for R code.

A self-imposed style within FIMS is the use of `typename` instead of `class` when defining templates for consistency with the `TMB` package. While types may be defined in many ways, we use `Type` instead of `T` to define Types.

Commit messages communicate details about changes that have occurred to collaborators and improve team efficiency. The best guidance for how to create an excellent commit message can be found on [Conventional Commits](https://www.conventionalcommits.org), which is our style guide for commit messages. See [Conventional Comments](https://conventionalcomments.org/) for how to format reviewer comments.
