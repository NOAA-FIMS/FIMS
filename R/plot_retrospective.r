#' Plot Retrospective Analysis Results
#'
#' @title Plot Retrospective Analysis
#'
#' @description
#' Creates a visualization of retrospective analysis results, displaying
#' spawning biomass estimates from multiple retrospective peels.
#' The plot includes point estimates with uncertainty
#' intervals to help identify retrospective patterns or bias.
#'
#' @details
#' The function generates line plots showing how estimates change when
#' terminal years of data are removed. Each retrospective peel is displayed
#' as a separate line with a unique color and line type. Confidence intervals
#' are shown as shaded ribbons around each line, calculated as the estimate
#' +/- 1.96 * standard error (assuming normality).
#'
#' When multiple quantities are specified, the plot uses facets to display
#' each quantity in a separate panel with independent y-axes. This is useful
#' for comparing retrospective patterns across different model outputs.
#'
#' The plot automatically saves to "retrospective.png" in the current working
#' directory in addition to returning a ggplot object that can be further
#' customized.
#'
#' @param retro_fit A list returned by [run_fims_retrospective()] containing:
#'   * `years_to_remove` - Vector of years removed for each peel
#'   * `estimates` - Data frame with model estimates for each retrospective run
#' @param quantity A character vector specifying which quantity to plot.
#'   Currently, the only option is the default,
#'   `"spawning_biomass"`. The values must
#'   match entries in the `label` column of the estimates data frame.
#'
#' @return
#' A ggplot object displaying retrospective patterns. The plot includes:
#' * Line plots for each retrospective peel
#' * Shaded confidence intervals (+/-1.96 SE)
#' * Different colors and line types for each peel
#' * Facets by quantity if multiple quantities are specified
#' * NOAA-themed styling via [stockplotr::theme_noaa()]
#'
#' The plot is also automatically saved as "retrospective.png".
#'
#' @references
#' Mohn, R. 1999. The retrospective problem in sequential population analysis:
#' An investigation using cod fishery and simulated data. ICES Journal of
#' Marine Science 56: 473-488.
#'
#' @seealso
#' * [run_fims_retrospective()] for running retrospective analysis
#' * [calculate_mohns_rho()] for calculating retrospective bias statistic
#'
#' @family diagnostic_functions
#' @family plotting_functions
#'
#' @export
#'
#' @importFrom rlang .data
#'
#' @keywords diagnostics
#'
#' @examples
#' \dontrun{
#' library(FIMS)
#'
#' # Use built-in dataset from FIMS
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#'
#' # Create parameters object
#' parameters <- data_4_model |>
#'   create_default_configurations() |>
#'   create_default_parameters(data = data_4_model)
#'
#' # Run base model
#' base_model <- parameters |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#'
#' # Run retrospective analysis
#' retro_fit <- run_fims_retrospective(
#'   years_to_remove = 0:5,
#'   data = data_big,
#'   parameters = parameters,
#'   n_cores = 1
#' )
#'
#' # Plot spawning biomass only
#' plot_retrospective(
#'   retro_fit = retro_fit,
#'   quantity = "spawning_biomass"
#' )
#' }
#'
plot_retrospective <- function(retro_fit, quantity = "spawning_biomass") {
  # filter rows in estimates_df to get spawning_biomass
  retro_df <- retro_fit[["estimates"]] |>
    dplyr::filter(.data$label %in% quantity) |> # right now mortality_F is by year/age, so ignoring it for simplicity
    # TODO: some summarization to get total annual F values?
    dplyr::select(
      .data$label, .data$year_i, .data$age_i,
      .data$estimated, .data$uncertainty, .data$retrospective_peel
    ) |>
    dplyr::mutate(
      lower_CI = .data$estimated - (1.96 * .data$uncertainty),
      upper_CI = .data$estimated + (1.96 * .data$uncertainty),
      retrospective_peel = factor(.data$retrospective_peel)
    ) # assuming uncertainty is SE, TODO: check this assumption

  max_year <- max(retro_df$year_i, na.rm = TRUE)
  retro_df <- retro_df |>
    dplyr::mutate(retrospective_peel_num = as.numeric(as.character(.data$retrospective_peel))) |>
    dplyr::filter(.data$year_i <= (max_year - .data$retrospective_peel_num)) |>
    dplyr::select(-.data$retrospective_peel_num)

  retro_plot <- ggplot2::ggplot(data = retro_df, ggplot2::aes(x = .data$year_i)) +
    ggplot2::geom_ribbon(ggplot2::aes(
      ymin = .data$lower_CI, # for every line or just the reference line?
      ymax = .data$upper_CI,
      fill = .data$retrospective_peel
    ), alpha = .25) + # should this be conditional?
    ggplot2::geom_line(ggplot2::aes(
      y = .data$estimated,
      color = .data$retrospective_peel,
      linetype = .data$retrospective_peel
    ), linewidth = 1.2) +
    stockplotr::theme_noaa(discrete = TRUE) +
    ggplot2::labs(
      x = "Year", y = tools::toTitleCase(gsub("_", " ", quantity)),
      color = "Retrospective Peel",
      fill = "Retrospective Peel",
      linetype = "Retrospective Peel"
    )

  if (length(unique(quantity)) > 1) {
    retro_plot <- retro_plot +
      ggplot2::facet_wrap(~ .data$label, scales = "free_y", ncol = 1)
  }

  ggplot2::ggsave("retrospective.png")

  return(retro_plot)
}
