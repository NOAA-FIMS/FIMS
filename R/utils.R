# Internal helper: validate numeric type and allowed lengths.
assert_numeric_length <- function(x, x_name, valid_lengths, requirement) {
  if (!is.numeric(x) || !length(x) %in% valid_lengths) {
    local_bullets <- c(
      "i" = "{.var {x_name}} argument must be {requirement}.",
      "x" = "{.var {x_name}} has a length of {length(x)}.",
      "x" = "{.var {x_name}} is of the class {class(x)}."
    )
    names(local_bullets)[2] <- ifelse(length(x) > max(valid_lengths), "x", "i")
    names(local_bullets)[3] <- ifelse(inherits(x, "numeric"), "i", "x")
    cli::cli_abort(local_bullets)
  }
}

#' Convert snake_case strings to PascalCase
#'
#' This function takes a vector of strings in snake_case format and converts
#' them to PascalCase.
#'
#' @param snake_strings A vector of strings in snake_case format.
#' @return A vector of strings in PascalCase format.
#' @examples
#' snake_to_pascal(c("age_comp", "length_comp"))
#' snake_to_pascal("index")
#' @noRd
snake_to_pascal <- function(snake_strings) {
  purrr::map_chr(snake_strings, \(x) {
    parts <- strsplit(x, "_")[[1]]
    paste(
      toupper(substring(parts, 1, 1)),
      substring(parts, 2),
      sep = "",
      collapse = ""
    )
  })
}
