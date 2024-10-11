#' Sets up a new distribution for a data type
#' @param data_type type of data. Options are: "index", "cpue", "agecomp", "lengthcomp"
#' @param module cpp fleet module that is linked to the data
#' @param family a description of the error distribution and link function to be used in the model. 
#' @param sd list with items, value, numeric vector or value indicating the initial value/s of the standard deviation, and estimated, boolean indicating whether or not sd is estimated 
#' @export
new_data_distribution <- function(data_type, module, family, 
                                  sd = list(value = 1, estimated = FALSE)){
    families <- c("lognormal", "gaussian", "multinomial")
    if(family$family == "normal") stop("use family = gaussian() instead")
    if(!(family$family %in% families)) stop("FIMS currently does not offer this distribution.")
    if((data_type == "agecomp" | data_type == "lengthcomp") & (family$family == "lognormal" | family$family == "gaussian")){
        stop("Did you mean family = multinomial()?")
    }
    if((data_type == "index" | data_type == "cpue") & family$family == "multinomial"){
        stop("Multinomial is not available for index or CPUE data")
    }

    if(data_type == "index" | data_type == "cpue"){
        obs_id_name <- "observed_index_data_id"
    }
    if(data_type == "agecomp" | data_type == "lengthcomp"){
        obs_id_name <- "observed_agecomp_data_id"
    }
    if(family$family == "lognormal"){
        new_module <- new(TMBDlnormDistribution)
        new_module$log_logsd <- new(ParameterVector, log(sd$value), length(sd$value))
        new_module$log_logsd$set_all_estimable(sd$estimated)
        if(family$link == "log") expected <- "log_expected_index"
        if(family$link == "identity") expected <- "expected_index"
    }
  
    if(family$family == "gaussian"){
        new_module <- new(TMBDnormDistribution)
        new_module$log_sd <- new(ParameterVector, log(sd$value), length(sd$value))
        new_module$log_sd$set_all_estimable(sd$estimated)
        if(family$link == "log") expected <- "log_expected_index"
        if(family$link == "identity") expected <- "expected_index"
    }

    if(family$family == "multinomial"){
        new_module <- new(TMBDmultinomDistribution)
        expected <- "proportion_catch_numbers_at_age"
    }
    if(data_type == "index" | data_type == "cpue"){
        new_module$set_observed_data(module$GetObservedIndexDataID())
    }
    if(data_type == "agecomp"){
        new_module$set_observed_data(module$GetObservedAgeCompDataID())
    }
    
    new_module$set_distribution_links("data", module$field(expected)$get_id())
    return(new_module)
}

#' Sets up a new distribution for a process
#' @param par parameter name
#' @param module cpp module that is linked to the parameter
#' @param family a description of the error distribution and link function to be used in the model. 
#' @param sd list with items, value, numeric vector or value indicating the initial value/s of the standard deviation, and estimated, boolean indicating whether or not sd is estimated 
#' is_random_effect boolean indicating whether or not the process is estimated as a random effect
#' @export
new_process_distribution <- function(par, module, family,
                           sd = list(value = 1, estimated = FALSE),
                           is_random_effect = FALSE){
    families <- c("lognormal", "gaussian")
    if(family$family == "normal") stop("use family = gaussian() instead")
    if(!(family$family %in% families)) stop("FIMS currently does not offer this distribution for processes.")
    if(family$family == "lognormal"){
        new_module <- new(TMBDlnormDistribution)
        new_module$log_logsd <- new(ParameterVector, log(sd$value), length(sd$value))
        new_module$log_logsd$set_all_estimable(sd$estimated)
    }
  
    if(family$family == "gaussian"){
        new_module <- new(TMBDnormDistribution)
        new_module$log_sd <- new(ParameterVector, log(sd$value), length(sd$value))
        new_module$log_sd$set_all_estimable(sd$estimated)
    }

    module$field(par)$set_all_random(is_random_effect)

    n_dim <- length(module$field(par))
    new_module$x <- new(ParameterVector, n_dim)
    new_module$expected_values <- new(ParameterVector, n_dim)
    for(i in 1:n_dim){
        new_module$x[i]$value <- 0
        new_module$expected_values[i]$value <- 0
    }

    new_module$set_distribution_links("random_effects", module$field(par)$get_id())

    return(new_module)

}

#' Lognormal family and link specification
#'
#' @param link link function association with family
#' @return An object of class "family"
#' @export
#' @examples
#' fam <- lognormal()
#' fam$family
#' fam$link
lognormal <- function(link = "log") {
  r <- list(family = "lognormal")
  f <- c(r, list(link = link), stats::make.link(link))
  class(f) <- "family"
  return(f)
}

#' Multinomial family and link specification
#'
#' @param link link function association with family
#' @return An object of class "family"
#' @export
#' @examples
#' fam <- multinomial()
#' fam$family
#' fam$link
multinomial <- function(link = "logit") {
  r <- list(family = "multinomial")
  f <- c(r, list(link = link), stats::make.link(link))
  class(f) <- "family"
  return(f)
}