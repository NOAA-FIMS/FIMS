set_lognormal_landings <- function(
  fleet,
  cv,
  years,
  estimable = FALSE
) {

  dist <- methods::new(DlnormDistribution)

  # convert CV → log SD once
  log_sd_value <- sqrt(log(cv^2 + 1))

  dist$log_sd$resize(years)
  for (y in seq_len(years)) {
    dist$log_sd[y]$value <- log_sd_value
  }

  dist$log_sd$set_all_estimable(estimable)

  dist$set_observed_data(
    fleet$GetObservedLandingsDataID()
  )

  dist$set_distribution_links(
    "data",
    fleet$log_landings_expected$get_id()
  )

  invisible(dist)
}

fishing_fleet_landings_distribution <- 
  set_lognormal_landings(
    fleet = fishing_fleet,
    cv = em_input[["cv.L"]][["fleet1"]],
    years = om_input[["nyr"]]
  )

  set_landings_distribution <- function(
    fleet,
    family = NULL,
    sd = tibble::tibble(
      value = 1,
      estimation_type = "constant")
  {
  # Set up distribution based on `family` argument`
    if (family[["family"]] == "lognormal") {
      # create new Rcpp module
      new_module <- methods::new(DlnormDistribution)

      # populate logged standard deviation parameter with log of input
      # Using resize() and then assigning value to each element of log_sd directly
      # is correct, as creating a new ParameterVector for log_sd here would
      # trigger an error in integration tests with wrappers.
      new_module$log_sd$resize(length(sd[["value"]]))

      purrr::walk(
        seq_along(sd[["value"]]),
        \(x) new_module[["log_sd"]][x][["value"]] <- log(sd[["value"]][x])
      )

      purrr::walk(
        seq_along(sd[["estimation_type"]]),
        \(x) new_module[["log_sd"]][x][["estimation_type"]]$set(sd[["estimation_type"]][x])
      )
    }

    if (family[["family"]] == "gaussian") {
      # create new Rcpp module
      new_module <- methods::new(DnormDistribution)

      # populate logged standard deviation parameter with log of input
      purrr::walk(
        seq_along(sd[["value"]]),
        \(x) new_module[["log_sd"]][x][["value"]] <- log(sd[["value"]][x])
      )

      purrr::walk(
        seq_along(sd[["estimation_type"]]),
        \(x) new_module[["log_sd"]][x][["estimation_type"]]$set(sd[["estimation_type"]][x])
      )
    }

    new_module$set_observed_data(fleet$GetObservedLandingsDataID())
    new_module$set_distribution_links("data", fleet$log_landings_expected$get_id())

  }