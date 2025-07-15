data(data1)
data <- FIMSFrame(data1)

fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution"
    )
  )
fleets <- list(fleet1 = fleet1, survey1 = survey1)

recruitment <- list(
  form = "BevertonHoltRecruitment",
  process_distribution = c(log_devs = "DnormDistribution")
)
growth <- list(form = "EWAAgrowth")
maturity <- list(form = "LogisticMaturity")
fleet_names <- names(fleets)

module_list <- list(
  fleets = fleets,
  recruitment = recruitment,
  growth = growth,
  maturity = maturity
)

fleet_temp <- list()
fleet_name <- names(fleets)[1]







# Update default selectivity parameters
# Change from 1 fixed_effects parameter to 30 random effects parameters
updated_selectivity <- selectivity_default |>
  dplyr::filter(label == "inflection_point") |>
  dplyr::slice(rep(1, each = get_n_years(data))) |>
  dplyr::mutate(
    time = 1:30,
    value = 2:31,
    estimation_type = "random_effects"
  ) |>
  dplyr::full_join(
    selectivity_default |>
    dplyr::filter(!(label == "inflection_point"))
  ) |>
  tidyr::nest(details = age:distribution)
print(updated_selectivity)


# New interface option 1
data(data1)
data <- FIMSFrame(data1)

fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Landings = "DlnormDistribution",
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution"
    )
  )
fleets <- list(fleet1 = fleet1, survey1 = survey1)

population1 <- population2 <- list(
  recruitment <- list(
    form = "BevertonHoltRecruitment",
    process_distribution = c(log_devs = "DnormDistribution")
  ),
  growth <- list(form = "EWAAgrowth"),
  maturity <- list(form = "LogisticMaturity")
)
populations <- list(population1 = population1, population2 = population2)

# New interface option 2
fleets <- dplyr::tibble(
  name = c("fleet1", "survey1"),
  selectivity_form = c("LogisticSelectivity"),
  landings_dist = c("DlnormDistribution"),
  index_dist = c("DlnormDistribution"),
  agecomp_dist = c("DmultinomDistribution")
)

populations <- dplyr::tibble(
  name = c("population1", "population2"),
  recruitment_form = c("BevertonHoltRecruitment"),
  recruitment_dist = c("DnormDistribution"),
  growth_form = c("EWAAgrowth"),
  maturity_form = c("LogisticMaturity")
)
