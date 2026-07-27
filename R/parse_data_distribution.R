#' Parse a distribution formula into structured components
#'
#' Parse a one-sided or two-sided formula-like distribution specification and
#' return the distribution family and parameter expressions as a list. This is
#' useful for translating user-written distribution declarations into a form
#' that can be validated or forwarded to downstream model-building utilities.
#'
#' @param expr A distribution specification provided as one of the following:
#'   a character string (for example, `"~ gaussian(mean = x, sd = 0.1)"`),
#'   an expression of length one (for example,
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
#' * `expression()` input must contain exactly one element, which is extracted
#'   and parsed as the language object.
#' * Language calls and formula objects are used directly.
#' * Any other input type throws an error.
#'
#' After conversion, the object must be a call whose head is `~`.
#' Both one-sided formulas (`~ dist(...)`) and two-sided formulas
#' (`response ~ dist(...)`) are supported; in both cases, only the
#' right-hand side distribution call is parsed.
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
#' environment (for example, `landings_expected`) so evaluation can occur later
#' in a context where those objects exist.
#'
#' @return A list with two elements:
#'
#' * `family`: character scalar containing the distribution family name.
#' * `parameters`: list of unevaluated arguments supplied to the distribution
#'   call.
#'
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
  # Preserve row alignment for upstream callers that keep NA placeholders.
  if (is.atomic(expr) && length(expr) == 1L && is.na(expr)) {
    all_parameter_names <- unique(unlist(
      FIMS::fims_distributions$other_parameters
    ))
    all_parameters <- rep(1, length(all_parameter_names))
    names(all_parameters) <- all_parameter_names
    return(c(
      family = NA_character_,
      as.list(all_parameters)
    ))
  }

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
      "i" = "(e.g., ~dnorm(mean = landings_expected, sd = 1))"
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
  dist_formals <- names(formals(dist_name))
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
  if (length(setdiff(names(args_list), dist_formals)) > 0) {
    cli::cli_abort(c(
      x = "The arguments you passed in your distribution, {expr}, are not
      formal arguments of the {.function {dist_name} function}",
      i = "Argument options are {dist_formals}",
      i = "Run {.code ?{dist_name}} for information on what each argument means"
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


  return(c(
    family = dist_name,
    list(link = args_list[[link_name]]),
    args_list
  ))
}

transpose_data_distribution <- function(x_list) {
  data <- purrr::transpose(x_list)
  unique_family <- unique(stats::na.omit(unlist(data[["family"]])))
  if (length(unique_family) > 1) {
    cli::cli_abort(c(
      "!" = "Cannot accommodate more than one family per data type right now.",
      x = "You passed the following types: {unique_family}"
    ))
  }

  # TODO: Check that the mean/prob args have a single value
  data
}
