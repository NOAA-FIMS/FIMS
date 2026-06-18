test_that("No residual R or C++ memory structures remain after clear", {
  load(test_path("fixtures", "integration_test_data.RData"))
  data_frame <- FIMSFrame(data_big)
  
  model_run <- create_default_configurations(data = data_frame) |>
    create_default_parameters(data = data_frame) |> 
    initialize_fims(data = data_frame)
  
  expect_no_warning(clear())
  
})

