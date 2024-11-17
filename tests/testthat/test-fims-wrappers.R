# # example
# fleets <- list(
#   fleet1 = list(
#     selectivity = list(form = "LogisticSelectivity"),
#     data_distribution = c(
#       Index = "DlnormDistribution",
#       AgeComp = "DmultinomDistribution"
#     )
#   ),
#   survey1 = list(
#     selectivity = list(form = "LogisticSelectivity"),
#     data_distribution = c(
#       Index = "DlnormDistribution",
#       AgeComp = "DmultinomDistribution"
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
#       process_distribution = c(log_devs = "DnormDistribution")
#     ),
#     growth = list(form = "EWAAgrowth"),
#     maturity = list(form = "LogisticMaturity")
#   )
#
# load(test_path("fixtures", "integration_test_data.RData"))
# update_arg <- list(
#   survey1 = list(
#     LogisticSelectivity.inflection_point.value = 1.5,
#     LogisticSelectivity.slope.value = 2,
#     Fleet.log_q.value = log(om_output_list[[1]]$survey_q$survey1)
#   ),
#   recruitment = list(
#     BevertonHoltRecruitment.log_devs.value = om_input_list[[1]]$logR.resid
#   ),
#   maturity = list(
#     LogisticMaturity.inflection_point.value = om_input_list[[1]]$A50.mat,
#     LogisticMaturity.inflection_point.estimated = FALSE,
#     LogisticMaturity.slope.value = om_input_list[[1]]$slope.mat,
#     LogisticMaturity.slope.estimated = FALSE
#   ),
#   population = list(
#     Population.log_init_naa.value = log(om_output_list[[1]]$N.age[1, ])
#   )
# )
#
# initial_input <- initial_input |>
#   update_input(
#     update_arg = update_arg
#   ) # |>
#  # jsonlite::toJSON(pretty = TRUE) |>
#   # initialize_fims(data = data)
#
# # success <- CreateTMBModel()
# # parameters <- list(p = get_fixed())
# # obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)
# # opt <- nlminb(obj$par, obj$fn, obj$gr,
# #               control = list(eval.max = 10000, iter.max = 10000)
# # )
#
# fleet1_selectivity <- initialize_selectivity(
#   input = initial_input,
#   data = data,
#   fleet_name = "fleet1"
# )
#
# fleet1 <- initialize_fleet(
#   input = initial_input,
#   data = data,
#   fleet_name = "fleet1",
#   linked_ids = fleet1_selectivity$get_id()
# )
#
# fleet1_index <- initialize_index(
#   data = data,
#   fleet_name = "fleet1"
# )
#
# fleet1_index_distribution <- initialize_distribution(
#   module_input = NULL,
#   distribution_name = initial_input$module_list$fleets$fleet1$data_distribution["Index"],
#   distribution_type = "data",
#   linked_ids = c(data_link = fleet1_index$get_id(),
#                  fleet_link = fleet1$log_expected_index$get_id())
# )
#
# fleet1_agecomp <- initialize_age_comp(
#   data = data,
#   fleet_name = "fleet1"
# )
#
# fleet1_agecomp_distribution <- initialize_distribution(
#   module_input = NULL,
#   distribution_name = initial_input$module_list$fleets$fleet1$data_distribution["AgeComp"],
#   distribution_type = "data",
#   linked_ids = c(data_link = fleet1_agecomp$get_id(),
#                  fleet_link = fleet1$proportion_catch_numbers_at_age$get_id())
# )
#
# recruitment <- initialize_recruitment(
#   input = initial_input,
#   data = data
# )
#
# recruitment_distribution <- initialize_distribution(
#   module_input = initial_input$parameter_input_list$recruitment,
#   distribution_name = initial_input$module_list$recruitment$process_distribution,
#   distribution_type = "process",
#   linked_ids = recruitment$get_id()
# )
#
# growth <- initialize_growth(
#   input = initial_input,
#   data = data
# )
#
# maturity <- initialize_maturity(
#   input = initial_input,
#   data = data
# )
#
# module_ids <- c(
#   recruitment = recruitment$get_id(),
#   growth = growth$get_id(),
#   maturity = maturity$get_id()
# )
#
# population_module <- initialize_population(
#   input = initial_input,
#   data = data,
#   linked_ids = module_ids
# )
#
# success <- CreateTMBModel()
# parameters <- list(p = get_fixed())
# obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)
# opt <- nlminb(obj$par, obj$fn, obj$gr,
#               control = list(eval.max = 10000, iter.max = 10000)
# )
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
# distribution_name <- initial_input$module_list$recruitment$process_distribution
# distribution_type = "process"
# module_input = input[["parameter_input_list"]][["recruitment"]]
#
# distribution_name <- initial_input$module_list$fleets$fleet1$data_distribution["Index"]
# distribution_type = "data"
# module_input = input[["parameter_input_list"]][["fleet1"]]
