load(test_path("fixtures", "integration_test_data.RData"))
iter_id <- 1
# Load operating model data
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

# Clear any previous FIMS settings
clear()

test_that("test new_process_distribution", {
  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- new(BevertonHoltRecruitment)

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- log(om_input$R0)
  recruitment$log_rzero[1]$is_random_effect <- FALSE
  recruitment$log_rzero[1]$estimated <- TRUE
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input$h) + 
    log(om_input$h - 0.2)
  recruitment$logit_steep[1]$is_random_effect <- FALSE
  recruitment$logit_steep[1]$estimated <- FALSE
  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs <- methods::new(
    ParameterVector,
    om_input$logR.resid[-1],
    om_input$nyr - 1
  )

  # set up logR_sd using the normal log_sd parameter
  recruitment_distribution <- new_process_distribution(
    module = recruitment,
    par = "log_devs",
    family = gaussian(),
    sd = list(value = om_input$logR_sd, estimated = FALSE),
    is_random_effect = FALSE
  )
  recruitment$estimate_log_devs <- TRUE

  expect_equal(log(om_input$logR_sd), recruitment_distribution$log_sd[1]$value)
  expect_equal(length(recruitment$log_devs), length(recruitment_distribution$x))
  expect_equal(length(recruitment_distribution$x), 
               length(recruitment_distribution$expected_values))
  expect_error(
    new_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = multinomial(),
      sd = list(value = om_input$logR_sd, estimated = FALSE),
      is_random_effect = FALSE
    )
  )
  expect_error(
    new_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = binomial(),
      sd = list(value = om_input$logR_sd, estimated = FALSE),
      is_random_effect = FALSE
    )
  )
  expect_error(
    new_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = gaussian(),
      sd = list(value = -1, estimated = FALSE),
      is_random_effect = FALSE
    )
  )
  expect_error(
    new_process_distribution(
      module = recruitment,
      par = "log_devs",
      family = gaussian(),
      sd = list(
        value = rep(om_input$logR_sd, 3), 
        estimated = rep(FALSE, 2)
      ),
      is_random_effect = FALSE)
    )
  clear()
})


test_that("test new_data_distribution", {
  # Data
  catch <- em_input$L.obs$fleet1
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- new(Index, om_input$nyr)
  fishing_fleet_index$index_data <- catch
  fishing_fleet <- new(Fleet)
  fishing_fleet$nages <- om_input$nages
  fishing_fleet$nyears <- om_input$nyr
  fishing_fleet$log_Fmort <- methods::new(ParameterVector, log(om_output$f), om_input$nyr)
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$estimate_q <- FALSE
  fishing_fleet$random_q <- FALSE
  fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())

  # Set up fishery index data using the lognormal
  fleet_sd <- rep(sqrt(log(em_input$cv.L$fleet1^2 + 1)), om_input$nyr)
  fishing_fleet_index_distribution <- new_data_distribution(
    module = fishing_fleet,
    family = lognormal(link = "log"),
    sd = list(value = fleet_sd, estimated = FALSE),
    data_type = "index"
  )
  expect_equal(log(fleet_sd[1]),
               fishing_fleet_index_distribution$log_logsd[1]$value)
  expect_error(
    new_data_distribution(
      module = fishing_fleet,
      family = multinomial(),
      sd = list(value = fleet_sd, estimated = FALSE),
      data_type = "index"
    )
  )
  expect_error(
    new_data_distribution(
      module = fishing_fleet,
      family = multinomial(),
      sd = list(value = fleet_sd, estimated = FALSE),
      data_type = "index"
    )
  )
  expect_error(
    new_data_distribution(
      module = fishing_fleet,
      family = gaussian(),
      sd = list(value = fleet_sd, estimated = FALSE),
      data_type = "agecomp"
    )
  )
  expect_error(
    new_data_distribution(
      module = fishing_fleet,
      family = lognormal(),
      sd = list(value = fleet_sd, estimated = FALSE),
      data_type = "lengthcomp"
    )
  )
  clear()
})
