#' Model graph helpers
#'
#' These helpers represent a FIMS model as modules and links. A module defines
#' one component once, while a reference can point multiple parents to that same
#' module.
#'
#' @param id Character identifier for a module.
#' @param spec A FIMS component, observation, or distribution spec.
#' @param x A module spec or list of module specs.
#'
#' @return
#' `module()` returns a `fims_module_spec`. `ref()` returns a `fims_ref_spec`.
#' `as_model_graph()` returns a list with `modules` and `links` tibbles.
#'
#' @name model_graph
#' @keywords model
NULL

#' @rdname model_graph
#' @export
module <- function(id, spec) {
  check_graph_id(id, "id")
  if (!inherits(spec, "fims_spec") || inherits(spec, "fims_model_spec")) {
    stop("`spec` must be a FIMS module-level specification.", call. = FALSE)
  }

  out <- list(
    id = id,
    spec = spec
  )
  class(out) <- c("fims_module_spec", "fims_spec")
  out
}

#' @rdname model_graph
#' @export
ref <- function(id) {
  check_graph_id(id, "id")
  out <- list(id = id)
  class(out) <- c("fims_ref_spec", "fims_spec")
  out
}

#' @rdname model_graph
#' @export
as_model_graph <- function(x) {
  modules <- as_module_list(x)
  module_ids <- vapply(modules, `[[`, character(1), "id")

  if (any(duplicated(module_ids))) {
    duplicated_ids <- unique(module_ids[duplicated(module_ids)])
    stop(
      "Module ids must be unique. Duplicated ids: ",
      paste(duplicated_ids, collapse = ", "),
      call. = FALSE
    )
  }

  module_rows <- lapply(modules, module_row)
  link_rows <- unlist(lapply(modules, module_links), recursive = FALSE)

  links <- tibble::tibble(
    from = character(),
    role = character(),
    to = character()
  )
  if (length(link_rows) > 0L) {
    links <- do.call(rbind, link_rows)
    missing_ids <- setdiff(links[["to"]], module_ids)
    if (length(missing_ids) > 0L) {
      stop(
        "References must point to existing module ids. Missing ids: ",
        paste(missing_ids, collapse = ", "),
        call. = FALSE
      )
    }
  }

  out <- list(
    modules = do.call(rbind, module_rows),
    links = links
  )
  class(out) <- c("fims_model_graph", "list")
  out
}

as_module_list <- function(x) {
  if (inherits(x, "fims_module_spec")) {
    return(list(x))
  }
  if (!is.list(x) || !all(vapply(x, inherits, logical(1), "fims_module_spec"))) {
    stop("`x` must be a module spec or list of module specs.", call. = FALSE)
  }
  x
}

module_row <- function(x) {
  spec <- x[["spec"]]
  tibble::tibble(
    id = x[["id"]],
    type = spec_type(spec),
    name = spec_name(spec),
    spec = list(spec)
  )
}

module_links <- function(x) {
  spec <- x[["spec"]]
  reference_fields <- names(spec)[vapply(spec, inherits, logical(1), "fims_ref_spec")]

  lapply(reference_fields, function(field) {
    tibble::tibble(
      from = x[["id"]],
      role = field,
      to = spec[[field]][["id"]]
    )
  })
}

spec_type <- function(spec) {
  if (inherits(spec, "fims_component_spec")) {
    return(spec[["component"]])
  }
  if (inherits(spec, "fims_observation_spec")) {
    return(spec[["data_type"]])
  }
  spec[["family"]]
}

spec_name <- function(spec) {
  if (!is.null(spec[["name"]])) {
    return(spec[["name"]])
  }
  if (!is.null(spec[["fleet"]])) {
    return(spec[["fleet"]])
  }
  NA_character_
}

check_graph_id <- function(id, name) {
  if (!is.character(id) || length(id) != 1L || is.na(id) || id == "") {
    stop("`", name, "` must be a non-empty character scalar.", call. = FALSE)
  }
  invisible(id)
}
