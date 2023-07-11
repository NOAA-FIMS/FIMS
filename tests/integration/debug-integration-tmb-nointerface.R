#run script from R in the command terminal
#compiler flags based on Windows machine
TMB:::gdbsource("tests/integration/integration_test_population_tmb_nointerface.R", interactive = TRUE)
