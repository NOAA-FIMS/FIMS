






add_to_fims_tmb <- function(interface_name = character, 
                            model = character(),
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
  
  
  cat("class ")
  cat(interface_name)
  cat("{\n")
  cat("public:\n\n")
  # cat("     ")
  for(i in 1:length(parameters)){
    cat("   parameter ")
    cat(parameters[i])
    cat(";\n")
    
  }
  cat("\n\n   ")
  cat(interface_name)
  cat("(){}\n\n")

  
 
  cat("bool add_to_fims_tmb(){\n")
  for (i in 1:4) {
    cat(paste0("    std::shared_ptr<fims::Information<", types[i]))
    cat(paste("> >", itypes[i]))
    cat(" =\n")
    cat("    fims::Information<")
    cat(types[i])
    cat(">::get_instance();\n\n")
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
    
    for (j in 1:length(parameters)) {
     cat("\n   ")
      cat(mtypes[i])
      cat("->id = this->id;\n   ")
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
      cat(mtypes[i])
      cat("->register_random_effect(")
      cat(mtypes[i])
      cat("->")
      cat(parameters[j])
      cat(");\n")
      cat("   } else {\n      ")
      cat(mtypes[i])
      cat("->register_parameter(")
      cat(mtypes[i])
      cat("->")
      cat(parameters[j])
      cat(");\n")
      cat("   }\n")
      
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
  cat("}\n\n};")
}


add_to_fims_tmb("logistic_selectivity","LogisticSelectivity",
                "selectivity_models",
                c("slope", "median"))
