# Should FIMS be verbose?

Verbosity is set globally for FIMS using
`options(rlib_message_verbosity = "quiet")` to stop the printing of
messages from
[`cli::cli_inform()`](https://cli.r-lib.org/reference/cli_abort.html).
Using a global option allows for verbose to not have to be an argument
to every function. All
[`cli::cli_abort()`](https://cli.r-lib.org/reference/cli_abort.html)
messages are printed to the console no matter what the global option is
set to.

## Usage

``` r
is_fims_verbose()
```

## Value

A logical is returned where `TRUE` ensures messages from
[`cli::cli_inform()`](https://cli.r-lib.org/reference/cli_abort.html)
are printed to the console.

## Examples

``` r
# function is not exported
if (FALSE) { # \dontrun{
FIMS:::is_fims_verbose()
} # }
```
