load(test_path("fixtures", "integration_test_data.RData"))
iter_id <- 1
# Load operating model data
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

# Clear any previous FIMS settings
clear()

test_that("test initialize_process_distribution", {
  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- log(om_input$R0)
  recruitment$log_rzero[1]$estimation_type <- "fixed_effects"
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input$h) +
    log(om_input$h - 0.2)
  recruitment$logit_steep[1]$estimation_type <- "constant"
  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs$resize(om_input$nyr - 1)
  logR_resid <- om_input$logR.resid[-1]
  purrr::walk(
    seq_along(logR_resid),
    \(x) recruitment$log_devs[x]$value <- logR_resid[x]
  )

  # set up logR_sd using the normal log_sd parameter
  recruitment_distribution <- initialize_process_distribution(
    module = recruitment,
    par = "log_devs",
    family = gaussian(),
    sd = list(value = om_input$logR_sd, estimation_type = "constant"),
    is_random_effect = FALSE
  )
  recruitment$log_devs$set_all_estimable(TRUE)

  expect_equal(log(om_input$logR_sd), recruitment_distribution$log_sd[1]$value)
  expect_equal(length(recruitment$log_devs), length(recruitment_distribution$x))
  expect_equal(
    length(recruitment_distribution$x),
    length(recruitment_distribution$expected_values)
  )
  expect_error(
    initialize_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = multinomial(),
      sd = list(value = om_input$logR_sd, estimation_type = "constant"),
      is_random_effect = FALSE
    )
  )
  expect_error(
    initialize_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = binomial(),
      sd = list(value = om_input$logR_sd, estimation_type = "constant"),
      is_random_effect = FALSE
    )
  )
  expect_error(
    initialize_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = gaussian(),
      sd = list(value = -1, estimation_type = "constant"),
      is_random_effect = FALSE
    )
  )
  expect_error(
    initialize_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = gaussian(),
      sd = list(
        value = rep(om_input$logR_sd, 3),
        estimation_type = rep("constant", 2)
      ),
      is_random_effect = FALSE
    )
  )
  clear()
})


test_that("test initialize_data_distribution", {
  # Data
  catch <- em_input$L.obs$fleet1
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- methods::new(Index, om_input$nyr)
  purrr::walk(
    seq_along(catch), 
    \(x) fishing_fleet_index$index_data$set(x-1, catch[x])
  )
  fishing_fleet <- methods::new(Fleet)
  fishing_fleet$nages$set(om_input$nages)
  fishing_fleet$nyears$set(om_input$nyr)
  fishing_fleet$log_Fmort$resize(om_input$nyr)
  purrr::walk(
    seq_along(log(om_output$f)),
    \(x) fishing_fleet$log_Fmort[x]$value <- log(om_output$f)[x]
  )
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$log_q[1]$estimation_type <- "constant"
  fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())

  # Set up fishery index data using the lognormal
  fleet_sd <- rep(sqrt(log(em_input$cv.L$fleet1^2 + 1)), om_input$nyr)
  fishing_fleet_index_distribution <- initialize_data_distribution(
    module = fishing_fleet,
    family = lognormal(link = "log"),
    sd = list(value = fleet_sd, estimation_type = "constant"),
    data_type = "index"
  )
  expect_equal(
    log(fleet_sd[1]),
    fishing_fleet_index_distribution$log_sd[1]$value
  )
  expect_error(
    initialize_data_distribution(
      module = fishing_fleet,
      family = multinomial(),
      sd = list(value = fleet_sd, estimation_type = "constant"),
      data_type = "index"
    )
  )
  expect_error(
    initialize_data_distribution(
      module = fishing_fleet,
      family = multinomial(),
      sd = list(value = fleet_sd, estimation_type = "constant"),
      data_type = "index"
    )
  )
  expect_error(
    initialize_data_distribution(
      module = fishing_fleet,
      family = gaussian(),
      sd = list(value = fleet_sd, estimation_type = "constant"),
      data_type = "agecomp"
    )
  )
  expect_error(
    initialize_data_distribution(
      module = fishing_fleet,
      family = lognormal(),
      sd = list(value = fleet_sd, estimation_type = "constant"),
      data_type = "lengthcomp"
    )
  )
  expect_error(
    initialize_data_distribution(
      module = fishing_fleet,
      family = multinomial(),
      sd = list(value = fleet_sd, estimation_type = c("constant", "constant")),
      data_type = "agecomp"
    )
  )
  expect_error(
    initialize_data_distribution(
      module = fishing_fleet,
      family = multinomial(),
      sd = list(value = fleet_sd),
      data_type = "agecomp"
    )
  )
  clear()
})
