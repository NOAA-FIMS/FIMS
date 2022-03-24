#' use_module
#'
#' @description This function will generate the necessary files to
#' add a new module to the FIMS system.
#'
#' @param module_name
#'
#' @return
#' @export
#'
#' @examples
use_module <- function(module_name){

  if(typeof(module_name)!="character"){
   warning(paste(module_name, " is not of the correct type, please enter
                 a string.", sep=""))
  }

  return(TRUE)
}
