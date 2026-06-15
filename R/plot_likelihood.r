#' Plot Likelihood Profile Results
#'
#' @title Plot Likelihood Profile
#'
#' @description
#' Creates a visualization of likelihood profile results, displaying the change
#' in log-likelihood as a function of a fixed parameter value. The plot shows
#' profiles for different data components and the total likelihood, helping to
#' assess parameter identifiability and uncertainty.
#'
#' @details
#' The function generates line plots showing how the log-likelihood changes
#' relative to its maximum value as a parameter is varied. Each data component
#' (e.g., landings, index, age composition) is shown as a separate line, along
#' with the total likelihood across all components. This reveals which data
#' sources provide information about the parameter and whether there are
#' conflicting signals.
#'
#' The y-axis shows the change in log-likelihood from the maximum (best-fit)
#' value. A parabolic shape indicates a well-identified parameter with symmetric
#' uncertainty. Flat profiles suggest weak identifiability, while multiple local
#' minima indicate potential convergence issues.
#'
#' The plot automatically saves to "likelihood.png" in the current working
#' directory in addition to returning a ggplot object that can be further
#' customized. The total likelihood is shown in black as a solid line, while
#' individual data components use colors from the viridis palette.
#'
#' @param like_fit A list returned by [run_fims_likelihood()] containing:
#'   * `vec` - Vector of parameter values used in the profile
#'   * `estimates` - Data frame with model estimates and likelihoods for each
#'     profiled value
#' @param group A character string or vector specifying the grouping variable(s)
#'   for likelihood components. Default is `"label"` to group by data type.
#'   Other options could include `c("label", "fleet_name")` for more detailed
#'   grouping, though fleet names are not currently in FIMS output. Must match
#'   column name(s) in the estimates data frame
#'
#' @return
#' A ggplot object displaying likelihood profiles. The plot includes:
#' * Line plots for each data component
#' * Total likelihood across all components (black solid line)
#' * Different colors (viridis palette) and line types for components
#' * X-axis showing the profiled parameter values
#' * Y-axis showing change in log-likelihood from maximum
#' * NOAA-themed styling via [stockplotr::theme_noaa()]
#'
#' The plot is also automatically saved as "likelihood.png".
#'
#' @references
#' Venzon, D.J. and Moolgavkar, S.H. 1988. A method for computing
#' profile-likelihood-based confidence intervals. Applied Statistics 37: 87-94.
#'
#' Hilborn, R. and Mangel, M. 1997. The Ecological Detective: Confronting
#' Models with Data. Princeton University Press.
#'
#' @seealso
#' * [run_fims_likelihood()] for running likelihood profile analysis
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
#' # Run likelihood profile
#' like_fit <- run_fims_likelihood(
#'   model = base_model,
#'   parameters = parameters,
#'   data = data_big,
#'   parameter_name = "log_rzero",
#'   n_cores = 3,
#'   min = -1,
#'   max = 1,
#'   length = 21
#' )
#'
#' # Plot the likelihood profile
#' plot_likelihood(like_fit)
#'
#' # Plot with custom grouping
#' plot_likelihood(like_fit, group = "label")
#' }
#'
plot_likelihood <- function(like_fit, group = "label") {
  # check that like_fit is actually the output from fims_likelihood()
  if (!"vec" %in% names(like_fit) | !"estimates" %in% names(like_fit)) {
    cli::cli_abort("like_fit needs to be a list returned by `fims_likelihood()` that contains `vec` and `estimates`")
  }

  # get column name for parameter being profiled
  colname <- like_fit$estimates |>
    names() |>
    grep(pattern = "value_", value = TRUE)
  if (length(colname) > 1) {
    cli::cli_abort("Function doesn't yet support multiple profile parameters")
  }
  if (length(colname) == 0) {
    cli::cli_abort(
      "No column with profile parameter found (column name should contain 'value_')"
    )
  }

  # TODO: add column to store the grouping variable(s)
  # could use paste() to combine multiple grouping variables into one column
  # then use that column for grouping below

  # summing total likelihood by parameter value and data type, use later for plotting
  by_type <- like_fit$estimates |>
    dplyr::filter(!is.na(.data$lpdf)) |>
    dplyr::group_by(.data[[colname]], .data[[group]]) |> # grouping by parameter and data type
    dplyr::distinct(.data$lpdf) |>
    dplyr::summarize(total_like = sum(.data$lpdf, na.rm = TRUE), .groups = "drop")

  ### add total likelihood across all groups
  total <- like_fit$estimates |>
    dplyr::filter(!is.na(.data$lpdf)) |>
    dplyr::group_by(.data[[colname]]) |>
    dplyr::distinct(.data$lpdf) |>
    dplyr::summarize(total_like = sum(.data$lpdf, na.rm = TRUE), .groups = "drop") |>
    dplyr::mutate(label = "Total") |>
    dplyr::select(.data[[colname]], .data$label, .data$total_like)

  # group the data type totals and the overall total and then
  # for each vector of sums, subtract the maximum across within that vector
  grouped_like <- by_type |>
    dplyr::bind_rows(total) |>
    dplyr::arrange(.data[[colname]]) |>
    dplyr::group_by(.data[[group]]) |>
    dplyr::mutate(total_like_change = max(.data$total_like, na.rm = TRUE) - .data$total_like) |>
    dplyr::ungroup()

  # Get all unique group values
  all_groups_clean <- unique(gsub("_expected", "", grouped_like[[group]]))
  other_groups_clean <- setdiff(all_groups_clean, "Total")

  # Create color vector: black for Total, default ggplot colors for others
  color_values <- c(
    "Total" = "black",
    stats::setNames(
      viridisLite::viridis(length(other_groups_clean),
        option = "mako",
        begin = 0.35, end = 0.9
      ),
      other_groups_clean
    )
  )

  linetype_values <- c(
    "Total" = "solid",
    stats::setNames(
      rep(c("dashed", "dotted", "dotdash", "longdash"),
        length.out = length(other_groups_clean)
      ),
      other_groups_clean
    )
  )

  # plot all the lines
  p1 <- grouped_like |>
    dplyr::mutate(
      group_clean = gsub("_expected", "", .data[[group]])
    ) |>
    ggplot2::ggplot() +
    ggplot2::geom_line(ggplot2::aes(
      x = .data[[colname]],
      y = .data$total_like_change,
      colour = .data$group_clean,
      linetype = .data$group_clean
    ), linewidth = 1.2) +
    stockplotr::theme_noaa(discrete = TRUE) +
    ggplot2::scale_color_manual(
      values = color_values,
      name = "Data Type"
    ) +
    ggplot2::scale_linetype_manual(
      values = linetype_values,
      name = "Data Type"
    ) +
    ggplot2::labs(
      x = colname |> gsub("value_", "", x = _), # remove "value_" from label
      y = "Change in log-likelihood",
      color = "Data Type"
    )

  ggplot2::ggsave("likelihood.png")

  return(p1)
}
