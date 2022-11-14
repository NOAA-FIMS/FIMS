# Contributing to tools in the NOAA Fisheries Toolbox

The following is a set of guidelines for contributing to tools hosted on the NOAA Fisheries Integrated Toolbox, which are hosted in the [NMFS Fish Tools Organization](https://github.com/nmfs-fish-tools) on GitHub. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request. Heavily influenced by Atom's [CONTRIBUTING.md](https://github.com/atom/atom/blob/master/CONTRIBUTING.md).

#### Table Of Contents

[Code of Conduct](#code-of-conduct)

[How Can I Contribute?](#how-can-i-contribute)
  * [Reporting Bugs](#reporting-bugs)
  * [Suggesting Enhancements](#suggesting-enhancements)
  * [Your First Code Contribution](#your-first-code-contribution)
  * [Pull Requests](#pull-requests)

[Additional Notes](#additional-notes)
  * [Issue Labels](#issue-labels)

## Code of Conduct

This project and everyone participating in it is governed by the [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to [fisheries.toolbox@noaa.gov](mailto:fisheries.toolbox@noaa.gov).

## I don't want to read this whole thing I just have a question!!!

> **Note:** Please don't file an issue to ask a question. You'll get faster results by using the resources below.

Please email the toolbox maintainers [fisheries.toolbox@noaa.gov](mailto:fisheries.toolbox@noaa.gov) or the owners of the specific repository if you have a question.

## How Can I Contribute?

Not all contributions are code! Writing documentation, teaching, and other activities are also key parts of contributing to software. For more information about open source contributions, see [the Open Source Guide's How to Contribute page](https://opensource.guide/how-to-contribute/).

### Reporting Bugs

This section guides you through submitting a bug report for any toolbox tool. Following these guidelines helps maintainers and the community understand your report, reproduce the behavior, and find related reports.

#### Before Submitting A Bug Report

* **Check if it is related to version.** We recommend using `sessionInfo()` within your `R` console and submitting the results in your bug report. Also, please check your R version against the required R version in the DESCRIPTION file and update if needed to see if that fixes the issue.
* **Perform a cursory search of issues** to see if the problem has already been reported. If it has **and the issue is still open**, add a comment to the existing issue instead of opening a new one. If it has **and the issue is closed**, open a new issue and include a link to the original issue in the body of your new one.

#### How Do I Submit A (Good) Bug Report?

Bugs are tracked as [GitHub issues](https://guides.github.com/features/issues/). Create an issue on the toolbox Github repository and provide the following information by following the steps outlined in the [reprex package](https://reprex.tidyverse.org/articles/reprex-dos-and-donts.html). 

Explain the problem and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem.
* **Describe the exact steps which reproduce the problem** in as many details as possible. For example, start by including the output of `sessionInfo()` and details on your operating system and whether you encountered the issue in base `R` or RStudio.
* **Provide specific examples to demonstrate the steps**. Include links to GitHub projects, or copy/pasteable snippets, which you use in those examples. If you're providing snippets in the issue, use the [reprex package](https://reprex.tidyverse.org/articles/reprex-dos-and-donts.html). 
* **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
* **Explain which behavior you expected to see instead and why.**
* **Include screenshots and animated GIFs** which show you following the described steps and clearly demonstrate the problem if there is a graphical issue. You can use [LICEcap](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [silentcast](https://github.com/colinkeenan/silentcast) on Linux.
* **If the problem wasn't triggered by a specific action**, describe what you were doing before the problem happened and share more information using the guidelines below.

Provide more context by answering these questions:

* **Did the problem start happening recently** (e.g. after updating to a new version of R) or was this always a problem?
* If the problem started happening recently, **can you reproduce the problem in an older version of R?** What's the most recent version in which the problem doesn't happen? 
* **Can you reliably reproduce the issue?** If not, provide details about how often the problem happens and under which conditions it normally happens.
* If the problem is related to working with files (e.g. reading in data files), **does the problem happen for all files and projects or only some?** Does the problem happen only when working with local or remote files (e.g. on network drives), with files of a specific type (e.g. only JavaScript or Python files), with large files or files with very long lines, or with files in a specific encoding? Is there anything else special about the files you are using?

Include details about your configuration and environment:

* **Which version of the tool are you using?** 
* **What's the name and version of the OS you're using?**
* **Which packages do you have installed?** You can get that list by running `sessionInfo()`.


### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for toolbox packages, including completely new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion and find related suggestions.

Before creating enhancement suggestions, please check the issues list as you might find out that you don't need to create one. When you are creating an enhancement suggestion, please include an "enhancement" tag in the issues.

#### Before Submitting An Enhancement Suggestion

* **Check you have the latest version of the package.**
* **Check if the development branch has that enhancement in the works.**
* **Perform a cursory search of the issues and enhancement tags** to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.

#### How Do I Submit A (Good) Enhancement Suggestion?

Enhancement suggestions are tracked as [GitHub issues](https://guides.github.com/features/issues/). Create an issue on the repository and provide the following information:

* **Use a clear and descriptive title** for the issue to identify the suggestion.
* **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
* **Provide specific examples to demonstrate the steps**. Include copy/pasteable snippets which you use in those examples, as [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
* **Include screenshots and animated GIFs** which help you demonstrate the steps if needed. You can use [licecap](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [silentcast](https://github.com/colinkeenan/silentcast) on Linux.
* **Explain why this enhancement would be useful.** 
* **List some other text editors or applications where this enhancement exists.**

### Your First Code Contribution

Unsure where to begin contributing? Some repositories include `help-wanted` issues in their issue list. This is a good place to start!

Both issue lists are sorted by total number of comments. While not perfect, number of comments is a reasonable proxy for impact a given change will have.

To learn more about workflows for contributing code, see the [Github collaborating with issues and pull requests guide](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests).

#### Make changes online

Github has an [online web-based editor](https://docs.github.com/en/codespaces/the-githubdev-web-based-editor) which makes contributing simple changes (like editing a typo) easy.

#### Local development

Code changes can be done locally using the following Github workflow: 
- Clone the package locally
- Create a feature branch for your change with a meaningful name. For example, if you are adding a biomass graph, you might call it biomassgraph. From the command line:
```console
git checkout -b biomassgraph
```
- Make your change in your local copy of the repository.
- For testing R code: Restart R, load the package locally from the R console:
```r
devtools::load_all(".")
``` 
Then test your change.
- For documenting R code: Add or edit documentation using the `roxygen` comment standard. A good overview is [here](http://r-pkgs.had.co.nz/man.html). If you are adding a new function, please include an example that illustrates use of your function that works with only the data included in the package.
- For testing R code: Run:
```r
devtools::check()
``` 
to ensure the package can still be built with your change.
- Submit a pull request. Please follow pull request [best practices.](https://www.atlassian.com/blog/git/written-unwritten-guide-pull-requests)
- Once your pull request is approved and merged in, delete your feature branch.

### Pull Requests

Please follow these steps to have your contribution considered by the maintainers:

1. Follow all instructions above.
2. For R code: Follow the [R style guide](https://style.tidyverse.org/) when writing your code.
3. After you submit your pull request, verify that all [status checks](https://help.github.com/articles/about-status-checks/) are passing. <details><summary>What if the status checks are failing?</summary>If a status check is failing, and you believe that the failure is unrelated to your change, please leave a comment on the pull request explaining why you believe the failure is unrelated. A maintainer will re-run the status check for you.</details>

While the prerequisites above must be satisfied prior to having your pull request reviewed, the reviewer(s) may ask you to complete additional changes before your pull request can be ultimately accepted.


## Additional Notes

### Issue Labels

This section lists the labels we use to help us track and manage issues and pull requests. Not all labels are used in each toolbox repository and some repositories may use additional labels. 
[GitHub search](https://help.github.com/articles/searching-issues/) makes it easy to use labels for finding groups of issues or pull requests you're interested in. We  encourage you to read about [other search filters](https://help.github.com/articles/searching-issues/) which will help you write more focused queries.

#### Type of Issue and Issue State

| Label name  | Description |
| --- | --- |
| `enhancement` | Feature requests. |
| `bug` |  Confirmed bugs or reports that are very likely to be bugs. |
| `question` |  Questions more than bug reports or feature requests (e.g. how do I do X). |
| `feedback` | General feedback more than bug reports or feature requests. |
| `help-wanted` |  The development team would appreciate help from the community in resolving these issues. |
| `more-information-needed` | More information needs to be collected about these problems or feature requests (e.g. steps to reproduce). |
| `needs-reproduction` | Likely bugs, but haven't been reliably reproduced. |
| `blocked` | Issues cannot currently be worked because it is blocked on other issues. |
| `duplicate`  | Issues which are duplicates of other issues, i.e. they have been reported before. |
| `wontfix` | The repository core development team has decided not to fix these issues for now, either because they're working as intended or for some other reason. |
