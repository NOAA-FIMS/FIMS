# # example
# fleets <- list(
#   fleet1 = list(
#     selectivity = "LogisticSelectivity",
#     data_distribution = c(
#       Index = "TMBDlnormDistribution",
#       AgeComp = "TMBDmultinomDistribution"
#     )
#   ),
#   survey1 = list(
#     selectivity = "LogisticSelectivity",
#     data_distribution = c(
#       Index = "TMBDlnormDistribution",
#       AgeComp = "TMBDmultinomDistribution"
#     )
#   )
# )
#
# data(data_mile1)
# data <- FIMS::FIMSFrame(data_mile1)
# initial_input <- data_mile1 |>
#   FIMS::FIMSFrame() |>
#   prepare_input(
#     fleets = fleets,
#     recruitment = list(
#       form = "BevertonHoltRecruitment",
#       process_distribution = c(log_devs = "TMBDnormDistribution")
#     ),
#     growth = "EWAAgrowth",
#     maturity = "LogisticMaturity"
#   )
#
# load(test_path("fixtures", "integration_test_data.RData"))
# update_arg <- list(
#   fleet1 = list(
#     LogisticSelectivity.inflection_point.estimated = FALSE,
#     LogisticSelectivity.slope.value = 3,
#     Fleet.log_Fmort = log(om_output_list[[1]]$f)
#   ),
#   maturity = list(LogisticMaturity.slope.value = 1.5)
# )
#
# initial_input |>
#   update_input(
#     update_arg = update_arg
#   ) |>
#  # jsonlite::toJSON(pretty = TRUE) |>
#   setup_input(data = data)
#
#
# population_module <-
#
# initial_input |>
#   initialize_population(data) |>
#   initialize_population(input= initial_input, data = data)
#
# a <- initialize_population(input= initial_input, data = data)
#
#
#   setup_fleet(data = data)
#
#
#
# setup_recruitment("BevertonHoltRecruitment", data, parameter) |>
#   setup_distribution() |>
#   setup_growth("EWAAgrowth", data, parameter) |>
#   setup_maturity("LogisticMaturity", data, parameter) |>
#   setup_selectivity("Fleet1", data, parameter) |>
#   setup_fleet("Fleet1", parameter) |>
#   setup_selectivity("Survey1", data, parameter) |>
#   setup_fleet("Survey1", parameter)
# #
#
