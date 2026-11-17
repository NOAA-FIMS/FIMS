normalize_composition_spec <- function(spec) {
  if (is.null(spec)) return(NULL)
  if (!is.list(spec) || is.null(names(spec)) || anyNA(names(spec)) ||
      anyDuplicated(names(spec)) || any(!names(spec) %in% c("type", "concentration", "estimation_type")) ||
      !identical(spec$type, "dirichlet_multinomial") || !is.numeric(spec$concentration) ||
      length(spec$concentration) != 1L || !is.finite(spec$concentration) || spec$concentration <= 0) {
    cli::cli_abort("Composition specifications require type = 'dirichlet_multinomial' and one finite positive concentration.")
  }
  role <- if (is.null(spec$estimation_type)) "constant" else spec$estimation_type
  if (!is.character(role) || length(role) != 1L || is.na(role) || !role %in% c("constant", "fixed_effects"))
    cli::cli_abort("Composition concentration estimation_type must be constant or fixed_effects.")
  list(type = "dirichlet_multinomial", concentration = spec$concentration, estimation_type = role)
}

normalize_composition_likelihood <- function(specifications, data) {
  if (is.null(specifications)) return(list())
  named <- function(x) is.list(x) && length(x) > 0L && !is.null(names(x)) &&
    !anyNA(names(x)) && all(nzchar(names(x))) && !anyDuplicated(names(x))
  if (!named(specifications)) cli::cli_abort("composition_likelihood must be a named list of fleet specifications.")
  raw <- get_data(data)
  output <- lapply(names(specifications), function(fleet) {
    specification <- specifications[[fleet]]
    if (!named(specification) || any(!names(specification) %in% c("age_comp", "length_comp")))
      cli::cli_abort("Each composition fleet entry must name age_comp and/or length_comp.")
    result <- lapply(names(specification), function(type) {
      rows <- raw[raw$fleet %in% fleet & raw$type %in% type, ]
      if (!nrow(rows)) cli::cli_abort("Configured composition type must exist for fleet {.val {fleet}}.")
      families <- parse_data_distribution(rows$uncertainty)$family
      if (anyNA(families) || any(families != "dmultinom"))
        cli::cli_abort("Composition overrides require multinomial uncertainty formulas for input sample sizes.")
      if (is.null(specification[[type]])) cli::cli_abort("Each requested composition override requires a specification.")
      normalize_composition_spec(specification[[type]])
    })
    stats::setNames(result, names(specification))
  })
  stats::setNames(output, names(specifications))
}
