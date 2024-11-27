
#' Create an immutable, enumerated environment object of interger types
#' for a FIMS Model.
#'
#' @description
#' This function generates a immutable environment of values.
#' @return A immutable environment containing enumerated values starting at 1L.
#' @export
#' @examples
#' \dontrun{
#` selex<-FIMS::enum("LOGISTIC","DOUBLE_LOGISTIC")
#` ls(selex)
#' }
 enum <- function(...){
   args <- list(...)
   ret<-list()
   for(i in 1:length(args)){
     ret[args[[i]]]<-as.integer(i)
   }
   enum_env <- list2env(ret)
   #make it immutable
   lockEnvironment(enum_env, bindings = TRUE)
   return(enum_env)
 }
