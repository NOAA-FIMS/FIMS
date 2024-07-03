#' Fit FIMS model using nlminb
#'
#' Fits the compiled FIMS model using \code{\link[TMB:MakeADFun]{TMB::MakeADFun}} and
#' \code{\link[stats:nlminb]{stats::nlminb}} or
#' \code{\link[TMBhelper:fit_tmb]{TMBhelper:fit_tmb}}.
#'
#' @param input Named list
#' @param newtonsteps integer, number of additional Newton steps after optimization.
#' Passed to \code{\link{fit_tmb}}. Default = \code{3}.
#' @param do.sdrep T/F, calculate standard deviations of model parameters?
#' See \code{\link[TMB]{sdreport}}. Default = \code{TRUE}.
#' @param model (optional), a previously fit FIMS model.
#' @param do.check T/F, check if model parameters are identifiable?
#' Passed to \code{\link{fit_tmb}}. Runs internal function \code{check_estimability},
#' originally provided by https://github.com/kaskr/TMB_contrib_R/TMBhelper.
#' Default = \code{TRUE}.
#' @param MakeADFun.silent T/F, Passed to silent argument of
#' \code{\link[TMB:MakeADFun]{TMB::MakeADFun}}. Default = \code{FALSE}.
#' @param do.fit T/F, fit the model using \code{fit_tmb}. Default = \code{TRUE}.
#' @param save.sdrep T/F, save the full \code{\link[TMB]{TMB::sdreport}} object?
#' If \code{FALSE}, only save \code{\link[TMB:summary.sdreport]{summary.sdreport}}
#' to reduce model object file size. Default = \code{TRUE}.
#' @param filename Character string giving a file name to save the fitted object
#' as an RDS object. A value of NULL indicates not to save it.
#' If specified, it must end in .RDS. The file is written to folder given by input$path.
#' @param verbose T/F, whether to print output (default) or suppress as much as possible.
#' @param control A list of control parameters.
#' @return a fit TMB model with additional output if specified:
#'   \describe{
#'     \item{\code{$obj}}{Output from \code{\link[TMB::MakeADFun]{TMB::MakeADFun}}}
#'     \item{\code{$opt}}{Output from \code{\link[TMBHelper::fit_tmb]{TMBHelper::fit_tmb}}}
#'     \item{\code{$report}}{List of derived quantity estimates (see examples)}
#'     \item{\code{$parList}}{List of parameters}
#'     \item{\code{$std}}{formatted data frame}
#'     \item{\code{$sdrep}}{Parameter estimates (and standard errors if \code{do.sdrep=TRUE})}
#'   }
#'
#' @useDynLib FIMS
#' @export
#'
#' @examples
#' \dontrun{
#'
#' }
fit_fims = function(input, newtonsteps = 3, do.sdrep = TRUE,
                    model = NULL, do.check = FALSE,
                    MakeADFun.silent = FALSE,
                    do.fit = TRUE, save.sdrep = TRUE,
                    filename = 'fit.RDS', verbose = TRUE, control = NULL)
{
  # Check inputs


  # Create a TMB model

  FIMS::CreateTMBModel()
  obj <- TMB::MakeADFun(data = input$data,
                        parameters = input$parameters,
                        map = input$map,
                        random = input$random,
                        DLL = "FIMS",
                        silent = MakeADFun.silent)
  # Model not fit, but generate report and parList
  if(!do.fit) {
    model <- list(input = input,
                  obj = obj,
                  report = obj$report(),
                  parList = obj$env$parList())
    return(model)
  }

  # Fit model
  if(do.fit){
    # ToDO
    # - Calculate run time?

    # Fit FIMS model using nlminb
    opt <- TMBhelper::fit_tmb(obj, control = control,
                              newtonsteps = newtonsteps,
                              getsd = FALSE)

    if(do.sdrep){
      sdrep <- TMB::sdreport(obj)
      std <- summary(sdrep)
      std <- data.frame(dimnames(std)[[1]], std)
      names(std) <- c('name', 'est', 'se')
      row.names(std) <- NULL
    }

    fit <- list(input = input,
                obj = obj,
                opt = opt,
                report = obj$report(obj$env$last.par.best),
                parList = obj$env$parList(),
                sd = std)
    if(save.sdrep) fit <- c(fit, sdrep = sdrep)

  }

  # Check outputs

}
