#' Constructor for the "fimsfit" class
#' @param x Fitted object from \code{\link{fit_tmb}}
#' @return An object of class "fimsfit"
#' @export
fimsfit <- function(x){
  if(!is.list(x)){
    warning("Object passed to fimsfit is not a list -- something went wrong in fitting?")
    return(x)
  }
  if(is.null(x$version)) stop("No version found, something went wrong")
  class(x) <- c('fimsfit', 'list')
  x
}

#' Check if an object is of class fimsfit
#' @param x Returned list from \code{\link{fit_tmb}}
#' @export
is.fimsfit <- function(x) inherits(x, "fimsfit")

#' Check if an object is a list of fimsfit objects
#' @param x List of fits returned from \code{\link{fit_tmb}}
#' @export
is.fimsfits <- function(x){
  if(!is.list(x)) {
    warning("Object passed to is.fimsfits is not a list -- something went wrong")
    return(FALSE)
  }
  all(sapply(x, function(i) inherits(i, "fimsfit")))
}



#' Print summary of fimsfit object
#' @param fit Fitted object from \code{\link{fit_pk}}
#' @param ... Ignored for now
#' @return Summary printed to console
#' @method print fimsfit
#' @export
print.fimsfit <- function(fit, ...){
  cat("FIMS model version: ", fit$version, "\n")
  rt <- as.numeric(fit$timing$time_total, units='secs')
  ru <- 'seconds'
  if(rt>60*60*24) {
    rt <- rt/(60*60*24); ru <- 'days'
  } else if(rt>60*60) {
    rt <- rt/(60*60); ru <- 'hours'
  } else if(rt>60){
    rt <- rt/60; ru <- 'minutes'
  }

  cat("Total run time was", round(rt,2),  ru, '\n')
  cat("Number of parameters:", paste(names(fit$opt$num_pars),
                                     fit$opt$num_pars, sep='='),"\n")
  cat("Final maximum gradient=",
      sprintf("%.3g", fit$opt$max_gradient), "\n")
  cat("Marginal NLL=",  round(fit$opt$objective,5), "\n")
  cat("Total NLL=", round(fit$rep$jnll,5), "\n")
  cat("Terminal SSB=", sapply(fit$rep$ssb, function(x) tail(x,1)))
}



#' Fit a FIMS model (BETA)
#' @param input Input list as returned by
#'   \code{prepare_input}.
#' @param newtonsteps The number of Newton steps using the inverse
#'    Hessian to do after optimization.
#' @param control A list of optimizer settings passed to code{nlminb}
#' @param getsd Calculate and return sdreport?
#' @param do.fit Optimize or return obj? Used for testing.
#' @param save.sdrep Whether to return the sdreport object in the
#'   fitted model. This is rarely used and large so turned off by
#'   default. When returned it is named `sdrep`.
#' @param filename Character string giving a file name to save
#'   the fitted object as an RDS object. Defaults to 'fit.RDS',
#'   and a value of NULL indicates not to save it. If specified,
#'   it must end in .RDS. The file is written to folder given by
#'   \code{input$path}.
#' @param verbose Whether to print output (default) or suppress
#'   as much as possible.
#' @return A list object of class 'fimsfit' which contains a
#'   "version" model name, rep, parList (MLE in list format), opt
#'   as returned by \code{nlminb}, std (formatted data frame) and sdrep if
#'   \code{getsd=TRUE}, and the obj.
#' @details This function is a beta version still and subject to change
#'   without warning.
#' @export
fit_fims <- function(input, getsd=TRUE, loopnum=3, do.fit=TRUE, newtonsteps=0,
                     control=NULL, verbose=TRUE, save.sdrep=FALSE,
                     filename=NULL){
if(!is.null(input$random)) stop("Random effects declared but not implemetned yet")
if(newtonsteps>0) stop("Newton steps not implemented yet")
stopifnot(loopnum>=0)
obj <- MakeADFun(data=list(), parameters=input$parameters,
                 map=input$map, random=input$random,
                 DLL='FIMS', silent=TRUE)
if(!do.fit) return(obj)
# to do: max this update elements that are not supplied by default
if(is.null(control))
  control <- list(eval.max=10000, iter.max=10000, trace=0)
if(!verbose) control$trace <- 0
## optimize and compare
t0 <- Sys.time()
 if(verbose) message("Starting optimization...")
opt0 <- opt <-
  with(obj, nlminb(start = par, objective = fn, gradient = gr, control=control))
maxgrad0 <- maxgrad <- max(abs(obj$gr(opt$par)))
if(loopnum>0){
  if(verbose) message("Restarting optimizer ", loopnum, " times silently to improve gradient")
    for(ii in 2:loopnum){
      control$trace <- 0
    opt <- with(obj, nlminb(start = opt$par, objective = fn,
                            gradient = gr, control=control))
    maxgrad <- max(abs(obj$gr(opt$par)))
  }
  if(verbose) message("Maximum gradient went from ", sprintf("%.3g", maxgrad0), " to ",
          sprintf("%.3g",maxgrad), " after ", loopnum," steps.")
}
n_total <- length(obj$env$last.par.best)
n_fe <- length(obj$par)
opt$num_pars <- list(total=n_total, fixed_effects=n_fe, random_effects=n_total-n_fe)
if(is.null(input$version)) {
  warning("No model version string provided, using default of 'FIMS model'")
  input$version <- 'FIMS model'
}
time_optimization <- Sys.time() - t0
if(verbose) message("Finished optimization")
opt$max_gradient <- maxgrad

rep <- c(version=input$version, obj$report())
sdrep <- std <- NULL
time_sdreport <- NA
if(getsd){
  t2 <- Sys.time()
  sdrep <- TMB::sdreport(obj)
  std <- summary(sdrep)
  std <- data.frame(dimnames(std)[[1]], std)
  names(std) <- c('name', 'est', 'se')
  std$lwr <- std$est - 1.96*std$se
  std$upr <- std$est + 1.96*std$se
  row.names(std) <- NULL
  if(verbose) message("Finished sdreport")
  time_sdreport <- Sys.time() - t2
}
parList <- obj$env$parList()
parnames <- names(obj$par)
parnames <- as.vector((unlist(sapply(unique(parnames), function(x){
  temp <- parnames[parnames==x]
  if(length(temp)>1) paste0(temp,'[',1:length(temp),']') else temp
}))))
timing <- list(time_optimization=time_optimization,
               time_sdreport=time_sdreport,
               time_total=Sys.time() - t0)
fit <- list(version=input$version,
            rep=rep, opt=opt, sd=std, timing=timing,
            obj=obj, parList=parList, input=input, parnames=parnames)
if(save.sdrep) fit <- c(fit,sdrep=sdrep)
class(fit) <- c('fimsfit', 'list')
if(verbose) print(fit)
if(!is.null(filename)) {
  warning("Saving output to file is not yet implemented")
  ## saveRDS(fit, file=paste0(input$path,'/', filename))
}
return(fit)
}
