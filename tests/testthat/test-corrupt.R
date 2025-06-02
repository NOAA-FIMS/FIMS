library(FIMS)
library(ggplot2)

# clear memory
clear()

# Bring the package data into your environment
data("data1")
# Prepare the package data for being used in a FIMS model
data_4_model <- FIMSFrame(data1)

# Use dplyr to subset the data for just the landings
get_data(data_4_model) |>
  dplyr::filter(type == "landings")

  # Define the same fleet specifications for fleet1 and survey1
fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Index = "DlnormDistribution",
    AgeComp = "DmultinomDistribution",
    LengthComp = "DmultinomDistribution"
  )
)

# Create default recruitment, growth, and maturity parameters
default_parameters <- data_4_model |>
  create_default_parameters(
    fleets = list(fleet1 = fleet1, survey1 = survey1)
  )

# Each call to update_parameters() returns the full list so the pipe can be
# used to daisy chain all of these updates together to a new object called
# parameters that will be used to fit the model
parameters <- default_parameters |>
  update_parameters(
    modified_parameters = list(
      fleet1 = list(
        Fleet.log_Fmort.value = log(c(
          0.009459165, 0.027288858, 0.045063639,
          0.061017825, 0.048600752, 0.087420554,
          0.088447204, 0.186607929, 0.109008958,
          0.132704335, 0.150615473, 0.161242955,
          0.116640187, 0.169346119, 0.180191913,
          0.161240483, 0.314573212, 0.257247574,
          0.254887252, 0.251462108, 0.349101406,
          0.254107720, 0.418478117, 0.345721184,
          0.343685540, 0.314171227, 0.308026829,
          0.431745298, 0.328030899, 0.499675368
        ))
      )
    )
  ) |>
  update_parameters(
    modified_parameters = list(
      survey1 = list(
        LogisticSelectivity.inflection_point.value = 1.5,
        LogisticSelectivity.slope.value = 2,
        Fleet.log_q.value = log(3.315143e-07)
      )
    )
  ) |>
  update_parameters(
    modified_parameters = list(
      recruitment = list(
        BevertonHoltRecruitment.log_rzero.value = log(1e+06),
        BevertonHoltRecruitment.log_devs.value = c(
          0.43787763, -0.13299042, -0.43251973, 0.64861200, 0.50640852,
          -0.06958319, 0.30246260, -0.08257384, 0.20740372, 0.15289604,
          -0.21709207, -0.13320626, 0.11225374, -0.10650836, 0.26877132,
          0.24094126, -0.54480751, -0.23680557, -0.58483386, 0.30122785,
          0.21930545, -0.22281699, -0.51358369, 0.15740234, -0.53988240,
          -0.19556523, 0.20094360, 0.37248740, -0.07163145
        ),
        BevertonHoltRecruitment.log_devs.estimated = FALSE
      )
    )
  ) |>
  update_parameters(
    modified_parameters = list(
      maturity = list(
        LogisticMaturity.inflection_point.value = 2.25,
        LogisticMaturity.inflection_point.estimated = FALSE,
        LogisticMaturity.slope.value = 3,
        LogisticMaturity.slope.estimated = FALSE
      )
    )
  ) |>
  update_parameters(
    modified_parameters = list(
      population = list(
        Population.log_init_naa.value = c(
          13.80944, 13.60690, 13.40217, 13.19525, 12.98692, 12.77791,
          12.56862, 12.35922, 12.14979, 11.94034, 11.73088, 13.18755
        )
      )
    )
  )

test_fit <- parameters |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = FALSE)

# Run the  model with optimization
fit <- parameters |>
  initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
