#' Function to create the Rcpp interface classes
#' for FIMS model objects.
#' @param interface_name class name
#' @param model fims model (i.e. LogisticSelectivity) without "fims" namespace
#' @param base_class The interface base class
#' @param container The information model container
#' @param parameters The parameters of the model object
#' @return Text string with the code for the new Rcpp interface class
#' @export
#' @examples
#' create_fims_rcpp_interface(
#'   "LogisticSelectivityInterface",
#'   "LogisticSelectivity",
#'   "selectivity_interface_base",
#'   "selectivity_models",
#'   c("slope", "median")
#' )
create_fims_rcpp_interface <- function(interface_name = character(),
                                       model = character(),
                                       base_class = character(),
                                       container = character(),
                                       parameters = vector()) {
  types <- c(
    "TMB_FIMS_REAL_TYPE",
    "TMB_FIMS_FIRST_ORDER",
    "TMB_FIMS_SECOND_ORDER",
    "TMB_FIMS_THIRD_ORDER"
  )
  itypes <- c("d0", "d1", "d2", "d3")
  mtypes <- c("model0", "model1", "model2", "model3")
  # mtypes <- paste0(model, c("_model0", "_model1", "_model2", "_model3"))

  cat("class ")
  cat(interface_name)
  cat(" : public ")
  cat(base_class)
  cat(" {\n")
  cat("public:\n\n")
  # cat("     ")
  for (i in 1:length(parameters)) {
    cat("   Parameter ")
    cat(parameters[i])
    cat(";\n")
  }
  cat("\n\n   ")
  cat(interface_name)
  cat("() : ")
  cat(base_class)
  cat("() {}\n\n")


  cat("virtual ~")
  cat(paste0(interface_name, "() {}\n\n\n"))

  cat("virtual ")
  cat("uint32_t get_id(){return this->id;}\n\n\n")


  cat("virtual ")
  cat("bool add_to_fims_tmb(){\n")
  for (i in 1:4) {
    cat(paste0("    std::shared_ptr<fims::Information<", types[i]))
    cat(paste("> >", itypes[i]))
    cat(" =\n")
    cat("    fims::Information<")
    cat(types[i])
    cat(">::GetInstance();\n\n")
    cat("   std::shared_ptr<fims::")
    cat(model)
    cat("<")
    cat(types[i])
    cat("> > ")
    cat(mtypes[i])
    cat(" =\n")
    cat("   std::make_shared<fims::")
    cat(model)
    cat("<")
    cat(types[i])
    cat("> >();")
    cat("\n\n")

    cat("\n   ")
    cat(mtypes[i])
    cat("->id = this->id;\n   ")

    for (j in 1:length(parameters)) {
      cat(mtypes[i])
      cat("->")
      cat(parameters[j])
      cat("= this->")
      cat(parameters[j])
      cat(".value;\n   ")
      cat("if (this->")
      cat(parameters[j])
      cat(".estimated) {\n  ")
      cat("      if (this->")
      cat(parameters[j])
      cat(".is_random_effect) {\n          ")
      cat(itypes[i])
      cat("->RegisterRandomEffect(")
      cat(mtypes[i])
      cat("->")
      cat(parameters[j])
      cat(");\n")
      cat("   } else {\n      ")
      cat(itypes[i])
      cat("->RegisterParameter(")
      cat(mtypes[i])
      cat("->")
      cat(parameters[j])
      cat(");\n")
      cat("   }\n} \n")
    }
    cat("   ")
    cat(itypes[i])
    cat("->")
    cat(container)
    cat("[")
    cat(mtypes[i])
    cat("->id]=")
    cat(mtypes[i])
    cat(";\n\n\n")
  }

  cat("return true;\n\n\n")

  cat("}\n\n};")

  cat("\n//Add the following to the RCpp module definition: rcpp_interface.hpp\n\n")

  cat("Rcpp::class_<")
  cat(interface_name)
  cat(">(\"")
  cat(sub("Interface", "", interface_name))
  cat("\"")
  cat(")\n.constructor()\n")
  cat(".method(\"get_id\",  &")
  cat(interface_name)
  cat("::get_id)\n")
  for (i in 1:length(parameters)) {
    cat(".field(\"")
    cat(parameters[i])
    cat("\", &")
    cat(interface_name)
    cat("::")
    cat(parameters[i])
    cat(")")
    if (i < length(parameters)) {
      cat("\n")
    }
  }
  cat(";")
}
