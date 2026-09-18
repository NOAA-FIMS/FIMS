# Independent checks for observation-date biological products.
test_that("dated parametric biology uses fractional ages and fleet-specific bins", {
  #' @description Fractional growth, maturity, length mapping, sharing, AD, and annual invariance agree with analytic expectations.
  d <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4)) |>
    dplyr::filter(type != "age_to_length_conversion")
  d$timing <- ifelse(is.na(d$timing), NA_character_, sprintf("%04d", d$timing + 2026L))
  second <- d[d$fleet == "survey1" & d$type %in% c("index", "age_comp", "length_comp"), ]
  second$fleet <- "survey2"
  second <- second[second$type != "length_comp" | second$length <= 550, ]
  second <- second |>
    dplyr::group_by(type, timing) |>
    dplyr::mutate(observed = if (dplyr::first(type) == "length_comp") observed / sum(observed) else observed) |>
    dplyr::ungroup()
  d <- dplyr::bind_rows(d, second)
  annual <- FIMSFrame(d)
  extra <- d[d$fleet %in% c("survey1", "survey2") & d$timing == "2028" &
    d$type %in% c("index", "age_comp", "length_comp"), ]
  extra$timing <- ifelse(extra$type == "age_comp", "2028-10-01", "2028-07-01")
  dated <- FIMSFrame(dplyr::bind_rows(d, extra))
  parameters <- setup_default_parameters(dated) |>
    dplyr::filter(module_name != "Growth") |>
    dplyr::bind_rows(setup_default_Growth(dated, module_type = "VonBertalanffySchnute"))
  devs <- which(parameters$module_name == "Recruitment" & parameters$label == "log_devs")
  parameters$value[devs] <- seq_along(devs) * 0.01
  on.exit(clear(), add = TRUE)
  expect_no_warning(input <- initialize_fims(parameters, dated))
  fit <- fit_fims(input, optimize = FALSE, get_sd = FALSE)
  report <- get_report(fit)
  biology <- report$observation_biology
  colnames(biology) <- c("day", "year_i", "age", "fraction", "length", "sd", "weight", "maturity")
  ages <- get_ages(dated)
  na <- length(ages)
  # Two dates, shared by two fleets and by index/length streams, each prepared once.
  expect_equal(nrow(biology), 2L * na)
  expect_equal(unname(report$observation_work), c(2, na, 6))
  expect_equal(sort(unique(biology[, "fraction"])), c(182, 274) / 366)
  value <- function(module, label) parameters$value[parameters$module_name == module & parameters$label == label]
  young <- value("Growth", "mean_length_young")
  old <- value("Growth", "mean_length_old")
  k <- value("Growth", "growth_coefficient")
  ay <- value("Growth", "reference_age_for_length_young")
  ao <- value("Growth", "reference_age_for_length_old")
  # Account for the existing smooth AD floor in the growth equation.
  smooth_max <- function(a, b) (a + b + sqrt((a - b)^2 + 1e-5)) / 2
  denom <- smooth_max(sqrt((1 - exp(-k * (ao - ay)))^2 + 1e-5), 1e-8)
  length_at <- function(age) young + (old - young) * (1 - exp(-k * (age - ay))) / denom
  expected_length <- length_at(biology[, "age"] + biology[, "fraction"])
  spread <- value("Growth", "length_at_age_sd_at_reference_ages")
  reference_delta <- smooth_max(sqrt((length_at(ao) - length_at(ay))^2 + 1e-5), 1e-8)
  expected_sd <- smooth_max(spread[1] + diff(spread) * (expected_length - length_at(ay)) / reference_delta, 1e-8)
  expected_weight <- value("Growth", "length_weight_a") * expected_length^value("Growth", "length_weight_b")
  expected_maturity <- plogis(value("Maturity", "slope") *
    (biology[, "age"] + biology[, "fraction"] - value("Maturity", "inflection_point")))
  expect_equal(biology[, "length"], expected_length, tolerance = 1e-10)
  expect_equal(biology[, "sd"], expected_sd, tolerance = 1e-10)
  expect_equal(biology[, "weight"], expected_weight, tolerance = 1e-10)
  expect_equal(biology[, "maturity"], expected_maturity, tolerance = 1e-10)
  # Includes the oldest plus group at representative age + fraction.
  expect_gt(min(biology[biology[, "age"] == max(ages), "length"]), length_at(max(ages)))

  raw <- jsonlite::fromJSON(get_model_output(fit), simplifyVector = FALSE)
  fleet_names <- vapply(raw$fleets, `[[`, "", "fleet")
  july <- biology[biology[, "fraction"] == 182 / 366, ]
  july <- july[order(july[, "age"]), ]
  cells <- na + seq_len(na)
  survival <- exp(-report$mortality_Z[[1]][cells] * 182 / 366)
  # The default biological grid uses 1-unit bins plus a terminal tail bin.
  full_centers <- sort(unique(d$length[d$type == "length_comp"]))
  grid_edges <- seq(min(full_centers) - 25, max(full_centers) + 26, by = 1)
  grid_left <- head(grid_edges, -1)
  grid_right <- tail(grid_edges, -1)
  for (fleet in c("survey1", "survey2")) {
    fi <- match(fleet, fleet_names)
    samples <- get_observations(dated)
    si <- samples$sample_i[samples$fleet == fleet & samples$type == "index" & samples$date == as.Date("2028-07-01")]
    selected <- report$index_numbers_at_age[[fi]][cells] * survival
    expect_equal(report$index_expected[[fi]][si], sum(selected * july[, "weight"]), tolerance = 1e-10)
    expect_gt(report$index_expected[[fi]][si], sum(report$index_weight_at_age[[fi]][cells] * survival))
    centers <- sort(unique(d$length[d$fleet == fleet & d$type == "length_comp"]))
    edges <- c(min(grid_edges), head(centers, -1) + diff(centers) / 2, max(grid_edges))
    conversion <- t(vapply(seq_len(na), function(a) {
      cdf <- pnorm(grid_edges, july[a, "length"], july[a, "sd"])
      cdf[c(1, length(cdf))] <- c(0, 1)
      probability <- diff(cdf) + 1e-12
      probability <- probability / sum(probability)
      mapped <- vapply(seq_along(centers), function(b) {
        overlap <- pmax(0, pmin(grid_right, edges[b + 1]) - pmax(grid_left, edges[b]))
        sum(probability * overlap / (grid_right - grid_left))
      }, 0.0) + 1e-12
      mapped / sum(mapped)
    }, numeric(length(centers))))
    expected <- as.numeric(selected %*% conversion)
    expected <- expected / sum(expected)
    bins <- length(centers)
    expect_equal(report$lengthcomp_proportion[[fi]][(si - 1L) * bins + seq_len(bins)], expected, tolerance = 1e-6)
  }
  metadata <- attr(input, "observation_timing")
  expect_true(all(metadata$weight_timing == "fractional_age"))
  expect_true(all(metadata$length_mapping_timing == "fractional_age"))
  estimates <- get_estimates(fit)
  expect_true(all(estimates$weight_timing[!is.na(estimates$observation_id)] == "fractional_age"))

  obj <- get_obj(fit)
  par <- obj$par
  initial <- obj$fn(par)
  gradient <- obj$gr(par)
  # Include every fixed parameter, including all estimated growth parameters.
  h <- 1e-5
  numerical <- vapply(seq_along(par), function(j) {
    plus <- minus <- par
    plus[j] <- plus[j] + h
    minus[j] <- minus[j] - h
    (obj$fn(plus) - obj$fn(minus)) / (2 * h)
  }, 0.0)
  expect_equal(as.numeric(gradient), numerical, tolerance = 1e-4)
  expect_equal(obj$fn(par), initial, tolerance = 1e-8)
  clear()
  baseline <- fit_fims(initialize_fims(parameters, annual), optimize = FALSE, get_sd = FALSE)
  for (quantity in c("numbers_at_age", "catch_expected", "spawning_biomass", "growth_mean_LAA", "growth_sd_LAA", "growth_mean_WAA")) {
    expect_equal(report[[quantity]], get_report(baseline)[[quantity]])
  }
})
