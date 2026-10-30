# Synthetic two-sex example using group-specific growth, maturity and selectivity.
# Run after installing this branch. Weights must use the model's input units.
library(FIMS)

model_data <- FIMSFrame(data_big)
parameters <- setup_default_parameters(data = model_data)
ages <- get_ages(model_data)

female_maturity <- parameters |>
  dplyr::filter(.data$module_name == "Maturity") |>
  dplyr::mutate(
    value = dplyr::if_else(.data$label == "inflection_point", 3, 0.7),
    estimation_type = "constant"
  )
female_selectivity <- parameters |>
  dplyr::filter(.data$module_name == "Selectivity", .data$fleet == "fleet1") |>
  dplyr::mutate(
    value = dplyr::if_else(.data$label == "inflection_point", 2, 0.9),
    estimation_type = "fixed_effects"
  )

groups <- list(
  list(
    name = "female", initial_fraction = 0.5, recruitment_fraction = 0.5,
    reproductive_contribution = 1,
    growth = list(weights = 0.1 * ages^2),
    maturity = list(parameters = female_maturity),
    selectivity = list(fleet1 = list(parameters = female_selectivity))
  ),
  list(
    name = "male", initial_fraction = 0.5, recruitment_fraction = 0.5,
    reproductive_contribution = 0
  )
)
# Males and unspecified fleets inherit their shared modules. M is shared here;
# the C++ mixed-biology regression test also exercises separate M by sex.
input <- initialize_fims(parameters, model_data, subpopulations = groups)
fit <- fit_fims(input, optimize = FALSE)
report <- get_report(fit)

# State vectors are ordered by group, then year, then age (age varies fastest).
n <- report$numbers_at_age_by_subpopulation[[1]]
weights <- report$weight_at_age_by_subpopulation[[1]]
maturity <- report$proportion_mature_at_age_by_subpopulation[[1]]
plane <- length(n) / 2
female <- seq_len(plane)
male <- plane + female
stopifnot(isTRUE(all.equal(
  report$biomass[[1]],
  rowSums(matrix(n[female] * weights[female] + n[male] * weights[male],
    ncol = length(ages), byrow = TRUE
  ))
)))
stopifnot(isTRUE(all.equal(
  report$spawning_biomass[[1]],
  rowSums(matrix(n[female] * weights[female] * maturity[female],
    ncol = length(ages), byrow = TRUE
  ))
)))
get_estimates(fit) |>
  dplyr::filter(.data$label %in% c(
    "proportion_at_age_by_subpopulation", "mortality_F_by_subpopulation"
  ))
