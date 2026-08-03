#' Register derived quantity report requests using component ids
#'
#' @description
#' Registers population or fleet derived quantity report requests on a model
#' object. Requests are id-based and can be used to include selected derived
#' quantities in value and uncertainty reporting.
#'
#' @param model A model object, such as `methods::new(CatchAtAge)`.
#' @param derived_quantity_reports A data frame or tibble with required columns
#'   `component_type`, `component_id`, and `quantity_name`.
#'   Optional columns are `report_se`, `report_value`, and `report_name`.
#' @param clear_existing A logical indicating whether existing requests are
#'   cleared before adding new requests. Defaults to `TRUE`.
#'
#' @return
#' The input `model`, invisibly.
#'
#' @export
set_derived_quantity_reports <- function(model,
                                         derived_quantity_reports,
                                         clear_existing = TRUE) {
    if (is.null(derived_quantity_reports)) {
        return(invisible(model))
    }

    reports <- tibble::as_tibble(derived_quantity_reports)

    required_columns <- c("component_type", "component_id", "quantity_name")
    missing_columns <- setdiff(required_columns, names(reports))
    if (length(missing_columns) > 0) {
        cli::cli_abort(c(
            "Derived quantity report input is missing required columns.",
            i = "Missing: {missing_columns}."
        ))
    }

    if (nrow(reports) == 0) {
        return(invisible(model))
    }

    reports <- reports |>
        dplyr::mutate(
            component_type = tolower(trimws(.data$component_type)),
            component_id = suppressWarnings(as.integer(.data$component_id)),
            quantity_name = as.character(.data$quantity_name),
            report_se = if ("report_se" %in% names(reports)) {
                as.logical(.data$report_se)
            } else {
                TRUE
            },
            report_value = if ("report_value" %in% names(reports)) {
                as.logical(.data$report_value)
            } else {
                TRUE
            },
            report_name = if ("report_name" %in% names(reports)) {
                as.character(.data$report_name)
            } else {
                ""
            }
        )

    invalid_component_type <- reports |>
        dplyr::filter(!.data$component_type %in% c("population", "fleet"))
    if (nrow(invalid_component_type) > 0) {
        bad_values <- invalid_component_type |>
            dplyr::pull(.data$component_type) |>
            unique()
        bad_values_label <- paste(bad_values, collapse = ", ")
        invalid_component_type_message <-
            paste0("Invalid values: ", bad_values_label, ".")
        cli::cli_abort(c(
            "`component_type` must be one of 'population' or 'fleet'.",
            i = invalid_component_type_message
        ))
    }

    invalid_component_id <- reports |>
        dplyr::filter(is.na(.data$component_id) | .data$component_id < 0)
    if (nrow(invalid_component_id) > 0) {
        cli::cli_abort(
            "`component_id` must be a non-negative integer for each request."
        )
    }

    invalid_quantity_name <- reports |>
        dplyr::filter(is.na(.data$quantity_name) | trimws(.data$quantity_name) == "")
    if (nrow(invalid_quantity_name) > 0) {
        cli::cli_abort("`quantity_name` must be a non-empty string for each request.")
    }

    if (clear_existing) {
        model$ClearDerivedQuantityReportRequests()
    }

    for (i in seq_len(nrow(reports))) {
        report_name_i <- reports[["report_name"]][[i]]
        if (is.na(report_name_i)) {
            report_name_i <- ""
        }

        if (reports[["component_type"]][[i]] == "population") {
            model$ReportPopulationDerivedQuantity(
                reports[["component_id"]][[i]],
                reports[["quantity_name"]][[i]],
                reports[["report_se"]][[i]],
                reports[["report_value"]][[i]],
                report_name_i
            )
        } else {
            model$ReportFleetDerivedQuantity(
                reports[["component_id"]][[i]],
                reports[["quantity_name"]][[i]],
                reports[["report_se"]][[i]],
                reports[["report_value"]][[i]],
                report_name_i
            )
        }
    }

    invisible(model)
}
