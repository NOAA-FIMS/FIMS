#' Parse a distribution formula into structured components
#'
#' Parse one-sided formula-like distribution specifications and return their
#' distribution families and parameter expressions as an aligned list.
#'
#' @param expr One or more distribution specifications provided as a character
#'   vector (for example, `"~ gaussian(mean = x, sd = 0.1)"`), an expression
#'   (for example,
#'   `expression(~ gaussian(mean = x, sd = 0.1))`), a language call produced
#'   by `quote()`, or an R formula object.
#'
#' @details
#' The parser accepts inputs that represent a formula containing a distribution
#' call on the right-hand side, such as `~ gaussian(mean = x, sd = 0.1)`.
#'
#' Input handling and validation proceed in stages:
#'
#' * Character input is converted with [base::str2lang()].
#' * Elements of `expression()` input are extracted and parsed as language
#'   objects.
#' * A language call or formula object is treated as a scalar input.
#' * Any other input type throws an error.
#'
#' After conversion, the object must be a call whose head is `~`.
#' Only one-sided formulas (`~ dist(...)`) are supported.
#'
#' The distribution call is decomposed into:
#'
#' * `family`: the function name used for the distribution, as a character
#'   scalar (for example, `"gaussian"`, `"multinomial"`).
#' * `parameters`: a list of unevaluated parameter expressions from the
#'   distribution call. Named arguments preserve their names, while positional
#'   arguments remain unnamed.
#'
#' Parameter values are returned as language objects rather than evaluated
#' numeric vectors. This preserves references to symbols in the calling
#' environment (for example, `catch_expected`) so evaluation can occur later
#' in a context where those objects exist.
#'
#' @return A column-oriented list. Every component has the same length as
#'   `expr`, and every parameter supported by [FIMS::fims_distributions] is
#'   present. This includes:
#'
#' * `family`: a character vector containing the distribution family names.
#' * `link`: a character vector containing the central parameter field names.
#' * One list component for each supported distribution parameter.
#'
#' Parameters that do not apply to a non-missing specification are represented
#' by `NA`. Missing specifications use `NA_character_` for the family and link,
#' `NA` for central parameters, and `1` for ancillary parameters. These neutral
#' ancillary values prevent missing observations from introducing `NA` into
#' vectorized likelihood calculations. [initialize_data_distribution()] skips
#' these rows when it reads the multinomial offsets.
#'
#' @keywords internal
#' @examples
#' FIMS:::parse_data_distribution(
#'   "~ dmultinom(prob = age_comp_expected, size = 300)"
#' )
#' FIMS:::parse_data_distribution(
#'   quote(~ dnorm(mean = 0, sd = 1))
#' )
#' FIMS:::parse_data_distribution(
#'   expression(~ dlnorm(meanlog = mu, sdlog = 0.1))
#' )
parse_data_distribution <- function(expr) {
  if (is.character(expr) || is.expression(expr)) {
    expr <- as.list(expr)
  } else if (is.call(expr) || inherits(expr, "formula")) {
    expr <- list(expr)
  } else {
    cli::cli_abort(
      ".arg{expr} must be a character vector, expression, call, or formula."
    )
  }

  all_parameter_names <- unique(c(
    FIMS::fims_distributions$central_parameter,
    unlist(FIMS::fims_distributions$other_parameters)
  ))
  ancillary_parameter_names <- unique(unlist(
    FIMS::fims_distributions$other_parameters
  ))
  output <- c(
    list(family = rep(NA_character_, length(expr))),
    list(link = rep(NA_character_, length(expr))),
    stats::setNames(
      rep(list(rep(list(NA), length(expr))), length(all_parameter_names)),
      all_parameter_names
    )
  )

  for (i in seq_along(expr)) {
    if (length(expr[[i]]) == 1L && is.atomic(expr[[i]]) && is.na(expr[[i]])) {
      for (parameter in ancillary_parameter_names) {
        output[[parameter]][[i]] <- 1
      }
      next
    }
    parsed <- parse_one_data_distribution(expr[[i]])
    output$family[[i]] <- parsed$family
    output$link[[i]] <- rlang::as_label(parsed$link)
    for (parameter in names(parsed$parameters)) {
      output[[parameter]][[i]] <- parsed$parameters[[parameter]]
    }
  }
  output
}

parse_one_data_distribution <- function(expr) {
  # Convert string to a language object if it isn't already
  if (is.character(expr)) {
    lang <- str2lang(expr)
  } else if (is.expression(expr) && length(expr) == 1) {
    lang <- expr[[1]]
  } else if (is.call(expr) || inherits(expr, "formula")) {
    lang <- expr
  } else {
    cli::cli_abort(".arg{expr} must be a string, expression, call, or formula.")
  }
  if (!is.call(lang)) {
    cli::cli_abort("You must pass a string call.")
  }
  if (lang[[1]] != quote(`~`)) {
    cli::cli_abort(c(
      "Expression must be a distribution call",
      "i" = "(e.g., ~dnorm(mean = catch_expected, sd = 1))"
    ))
  }
  if (length(lang) == 3) {
    cli::cli_abort(c(
      "Two-sided formulas are not allowed in the data specifications",
      "i" = "You passed {.code {lang}}"
    ))
  }
  right_hand <- lang[[2]]

  # Extract the distribution family name
  dist_name <- as.character(right_hand[[1]])
  # FIMS arguments, not formals(), because the offsets are not arguments of
  # stats::dmultinom(). Base R because this runs once per data row.
  in_family <- FIMS::fims_distributions[["family"]] == dist_name
  dist_arguments <- c(
    FIMS::fims_distributions[["central_parameter"]][in_family],
    unlist(FIMS::fims_distributions[["other_parameters"]][in_family])
  )
  args_list <- as.list(right_hand[-1])
  link_name <- dplyr::filter(
    FIMS::fims_distributions,
    family == dist_name
  ) |>
    dplyr::pull(.data$central_parameter)

  # Checks
  if (!dist_name %in% FIMS::fims_distributions[["family"]]) {
    cli::cli_abort(c(
      x = "The distribution you specified, {.code {dist_name}} is not available
      in FIMS",
      i = "Available distributions include
      {.code {FIMS::fims_distributions$family}}."
    ))
  }
  if (length(args_list[[link_name]]) != 1) {
    cli::cli_abort(c(
      "The central parameter does not exist for {dist_name} in your formula",
      "You passed {expr}",
      "The {dist_name} expects {.code {link_name}} as a parameter"
    ))
  }
  unknown_arguments <- setdiff(names(args_list), dist_arguments)
  if (length(unknown_arguments) > 0) {
    cli::cli_abort(c(
      x = "{.code {dist_name}} in FIMS does not take
      {.val {unknown_arguments}}, in your distribution {expr}.",
      i = "Argument options are {.val {dist_arguments}}.",
      i = "See the Uncertainty section of {.code ?FIMSFrame}."
    ))
  }
  if (dist_name == "dnorm" && eval(args_list[["sd"]]) <= 0) {
    cli::cli_abort(c(
      x = "{.var sd} must be positive",
      i = "In your distribution, {expr}, you passed {eval(args_list$sd)}"
    ))
  }
  if (dist_name == "dlnorm" && eval(args_list[["sdlog"]]) <= 0) {
    cli::cli_abort(c(
      x = "{.var sdlog} must be positive",
      i = "In your distribution, {expr}, you passed {eval(args_list$sdlog)}"
    ))
  }
  offset_names <- intersect(
    c("offset_expected", "offset_observed"),
    names(args_list)
  )
  for (offset_name in offset_names) {
    # Store the value so an expression such as 1/1000 works downstream.
    args_list[[offset_name]] <- eval(args_list[[offset_name]])
    offset <- args_list[[offset_name]]
    if (!is.numeric(offset) || length(offset) != 1 || !is.finite(offset) ||
      offset < 0) {
      cli::cli_abort(c(
        x = "{.var {offset_name}} must be a single finite number, 0 or
        greater",
        i = "In your distribution, {expr}, you passed {.val {offset}}"
      ))
    }
  }


  list(
    family = dist_name,
    link = args_list[[link_name]],
    parameters = args_list
  )
}

validate_distribution_families <- function(data) {
  unique_family <- unique(stats::na.omit(data[["family"]]))
  if (length(unique_family) > 1L) {
    cli::cli_abort(c(
      "!" = "Cannot accommodate more than one family per data type right now.",
      x = "You passed the following types: {unique_family}"
    ))
  }

  data
}
