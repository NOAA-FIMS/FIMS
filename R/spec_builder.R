#' Build FIMS Rcpp objects from R model specs
#'
#' `build_fims()` translates the declarative R specification layer into the
#' current Rcpp module interface. This first builder slice supports CatchAtAge
#' specs with EWAA growth, logistic maturity, Beverton-Holt recruitment,
#' logistic selectivity fleets, and landings/index observations with lognormal
#' data distributions.
#'
#' @param spec A FIMS model spec, such as one created by [catch_at_age()].
#' @param clear_existing Logical. If `TRUE`, clear existing FIMS Rcpp objects
#'   before building.
#' @param create_model Logical. If `TRUE`, call [CreateTMBModel()] after
#'   creating and linking Rcpp objects.
#'
#' @return
#' A list with the created Rcpp model, component objects, data objects, and
#' distributions. The returned object has class `FIMSBuilderResult`.
#'
#' @keywords model
#' @export
build_fims <- function(spec, clear_existing = TRUE, create_model = TRUE) {
  if (!inherits(spec, "fims_model_spec")) {
    stop("`spec` must be a FIMS model spec.", call. = FALSE)
  }
  if (spec[["model"]] != "catch_at_age") {
    stop("Only `catch_at_age` specs are supported by this builder.", call. = FALSE)
  }
  if (!is.logical(clear_existing) || length(clear_existing) != 1L ||
      is.na(clear_existing)) {
    stop("`clear_existing` must be TRUE or FALSE.", call. = FALSE)
  }
  if (!is.logical(create_model) || length(create_model) != 1L ||
      is.na(create_model)) {
    stop("`create_model` must be TRUE or FALSE.", call. = FALSE)
  }

  if (clear_existing) {
    clear()
  }

  population_spec <- spec[["population"]]
  n_years <- length(population_spec[["years"]])
  n_ages <- length(population_spec[["ages"]])

  recruitment <- build_recruitment(population_spec[["recruitment"]], n_years)
  growth <- build_growth(population_spec[["growth"]], n_years)
  maturity <- build_logistic_maturity(population_spec[["maturity"]])

  fleets <- list()
  selectivities <- list()
  for (fleet_spec in spec[["fleets"]]) {
    built_fleet <- build_fleet(fleet_spec, n_years, n_ages)
    fleets[[fleet_spec[["name"]]]] <- built_fleet[["fleet"]]
    selectivities[[fleet_spec[["name"]]]] <- built_fleet[["selectivity"]]
  }

  observations <- list()
  distributions <- list()
  for (observation_spec in spec[["observations"]]) {
    built_observation <- build_observation(
      observation_spec = observation_spec,
      fleets = fleets,
      n_years = n_years
    )
    observations[[length(observations) + 1L]] <- built_observation[["data"]]
    distributions[[length(distributions) + 1L]] <-
      built_observation[["distribution"]]
  }

  population <- build_population(
    population_spec = population_spec,
    recruitment = recruitment,
    growth = growth,
    maturity = maturity,
    fleets = fleets
  )

  model <- methods::new(CatchAtAge)
  model$AddPopulation(population$get_id())
  model$UseLikelihoodTerms(spec[["likelihood"]] == "terms")

  if (create_model) {
    CreateTMBModel()
  }

  result <- list(
    model = model,
    population = population,
    recruitment = recruitment,
    growth = growth,
    maturity = maturity,
    fleets = fleets,
    selectivities = selectivities,
    observations = observations,
    distributions = distributions
  )
  class(result) <- c("FIMSBuilderResult", "list")
  result
}

build_logistic_selectivity <- function(spec) {
  selectivity <- methods::new(LogisticSelectivity)
  set_scalar_parameter(selectivity$inflection_point, spec[["a50"]])
  set_scalar_parameter(selectivity$slope, spec[["slope"]])
  selectivity
}

build_logistic_maturity <- function(spec) {
  maturity <- methods::new(LogisticMaturity)
  set_scalar_parameter(maturity$inflection_point, spec[["a50"]])
  set_scalar_parameter(maturity$slope, spec[["slope"]])
  maturity
}

build_growth <- function(spec, n_years) {
  growth <- methods::new(EWAAGrowth)
  growth$n_years$set(n_years)
  set_real_vector(growth$ages, spec[["ages"]])

  weights <- as.numeric(spec[["weights"]])
  n_ages <- length(spec[["ages"]])
  expected_weight_count <- (n_years + 1L) * n_ages
  if (length(weights) == n_ages) {
    weights <- rep(weights, n_years + 1L)
  }
  if (length(weights) != expected_weight_count) {
    stop(
      "`weights` must have length `length(ages)` or `(n_years + 1) * length(ages)`.",
      call. = FALSE
    )
  }
  set_real_vector(growth$weights, weights)
  growth
}

build_recruitment <- function(spec, n_years) {
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment$n_years$set(n_years)
  set_scalar_parameter(recruitment$log_rzero, spec[["log_rzero"]])

  steepness <- spec[["steepness"]]
  steepness_value <- role_value(steepness)
  if (length(steepness_value) != 1L || steepness_value <= 0.2 ||
      steepness_value >= 1.0) {
    stop("`steepness` must be a numeric value between 0.2 and 1.0.", call. = FALSE)
  }
  steepness[["value"]] <- log(steepness_value - 0.2) - log(1.0 - steepness_value)
  set_scalar_parameter(recruitment$logit_steep, steepness)

  if (!is.null(spec[["deviations"]])) {
    set_parameter_vector(
      recruitment$log_devs,
      rep(0, n_years - 1L),
      estimation_type = "random_effects"
    )
  } else {
    set_parameter_vector(
      recruitment$log_devs,
      rep(0, n_years - 1L),
      estimation_type = "constant"
    )
  }
  recruitment
}

build_fleet <- function(spec, n_years, n_ages) {
  selectivity <- build_logistic_selectivity(spec[["selectivity"]])
  fleet <- methods::new(Fleet)
  fleet$SetName(spec[["name"]])
  fleet$n_years$set(n_years)
  fleet$n_ages$set(n_ages)
  fleet$n_lengths$set(0L)
  fleet$SetSelectivityID(selectivity$get_id())

  fishing_mortality <- spec[["fishing_mortality"]]
  if (is.null(fishing_mortality)) {
    set_parameter_vector(fleet$log_Fmort, rep(log(0.1), n_years))
  } else {
    value <- role_value(fishing_mortality)
    if (length(value) == 1L) {
      value <- rep(value, n_years)
    }
    if (length(value) != n_years) {
      stop("`fishing_mortality` must be scalar or have length `n_years`.",
           call. = FALSE)
    }
    set_parameter_vector(
      fleet$log_Fmort,
      value,
      estimation_type = role_estimation_type(fishing_mortality)
    )
  }
  set_parameter_vector(fleet$log_q, 0)
  list(fleet = fleet, selectivity = selectivity)
}

build_population <- function(population_spec, recruitment, growth, maturity,
                             fleets) {
  n_years <- length(population_spec[["years"]])
  n_ages <- length(population_spec[["ages"]])

  population <- methods::new(Population)
  population$SetName(population_spec[["name"]])
  population$n_years$set(n_years)
  population$n_ages$set(n_ages)
  population$n_fleets$set(length(fleets))
  population$n_lengths$set(0L)
  set_real_vector(population$ages, population_spec[["ages"]])
  set_parameter_vector(population$log_M, rep(log(0.2), n_years * n_ages))
  set_parameter_vector(population$log_f_multiplier, rep(0, n_years))
  set_parameter_vector(population$spawning_biomass_ratio, rep(1, n_years + 1L))
  set_parameter_vector(
    population$log_init_naa,
    log(rep(1000, n_ages)),
    estimation_type = "fixed_effects"
  )

  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(growth$get_id())
  population$SetMaturityID(maturity$get_id())
  for (fleet in fleets) {
    population$AddFleet(fleet$get_id())
  }
  population
}

build_observation <- function(observation_spec, fleets, n_years) {
  fleet_name <- observation_spec[["fleet"]]
  fleet <- fleets[[fleet_name]]
  if (is.null(fleet)) {
    stop("No fleet named `", fleet_name, "` was found.", call. = FALSE)
  }

  data_type <- observation_spec[["data_type"]]
  if (!data_type %in% c("landings", "index")) {
    stop(
      "This builder slice currently supports landings and index observations.",
      call. = FALSE
    )
  }

  data <- as.numeric(observation_spec[["data"]])
  if (length(data) != n_years) {
    stop("Observation data must have length `n_years`.", call. = FALSE)
  }

  if (data_type == "landings") {
    data_object <- methods::new(Landings, n_years)
    set_real_vector(data_object$landings_data, data)
    fleet$SetObservedLandingsDataID(data_object$get_id())
    expected_id <- fleet$log_landings_expected$get_id()
  } else {
    data_object <- methods::new(Index, n_years)
    set_real_vector(data_object$index_data, data)
    fleet$SetObservedIndexDataID(data_object$get_id())
    expected_id <- fleet$log_index_expected$get_id()
  }

  distribution <- build_data_distribution(
    observation_spec[["distribution"]],
    n_years = n_years
  )
  distribution$set_observed_data(data_object$get_id())
  distribution$set_distribution_links("data", expected_id)

  list(data = data_object, distribution = distribution)
}

build_data_distribution <- function(distribution_spec, n_years) {
  family <- distribution_spec[["family"]]
  if (family == "lognormal") {
    distribution <- methods::new(DlnormDistribution)
  } else if (family == "normal") {
    distribution <- methods::new(DnormDistribution)
  } else {
    stop(
      "This builder slice currently supports normal and lognormal data distributions.",
      call. = FALSE
    )
  }

  sd <- distribution_sd(distribution_spec)
  if (length(sd) == 1L) {
    sd <- rep(sd, n_years)
  }
  if (length(sd) != n_years) {
    stop("Distribution `sd` must be scalar or have length `n_years`.",
         call. = FALSE)
  }
  set_parameter_vector(distribution$log_sd, log(sd))
  distribution
}

distribution_sd <- function(distribution_spec) {
  if (inherits(distribution_spec, "family")) {
    sd <- distribution_spec[["fims_spec"]][["args"]][["sd"]]
  } else {
    sd <- distribution_spec[["args"]][["sd"]]
  }
  if (is.null(sd)) {
    sd <- 1
  }
  sd
}

set_scalar_parameter <- function(parameter_vector, role_spec) {
  set_parameter_vector(
    parameter_vector,
    role_value(role_spec),
    estimation_type = role_estimation_type(role_spec)
  )
}

set_parameter_vector <- function(parameter_vector, values,
                                 estimation_type = "constant") {
  parameter_vector$resize(length(values))
  if (length(estimation_type) == 1L) {
    estimation_type <- rep(estimation_type, length(values))
  }
  if (length(estimation_type) != length(values)) {
    stop("`estimation_type` must be scalar or match the value length.",
         call. = FALSE)
  }
  for (i in seq_along(values)) {
    parameter_vector[i]$value <- values[i]
    parameter_vector[i]$estimation_type$set(estimation_type[i])
  }
  invisible(parameter_vector)
}

set_real_vector <- function(real_vector, values) {
  real_vector$resize(length(values))
  for (i in seq_along(values)) {
    real_vector$set(i - 1L, values[i])
  }
  invisible(real_vector)
}

role_value <- function(role_spec) {
  role_spec[["value"]]
}

role_estimation_type <- function(role_spec) {
  switch(
    role_spec[["role"]],
    fixed_effect = "fixed_effects",
    prior = "fixed_effects",
    random_effect = "random_effects",
    constant = "constant",
    "constant"
  )
}
