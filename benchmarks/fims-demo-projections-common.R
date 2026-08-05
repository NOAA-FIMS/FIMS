# Shared model setup for the matched TMB and Quadra projection scripts.
# This mirrors vignettes/fims-demo-projections.Rmd through initialize_fims().

build_fims_demo_projection <- function(years_of_projection = 10L) {
  years_of_projection <- as.integer(years_of_projection)
  if (length(years_of_projection) != 1L || is.na(years_of_projection) ||
      years_of_projection < 0L) {
    stop("years_of_projection must be one nonnegative integer.")
  }

  data("data_big", package = "FIMS", envir = environment())
  data_4_model <- FIMS::FIMSFrame(data_big)
  default_configurations <- FIMS::create_default_configurations(
    data = data_4_model
  )

  end_year <- FIMS::get_end_year(data_4_model)
  data_big_with_extra_year <- dplyr::add_row(
    data_big,
    type = "landings",
    timing = end_year + years_of_projection,
    fleet = "fleet1",
    value = -999,
    unit = "mt"
  ) |>
    dplyr::filter(
      !(.data$type == "age_to_length_conversion" |
          .data$type == "length_comp")
    )

  projection_weight_years <- max(data_big[["timing"]], na.rm = TRUE):
    (max(data_big[["timing"]], na.rm = TRUE) + years_of_projection - 1L)

  data_4_projections <- data_big_with_extra_year |>
    dplyr::bind_rows(
      dplyr::filter(
        data_big,
        .data$type == "weight_at_age",
        .data$timing == 1
      ) |>
        dplyr::select(-"timing") |>
        merge(data.frame(timing = projection_weight_years))
    ) |>
    FIMS::FIMSFrame() |>
    FIMS::get_data() |>
    dplyr::mutate(
      uncertainty = ifelse(
        .data$type == "landings" & .data$value == -999,
        0.00999975,
        .data$uncertainty
      ),
      uncertainty = ifelse(
        .data$type == "index" & .data$value == -999,
        0.19804220,
        .data$uncertainty
      ),
      uncertainty = ifelse(
        .data$type %in% c("age_comp", "length_comp") & .data$value == -999,
        0,
        .data$uncertainty
      )
    ) |>
    FIMS::FIMSFrame()

  projection_start <- FIMS::get_end_year(data_4_projections) -
    years_of_projection + 1L
  projection_end <- FIMS::get_end_year(data_4_projections)

  parameters_projection <- FIMS::create_default_parameters(
    configurations = default_configurations,
    data = data_4_projections
  ) |>
    tidyr::unnest(cols = "data") |>
    dplyr::rows_update(
      tibble::tibble(
        fleet = "fleet1",
        label = "log_Fmort",
        time = FIMS::get_start_year(data_4_projections):projection_end,
        value = log(c(
          0.009459165, 0.027288858, 0.045063639,
          0.061017825, 0.048600752, 0.087420554,
          0.088447204, 0.186607929, 0.109008958,
          0.132704335, 0.150615473, 0.161242955,
          0.116640187, 0.169346119, 0.180191913,
          0.161240483, 0.314573212, 0.257247574,
          0.254887252, 0.251462108, 0.349101406,
          0.254107720, 0.418478117, 0.345721184,
          0.343685540, 0.314171227, 0.308026829,
          0.431745298, 0.328030899, 0.499675368,
          rep(0.499675368, years_of_projection)
        ))
      ),
      by = c("fleet", "label", "time")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        label = "log_Fmort",
        time = projection_start:projection_end,
        estimation_type = rep("constant", years_of_projection)
      ),
      by = c("label", "time")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        fleet = "survey1",
        label = c("inflection_point", "slope", "log_q"),
        value = c(1.5, 2, log(3.315143e-07))
      ),
      by = c("fleet", "label")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        label = "log_devs",
        time = (FIMS::get_start_year(data_4_projections) + 1L):projection_end,
        value = c(
          0.43787763, -0.13299042, -0.43251973, 0.64861200, 0.50640852,
          -0.06958319, 0.30246260, -0.08257384, 0.20740372, 0.15289604,
          -0.21709207, -0.13320626, 0.11225374, -0.10650836, 0.26877132,
          0.24094126, -0.54480751, -0.23680557, -0.58483386, 0.30122785,
          0.21930545, -0.22281699, -0.51358369, 0.15740234, -0.53988240,
          -0.19556523, 0.20094360, 0.37248740, -0.07163145,
          rep(0, years_of_projection)
        ),
        estimation_type = "random_effects"
      ),
      by = c("label", "time")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        label = "log_devs",
        time = projection_start:projection_end,
        estimation_type = rep("constant", years_of_projection),
        distribution_type = rep(NA_character_, years_of_projection),
        distribution = rep(NA_character_, years_of_projection)
      ),
      by = c("label", "time")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        module_name = "Recruitment",
        label = "log_sd",
        value = 0.4,
        estimation_type = "fixed_effects"
      ),
      by = c("module_name", "label")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        module_name = "Maturity",
        label = c("inflection_point", "slope"),
        value = c(2.25, 3)
      ),
      by = c("module_name", "label")
    ) |>
    dplyr::rows_update(
      tibble::tibble(
        label = "log_init_naa",
        age = seq_len(FIMS::get_n_ages(data_4_projections)),
        value = c(
          13.80944, 13.60690, 13.40217, 13.19525, 12.98692, 12.77791,
          12.56862, 12.35922, 12.14979, 11.94034, 11.73088, 13.18755
        )
      ),
      by = c("label", "age")
    )

  initialized <- FIMS::initialize_fims(
    parameters = parameters_projection,
    data = data_4_projections
  )

  list(
    initialized = initialized,
    data = data_4_projections,
    parameter_specification = parameters_projection,
    projection_years = years_of_projection
  )
}

projection_script_arguments <- function(default_output) {
  args <- commandArgs(trailingOnly = TRUE)
  projection_years <- if (length(args) >= 1L) as.integer(args[[1]]) else 10L
  output <- if (length(args) >= 2L) args[[2]] else default_output
  max_iterations <- if (length(args) >= 3L) as.integer(args[[3]]) else 500L
  gradient_tolerance <- if (length(args) >= 4L) {
    as.numeric(args[[4]])
  } else {
    1e-5
  }
  if (is.na(projection_years) || projection_years < 1L ||
      is.na(max_iterations) || max_iterations < 1L ||
      !is.finite(gradient_tolerance) || gradient_tolerance <= 0) {
    stop(
      "Usage: Rscript SCRIPT [projection-years>=1] [output.rds] ",
      "[max-iterations>=1] [gradient-tolerance>0]"
    )
  }
  list(
    projection_years = projection_years,
    output = output,
    max_iterations = max_iterations,
    gradient_tolerance = gradient_tolerance
  )
}

find_projection_common <- function() {
  candidates <- c(
    file.path("benchmarks", "fims-demo-projections-common.R"),
    "fims-demo-projections-common.R"
  )
  match <- candidates[file.exists(candidates)]
  if (!length(match)) {
    stop("Run from the FIMS repository root or the benchmarks directory.")
  }
  match[[1]]
}
