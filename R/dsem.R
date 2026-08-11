#' Define a Dynamic Structural Equation Model (DSEM) process
#'
#' This helper function creates and links the necessary C++ objects to define a
#' DSEM for a given set of random effects. It abstracts away the low-level
#' details of creating a precision matrix builder (`DSEMInterface`) and linking
#' it to a GMRF likelihood component (`GMRFDistributionsInterface`).
#'
#' @param random_effects A `VariableVector` Rcpp object representing the random
#'   effects to be modeled by the DSEM process.
#' @param ram A data frame or matrix specifying the Reticular Action Model (RAM)
#'   paths. It must have 4 columns: `type` (1 for directed path, 2 for
#'   variance/covariance), `from` (1-based global index), `to` (1-based global
#'   index), and `beta_index` (1-based index into `parameters`).
#' @param parameters A data frame specifying the path coefficients. It must have
#'   columns `value` (initial value) and `estimation_type` ("fixed_effects",
#'   "random_effects", or "constant").
#' @param n_variables An integer specifying the number of unique variables being
#'   modeled at each time step.
#'
#' @return A list containing the configured `DSEMInterface` and
#'   `GMRFDistributionsInterface` Rcpp objects. These should be included in the
#'   list of components passed to `initialize_fims`.
#' @export
dsem <- function(random_effects, ram, parameters, n_variables) {
  # 1. Create the DSEM precision builder interface
  dsem_builder <- new(DSEMInterface)

  # 2. Configure the builder
  # Ensure n_time is an integer
  n_time <- as.integer(random_effects$size() / n_variables)
  if (n_time * n_variables != random_effects$size()) {
    stop("Size of 'random_effects' must be a multiple of 'n_variables'.")
  }
  dsem_builder$n_time <- n_time
  dsem_builder$n_variables <- as.integer(n_variables)
  dsem_builder$ram_matrix <- as.matrix(ram)

  # Configure beta_z (the path coefficients)
  dsem_builder$beta_z$resize(nrow(parameters))
  dsem_builder$beta_z$set_values(parameters$value)
  dsem_builder$beta_z$set_estimation_types(parameters$estimation_type)

  # Register the builder so initialize_fims() can discover it.
  dsem_builder$register_self(dsem_builder)

  # 3. Create the GMRF distribution interface
  gmrf_dist <- new(GMRFDistributionsInterface)

  # 4. Link the GMRF distribution to the precision builder
  gmrf_dist$set_precision_builder_id(dsem_builder$get_id())

  # 5. Link the GMRF distribution to the random effects vector
  gmrf_dist$set_distribution_links(input_type = "random_effects", ids = random_effects$get_id())

  # Return both objects to be added to the FIMS model configuration
  return(list(builder = dsem_builder, distribution = gmrf_dist))
}

