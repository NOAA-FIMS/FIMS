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