FIMS_dmultinom <- function(x, p) {
  lgamma(sum(x) + 1) - sum(lgamma(x + 1)) + sum(x * log(p))
}

setup_and_run_FIMS_with_wrappers <- function(iter_id,
                                             om_input_list,
                                             om_output_list,
                                             em_input_list,
                                             estimation_mode = TRUE,
                                             random_effects = FALSE,
                                             modified_parameters,
                                             map = list()) {
  data <- FIMS::FIMSFrame(data_big)
  parameters <- if (tibble::is_tibble(modified_parameters)) {
    modified_parameters
  } else {
    modified_parameters[[iter_id]]
  }

  if (isTRUE(estimation_mode)) {
    parameters <- parameters |>
      dplyr::mutate(
        value = dplyr::if_else(
          .data$fleet == "survey1" & .data$label == "log_q",
          log(1),
          .data$value
        )
      )
  }

  input <- initialize_fims(parameters = parameters, data = data)
  if (length(map) > 0L) {
    input$map <- map
  }
  fit <- fit_fims(input = input, optimize = estimation_mode)
  clear()
  fit
}
