reporter <- function() {
    ans <- list()
    set <- function(nm, x) {
        ans[[nm]] <<- x
        NULL
    }
    get <- function(nm) ans[[nm]]
    report <- function(x) {
        nm <- deparse(substitute(x))
        set(nm, x)
    }
    result <- function() ans
    namevec <- function() {
        lgts <- lengths(ans)
        rep(names(lgts), lgts)
    }
    dims <- function() {
        getd <- function(x) { d <- dim(x); if(is.null(d)) length(x) else d}
        lapply(ans, getd)
    }
    clear <- function() ans <<- list()
    environment()
}
ADREPORT_ENV <- reporter()
REPORT_ENV <- reporter()
##' @describeIn TMB-interface Can be used inside the objective function to report quantities for which uncertainties will be calculated by \link{sdreport}.
##' @param x Object to report
ADREPORT <- ADREPORT_ENV$report
##' @describeIn TMB-interface Can be used inside the objective function to report quantities via the model object using \code{obj$report()}.
REPORT <- REPORT_ENV$report

## Clear *all* exchange environments:
clear_all <- function() {
    OBS_ENV$clear()
    SIM_ENV$clear()
    REPORT_ENV$clear()
    ADREPORT_ENV$clear()
}