#' Plot Realized Age-to-Length Conversion
#'
#' Creates a ridgeline histogram plot of realized age-to-length conversion
#' probabilities by age for a selected year.
#'
#' @param age_to_length_conversion_dataframe A data frame or tibble with columns
#'   `length`, `age`, `year`, and `proportion`.
#' @param year Optional integer year to plot. If `NULL`, the maximum available
#'   year in `age_to_length_conversion_dataframe` is used.
#'
#' @return A ggplot object.
#'
#' @export
plot_age_to_length_conversion <- function(age_to_length_conversion_dataframe, year = NULL) {
  if (!requireNamespace("ggridges", quietly = TRUE)) {
    cli::cli_abort(
      "Package {.pkg ggridges} is required to plot age-to-length conversion."
    )
  }

  selected_year <- if (is.null(year)) {
    max(age_to_length_conversion_dataframe[["year"]])
  } else {
    year
  }

  if (!selected_year %in% age_to_length_conversion_dataframe[["year"]]) {
    cli::cli_abort(
      "Input year {.val {selected_year}} is not present in the supplied data."
    )
  }

  age_to_length_conversion_dataframe |>
    dplyr::filter(.data[["year"]] == selected_year) |>
    ggplot2::ggplot(
      ggplot2::aes(
        x = .data[["length"]],
        y = as.factor(.data[["age"]]),
        weight = .data[["proportion"]],
        group = .data[["age"]]
      )
    ) +
    ggridges::geom_density_ridges(
      stat = "binline",
      bins = dplyr::n_distinct(age_to_length_conversion_dataframe[["length"]]),
      scale = 1.5,
      alpha = 0.8
    ) +
    ggplot2::labs(
      title = "Realized age-to-length conversion",
      subtitle = paste(
        "Length distributions by age in modeled year",
        selected_year
      ),
      x = "Length",
      y = "Age"
    ) +
    ggplot2::scale_x_continuous(
      breaks = unique(age_to_length_conversion_dataframe[["length"]])
    ) +
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