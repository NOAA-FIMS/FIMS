#' Plot Realized Age-to-Length Conversion
#'
#' Creates a ridgeline histogram plot of realized age-to-length conversion
#' probabilities by age for a selected year.
#'
#' @param data A data frame or tibble with columns `length`, `age`, and
#'   `timing`, plus either `estimated` or `observed`.
#'
#' @return A ggplot object.
#'
#' @export
plot_age_to_length_conversion <- function(data) {
  if (!requireNamespace("ggridges", quietly = TRUE)) {
    cli::cli_abort(
      "Package {.pkg ggridges} is required to plot age-to-length conversion."
    )
  }
  value_column <- if ("estimated" %in% names(data)) {
    "estimated"
  } else if ("observed" %in% names(data)) {
    "observed"
  } else {
    cli::cli_abort(
      "Data must contain either an {.field estimated} or {.field observed} column."
    )
  }
  length_bins <- unique(data[["length"]])
  n_length_bins <- dplyr::n_distinct(data[["length"]])

  data |>
    ggplot2::ggplot(
      ggplot2::aes(
        x = .data[["length"]],
        y = as.factor(.data[["age"]]),
        weight = .data[[value_column]],
        group = .data[["age"]]
      )
    ) +
    ggridges::geom_density_ridges(
      stat = "binline",
      bins = n_length_bins,
      scale = 1.5,
      alpha = 0.8
    ) +
    ggplot2::labs(
      title = paste(
        dplyr::if_else(
          value_column == "observed",
          "Observed",
          "Realized"
        ),
        "age-to-length conversion"
      ),
      subtitle = "Length distributions by age (x-axis) and time (panel)",
      x = "Length",
      y = "Age"
    ) +
    ggplot2::scale_x_continuous(
      breaks = length_bins
    ) +
    ggplot2::facet_grid(timing ~ .) +
    ggridges::theme_ridges() +
    ggplot2::coord_flip() +
    ggplot2::theme_minimal() +
    ggplot2::theme(
      panel.grid.minor = ggplot2::element_blank(),
      panel.grid.major.x = ggplot2::element_line(
        color = "grey80",
        linewidth = 0.3
      )
    )
}