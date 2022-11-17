#' Function to create the Rcpp interface classes
#' for FIMS model objects.
#' @param interface_name class name
#' @param model fims model (i.e. LogisticSelectivity) without "fims" namespace
#' @param base_class The interface base class
#' @param container The information model container
#' @param parameters The parameters of the model object
#' @param evaluate_parameter The parameter of the evaluate method
#' @param evaluate_parameter_type The type of the parameter of the evaluate method
#' @return Text string with the code for the new Rcpp interface class
#' @export
#' @examples
#' create_fims_rcpp_interface(
#'   interface_name = "DnormDistributionsInterface",
#'   model = "Dnorm",
#'   base_class = "DistributionsInterfaceBase",
#'   container = "distribution_models",
#'   parameters = c("x", "mean", "sd"),
#'   evaluate_parameter = "do_log",
#'   evaluate_parameter_type = "bool"
#' )
#' create_fims_rcpp_interface(
#'   interface_name = "LogisticSelectivityInterface",
#'   model = "LogisticSelectivity",
#'   base_class = "SelectivityInterfaceBase",
#'   container = "selectivity_models",
#'   parameters = c("slope", "median"),
#'   evaluate_parameter = "x",
#'   evaluate_parameter_type = "double"
#' )
create_fims_rcpp_interface <- function(interface_name = character(),
                                       model = character(),
                                       base_class = character(),
                                       container = character(),
                                       parameters = vector(),
                                       evaluate_parameter = vector(),
                                       evaluate_parameter_type = vector()) {
  types <- c(
    "TMB_FIMS_REAL_TYPE",
    "TMB_FIMS_FIRST_ORDER",
    "TMB_FIMS_SECOND_ORDER",
    "TMB_FIMS_THIRD_ORDER"
  )
  itypes <- c("d0", "d1", "d2", "d3")
  mtypes <- c("model0", "model1", "model2", "model3")

  cat("class ")
  cat(interface_name)
  cat(" : public ")
  cat(base_class)
  cat(" {\n")
  cat(" public:\n\n")
  for (i in 1:length(parameters)) {
    cat("  Parameter ")
    cat(parameters[i])
    cat(";\n")
  }
  cat("\n\n  ")
  cat(interface_name)
  cat("() : ")
  cat(base_class)
  cat("() {}\n\n")


  cat("  virtual ~")
  cat(paste0(interface_name, "() {}\n\n\n"))

  cat("  virtual uint32_t get_id() { return this->id; }\n\n\n")

  if (!is.null(evaluate_parameter)) {
    cat(paste0("  virtual double evaluate(", evaluate_parameter_type, " ", evaluate_parameter, ") {\n"))
    cat(paste0("    fims::", model, "<double> object;\n"))
    for (i in 1:length(parameters)) {
      cat(paste0("    object.", parameters[i], " = this->", parameters[i], ".value;\n"))
    }
    cat(paste0("    return object.evaluate(", evaluate_parameter_type, ");\n  }\n\n\n"))
  }

  cat("  virtual bool add_to_fims_tmb(){\n")
  for (i in 1:4) {
    cat(paste0("   std::shared_ptr<fims::Information<", types[i]))
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

  cat("\n\n//Add the following to the RCpp module definition: rcpp_interface.hpp\n\n")

  cat("Rcpp::class_<")
  cat(interface_name)
  cat(">(\"")
  cat(sub("Interface", "", interface_name))
  cat("\"")
  cat(")\n.constructor()\n")
  cat(".method(\"get_id\",  &")
  cat(interface_name)
  cat("::get_id)\n")
  if (!is.null(evaluate_parameter)) {
    cat(".method(\"evaluate\", &", interface_name, "::evaluate)\n")
  }
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
