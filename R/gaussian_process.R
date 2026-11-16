# Internal configuration for the opt-in Gaussian process pathway.
normalize_gaussian_process <- function(process) {
  if (!is.list(process) || is.null(names(process)) ||
      anyDuplicated(names(process)) ||
      any(!names(process) %in% c("type", "rho", "estimation_type"))) {
    cli::cli_abort("{.var process} must be a named list with type, rho, and/or estimation_type.")
  }
  if (is.null(process$type) || length(process$type) != 1 ||
      is.na(process$type) || !process$type %in% c("iid", "ar1")) {
    cli::cli_abort("Gaussian process type must be {.val iid} or {.val ar1}.")
  }
  rho <- if (is.null(process$rho)) 0 else process$rho
  if (!is.numeric(rho) || length(rho) != 1 || !is.finite(rho) || abs(rho) >= 1) {
    cli::cli_abort("Gaussian process rho must be one finite number strictly between -1 and 1.")
  }
  estimation_type <- process$estimation_type
  if (is.null(estimation_type)) {
    estimation_type <- if (process$type == "ar1") "fixed_effects" else "constant"
  }
  if (length(estimation_type) != 1 || is.na(estimation_type) ||
      !estimation_type %in% c("constant", "fixed_effects")) {
    cli::cli_abort("Correlation estimation_type must be constant or fixed_effects.")
  }
  if (process$type == "iid" && (rho != 0 || estimation_type != "constant")) {
    cli::cli_abort("IID requires constant zero correlation.")
  }
  list(type = process$type, rho = rho, estimation_type = estimation_type)
}

# Validate high-level metadata without changing the active model.
validate_recruitment_process <- function(parameters, data, process) {
  process <- normalize_gaussian_process(process)
  rows <- parameters |>
    dplyr::filter(.data$module_name == "Recruitment",
                  .data$label %in% c("log_devs", "log_r"))
  years <- get_start_year(data) + seq_len(get_n_years(data) - 1L)
  if (length(unique(rows$label)) != 1 || nrow(rows) != length(years) ||
      anyNA(rows$timing) || !identical(as.numeric(rows$timing), as.numeric(years))) {
    cli::cli_abort("Gaussian recruitment requires one target in contiguous annual order for all modeled recruitment years.")
  }
  if (anyNA(rows$value) || any(!is.finite(rows$value)) ||
      anyNA(rows$estimation_type) ||
      any(!rows$estimation_type %in% c("fixed_effects", "random_effects"))) {
    cli::cli_abort("Gaussian recruitment requires finite estimated targets; fixed elements are not supported.")
  }
  if (anyNA(rows$distribution_type) || any(rows$distribution_type != "process") ||
      anyNA(rows$distribution) || any(rows$distribution != "Dnorm")) {
    cli::cli_abort("Gaussian recruitment requires Dnorm process rows for every target.")
  }
  sd_rows <- parameters |>
    dplyr::filter(.data$module_name == "Recruitment", .data$label == "log_sd")
  if (nrow(sd_rows) != 1 || !is.finite(sd_rows$value) ||
      !sd_rows$estimation_type %in% c("constant", "fixed_effects") ||
      is.na(sd_rows$distribution_type) || sd_rows$distribution_type != "process" ||
      is.na(sd_rows$distribution) || sd_rows$distribution != "Dnorm") {
    cli::cli_abort("Gaussian recruitment requires one finite Dnorm process log_sd with constant or fixed_effects estimation.")
  }
  process
}

# Bind an existing vector; parameter ownership remains with its module.
initialize_gaussian_process <- function(module, par, family, sd, process) {
  process <- normalize_gaussian_process(process)
  check_distribution_validity(list(family = family, sd = sd))
  if (family$family != "gaussian" ||
      length(sd$value) != 1 || !is.finite(sd$value) || sd$value <= 0 ||
      length(sd$estimation_type) != 1 || is.na(sd$estimation_type) ||
      !sd$estimation_type %in% c("constant", "fixed_effects")) {
    cli::cli_abort("Gaussian processes require a Gaussian family and one finite positive innovation SD with constant or fixed_effects estimation.")
  }
  if (!inherits(module, "Rcpp_BevertonHoltRecruitment") ||
      length(par) != 1 || !par %in% c("log_devs", "log_r")) {
    cli::cli_abort("Gaussian recruitment supports BevertonHoltRecruitment log_devs or log_r targets.")
  }
  target <- module$field(par)
  if (length(target) == 0 ||
      any(!vapply(seq_len(length(target)), function(i) {
        is.finite(target[i]$value) &&
          target[i]$estimation_type$get() %in% c("fixed_effects", "random_effects")
      }, logical(1)))) {
    cli::cli_abort("Gaussian process targets must be nonempty, finite, and estimated; fixed elements are not supported.")
  }
  density <- methods::new(GaussianProcessDistribution)
  density$set_process(process$type)
  density$log_sd[] <- log(sd$value)
  density$log_sd$set_estimation_types(sd$estimation_type)
  density$unconstrained_rho[] <- atanh(process$rho)
  density$unconstrained_rho$set_estimation_types(process$estimation_type)
  density$observed_values[] <- rep(0, length(target))
  density$expected_values[] <- rep(0, length(target))
  ids <- target$get_id()
  if (par == "log_r") {
    ids <- c(ids, module$field("log_expected_recruitment")$get_id())
  }
  density$set_distribution_links("random_effects", ids)
  density
}
