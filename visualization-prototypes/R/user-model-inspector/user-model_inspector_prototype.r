# -------------------------------------------------------------------------
# 0. Setup
# -------------------------------------------------------------------------
# Purpose:
# Build a model-specific FIMS explorer with a compact overview graph and a
# persistent inspector panel. This prototype is meant to be easier to read
# than the full Graphviz blueprint because detailed text lives in the panel,
# not inside every graph node.
#
# This is a prototype script. It does not modify FIMS source code.
# Run from the FIMS repository root.

if (!file.exists("DESCRIPTION") ||
    !file.exists(file.path("R", "Rcpp_exports.R")) ||
    !dir.exists("src")) {
  stop("Run this script from the FIMS repository root.")
}

if (!requireNamespace("jsonlite", quietly = TRUE)) {
  stop("Install the jsonlite package to render the inspector prototype.")
}

html_escape <- function(x) {
  x <- ifelse(is.na(x), "", x)
  x <- gsub("&", "&amp;", x, fixed = TRUE)
  x <- gsub("<", "&lt;", x, fixed = TRUE)
  x <- gsub(">", "&gt;", x, fixed = TRUE)
  x <- gsub('"', "&quot;", x, fixed = TRUE)
  x
}

coalesce_empty <- function(x, replacement = "None detected") {
  if (length(x) == 0 || all(is.na(x)) || all(x == "")) {
    return(replacement)
  }

  x
}

short_list <- function(x, max_items = 5) {
  x <- unique(stats::na.omit(x))

  if (length(x) == 0) {
    return("None detected")
  }

  if (length(x) > max_items) {
    x <- c(x[seq_len(max_items)], paste0("+ ", length(x) - max_items, " more"))
  }

  paste(x, collapse = ", ")
}

safe_file_name <- function(x) {
  x <- tolower(trimws(x))
  x <- gsub("[^a-z0-9]+", "_", x, perl = TRUE)
  x <- gsub("^_+|_+$", "", x, perl = TRUE)

  if (identical(x, "")) {
    return("fims_model")
  }

  x
}

default_model_inspector_output <- function(
    model_label = "User model",
    output_dir = file.path(
      "visualization-prototypes",
      "examples",
      "user-model-inspector"
    )) {
  file.path(
    output_dir,
    paste0(safe_file_name(model_label), "_inspector.html")
  )
}

make_readable_equation <- function(equation) {
  equation <- gsub("\\\\_", "_", equation)
  equation <- gsub("\\\\mathrm\\{([^{}]+)\\}", "\\1", equation, perl = TRUE)
  equation <- gsub("\\\\text\\{([^{}]+)\\}", "\\1", equation, perl = TRUE)
  equation <- gsub("\\\\exp", "exp", equation)
  equation <- gsub("\\\\log", "log", equation)
  equation <- gsub("\\\\sqrt", "sqrt", equation)
  equation <- gsub("\\\\times", "*", equation)
  equation <- gsub("\\\\quad", " ", equation)
  equation <- gsub("\\\\left|\\\\right", "", equation, perl = TRUE)
  equation <- gsub("\\\\,", ",", equation)
  equation <- gsub("\\{([A-Za-z_][A-Za-z0-9_]*)\\}", "\\1", equation, perl = TRUE)

  previous_equation <- NA_character_

  while (!identical(previous_equation, equation)) {
    previous_equation <- equation
    equation <- gsub(
      "\\\\frac\\{([^{}]+)\\}\\{([^{}]+)\\}",
      "(\\1) / (\\2)",
      equation,
      perl = TRUE
    )
  }

  equation <- gsub("\\\\([A-Za-z]+)", "\\1", equation, perl = TRUE)
  equation <- gsub("\\s+", " ", equation)
  trimws(equation)
}

# -------------------------------------------------------------------------
# 1. Parse FIMS documentation links and Rcpp registrations
# -------------------------------------------------------------------------

parse_cpp_doc_nodes <- function() {
  doc_lines <- readLines(file.path("R", "Rcpp_exports.R"), warn = FALSE)
  current_doc_type <- NA_character_
  doc_rows <- list()

  for (line in doc_lines) {
    if (grepl("C\\+\\+ Classes Exported", line)) {
      current_doc_type <- "cpp_class"
    }

    if (grepl("C\\+\\+ Functions Exported", line)) {
      current_doc_type <- "cpp_function"
    }

    match <- regexec("^#' - \\[([^]]+)\\]\\(([^)]+)\\)", line)
    hit <- regmatches(line, match)[[1]]

    if (length(hit) == 3) {
      doc_rows[[length(doc_rows) + 1]] <- data.frame(
        name = hit[2],
        node_type = current_doc_type,
        url = hit[3],
        stringsAsFactors = FALSE
      )
    }
  }

  if (length(doc_rows) == 0) {
    return(data.frame(
      name = character(),
      node_type = character(),
      url = character(),
      stringsAsFactors = FALSE
    ))
  }

  do.call(rbind, doc_rows)
}

normalize_doxygen_lines <- function(lines) {
  lines <- gsub("/\\*\\*", "", lines)
  lines <- gsub("\\*/", "", lines)
  lines <- gsub("^\\s*\\*\\s?", "", lines)
  trimws(lines)
}

extract_doxygen_tag_value <- function(lines, tag) {
  normalized_lines <- normalize_doxygen_lines(lines)
  tag_indices <- grep(paste0("^@", tag, "\\b"), normalized_lines)

  if (length(tag_indices) == 0) {
    return("")
  }

  tag_index <- tag_indices[[1]]
  values <- sub(
    paste0("^@", tag, "\\s*"),
    "",
    normalized_lines[[tag_index]]
  )

  next_index <- tag_index + 1L

  while (next_index <= length(normalized_lines) &&
         !grepl("^@", normalized_lines[[next_index]])) {
    if (nzchar(normalized_lines[[next_index]])) {
      values <- c(values, normalized_lines[[next_index]])
    }

    next_index <- next_index + 1L
  }

  gsub("\\s+", " ", trimws(paste(values, collapse = " ")))
}

parse_doxygen_member_docs_file <- function(path) {
  lines <- readLines(path, warn = FALSE)
  current_entity <- NA_character_
  rows <- list()
  line_index <- 1L

  while (line_index <= length(lines)) {
    entity_match <- regexec(
      "^\\s*(class|struct)\\s+([A-Za-z_][A-Za-z0-9_]*)",
      lines[[line_index]],
      perl = TRUE
    )
    entity_hit <- regmatches(lines[[line_index]], entity_match)[[1]]

    if (length(entity_hit) == 3) {
      current_entity <- entity_hit[[3]]
    }

    if (grepl("/**", lines[[line_index]], fixed = TRUE)) {
      block_start <- line_index
      block_end <- line_index

      while (block_end <= length(lines) &&
             !grepl("\\*/", lines[[block_end]])) {
        block_end <- block_end + 1L
      }

      comment_lines <- lines[block_start:block_end]
      brief <- extract_doxygen_tag_value(comment_lines, "brief")
      copydoc <- extract_doxygen_tag_value(comment_lines, "copydoc")
      code_index <- block_end + 1L

      while (code_index <= length(lines) &&
             grepl("^\\s*(//.*)?$", lines[[code_index]])) {
        code_index <- code_index + 1L
      }

      code_line <- if (code_index <= length(lines)) {
        lines[[code_index]]
      } else {
        ""
      }

      entity_match <- regexec(
        "^\\s*(class|struct)\\s+([A-Za-z_][A-Za-z0-9_]*)",
        code_line,
        perl = TRUE
      )
      entity_hit <- regmatches(code_line, entity_match)[[1]]

      if (length(entity_hit) == 3) {
        current_entity <- entity_hit[[3]]
        line_index <- block_end + 1L
        next
      }

      member_match <- regexec(
        "^.*\\b([A-Za-z_][A-Za-z0-9_]*)\\s*(?:=|;).*$",
        code_line,
        perl = TRUE
      )
      member_hit <- regmatches(code_line, member_match)[[1]]

      if (!is.na(current_entity) &&
          length(member_hit) == 2 &&
          (nzchar(brief) || nzchar(copydoc))) {
        rows[[length(rows) + 1L]] <- data.frame(
          entity = current_entity,
          member = member_hit[[2]],
          brief = brief,
          copydoc = copydoc,
          source_file = path,
          stringsAsFactors = FALSE
        )
      }

      line_index <- block_end + 1L
      next
    }

    line_index <- line_index + 1L
  }

  if (length(rows) == 0) {
    return(data.frame(
      entity = character(),
      member = character(),
      brief = character(),
      copydoc = character(),
      source_file = character(),
      stringsAsFactors = FALSE
    ))
  }

  do.call(rbind, rows)
}

copydoc_to_member_key <- function(copydoc) {
  copydoc <- gsub("`|\"", "", copydoc)
  copydoc <- trimws(copydoc)
  copydoc_parts <- strsplit(copydoc, "::", fixed = TRUE)[[1]]

  if (length(copydoc_parts) < 2) {
    return("")
  }

  paste(utils::tail(copydoc_parts, 2), collapse = "::")
}

parse_doxygen_member_docs <- function() {
  header_paths <- list.files(
    file.path("inst", "include"),
    pattern = "\\.hpp$",
    recursive = TRUE,
    full.names = TRUE
  )

  member_docs <- do.call(
    rbind,
    lapply(header_paths, parse_doxygen_member_docs_file)
  )

  if (nrow(member_docs) == 0) {
    member_docs$definition <- character()
    return(member_docs)
  }

  member_docs$key <- paste(member_docs$entity, member_docs$member, sep = "::")
  member_docs$definition <- member_docs$brief
  brief_lookup <- stats::setNames(member_docs$brief, member_docs$key)
  copydoc_rows <- which(!nzchar(member_docs$definition) &
                          nzchar(member_docs$copydoc))

  for (row_index in copydoc_rows) {
    reference_key <- copydoc_to_member_key(member_docs$copydoc[[row_index]])

    if (nzchar(reference_key) &&
        reference_key %in% names(brief_lookup) &&
        nzchar(brief_lookup[[reference_key]])) {
      member_docs$definition[[row_index]] <- brief_lookup[[reference_key]]
    }
  }

  member_docs
}

get_field_definitions <- function(component_name, interface, fields) {
  fields <- unique(stats::na.omit(fields))

  if (length(fields) == 0 || nrow(doxygen_member_docs) == 0) {
    return(stats::setNames(character(), character()))
  }

  interface <- sub("^.*::", "", interface)
  preferred_entities <- unique(c(interface, component_name))
  definitions <- vapply(fields, function(field) {
    matching_rows <- doxygen_member_docs[
      doxygen_member_docs$member == field &
        doxygen_member_docs$entity %in% preferred_entities &
        nzchar(doxygen_member_docs$definition),
    ]

    if (nrow(matching_rows) == 0) {
      matching_rows <- doxygen_member_docs[
        doxygen_member_docs$member == field &
          nzchar(doxygen_member_docs$definition),
      ]
    }

    if (nrow(matching_rows) == 0) {
      return("")
    }

    matching_rows$definition[[1]]
  }, character(1))

  definitions <- definitions[nzchar(definitions)]
  definitions
}

parse_rcpp_registration_file <- function(path) {
  source_text <- paste(readLines(path, warn = FALSE), collapse = "\n")
  starts <- gregexpr("Rcpp::class_<", source_text, fixed = TRUE)[[1]]

  if (identical(starts, -1L)) {
    return(data.frame(
      r_name = character(),
      cpp_interface = character(),
      fields = I(list()),
      methods = I(list()),
      source_file = character(),
      stringsAsFactors = FALSE
    ))
  }

  out <- list()

  for (i in seq_along(starts)) {
    chunk_start <- starts[[i]]
    chunk_end <- if (i < length(starts)) {
      starts[[i + 1]] - 1L
    } else {
      nchar(source_text)
    }

    chunk <- substr(source_text, chunk_start, chunk_end)
    class_match <- regexec(
      "Rcpp::class_<\\s*([^>]+?)\\s*>\\s*\\(\\s*\"([^\"]+)\"",
      chunk,
      perl = TRUE
    )
    class_hit <- regmatches(chunk, class_match)[[1]]

    if (length(class_hit) != 3) {
      next
    }

    field_hits <- gregexpr("\\.field\\s*\\(\\s*\"([^\"]+)\"", chunk, perl = TRUE)
    field_matches <- regmatches(chunk, field_hits)[[1]]
    fields <- if (length(field_matches) == 1 && field_matches[[1]] == "") {
      character()
    } else {
      sub(".*\\.field\\s*\\(\\s*\"([^\"]+)\".*", "\\1", field_matches)
    }

    method_hits <- gregexpr("\\.method\\s*\\(\\s*\"([^\"]+)\"", chunk, perl = TRUE)
    method_matches <- regmatches(chunk, method_hits)[[1]]
    methods <- if (length(method_matches) == 1 && method_matches[[1]] == "") {
      character()
    } else {
      sub(".*\\.method\\s*\\(\\s*\"([^\"]+)\".*", "\\1", method_matches)
    }

    out[[length(out) + 1]] <- data.frame(
      r_name = class_hit[3],
      cpp_interface = trimws(class_hit[2]),
      fields = I(list(unique(fields))),
      methods = I(list(unique(methods))),
      source_file = path,
      stringsAsFactors = FALSE
    )
  }

  if (length(out) == 0) {
    return(data.frame(
      r_name = character(),
      cpp_interface = character(),
      fields = I(list()),
      methods = I(list()),
      source_file = character(),
      stringsAsFactors = FALSE
    ))
  }

  do.call(rbind, out)
}

cpp_doc_nodes <- parse_cpp_doc_nodes()
doxygen_member_docs <- parse_doxygen_member_docs()

rcpp_files <- list.files(
  "src",
  pattern = "^rcpp_.*\\.cpp$",
  full.names = TRUE
)

rcpp_registration <- do.call(
  rbind,
  lapply(rcpp_files, parse_rcpp_registration_file)
)

rcpp_registration <- merge(
  rcpp_registration,
  cpp_doc_nodes[, c("name", "url")],
  by.x = "r_name",
  by.y = "name",
  all.x = TRUE
)

# -------------------------------------------------------------------------
# 2. Load equation catalog
# -------------------------------------------------------------------------

equation_catalog_path <- file.path(
  "visualization-prototypes",
  "data",
  "fims_equation_catalog.csv"
)
equation_extraction_script <- file.path(
  "visualization-prototypes",
  "R",
  "package-level-explorer",
  "equation_extraction_prototype.r"
)

if (!file.exists(equation_catalog_path)) {
  if (!file.exists(equation_extraction_script)) {
    stop(
      "Equation catalog not found and the equation extraction script ",
      "is not available at ",
      equation_extraction_script,
      "."
    )
  }

  source(equation_extraction_script)
}

equation_catalog <- utils::read.csv(
  equation_catalog_path,
  stringsAsFactors = FALSE
)

get_equation_rows_for_component <- function(component_name) {
  r_visible_name <- equation_catalog$r_visible_name
  entity <- equation_catalog$entity
  enclosing_class <- equation_catalog$enclosing_class

  r_visible_name[is.na(r_visible_name)] <- ""
  entity[is.na(entity)] <- ""
  enclosing_class[is.na(enclosing_class)] <- ""

  rows <- equation_catalog[
    r_visible_name %in% component_name |
      entity %in% component_name |
      enclosing_class %in% component_name,
  ]

  rows <- rows[
    !is.na(rows$equation) &
      rows$equation != "" &
      rows$equation != "NA",
  ]

  rows[!duplicated(rows$equation), ]
}

# -------------------------------------------------------------------------
# 3. Parse Doxygen-style assumption tags
# -------------------------------------------------------------------------

clean_doxygen_lines <- function(lines) {
  lines <- gsub("^\\s*/\\*\\*\\s?", "", lines)
  lines <- gsub("\\s*\\*/\\s*$", "", lines)
  lines <- gsub("^\\s*\\*\\s?", "", lines)
  trimws(lines)
}

collapse_doxygen_tag <- function(lines, tag) {
  tag_pattern <- paste0("^@", tag, "\\s+")
  tag_indices <- grep(tag_pattern, lines)

  if (length(tag_indices) == 0) {
    return(character())
  }

  vapply(tag_indices, function(tag_index) {
    next_tag <- grep("^@[A-Za-z_]+", lines)
    next_tag <- next_tag[next_tag > tag_index]
    end_index <- if (length(next_tag) == 0) {
      length(lines)
    } else {
      next_tag[[1]] - 1L
    }

    text_lines <- lines[tag_index:end_index]
    text_lines[[1]] <- sub(tag_pattern, "", text_lines[[1]])
    text_lines <- text_lines[text_lines != ""]
    gsub("\\s+", " ", paste(text_lines, collapse = " "))
  }, character(1))
}

find_cpp_entity_after_block <- function(lines, block_end) {
  lookahead <- lines[seq.int(
    from = min(block_end + 1L, length(lines)),
    to = min(block_end + 8L, length(lines))
  )]
  entity_match <- regexec("\\b(struct|class)\\s+([A-Za-z_][A-Za-z0-9_]*)", lookahead)
  entity_hit <- regmatches(lookahead, entity_match)
  entity_hit <- entity_hit[lengths(entity_hit) == 3]

  if (length(entity_hit) == 0) {
    return("")
  }

  entity_hit[[1]][[3]]
}

component_for_cpp_entity <- function(entity) {
  if (is.na(entity) || entity == "") {
    return("")
  }

  if (entity %in% rcpp_registration$r_name) {
    return(entity)
  }

  r_visible_name <- equation_catalog$r_visible_name
  source_entity <- equation_catalog$entity
  enclosing_class <- equation_catalog$enclosing_class

  r_visible_name[is.na(r_visible_name)] <- ""
  source_entity[is.na(source_entity)] <- ""
  enclosing_class[is.na(enclosing_class)] <- ""

  candidate <- unique(r_visible_name[
    source_entity %in% entity |
      enclosing_class %in% entity
  ])
  candidate <- candidate[candidate != ""]

  if (length(candidate) > 0) {
    return(candidate[[1]])
  }

  if (grepl("BevertonHolt", entity)) {
    return("BevertonHoltRecruitment")
  }

  ""
}

parse_assumption_tags_from_file <- function(path) {
  source_lines <- readLines(path, warn = FALSE)
  starts <- grep("/\\*\\*", source_lines)
  out <- list()

  for (start in starts) {
    possible_ends <- grep("\\*/", source_lines)
    possible_ends <- possible_ends[possible_ends >= start]

    if (length(possible_ends) == 0) {
      next
    }

    end <- possible_ends[[1]]
    block <- clean_doxygen_lines(source_lines[start:end])
    assumptions <- collapse_doxygen_tag(block, "assumption")

    if (length(assumptions) == 0) {
      next
    }

    explicit_class <- collapse_doxygen_tag(block, "class")
    explicit_struct <- collapse_doxygen_tag(block, "struct")
    entity <- c(explicit_class, explicit_struct)
    entity <- entity[entity != ""]
    entity <- if (length(entity) > 0) {
      strsplit(entity[[1]], "\\s+")[[1]][[1]]
    } else {
      find_cpp_entity_after_block(source_lines, end)
    }

    component <- component_for_cpp_entity(entity)

    for (assumption in assumptions) {
      out[[length(out) + 1]] <- data.frame(
        component = component,
        cpp_entity = entity,
        assumption = assumption,
        status = "source-tagged",
        source_file = path,
        source_line = start,
        stringsAsFactors = FALSE
      )
    }
  }

  if (length(out) == 0) {
    return(data.frame(
      component = character(),
      cpp_entity = character(),
      assumption = character(),
      status = character(),
      source_file = character(),
      source_line = integer(),
      stringsAsFactors = FALSE
    ))
  }

  do.call(rbind, out)
}

source_files_for_assumptions <- c(
  list.files("inst/include", pattern = "\\.(hpp|cpp)$", recursive = TRUE, full.names = TRUE),
  list.files("src", pattern = "\\.(hpp|cpp)$", recursive = TRUE, full.names = TRUE)
)

source_assumption_catalog <- do.call(
  rbind,
  lapply(source_files_for_assumptions, parse_assumption_tags_from_file)
)

discussion_assumption_catalog <- data.frame(
  component = c("BevertonHoltRecruitment", "BevertonHoltRecruitment"),
  cpp_entity = c("BevertonHolt", "BevertonHolt"),
  assumption = c(
    "Recruitment is strictly deterministic.",
    "Spawning biomass must be greater than zero."
  ),
  status = c("prototype draft from discussion example", "prototype draft from discussion example"),
  source_file = c("", ""),
  source_line = c(NA_integer_, NA_integer_),
  stringsAsFactors = FALSE
)

assumption_catalog <- rbind(
  source_assumption_catalog,
  discussion_assumption_catalog
)

get_assumption_rows_for_component <- function(component_name) {
  rows <- assumption_catalog[
    assumption_catalog$component %in% component_name,
  ]

  rows[!duplicated(paste(rows$component, rows$assumption, rows$status)), ]
}

format_assumptions_for_component <- function(component_name) {
  rows <- get_assumption_rows_for_component(component_name)

  if (nrow(rows) == 0) {
    return(character())
  }

  paste0(rows$assumption, " [", rows$status, "]")
}

# -------------------------------------------------------------------------
# 4. Define one example configured model
# -------------------------------------------------------------------------
# This is the only section that should need to change for a different model.
# A future parser could build the same structure from a FIMS model object or
# from model configuration JSON.

module_id <- function(model_role) {
  paste0("module_", model_role)
}

empty_spec_table <- function(column_names) {
  out <- as.data.frame(
    stats::setNames(replicate(length(column_names), character(), simplify = FALSE), column_names),
    stringsAsFactors = FALSE
  )
  out
}

find_fims_helper <- function(function_name) {
  if (exists(function_name, mode = "function", inherits = TRUE)) {
    return(get(function_name, mode = "function", inherits = TRUE))
  }

  if (requireNamespace("FIMS", quietly = TRUE) &&
      exists(function_name, envir = asNamespace("FIMS"), mode = "function")) {
    return(get(function_name, envir = asNamespace("FIMS"), mode = "function"))
  }

  NULL
}

attach_fims_for_model_inspector <- function() {
  if ("package:FIMS" %in% search()) {
    return(TRUE)
  }

  if (!requireNamespace("FIMS", quietly = TRUE)) {
    return(FALSE)
  }

  suppressPackageStartupMessages(
    library("FIMS", character.only = TRUE)
  )

  TRUE
}

setup_helper_for_model_role <- function(model_role) {
  switch(
    model_role,
    Growth = "setup_default_Growth",
    Maturity = "setup_default_Maturity",
    Selectivity = "setup_default_Selectivity",
    Recruitment = "setup_default_Recruitment",
    ""
  )
}

module_options_from_setup_helper <- function(model_role) {
  helper_name <- setup_helper_for_model_role(model_role)

  if (!nzchar(helper_name)) {
    return(character())
  }

  helper <- find_fims_helper(helper_name)

  if (is.null(helper)) {
    return(character())
  }

  helper_formals <- formals(helper)

  if (!"module_type" %in% names(helper_formals)) {
    return(character())
  }

  options <- tryCatch(
    eval(helper_formals[["module_type"]], envir = parent.frame()),
    error = function(error) character()
  )

  if (!is.character(options)) {
    return(character())
  }

  options[!is.na(options) & nzchar(options)]
}

fallback_module_option_catalog <- function(model_role) {
  switch(
    model_role,
    Growth = c("EWAA", "VonBertalanffySchnute"),
    Maturity = "Logistic",
    Observation = "CatchAtAge",
    Selectivity = c("Logistic", "DoubleLogistic", "AgeSpecific"),
    Recruitment = "BevertonHolt",
    character()
  )
}

prepare_model_inspector_data <- function(data, require_fims_frame = FALSE) {
  if (is.null(data)) {
    stop("Provide `data` as a FIMSFrame or data frame.")
  }

  if (inherits(data, "FIMSFrame")) {
    return(data)
  }

  if (is.data.frame(data) && isTRUE(require_fims_frame)) {
    attach_fims_for_model_inspector()
    fims_frame <- find_fims_helper("FIMSFrame")

    if (is.null(fims_frame)) {
      stop(
        "`parameters` was not supplied, so the helper needs `FIMSFrame()` ",
        "and `setup_default_parameters()` to derive model settings. ",
        "Load FIMS first or pass an explicit `parameters` table."
      )
    }

    return(fims_frame(data))
  }

  if (is.data.frame(data)) {
    return(data)
  }

  stop("The `data` argument must be a FIMSFrame or data frame for this prototype.")
}

derive_model_inspector_parameters <- function(data) {
  attach_fims_for_model_inspector()
  setup_default_parameters <- find_fims_helper("setup_default_parameters")

  if (is.null(setup_default_parameters)) {
    stop(
      "`parameters` was not supplied and `setup_default_parameters()` ",
      "is not available. Load FIMS first or pass an explicit `parameters` table."
    )
  }

  setup_default_parameters(data = data)
}

prepare_model_inspector_inputs <- function(
    data,
    parameters = NULL,
    setup_default_if_missing = TRUE) {
  data <- prepare_model_inspector_data(
    data = data,
    require_fims_frame = is.null(parameters) && isTRUE(setup_default_if_missing)
  )

  if (is.null(parameters) && isTRUE(setup_default_if_missing)) {
    parameters <- derive_model_inspector_parameters(data)
  }

  if (is.null(parameters)) {
    stop(
      "Provide `parameters`, or allow the helper to derive them by passing ",
      "`setup_default_if_missing = TRUE` with FIMS loaded."
    )
  }

  if (!is.data.frame(parameters)) {
    stop("The `parameters` argument must be a data frame or tibble.")
  }

  list(
    data = data,
    parameters = as.data.frame(parameters, stringsAsFactors = FALSE)
  )
}

extract_fims_data_frame <- function(data) {
  if (is.null(data)) {
    return(NULL)
  }

  if (inherits(data, "FIMSFrame")) {
    get_data <- find_fims_helper("get_data")

    if (!is.null(get_data)) {
      return(get_data(data))
    }

    return(methods::slot(data, "data"))
  }

  if (is.data.frame(data)) {
    return(data)
  }

  stop(
    "The `data` argument must be a FIMSFrame or data frame for this prototype."
  )
}

has_data_type <- function(data_frame, data_type) {
  if (is.null(data_frame) || !"type" %in% names(data_frame)) {
    return(FALSE)
  }

  any(data_frame$type %in% data_type, na.rm = TRUE)
}

component_from_parameter_type <- function(model_role, module_type) {
  module_type <- unique(stats::na.omit(module_type))
  module_type <- module_type[module_type != ""]

  if (length(module_type) == 0) {
    return("")
  }

  module_type <- module_type[[1]]

  if (identical(model_role, "Growth") && module_type %in% c("EWAA", "Ewaa")) {
    return("EWAAGrowth")
  }

  if (identical(model_role, "Growth") &&
      module_type %in% c("VonBertalanffySchnute", "VonB-Schnute")) {
    return("VonBertalanffySchnuteGrowth")
  }

  if (identical(model_role, "Maturity") && identical(module_type, "Logistic")) {
    return("LogisticMaturity")
  }

  if (identical(model_role, "Selectivity") && identical(module_type, "Logistic")) {
    return("LogisticSelectivity")
  }

  if (identical(model_role, "Selectivity") && identical(module_type, "DoubleLogistic")) {
    return("DoubleLogisticSelectivity")
  }

  if (identical(model_role, "Selectivity") && identical(module_type, "AgeSpecific")) {
    return("AgeSpecificSelectivity")
  }

  if (identical(model_role, "Recruitment") && identical(module_type, "BevertonHolt")) {
    return("BevertonHoltRecruitment")
  }

  paste0(module_type, model_role)
}

module_type_from_component <- function(model_role, component_name) {
  if (identical(model_role, "Growth") &&
      identical(component_name, "EWAAGrowth")) {
    return("EWAA")
  }

  if (identical(model_role, "Growth") &&
      identical(component_name, "VonBertalanffySchnuteGrowth")) {
    return("VonBertalanffySchnute")
  }

  if (identical(model_role, "Maturity") &&
      identical(component_name, "LogisticMaturity")) {
    return("Logistic")
  }

  if (identical(model_role, "Observation") &&
      identical(component_name, "CatchAtAge")) {
    return("CatchAtAge")
  }

  if (identical(model_role, "Selectivity") &&
      identical(component_name, "LogisticSelectivity")) {
    return("Logistic")
  }

  if (identical(model_role, "Selectivity") &&
      identical(component_name, "DoubleLogisticSelectivity")) {
    return("DoubleLogistic")
  }

  if (identical(model_role, "Selectivity") &&
      identical(component_name, "AgeSpecificSelectivity")) {
    return("AgeSpecific")
  }

  if (identical(model_role, "Recruitment") &&
      identical(component_name, "BevertonHoltRecruitment")) {
    return("BevertonHolt")
  }

  component_name
}

module_option_catalog <- function(model_role) {
  options <- module_options_from_setup_helper(model_role)

  if (length(options) > 0) {
    return(options)
  }

  fallback_module_option_catalog(model_role)
}

module_option_source <- function(model_role) {
  if (length(module_options_from_setup_helper(model_role)) > 0) {
    return("R setup helper formals")
  }

  "prototype fallback"
}

display_module_option <- function(module_type) {
  labels <- c(
    EWAA = "EWAA",
    VonBertalanffySchnute = "VonBertalanffy-Schnute",
    Logistic = "Logistic",
    DoubleLogistic = "Double logistic",
    AgeSpecific = "Age specific",
    BevertonHolt = "Beverton-Holt",
    CatchAtAge = "Catch at age"
  )

  out <- module_type
  matched <- module_type %in% names(labels)
  out[matched] <- labels[module_type[matched]]
  out
}

clean_fleet_names <- function(fleets) {
  fleets <- as.character(fleets)
  fleets <- fleets[!is.na(fleets) & nzchar(fleets)]
  sort(unique(fleets))
}

fleet_count_label <- function(fleets) {
  fleets <- clean_fleet_names(fleets)
  n_fleets <- length(fleets)

  if (n_fleets == 0) {
    return("")
  }

  paste(n_fleets, ifelse(n_fleets == 1, "fleet", "fleets"))
}

fleet_group <- function(label, fleets) {
  fleets <- clean_fleet_names(fleets)
  list(
    label = label,
    fleets = fleets,
    count = length(fleets),
    count_label = fleet_count_label(fleets)
  )
}

fleet_group_fleets <- function(fleet_groups) {
  if (length(fleet_groups) == 0) {
    return(character())
  }

  clean_fleet_names(unlist(
    lapply(fleet_groups, function(group) group[["fleets"]]),
    use.names = FALSE
  ))
}

fleet_group_count_label <- function(fleet_groups) {
  fleet_count_label(fleet_group_fleets(fleet_groups))
}

display_data_type <- function(data_type) {
  labels <- c(
    catch = "Catch",
    index = "Index",
    age_comp = "Age composition",
    length_comp = "Length composition",
    weight_at_age = "Weight at age",
    age_to_length_conversion = "Age-to-length conversion"
  )

  if (data_type %in% names(labels)) {
    return(labels[[data_type]])
  }

  data_type
}

fleet_groups_for_data_type <- function(data_frame, data_type) {
  if (is.null(data_frame) || !"fleet" %in% names(data_frame)) {
    return(list())
  }

  rows <- data_frame[data_frame[["type"]] == data_type, , drop = FALSE]
  fleets <- clean_fleet_names(rows[["fleet"]])

  if (length(fleets) == 0) {
    return(list())
  }

  list(fleet_group("Fleets", fleets))
}

related_composition_links_for_data_type <- function(data_frame, data_type) {
  if (
    is.null(data_frame) ||
      !"type" %in% names(data_frame) ||
      !"fleet" %in% names(data_frame) ||
      !data_type %in% c("catch", "index")
  ) {
    return(list())
  }

  data_fleets <- clean_fleet_names(
    data_frame[["fleet"]][data_frame[["type"]] == data_type]
  )

  if (length(data_fleets) == 0) {
    return(list())
  }

  composition_types <- c(
    age_comp = "Age composition values",
    length_comp = "Length composition values"
  )
  composition_ids <- c(
    age_comp = "data_age_comp",
    length_comp = "data_length_comp"
  )

  links <- lapply(names(composition_types), function(composition_type) {
    composition_fleets <- clean_fleet_names(
      data_frame[["fleet"]][data_frame[["type"]] == composition_type]
    )
    matched_fleets <- intersect(data_fleets, composition_fleets)

    if (length(matched_fleets) == 0) {
      return(NULL)
    }

    list(
      label = composition_types[[composition_type]],
      target_id = composition_ids[[composition_type]],
      target_tab = "values",
      fleets = matched_fleets,
      count_label = fleet_count_label(matched_fleets)
    )
  })

  links[!vapply(links, is.null, logical(1))]
}

collapse_display_values <- function(values, max_values = 8) {
  values <- unique(values[!is.na(values)])

  if (length(values) == 0) {
    return("None")
  }

  display_values <- values[seq_len(min(length(values), max_values))]
  out <- paste(vapply(display_values, format_parameter_value, character(1)), collapse = ", ")

  if (length(values) > max_values) {
    out <- paste0(out, ", +", length(values) - max_values, " more")
  }

  out
}

format_numeric_range <- function(values) {
  values <- values[!is.na(values)]

  if (length(values) == 0) {
    return("not supplied")
  }

  values <- sort(unique(values))

  if (length(values) == 1) {
    return(format_parameter_value(values[[1]]))
  }

  paste0(
    format_parameter_value(min(values)),
    "-",
    format_parameter_value(max(values))
  )
}

as_numeric_values <- function(values) {
  suppressWarnings(as.numeric(as.character(values)))
}

extract_composition_sample_sizes <- function(values) {
  vapply(as.character(values), function(value) {
    direct_value <- suppressWarnings(as.numeric(value))

    if (!is.na(direct_value)) {
      return(direct_value)
    }

    size_match <- regexpr(
      "size\\s*=\\s*[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?",
      value,
      perl = TRUE
    )

    if (size_match < 0) {
      return(NA_real_)
    }

    matched_text <- regmatches(value, size_match)
    suppressWarnings(as.numeric(sub("^size\\s*=\\s*", "", matched_text)))
  }, numeric(1))
}

composition_related_input <- function(data_frame, fleet) {
  input_types <- c(
    catch = "Catch data",
    index = "Index data"
  )

  related <- vapply(names(input_types), function(data_type) {
    rows <- data_frame[
      data_frame[["type"]] == data_type &
        data_frame[["fleet"]] == fleet,
      ,
      drop = FALSE
    ]

    if (nrow(rows) > 0) {
      return(input_types[[data_type]])
    }

    ""
  }, character(1))

  related <- related[nzchar(related)]

  if (length(related) == 0) {
    return("No matching catch or index input")
  }

  paste(related, collapse = ", ")
}

composition_timing_summary <- function(rows) {
  timings <- rows[["timing"]]
  timings <- timings[!is.na(timings)]

  if (length(timings) == 0) {
    return(list(
      year_range = "not supplied",
      observed_years = "not supplied",
      missing_years = "not calculated"
    ))
  }

  numeric_timings <- sort(unique(as_numeric_values(timings)))
  numeric_timings <- numeric_timings[!is.na(numeric_timings)]
  is_integer_timing <- length(numeric_timings) > 0 &&
    all(abs(numeric_timings - round(numeric_timings)) < sqrt(.Machine$double.eps))

  if (is_integer_timing) {
    numeric_timings <- as.integer(round(numeric_timings))
    expected_timings <- seq(min(numeric_timings), max(numeric_timings))
    missing_timings <- setdiff(expected_timings, numeric_timings)

    return(list(
      year_range = format_numeric_range(numeric_timings),
      observed_years = paste0(length(numeric_timings), " of ", length(expected_timings)),
      missing_years = if (length(missing_timings) == 0) {
        "None"
      } else {
        collapse_display_values(missing_timings)
      }
    ))
  }

  character_timings <- sort(unique(as.character(timings)))
  list(
    year_range = collapse_display_values(character_timings, max_values = 2),
    observed_years = paste(length(character_timings), "observed"),
    missing_years = "not calculated"
  )
}

composition_sample_size_summary <- function(rows) {
  if (!"uncertainty" %in% names(rows) || !"timing" %in% names(rows)) {
    return("not supplied")
  }

  sample_sizes_by_timing <- unlist(lapply(split(rows, rows[["timing"]]), function(timing_rows) {
    uncertainty_values <- extract_composition_sample_sizes(timing_rows[["uncertainty"]])
    uncertainty_values <- unique(uncertainty_values[!is.na(uncertainty_values)])

    if (length(uncertainty_values) == 0) {
      return(NA_real_)
    }

    stats::median(uncertainty_values)
  }), use.names = FALSE)

  sample_sizes_by_timing <- sample_sizes_by_timing[!is.na(sample_sizes_by_timing)]

  if (length(sample_sizes_by_timing) == 0) {
    return("not supplied")
  }

  if (isTRUE(all.equal(
    min(sample_sizes_by_timing),
    max(sample_sizes_by_timing),
    tolerance = sqrt(.Machine$double.eps)
  ))) {
    return(format_parameter_value(sample_sizes_by_timing[[1]]))
  }

  paste0(
    format_parameter_value(stats::median(sample_sizes_by_timing)),
    " median, ",
    format_numeric_range(sample_sizes_by_timing),
    " range"
  )
}

composition_bin_summary <- function(rows, bin_column) {
  if (!bin_column %in% names(rows)) {
    return("not supplied")
  }

  observed_values <- as_numeric_values(rows[["observed"]])
  observed_rows <- rows[!is.na(observed_values) & observed_values > 0, , drop = FALSE]

  if (nrow(observed_rows) == 0) {
    observed_rows <- rows
  }

  bin_values <- as_numeric_values(observed_rows[[bin_column]])
  bin_values <- bin_values[!is.na(bin_values)]

  if (length(bin_values) == 0) {
    return("not supplied")
  }

  n_bins <- length(unique(bin_values))
  paste0(format_numeric_range(bin_values), " (", n_bins, ifelse(n_bins == 1, " bin", " bins"), ")")
}

composition_summaries_for_data_type <- function(data_frame, data_type) {
  if (
    is.null(data_frame) ||
      !"type" %in% names(data_frame) ||
      !"fleet" %in% names(data_frame) ||
      !data_type %in% c("age_comp", "length_comp")
  ) {
    return(list())
  }

  rows <- data_frame[data_frame[["type"]] == data_type, , drop = FALSE]
  fleets <- clean_fleet_names(rows[["fleet"]])

  if (length(fleets) == 0) {
    return(list())
  }

  bin_column <- if (identical(data_type, "age_comp")) "age" else "length"
  bin_label <- if (identical(data_type, "age_comp")) "Observed ages" else "Observed lengths"

  lapply(fleets, function(fleet) {
    fleet_rows <- rows[rows[["fleet"]] == fleet, , drop = FALSE]
    timing_summary <- composition_timing_summary(fleet_rows)

    list(
      fleet = fleet,
      related_input = composition_related_input(data_frame, fleet),
      year_range = timing_summary[["year_range"]],
      observed_years = timing_summary[["observed_years"]],
      missing_years = timing_summary[["missing_years"]],
      bin_label = bin_label,
      bin_range = composition_bin_summary(fleet_rows, bin_column),
      sample_size = composition_sample_size_summary(fleet_rows)
    )
  })
}

fleet_groups_for_observation <- function(data_frame) {
  observation_types <- c("catch", "index", "age_comp", "length_comp")

  groups <- lapply(observation_types, function(data_type) {
    rows <- data_frame[data_frame[["type"]] == data_type, , drop = FALSE]
    fleets <- clean_fleet_names(rows[["fleet"]])

    if (length(fleets) == 0) {
      return(NULL)
    }

    fleet_group(display_data_type(data_type), fleets)
  })

  groups[!vapply(groups, is.null, logical(1))]
}

fleet_groups_for_selectivity <- function(parameter_rows) {
  parameter_rows <- ensure_parameter_value_columns(parameter_rows)
  rows <- parameter_rows[
    !is.na(parameter_rows[["fleet"]]) &
      nzchar(as.character(parameter_rows[["fleet"]])) &
      !is.na(parameter_rows[["module_type"]]) &
      nzchar(as.character(parameter_rows[["module_type"]])),
    ,
    drop = FALSE
  ]

  if (nrow(rows) == 0) {
    return(list())
  }

  rows <- unique(rows[, c("fleet", "module_type"), drop = FALSE])
  module_types <- unique(as.character(rows[["module_type"]]))

  lapply(module_types, function(module_type) {
    fleets <- rows[["fleet"]][rows[["module_type"]] == module_type]
    fleet_group(display_module_option(module_type), fleets)
  })
}

empty_module_spec <- function() {
  data.frame(
    id = character(),
    model_role = character(),
    component = character(),
    field_strategy = character(),
    selected_module_type = character(),
    selected_option_label = character(),
    available_options = I(list()),
    available_option_labels = I(list()),
    parameter_values = I(list()),
    fleet_groups = I(list()),
    fleet_count_label = character(),
    stringsAsFactors = FALSE
  )
}

parameter_value_columns <- function() {
  c(
    "module_name",
    "fleet",
    "module_type",
    "label",
    "age",
    "length",
    "timing",
    "value",
    "estimation_type",
    "distribution_type",
    "distribution"
  )
}

empty_parameter_value_rows <- function() {
  data.frame(
    module_name = character(),
    fleet = character(),
    module_type = character(),
    label = character(),
    age = numeric(),
    length = numeric(),
    timing = integer(),
    value = numeric(),
    estimation_type = character(),
    distribution_type = character(),
    distribution = character(),
    stringsAsFactors = FALSE
  )
}

ensure_parameter_value_columns <- function(parameter_rows) {
  if (is.null(parameter_rows) || nrow(parameter_rows) == 0) {
    return(empty_parameter_value_rows())
  }

  parameter_rows <- as.data.frame(parameter_rows, stringsAsFactors = FALSE)

  for (column_name in parameter_value_columns()) {
    if (!column_name %in% names(parameter_rows)) {
      parameter_rows[[column_name]] <- NA
    }
  }

  parameter_rows[, parameter_value_columns(), drop = FALSE]
}

safe_default_parameter_call <- function(default_expression) {
  tryCatch(
    ensure_parameter_value_columns(default_expression),
    error = function(error) empty_parameter_value_rows()
  )
}

default_parameters_for_module <- function(model_role, module_type, data, current_rows) {
  if (!inherits(data, "FIMSFrame")) {
    return(empty_parameter_value_rows())
  }

  if (identical(model_role, "Growth")) {
    setup_default_growth <- find_fims_helper("setup_default_Growth")

    if (is.null(setup_default_growth)) {
      return(empty_parameter_value_rows())
    }

    return(safe_default_parameter_call(
      setup_default_growth(data = data, module_type = module_type)
    ))
  }

  if (identical(model_role, "Maturity")) {
    setup_default_maturity <- find_fims_helper("setup_default_Maturity")

    if (is.null(setup_default_maturity)) {
      return(empty_parameter_value_rows())
    }

    return(safe_default_parameter_call(
      setup_default_maturity(data = data, module_type = module_type)
    ))
  }

  if (identical(model_role, "Selectivity")) {
    setup_default_selectivity <- find_fims_helper("setup_default_Selectivity")
    get_fleets <- find_fims_helper("get_fleets")

    if (is.null(setup_default_selectivity)) {
      return(empty_parameter_value_rows())
    }

    fleets <- unique(stats::na.omit(as.character(current_rows[["fleet"]])))

    if (length(fleets) == 0 && !is.null(get_fleets)) {
      fleets <- get_fleets(data)
    }

    if (length(fleets) == 0) {
      return(empty_parameter_value_rows())
    }

    defaults <- lapply(fleets, function(fleet_i) {
      fleet_module_type <- unique(stats::na.omit(as.character(
        current_rows[["module_type"]][current_rows[["fleet"]] == fleet_i]
      )))
      fleet_module_type <- fleet_module_type[nzchar(fleet_module_type)]
      fleet_module_type <- if (length(fleet_module_type) == 0) {
        module_type
      } else {
        fleet_module_type[[1]]
      }

      safe_default_parameter_call(
        setup_default_selectivity(
          data = data,
          fleet = fleet_i,
          module_type = fleet_module_type
        )
      )
    })

    return(do.call(rbind, defaults))
  }

  if (identical(model_role, "Recruitment")) {
    setup_default_recruitment <- find_fims_helper("setup_default_Recruitment")

    if (is.null(setup_default_recruitment)) {
      return(empty_parameter_value_rows())
    }

    distribution <- unique(stats::na.omit(as.character(current_rows[["distribution"]])))
    distribution <- if (length(distribution) == 0) "Dnorm" else distribution[[1]]

    return(safe_default_parameter_call(
      setup_default_recruitment(
        data = data,
        module_type = module_type,
        distribution = distribution
      )
    ))
  }

  empty_parameter_value_rows()
}

parameter_compare_key <- function(parameter_rows) {
  parameter_rows <- ensure_parameter_value_columns(parameter_rows)
  key_columns <- c(
    "label",
    "fleet",
    "age",
    "length",
    "timing",
    "distribution_type",
    "distribution"
  )

  do.call(
    paste,
    c(
      lapply(key_columns, function(column_name) {
        values <- as.character(parameter_rows[[column_name]])
        values[is.na(values)] <- ""
        values
      }),
      sep = "\r"
    )
  )
}

format_parameter_value <- function(value) {
  if (length(value) == 0 || is.na(value)) {
    return("not set")
  }

  if (is.numeric(value)) {
    value <- signif(value, 6)

    if (!identical(value, 0) && (abs(value) < 0.001 || abs(value) >= 1e6)) {
      return(format(value, scientific = TRUE, trim = TRUE))
    }

    return(format(value, scientific = FALSE, trim = TRUE))
  }

  as.character(value)
}

format_parameter_context <- function(parameter_row) {
  context_columns <- c("fleet", "age", "length", "timing", "distribution")
  context <- vapply(context_columns, function(column_name) {
    value <- parameter_row[[column_name]]

    if (length(value) == 0 || is.na(value) || identical(as.character(value), "")) {
      return("")
    }

    paste(column_name, format_parameter_value(value))
  }, character(1))

  context <- context[nzchar(context)]

  if (length(context) == 0) {
    return("")
  }

  paste(context, collapse = "; ")
}

parameter_value_matches <- function(value_a, value_b) {
  if ((length(value_a) == 0 || is.na(value_a)) &&
      (length(value_b) == 0 || is.na(value_b))) {
    return(TRUE)
  }

  if (is.numeric(value_a) && is.numeric(value_b)) {
    return(isTRUE(all.equal(value_a, value_b, tolerance = sqrt(.Machine$double.eps))))
  }

  identical(as.character(value_a), as.character(value_b))
}

compare_module_parameter_values <- function(current_rows, default_rows) {
  current_rows <- ensure_parameter_value_columns(current_rows)
  default_rows <- ensure_parameter_value_columns(default_rows)

  current_rows <- current_rows[
    !is.na(current_rows[["label"]]) & nzchar(current_rows[["label"]]),
    ,
    drop = FALSE
  ]

  default_rows <- default_rows[
    !is.na(default_rows[["label"]]) & nzchar(default_rows[["label"]]),
    ,
    drop = FALSE
  ]

  if (nrow(current_rows) == 0) {
    return(list())
  }

  default_keys <- parameter_compare_key(default_rows)
  current_keys <- parameter_compare_key(current_rows)

  lapply(seq_len(nrow(current_rows)), function(row_index) {
    current_row <- current_rows[row_index, , drop = FALSE]
    default_index <- match(current_keys[[row_index]], default_keys)
    has_default <- !is.na(default_index)
    default_row <- if (has_default) {
      default_rows[default_index, , drop = FALSE]
    } else {
      empty_parameter_value_rows()
    }
    status <- if (!has_default) {
      "not in defaults"
    } else if (parameter_value_matches(
      current_row[["value"]][[1]],
      default_row[["value"]][[1]]
    )) {
      "default"
    } else {
      "changed"
    }

    list(
      parameter = current_row[["label"]][[1]],
      context = format_parameter_context(current_row),
      user_value = format_parameter_value(current_row[["value"]][[1]]),
      default_value = if (has_default) {
        format_parameter_value(default_row[["value"]][[1]])
      } else {
        "not available"
      },
      status = status,
      estimation_type = format_parameter_value(current_row[["estimation_type"]][[1]])
    )
  })
}

make_module_spec_row <- function(
    id,
    model_role,
    component,
    field_strategy,
    selected_module_type,
    selected_option_label = display_module_option(selected_module_type),
    parameter_values = list(),
    fleet_groups = list()) {
  available_options <- module_option_catalog(model_role)
  row <- data.frame(
    id = id,
    model_role = model_role,
    component = component,
    field_strategy = field_strategy,
    selected_module_type = selected_module_type,
    selected_option_label = selected_option_label,
    stringsAsFactors = FALSE
  )
  row[["available_options"]] <- I(list(available_options))
  row[["available_option_labels"]] <- I(list(display_module_option(available_options)))
  row[["parameter_values"]] <- I(list(parameter_values))
  row[["fleet_groups"]] <- I(list(fleet_groups))
  row[["fleet_count_label"]] <- fleet_group_count_label(fleet_groups)
  row
}

registration_for_component <- function(component_name) {
  registration_row <- rcpp_registration[
    match(component_name, rcpp_registration$r_name),
  ]

  if (nrow(registration_row) == 0 || is.na(registration_row$r_name)) {
    return(NULL)
  }

  registration_row
}

module_metadata_for_option <- function(model_role, module_type) {
  component_name <- component_from_parameter_type(model_role, module_type)
  registration_row <- registration_for_component(component_name)

  if (is.null(registration_row)) {
    fields <- character()
    methods <- character()
    source_file <- ""
    interface <- ""
    url <- ""
  } else {
    fields <- registration_row$fields[[1]]
    methods <- registration_row$methods[[1]]
    source_file <- registration_row$source_file[[1]]
    interface <- registration_row$cpp_interface[[1]]
    url <- ifelse(is.na(registration_row$url[[1]]), "", registration_row$url[[1]])
  }

  field_notes <- character()
  inactive_fields <- character()
  active_fields <- fields

  if (identical(component_name, "VonBertalanffySchnuteGrowth")) {
    growth_fields <- format_growth_fields(fields)
    active_fields <- growth_fields$active_fields
    field_notes <- growth_fields$field_notes
    inactive_fields <- growth_fields$inactive_fields
  }

  field_definitions <- get_field_definitions(
    component_name = component_name,
    interface = interface,
    fields = c(active_fields, inactive_fields)
  )
  field_definition_rows <- lapply(names(field_definitions), function(field_name) {
    list(
      name = field_name,
      definition = unname(field_definitions[[field_name]])
    )
  })
  equation_rows <- get_equation_rows_for_component(component_name)
  equations <- make_readable_equation(equation_rows$equation)

  list(
    module_type = module_type,
    label = display_module_option(module_type),
    rcpp_class = component_name,
    cpp_interface = interface,
    source_file = source_file,
    doxygen_url = url,
    fields = active_fields,
    inactive_fields = inactive_fields,
    field_notes = field_notes,
    field_definitions = field_definition_rows,
    methods = methods,
    equations = equations,
    assumptions = format_assumptions_for_component(component_name)
  )
}

build_inspector_metadata_catalog <- function() {
  model_roles <- c("Growth", "Maturity", "Observation", "Selectivity", "Recruitment")
  modules <- lapply(model_roles, function(model_role) {
    module_options <- module_option_catalog(model_role)

    list(
      model_role = model_role,
      setup_helper = setup_helper_for_model_role(model_role),
      valid_module_options = module_options,
      valid_module_option_labels = display_module_option(module_options),
      option_source = module_option_source(model_role),
      options = lapply(
        module_options,
        function(module_type) module_metadata_for_option(model_role, module_type)
      )
    )
  })

  list(
    schema_version = "0.1.0",
    generated_at = format(Sys.time(), "%Y-%m-%dT%H:%M:%S%z"),
    generated_by = "user-model_inspector_prototype.r",
    note = paste(
      "Prototype metadata catalog generated from FIMS R setup helpers,",
      "Rcpp registrations, Doxygen-style member comments, equation extraction,",
      "and tagged assumptions where available."
    ),
    sources = list(
      setup_helpers = "R/setup_default_parameters.R",
      rcpp_registration = "src/rcpp_*.cpp",
      doxygen_member_docs = "inst/include/**/*.hpp",
      equations = equation_catalog_path,
      assumptions = "Doxygen @assumption tags plus prototype discussion examples"
    ),
    modules = modules
  )
}

write_inspector_metadata_catalog <- function(
    output = file.path(
      "visualization-prototypes",
      "data",
      "fims_inspector_metadata_catalog.json"
    )) {
  output_dir <- dirname(output)
  dir.create(output_dir, showWarnings = FALSE, recursive = TRUE)
  catalog <- build_inspector_metadata_catalog()

  writeLines(
    jsonlite::toJSON(catalog, auto_unbox = TRUE, pretty = TRUE, null = "null"),
    output,
    useBytes = TRUE
  )

  normalizePath(output)
}

build_input_spec <- function(data, module_spec = NULL) {
  data_frame <- extract_fims_data_frame(data)
  input_columns <- c(
    "id",
    "label",
    "subtitle",
    "description",
    "assumptions",
    "fleet_groups",
    "fleet_count_label",
    "related_value_links",
    "composition_summaries",
    "target",
    "edge_label"
  )

  if (is.null(data_frame) || !"type" %in% names(data_frame)) {
    return(empty_spec_table(input_columns))
  }

  growth_component <- ""

  if (!is.null(module_spec) && nrow(module_spec) > 0) {
    growth_component <- module_spec$component[
      module_spec$model_role == "Growth"
    ]
    growth_component <- growth_component[!is.na(growth_component)]

    if (length(growth_component) == 0) {
      growth_component <- ""
    } else {
      growth_component <- growth_component[[1]]
    }
  }

  length_comp_target <- if (identical(growth_component, "VonBertalanffySchnuteGrowth")) {
    module_id("Growth")
  } else {
    module_id("Observation")
  }

  input_catalog <- data.frame(
    data_type = c(
      "catch",
      "age_comp",
      "length_comp",
      "index",
      "weight_at_age",
      "age_to_length_conversion"
    ),
    id = c(
      "data_catch",
      "data_age_comp",
      "data_length_comp",
      "data_index",
      "data_weight_at_age",
      "data_age_to_length_conversion"
    ),
    label = c(
      "Catch data",
      "Age composition",
      "Length composition",
      "Index data",
      "Weight-at-age data",
      "Age-to-length conversion"
    ),
    target = c(
      module_id("Observation"),
      module_id("Observation"),
      length_comp_target,
      module_id("Selectivity"),
      module_id("Growth"),
      module_id("Growth")
    ),
    edge_label = c(
      "observed catch",
      "composition",
      "length data",
      "survey index",
      "empirical weight",
      "conversion"
    ),
    stringsAsFactors = FALSE
  )

  available_types <- unique(as.character(data_frame$type))
  input_catalog <- input_catalog[input_catalog$data_type %in% available_types, ]

  if (identical(growth_component, "VonBertalanffySchnuteGrowth")) {
    input_catalog <- input_catalog[
      !input_catalog$data_type %in% c("weight_at_age", "age_to_length_conversion"),
      ,
      drop = FALSE
    ]
  }

  if (nrow(input_catalog) == 0) {
    return(empty_spec_table(input_columns))
  }

  fleet_groups <- lapply(
    input_catalog$data_type,
    function(data_type) fleet_groups_for_data_type(data_frame, data_type)
  )
  related_value_links <- lapply(
    input_catalog$data_type,
    function(data_type) related_composition_links_for_data_type(data_frame, data_type)
  )
  composition_summaries <- lapply(
    input_catalog$data_type,
    function(data_type) composition_summaries_for_data_type(data_frame, data_type)
  )
  fleet_count_labels <- vapply(fleet_groups, fleet_group_count_label, character(1))
  subtitles <- ifelse(
    nzchar(fleet_count_labels),
    fleet_count_labels,
    "Observed data stream"
  )

  input_rows <- data.frame(
    id = input_catalog$id,
    label = input_catalog$label,
    subtitle = subtitles,
    description = paste(
      input_catalog$label,
      "provided by the user and routed into the configured model."
    ),
    assumptions = "Input validation and dimensional checks depend on the FIMSFrame setup.",
    target = input_catalog$target,
    edge_label = input_catalog$edge_label,
    stringsAsFactors = FALSE
  )
  input_rows[["fleet_groups"]] <- I(fleet_groups)
  input_rows[["fleet_count_label"]] <- fleet_count_labels
  input_rows[["related_value_links"]] <- I(related_value_links)
  input_rows[["composition_summaries"]] <- I(composition_summaries)
  input_rows
}

build_module_spec <- function(parameters, data) {
  if (is.null(parameters)) {
    return(empty_module_spec())
  }

  required_columns <- c("module_name", "module_type")
  missing_columns <- setdiff(required_columns, names(parameters))

  if (length(missing_columns) > 0) {
    stop(
      "The `parameters` table must include: ",
      paste(missing_columns, collapse = ", "),
      "."
    )
  }

  data_frame <- extract_fims_data_frame(data)
  model_roles <- c("Growth", "Maturity", "Observation", "Selectivity", "Recruitment")
  module_rows <- list()

  for (model_role in model_roles) {
    if (identical(model_role, "Observation")) {
      has_observation_data <- any(vapply(
        c("catch", "age_comp", "length_comp", "index"),
        function(data_type) has_data_type(data_frame, data_type),
        logical(1)
      ))

      if (has_observation_data) {
        module_rows[[length(module_rows) + 1]] <- make_module_spec_row(
          id = module_id(model_role),
          model_role = model_role,
          component = "CatchAtAge",
          field_strategy = "",
          selected_module_type = "CatchAtAge",
          fleet_groups = fleet_groups_for_observation(data_frame)
        )
      }

      next
    }

    rows <- parameters[parameters$module_name %in% model_role, ]

    if (nrow(rows) == 0) {
      next
    }

    component <- component_from_parameter_type(model_role, rows$module_type)
    selected_module_type <- module_type_from_component(model_role, component)

    if (component == "") {
      next
    }

    field_strategy <- if (identical(component, "VonBertalanffySchnuteGrowth")) {
      "growth_linear_interpolation"
    } else {
      ""
    }

    default_rows <- default_parameters_for_module(
      model_role = model_role,
      module_type = selected_module_type,
      data = data,
      current_rows = rows
    )
    parameter_values <- compare_module_parameter_values(
      current_rows = rows,
      default_rows = default_rows
    )
    fleet_groups <- if (identical(model_role, "Selectivity")) {
      fleet_groups_for_selectivity(rows)
    } else {
      list()
    }
    selected_option_label <- if (identical(model_role, "Selectivity") &&
                                 length(fleet_groups) > 1) {
      "Varies by fleet"
    } else {
      display_module_option(selected_module_type)
    }

    module_rows[[length(module_rows) + 1]] <- make_module_spec_row(
      id = module_id(model_role),
      model_role = model_role,
      component = component,
      field_strategy = field_strategy,
      selected_module_type = selected_module_type,
      selected_option_label = selected_option_label,
      parameter_values = parameter_values,
      fleet_groups = fleet_groups
    )
  }

  if (length(module_rows) == 0) {
    return(empty_module_spec())
  }

  do.call(rbind, module_rows)
}

build_derived_spec <- function(module_spec) {
  module_ids <- module_spec$id

  if (!all(module_id(c("Growth", "Maturity", "Recruitment")) %in% module_ids)) {
    return(empty_spec_table(c("id", "label", "subtitle", "description", "assumptions")))
  }

  data.frame(
    id = "derived_spawning_biomass",
    label = "Spawning biomass",
    subtitle = "Derived quantity",
    description = "A model-derived quantity connecting growth, maturity, and recruitment.",
    assumptions = "Derived from selected biological processes and model state.",
    stringsAsFactors = FALSE
  )
}

build_output_spec <- function(module_spec, derived_spec, data) {
  data_frame <- extract_fims_data_frame(data)
  module_ids <- module_spec$id
  output_rows <- list()

  add_output <- function(id, label, description) {
    output_rows[[length(output_rows) + 1]] <<- data.frame(
      id = id,
      label = label,
      subtitle = "Model-derived result",
      description = description,
      assumptions = "Output interpretation depends on the selected module configuration.",
      stringsAsFactors = FALSE
    )
  }

  if (module_id("Observation") %in% module_ids && has_data_type(data_frame, "catch")) {
    add_output("output_expected_catch", "Expected catch", "Expected catch is produced by the observation model.")
  }

  if (module_id("Observation") %in% module_ids) {
    add_output("output_total_nll", "Total NLL", "Total NLL collects likelihood contributions for the configured model.")
  }

  if (module_id("Growth") %in% module_ids) {
    add_output("output_growth_curve", "Growth curve", "The growth curve is produced by the selected growth component.")
  }

  if (module_id("Selectivity") %in% module_ids && has_data_type(data_frame, "index")) {
    add_output("output_expected_index", "Expected index", "Expected index is produced from selectivity and survey-index structure.")
  }

  if (nrow(derived_spec) > 0) {
    add_output("output_spawning_biomass", "Spawning biomass", "Spawning biomass is a derived model quantity used by recruitment.")
  }

  if (module_id("Recruitment") %in% module_ids) {
    add_output("output_expected_recruits", "Expected recruits", "Expected recruits are produced by the selected recruitment relationship.")
  }

  if (length(output_rows) == 0) {
    return(empty_spec_table(c("id", "label", "subtitle", "description", "assumptions")))
  }

  do.call(rbind, output_rows)
}

build_connection_spec <- function(input_spec, module_spec, derived_spec, output_spec) {
  connection_rows <- list()
  node_ids <- c(input_spec$id, module_spec$id, derived_spec$id, output_spec$id)

  add_connection <- function(from, to, label, kind) {
    if (!from %in% node_ids || !to %in% node_ids) {
      return(invisible(NULL))
    }

    connection_rows[[length(connection_rows) + 1]] <<- data.frame(
      from = from,
      to = to,
      label = label,
      kind = kind,
      stringsAsFactors = FALSE
    )
  }

  if (nrow(input_spec) > 0) {
    for (i in seq_len(nrow(input_spec))) {
      add_connection(
        from = input_spec$id[[i]],
        to = input_spec$target[[i]],
        label = input_spec$edge_label[[i]],
        kind = "data"
      )
    }
  }

  add_connection(module_id("Growth"), "derived_spawning_biomass", "length/weight", "process")
  add_connection(module_id("Maturity"), "derived_spawning_biomass", "maturity", "process")
  add_connection("derived_spawning_biomass", module_id("Recruitment"), "stock-recruit", "process")
  add_connection(module_id("Growth"), module_id("Observation"), "age-length mapping", "process")
  add_connection(module_id("Selectivity"), module_id("Observation"), "selectivity", "process")

  add_connection(module_id("Observation"), "output_expected_catch", "prediction", "output")
  add_connection(module_id("Observation"), "output_total_nll", "likelihood", "output")
  add_connection(module_id("Growth"), "output_growth_curve", "growth output", "output")
  add_connection(module_id("Selectivity"), "output_expected_index", "prediction", "output")
  add_connection("derived_spawning_biomass", "output_spawning_biomass", "derived quantity", "output")
  add_connection(module_id("Recruitment"), "output_expected_recruits", "stock-recruit", "output")

  if (length(connection_rows) == 0) {
    return(empty_spec_table(c("from", "to", "label", "kind")))
  }

  do.call(rbind, connection_rows)
}

build_model_graph <- function(
    data,
    parameters = NULL,
    setup_default_if_missing = TRUE) {
  model_inputs <- prepare_model_inspector_inputs(
    data = data,
    parameters = parameters,
    setup_default_if_missing = setup_default_if_missing
  )
  data <- model_inputs$data
  parameters <- model_inputs$parameters

  module_spec <- build_module_spec(parameters = parameters, data = data)
  input_spec <- build_input_spec(data, module_spec = module_spec)
  derived_spec <- build_derived_spec(module_spec)
  output_spec <- build_output_spec(
    module_spec = module_spec,
    derived_spec = derived_spec,
    data = data
  )
  connection_spec <- build_connection_spec(
    input_spec = input_spec,
    module_spec = module_spec,
    derived_spec = derived_spec,
    output_spec = output_spec
  )

  list(
    metadata = list(
      source = "parameters + data",
      exact_runtime_ids = FALSE,
      note = paste(
        "Prototype graph inferred before initialize_fims();",
        "module IDs are display IDs, not runtime FIMS IDs."
      )
    ),
    inputs = input_spec,
    modules = module_spec,
    derived = derived_spec,
    outputs = output_spec,
    connections = connection_spec
  )
}

build_model_inspector_spec_from_graph <- function(
    model_graph,
    title = "FIMS User Model Explorer Prototype",
    model_label = "User model") {
  required_graph_elements <- c(
    "inputs",
    "modules",
    "derived",
    "outputs",
    "connections"
  )
  missing_graph_elements <- setdiff(required_graph_elements, names(model_graph))

  if (length(missing_graph_elements) > 0) {
    stop(
      "`model_graph` must include: ",
      paste(missing_graph_elements, collapse = ", "),
      "."
    )
  }

  input_spec <- model_graph$inputs
  module_spec <- model_graph$modules
  derived_spec <- model_graph$derived
  output_spec <- model_graph$outputs
  connection_spec <- model_graph$connections

  initial_focus_id <- if (module_id("Growth") %in% module_spec$id) {
    module_id("Growth")
  } else if (nrow(module_spec) > 0) {
    module_spec$id[[1]]
  } else if (nrow(input_spec) > 0) {
    input_spec$id[[1]]
  } else {
    ""
  }

  list(
    metadata = c(
      list(
        title = title,
        model_label = model_label,
        initial_focus_id = initial_focus_id
      ),
      model_graph$metadata
    ),
    inputs = input_spec[, setdiff(names(input_spec), c("target", "edge_label"))],
    modules = module_spec,
    derived = derived_spec,
    outputs = output_spec,
    connections = connection_spec
  )
}

build_model_inspector_spec <- function(
    parameters = NULL,
    data = NULL,
    title = "FIMS User Model Explorer Prototype",
    model_label = "User model",
    setup_default_if_missing = TRUE) {
  if (is.null(parameters) && is.null(data)) {
    return(example_model_spec)
  }

  if (is.null(data)) {
    stop("Provide `data`, or provide neither `parameters` nor `data` to use the example.")
  }

  model_graph <- build_model_graph(
    data = data,
    parameters = parameters,
    setup_default_if_missing = setup_default_if_missing
  )

  build_model_inspector_spec_from_graph(
    model_graph = model_graph,
    title = title,
    model_label = model_label
  )
}

example_model_spec <- list(
  metadata = list(
    title = "FIMS User Model Explorer Prototype",
    model_label = "Example model",
    initial_focus_id = module_id("Growth")
  ),
  inputs = data.frame(
    id = c(
      "data_catch",
      "data_age_comp",
      "data_length_comp",
      "data_index"
    ),
    label = c(
      "Catch data",
      "Age composition",
      "Length composition",
      "Index data"
    ),
    subtitle = rep("Observed data stream", 4),
    description = c(
      "Catch data provided by the user and routed into the observation model.",
      "Age-composition data provided by the user and routed into the observation model.",
      "Length-composition data provided by the user and routed into the growth model.",
      "Index data provided by the user and routed into the selectivity model."
    ),
    assumptions = rep(
      "Input validation and dimensional checks depend on the FIMSFrame setup.",
      4
    ),
    stringsAsFactors = FALSE
  ),
  modules = data.frame(
    id = module_id(c(
      "Growth",
      "Maturity",
      "Observation",
      "Selectivity",
      "Recruitment"
    )),
    model_role = c(
      "Growth",
      "Maturity",
      "Observation",
      "Selectivity",
      "Recruitment"
    ),
    component = c(
      "VonBertalanffySchnuteGrowth",
      "LogisticMaturity",
      "CatchAtAge",
      "LogisticSelectivity",
      "BevertonHoltRecruitment"
    ),
    field_strategy = c(
      "growth_linear_interpolation",
      "",
      "",
      "",
      ""
    ),
    stringsAsFactors = FALSE
  ),
  derived = data.frame(
    id = "derived_spawning_biomass",
    label = "Spawning biomass",
    subtitle = "Derived quantity",
    description = "A model-derived quantity connecting growth, maturity, and recruitment.",
    assumptions = "Derived from selected biological processes and model state.",
    stringsAsFactors = FALSE
  ),
  outputs = data.frame(
    id = c(
      "output_expected_catch",
      "output_total_nll",
      "output_growth_curve",
      "output_expected_index",
      "output_spawning_biomass",
      "output_expected_recruits"
    ),
    label = c(
      "Expected catch",
      "Total NLL",
      "Growth curve",
      "Expected index",
      "Spawning biomass",
      "Expected recruits"
    ),
    subtitle = rep("Model-derived result", 6),
    description = c(
      "Expected catch is produced by the observation model.",
      "Total NLL collects likelihood contributions for the configured model.",
      "The growth curve is produced by the selected growth component.",
      "Expected index is produced from selectivity and survey-index structure.",
      "Spawning biomass is a derived model quantity used by recruitment.",
      "Expected recruits are produced by the selected recruitment relationship."
    ),
    assumptions = rep(
      "Output interpretation depends on the selected module configuration.",
      6
    ),
    stringsAsFactors = FALSE
  ),
  connections = data.frame(
    from = c(
      "data_catch",
      "data_age_comp",
      "data_length_comp",
      "data_index",
      module_id("Growth"),
      module_id("Maturity"),
      "derived_spawning_biomass",
      module_id("Growth"),
      module_id("Selectivity"),
      module_id("Observation"),
      module_id("Observation"),
      module_id("Growth"),
      module_id("Selectivity"),
      "derived_spawning_biomass",
      module_id("Recruitment")
    ),
    to = c(
      module_id("Observation"),
      module_id("Observation"),
      module_id("Growth"),
      module_id("Selectivity"),
      "derived_spawning_biomass",
      "derived_spawning_biomass",
      module_id("Recruitment"),
      module_id("Observation"),
      module_id("Observation"),
      "output_expected_catch",
      "output_total_nll",
      "output_growth_curve",
      "output_expected_index",
      "output_spawning_biomass",
      "output_expected_recruits"
    ),
    label = c(
      "observed catch",
      "composition",
      "length data",
      "survey index",
      "length/weight",
      "maturity",
      "stock-recruit",
      "age-length mapping",
      "selectivity",
      "prediction",
      "likelihood",
      "growth output",
      "prediction",
      "derived quantity",
      "stock-recruit"
    ),
    kind = c(
      rep("data", 4),
      rep("process", 5),
      rep("output", 6)
    ),
    stringsAsFactors = FALSE
  )
)

# -------------------------------------------------------------------------
# 5. Build node and edge data
# -------------------------------------------------------------------------

make_input_node <- function(input_row) {
  fleet_groups <- module_row_list_value(
    module_row = input_row,
    column_name = "fleet_groups",
    default = list()
  )
  fleet_count_label_value <- module_row_list_value(
    module_row = input_row,
    column_name = "fleet_count_label",
    default = fleet_group_count_label(fleet_groups)
  )
  related_value_links <- module_row_list_value(
    module_row = input_row,
    column_name = "related_value_links",
    default = list()
  )
  composition_summaries <- module_row_list_value(
    module_row = input_row,
    column_name = "composition_summaries",
    default = list()
  )

  list(
    id = input_row$id,
    stage = "inputs",
    kind = "input",
    role = "Input data",
    label = input_row$label,
    subtitle = input_row$subtitle,
    description = input_row$description,
    badges = c("input"),
    fields = character(),
    field_definition_labels = character(),
    field_definition_text = character(),
    field_notes = character(),
    inactive_fields = character(),
    selected_module_type = "",
    selected_option_label = "",
    available_options = character(),
    available_option_labels = character(),
    parameter_values = list(),
    fleet_groups = fleet_groups,
    fleet_count_label = fleet_count_label_value,
    related_value_links = related_value_links,
    composition_summaries = composition_summaries,
    methods = character(),
    equations = character(),
    assumptions = input_row$assumptions,
    url = "",
    source = "",
    component = input_row$label,
    interface = "",
    developer = list(
      metadata_type = "input_data",
      source = "FIMSFrame data table",
      note = "Input nodes are inferred from the data `type` and `fleet` columns."
    )
  )
}

make_output_node <- function(output_row) {
  list(
    id = output_row$id,
    stage = "outputs",
    kind = "output",
    role = "Expected output",
    label = output_row$label,
    subtitle = output_row$subtitle,
    description = output_row$description,
    badges = c("output"),
    fields = character(),
    field_definition_labels = character(),
    field_definition_text = character(),
    field_notes = character(),
    inactive_fields = character(),
    selected_module_type = "",
    selected_option_label = "",
    available_options = character(),
    available_option_labels = character(),
    parameter_values = list(),
    fleet_groups = list(),
    fleet_count_label = "",
    methods = character(),
    equations = character(),
    assumptions = output_row$assumptions,
    url = "",
    source = "",
    component = output_row$label,
    interface = "",
    developer = list(
      metadata_type = "model_output",
      source = "prototype output rules",
      note = "Output nodes are inferred from configured modules and available data streams."
    )
  )
}

format_growth_fields <- function(fields) {
  active_fields <- c(
    "mean_length_young",
    "mean_length_old",
    "growth_coefficient",
    "reference_age_for_length_young",
    "reference_age_for_length_old",
    "length_weight_a",
    "length_weight_b",
    "length_at_age_sd_at_reference_ages",
    "n_ages"
  )
  inactive_delta_fields <- c(
    "log_sd_mean_length_young",
    "log_sd_mean_length_old",
    "log_sd_growth_coefficient",
    "mean_length_young_mean_length_old_logit_corr",
    "mean_length_young_growth_coefficient_logit_corr",
    "mean_length_old_growth_coefficient_logit_corr"
  )

  list(
    active_fields = intersect(active_fields, fields),
    field_notes = character(),
    inactive_fields = intersect(inactive_delta_fields, fields)
  )
}

module_row_list_value <- function(module_row, column_name, default) {
  if (!column_name %in% names(module_row)) {
    return(default)
  }

  value <- module_row[[column_name]]

  if (length(value) == 0) {
    return(default)
  }

  if (is.list(value) && !is.data.frame(value)) {
    return(value[[1]])
  }

  value
}

make_module_node <- function(module_row) {
  model_role <- module_row$model_role
  component_name <- module_row$component
  field_strategy <- if ("field_strategy" %in% names(module_row) &&
                        !is.na(module_row$field_strategy)) {
    module_row$field_strategy
  } else {
    ""
  }
  selected_module_type <- module_row_list_value(
    module_row = module_row,
    column_name = "selected_module_type",
    default = module_type_from_component(model_role, component_name)
  )
  selected_option_label <- module_row_list_value(
    module_row = module_row,
    column_name = "selected_option_label",
    default = display_module_option(selected_module_type)
  )
  available_options <- module_row_list_value(
    module_row = module_row,
    column_name = "available_options",
    default = module_option_catalog(model_role)
  )
  available_option_labels <- module_row_list_value(
    module_row = module_row,
    column_name = "available_option_labels",
    default = display_module_option(available_options)
  )
  parameter_values <- module_row_list_value(
    module_row = module_row,
    column_name = "parameter_values",
    default = list()
  )
  fleet_groups <- module_row_list_value(
    module_row = module_row,
    column_name = "fleet_groups",
    default = list()
  )
  fleet_count_label_value <- module_row_list_value(
    module_row = module_row,
    column_name = "fleet_count_label",
    default = fleet_group_count_label(fleet_groups)
  )
  registration_row <- rcpp_registration[
    match(component_name, rcpp_registration$r_name),
  ]

  if (nrow(registration_row) == 0 || is.na(registration_row$r_name)) {
    fields <- character()
    methods <- character()
    source_file <- ""
    interface <- ""
    url <- ""
  } else {
    fields <- registration_row$fields[[1]]
    methods <- registration_row$methods[[1]]
    source_file <- registration_row$source_file[[1]]
    interface <- registration_row$cpp_interface[[1]]
    url <- ifelse(is.na(registration_row$url[[1]]), "", registration_row$url[[1]])
  }

  equation_rows <- get_equation_rows_for_component(component_name)
  equations <- make_readable_equation(equation_rows$equation)
  assumptions <- format_assumptions_for_component(component_name)
  field_notes <- character()
  inactive_fields <- character()

  if (identical(field_strategy, "growth_linear_interpolation")) {
    growth_fields <- format_growth_fields(fields)
    fields <- growth_fields$active_fields
    field_notes <- growth_fields$field_notes
    inactive_fields <- growth_fields$inactive_fields
  }

  field_definitions <- get_field_definitions(
    component_name = component_name,
    interface = interface,
    fields = c(fields, inactive_fields)
  )

  badges <- c("module", "docs")

  if (length(equations) > 0) {
    badges <- c(badges, "equations")
  }

  if (length(assumptions) > 0) {
    badges <- c(badges, "assumptions")
  }

  list(
    id = paste0("module_", model_role),
    stage = "modules",
    kind = "module",
    role = paste(model_role, "module"),
    label = model_role,
    subtitle = if (model_role %in% c("Observation", "Selectivity") &&
                   nzchar(fleet_count_label_value)) {
      fleet_count_label_value
    } else {
      component_name
    },
    description = paste0(
    "The selected ",
      tolower(model_role),
      " component for this example model. It is exposed to R as ",
      component_name,
      " and registered through the Rcpp interface."
    ),
    badges = badges,
    fields = coalesce_empty(fields, character()),
    field_definition_labels = names(field_definitions),
    field_definition_text = unname(field_definitions),
    field_notes = coalesce_empty(field_notes, character()),
    inactive_fields = coalesce_empty(inactive_fields, character()),
    selected_module_type = selected_module_type,
    selected_option_label = selected_option_label,
    available_options = coalesce_empty(available_options, character()),
    available_option_labels = coalesce_empty(available_option_labels, character()),
    parameter_values = parameter_values,
    fleet_groups = fleet_groups,
    fleet_count_label = fleet_count_label_value,
    methods = coalesce_empty(methods, character()),
    equations = coalesce_empty(equations, character()),
    assumptions = coalesce_empty(assumptions, character()),
    url = url,
    source = source_file,
    component = component_name,
    interface = interface,
    developer = list(
      metadata_type = "configured_module",
      setup_helper = setup_helper_for_model_role(model_role),
      module_name = model_role,
      selected_module_type = selected_module_type,
      selected_option_label = selected_option_label,
      valid_module_options = coalesce_empty(available_options, character()),
      valid_module_option_labels = coalesce_empty(available_option_labels, character()),
      option_source = module_option_source(model_role),
      rcpp_class = component_name,
      cpp_interface = interface,
      source_file = source_file,
      doxygen_url = url,
      metadata_sources = c(
        "R setup helper formals",
        "Rcpp registration",
        "Doxygen member comments",
        "equation catalog",
        "tagged assumptions"
      )
    )
  )
}

make_derived_node <- function(derived_row) {
  list(
    id = derived_row$id,
    stage = "modules",
    kind = "derived",
    role = "Derived quantity",
    label = derived_row$label,
    subtitle = derived_row$subtitle,
    description = derived_row$description,
    badges = c("derived"),
    fields = character(),
    field_definition_labels = character(),
    field_definition_text = character(),
    field_notes = character(),
    inactive_fields = character(),
    selected_module_type = "",
    selected_option_label = "",
    available_options = character(),
    available_option_labels = character(),
    parameter_values = list(),
    fleet_groups = list(),
    fleet_count_label = "",
    methods = character(),
    equations = character(),
    assumptions = derived_row$assumptions,
    url = "",
    source = "",
    component = derived_row$label,
    interface = "",
    developer = list(
      metadata_type = "derived_quantity",
      source = "prototype graph rules",
      note = "Derived nodes are inferred from configured module combinations."
    )
  )
}

build_model_inspector_graph_data <- function(model_spec) {
  input_nodes <- lapply(seq_len(nrow(model_spec$inputs)), function(i) {
    make_input_node(model_spec$inputs[i, ])
  })

  configured_module_nodes <- lapply(seq_len(nrow(model_spec$modules)), function(i) {
    make_module_node(model_spec$modules[i, ])
  })

  derived_nodes <- lapply(seq_len(nrow(model_spec$derived)), function(i) {
    make_derived_node(model_spec$derived[i, ])
  })

  output_nodes <- lapply(seq_len(nrow(model_spec$outputs)), function(i) {
    make_output_node(model_spec$outputs[i, ])
  })

  first_module_indices <- seq_len(min(3, length(configured_module_nodes)))
  later_module_indices <- if (length(configured_module_nodes) >= 4) {
    seq.int(4, length(configured_module_nodes))
  } else {
    integer()
  }

  nodes <- c(
    input_nodes,
    configured_module_nodes[first_module_indices],
    derived_nodes,
    configured_module_nodes[later_module_indices],
    output_nodes
  )

  edges <- lapply(seq_len(nrow(model_spec$connections)), function(i) {
    connection <- model_spec$connections[i, ]
    list(
      from = connection$from,
      to = connection$to,
      label = connection$label,
      kind = connection$kind
    )
  })

  list(
    nodes = nodes,
    edges = edges,
    nodes_json = jsonlite::toJSON(nodes, auto_unbox = TRUE, null = "null"),
    edges_json = jsonlite::toJSON(edges, auto_unbox = TRUE, null = "null"),
    initial_focus_json = jsonlite::toJSON(
      model_spec$metadata$initial_focus_id,
      auto_unbox = TRUE
    ),
    model_title = model_spec$metadata$title,
    model_label = model_spec$metadata$model_label,
    configured_module_count = nrow(model_spec$modules)
  )
}

# -------------------------------------------------------------------------
# 6. Render HTML
# -------------------------------------------------------------------------

render_node_card <- function(node) {
  badge_html <- paste(
    vapply(node$badges, function(badge) {
      paste0("<span class='node-badge badge-", badge, "'>", badge, "</span>")
    }, character(1)),
    collapse = ""
  )

  paste0(
    "<button class='node-card node-", node$kind, "' data-node-id='",
    html_escape(node$id),
    "'>",
    "<span class='node-role'>", html_escape(node$role), "</span>",
    "<strong>", html_escape(node$label), "</strong>",
    "<span class='node-subtitle'>", html_escape(node$subtitle), "</span>",
    "<span class='node-badges'>", badge_html, "</span>",
    "</button>"
  )
}

render_stage <- function(stage_id, title, node_data) {
  stage_nodes <- node_data[vapply(node_data, `[[`, character(1), "stage") == stage_id]

  paste0(
    "<section class='graph-stage stage-", stage_id, "'>",
    "<h2>", title, "</h2>",
    paste(vapply(stage_nodes, render_node_card, character(1)), collapse = "\n"),
    "</section>"
  )
}

render_model_inspector <- function(
    model_spec,
    output = file.path(
      "visualization-prototypes",
      "examples",
      "user-model-inspector",
      "fims_user_model_inspector_example.html"
    ),
    open = FALSE,
    field_definition_mode = c("toggle", "hover")) {
  field_definition_mode <- match.arg(field_definition_mode)

  graph_data <- build_model_inspector_graph_data(model_spec)
  nodes <- graph_data$nodes
  edges <- graph_data$edges
  nodes_json <- graph_data$nodes_json
  edges_json <- graph_data$edges_json
  initial_focus_json <- graph_data$initial_focus_json
  model_title <- graph_data$model_title
  model_label <- graph_data$model_label
  configured_module_count <- graph_data$configured_module_count

html <- paste0(
  "<!doctype html>\n",
  "<html lang='en'>\n",
  "<head>\n",
  "<meta charset='utf-8'>\n",
  "<meta name='viewport' content='width=device-width, initial-scale=1'>\n",
  "<title>", html_escape(model_title), "</title>\n",
  "<style>\n",
  ":root{",
  "--ink:#213f4b;--muted:#5f7480;--line:#9bb8c2;--blue:#2e75b6;",
  "--teal:#0e6876;--green:#548235;--mint:#d9edf2;--soft:#f8fafb;",
  "--panel:#ffffff;--edge:#b7cbd3;--gold:#d6a338;",
  "}",
  "*{box-sizing:border-box}",
  "body{margin:0;font-family:Inter,Segoe UI,Arial,sans-serif;color:var(--ink);",
  "background:linear-gradient(180deg,#f7fafb,#eef4f6);}",
  ".page{min-height:100vh;padding:20px;}",
  ".header{display:flex;justify-content:space-between;gap:16px;align-items:flex-start;",
  "max-width:1700px;margin:0 auto 14px auto;}",
  ".header h1{margin:0;color:var(--teal);font-size:26px;letter-spacing:0}",
  ".header p{margin:4px 0 0 0;color:var(--muted);max-width:900px;line-height:1.4}",
  ".summary-pills{display:flex;gap:8px;flex-wrap:wrap;justify-content:flex-end}",
  ".pill{border:1px solid #cfd8dc;background:#fff;border-radius:999px;",
  "padding:6px 10px;font-size:12px;color:var(--muted);white-space:nowrap}",
  ".shell{max-width:1700px;margin:0 auto;display:grid;",
  "grid-template-columns:minmax(0,1fr) minmax(360px,420px);gap:16px;align-items:stretch}",
  ".graph-panel,.inspector{background:var(--panel);border:1px solid #d5dee2;",
  "border-radius:10px;box-shadow:0 10px 28px rgba(33,63,75,.08)}",
  ".graph-panel{position:relative;min-height:760px;padding:18px;overflow:hidden}",
  ".toolbar{display:flex;gap:10px;align-items:center;justify-content:space-between;",
  "margin-bottom:14px;position:relative;z-index:2}",
  ".toolbar-left{display:flex;gap:10px;align-items:center;flex-wrap:wrap}",
  "select{border:1px solid #bfccd2;background:white;color:var(--ink);",
  "border-radius:6px;padding:8px 10px;font-size:14px}",
  ".legend{display:flex;gap:8px;flex-wrap:wrap;font-size:12px;color:var(--muted)}",
  ".legend span{display:inline-flex;align-items:center;gap:5px}",
  ".swatch{width:11px;height:11px;border-radius:3px;display:inline-block}",
  ".graph-wrap{position:relative;height:720px;border:1px solid #edf1f3;border-radius:8px;",
  "background:linear-gradient(180deg,#ffffff,#fbfdfe);",
  "overflow:auto;scrollbar-color:#9bb8c2 #eef4f6}",
  "#edgeLayer{position:absolute;left:0;top:0;width:100%;min-width:1180px;height:100%;z-index:0;pointer-events:none}",
  ".graph-grid{position:relative;z-index:1;height:100%;display:grid;min-width:1180px;",
  "grid-template-columns:300px 520px 300px;gap:30px;padding:24px 26px}",
  ".graph-stage{display:flex;flex-direction:column;gap:18px;align-content:start;min-width:0}",
  ".graph-stage h2{font-size:13px;letter-spacing:.02em;text-transform:uppercase;",
  "color:var(--muted);margin:0 0 2px 0;white-space:normal}",
  ".stage-modules{display:grid;grid-template-columns:repeat(2,minmax(135px,1fr));",
  "align-content:center;gap:30px 18px}",
  ".stage-modules h2{grid-column:1/-1}",
  ".node-card{width:100%;text-align:left;border:1px solid #cfd8dc;border-radius:9px;",
  "background:#fff;color:var(--ink);padding:10px;cursor:pointer;min-height:82px;",
  "box-shadow:0 4px 12px rgba(33,63,75,.06);transition:.16s ease transform,.16s ease opacity,.16s ease border-color}",
  ".node-card:hover{transform:translateY(-1px);border-color:var(--teal)}",
  ".node-card.active{border-color:var(--teal);box-shadow:0 0 0 3px rgba(14,104,118,.14),0 8px 20px rgba(33,63,75,.12)}",
  ".node-card.dimmed{opacity:.2}.node-card.filtered-dim{opacity:.16}",
  ".node-card.related{border-color:#8db3bd}",
  ".node-role{display:block;font-size:11px;text-transform:uppercase;letter-spacing:.04em;color:var(--muted);margin-bottom:4px}",
  ".node-card strong{display:block;font-size:17px;line-height:1.12;color:var(--ink);overflow-wrap:anywhere}",
  ".node-subtitle{display:block;font-size:12px;color:var(--muted);margin-top:4px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}",
  ".node-badges{display:flex;gap:5px;flex-wrap:wrap;margin-top:8px}",
  ".node-badge{font-size:10px;border-radius:999px;padding:2px 6px;background:#edf2f4;color:var(--muted)}",
  ".badge-docs{background:#e7f2f5;color:var(--teal)}",
  ".badge-equations{background:#edf6eb;color:var(--green)}",
  ".badge-assumptions{background:#fff4df;color:#7a5a12}",
  ".badge-input{background:#eef5fb;color:var(--blue)}",
  ".badge-output,.badge-derived{background:#eef7f4;color:var(--green)}",
  ".node-module{border-left:5px solid var(--teal)}",
  ".node-input{border-left:5px solid var(--blue)}",
  ".node-output,.node-derived{border-left:5px solid var(--green)}",
  ".edge-path{fill:none;stroke:var(--edge);stroke-width:1.4;opacity:.82}",
  ".edge-path.output,.edge-path.process{stroke:var(--green)}",
  ".edge-path.data{stroke:#6c8fa0}",
  ".edge-path.dimmed{opacity:.08}.edge-path.active{stroke-width:2.15;opacity:.95}",
  ".edge-label{display:none}",
  ".port-dot{stroke:#fff;stroke-width:2px}.port-dot.data{fill:#6c8fa0}.port-dot.process,.port-dot.output{fill:var(--green)}",
  ".inspector{min-height:760px;display:flex;flex-direction:column;overflow:hidden}",
  ".inspector-header{padding:18px;border-bottom:1px solid #e1e7ea;background:#fbfcfd}",
  ".inspector-kicker{font-size:12px;text-transform:uppercase;letter-spacing:.05em;color:var(--muted);margin-bottom:4px}",
  ".inspector h2{margin:0;color:var(--teal);font-size:24px;line-height:1.15}",
  ".inspector-subtitle{color:var(--muted);margin-top:4px;font-size:14px}",
  ".inspector-tabs{display:flex;gap:6px;flex-wrap:wrap;padding:10px 14px;",
  "border-bottom:1px solid #e1e7ea;background:#fbfcfd}",
  ".tab-button{border:1px solid #ccd8dd;background:#fff;color:var(--ink);",
  "border-radius:999px;padding:6px 10px;font-size:12px;cursor:pointer}",
  ".tab-button.active{background:#e7f2f5;border-color:var(--teal);color:var(--teal)}",
  ".inspector-body{padding:16px 18px 20px 18px;overflow:auto}",
  ".section{border-top:1px solid #edf1f3;padding-top:14px;margin-top:14px}",
  ".section:first-child{border-top:0;margin-top:0;padding-top:0}",
  ".section h3{font-size:13px;text-transform:uppercase;letter-spacing:.04em;",
  "color:var(--muted);margin:0 0 8px 0}",
  ".section p{margin:0;color:var(--ink);line-height:1.45}",
  ".field-note{border:1px solid #d6e6d1;background:#f6faf8;border-radius:7px;",
  "padding:8px 10px;margin:0 0 10px 0;font-size:12px;line-height:1.4}",
  ".field-subheading{font-size:11px;text-transform:uppercase;letter-spacing:.04em;",
  "color:var(--muted);margin:10px 0 6px 0}",
  ".chip-list{display:flex;gap:6px;flex-wrap:wrap}",
  ".chip{border:1px solid #ccd8dd;background:#f8fafb;border-radius:999px;",
  "padding:5px 8px;font-size:12px;color:var(--ink)}",
  ".option-context{display:grid;gap:8px}",
  ".option-context strong{display:block;font-size:12px;color:var(--muted);",
  "text-transform:uppercase;letter-spacing:.04em;margin-bottom:5px}",
  ".fleet-groups{display:grid;gap:8px}",
  ".fleet-group{border:1px solid #e1e7ea;background:#fbfcfd;border-radius:8px;",
  "padding:9px 10px}",
  ".fleet-group-title{display:flex;justify-content:space-between;gap:10px;",
  "align-items:center;margin-bottom:7px;font-size:12px;font-weight:700;color:var(--ink)}",
  ".fleet-count{color:var(--muted);font-weight:500;white-space:nowrap}",
  ".related-link-list{display:flex;flex-wrap:wrap;gap:8px}",
  ".related-link-button{border:1px solid #ccd8dd;background:#fff;color:var(--teal);",
  "border-radius:999px;padding:6px 10px;font-size:12px;cursor:pointer}",
  ".related-link-button:hover,.related-link-button:focus{background:#e7f2f5;border-color:var(--teal)}",
  ".composition-section-header{display:flex;justify-content:space-between;gap:10px;align-items:center;margin-bottom:8px}",
  ".composition-section-header h3{margin:0}",
  ".composition-actions{display:flex;gap:6px;flex-wrap:wrap;justify-content:flex-end}",
  ".composition-toggle-button{border:1px solid #ccd8dd;background:#fff;color:var(--teal);",
  "border-radius:999px;padding:4px 8px;font-size:11px;cursor:pointer}",
  ".composition-toggle-button:hover,.composition-toggle-button:focus{background:#e7f2f5;border-color:var(--teal)}",
  ".composition-summaries{display:grid;gap:10px}",
  ".composition-summary{border:1px solid #e1e7ea;background:#fbfcfd;border-radius:8px;overflow:hidden}",
  ".composition-summary summary{display:flex;gap:8px;align-items:center;",
  "padding:9px 10px;cursor:pointer;font-weight:700;color:var(--ink);list-style:none}",
  ".composition-summary summary::-webkit-details-marker{display:none}",
  ".composition-summary summary::before{content:'+';display:inline-flex;align-items:center;justify-content:center;",
  "width:16px;height:16px;border:1px solid #ccd8dd;border-radius:999px;color:var(--teal);font-size:11px;line-height:1}",
  ".composition-summary[open] summary::before{content:'-'}",
  ".composition-summary-list{border-top:1px solid #edf1f3;margin:0;padding:8px 10px;display:grid;gap:7px}",
  ".composition-summary-list div{display:grid;grid-template-columns:minmax(95px,38%) 1fr;gap:8px}",
  ".composition-summary-list dt{color:var(--muted);font-size:12px}",
  ".composition-summary-list dd{margin:0;color:var(--ink);font-size:12px;line-height:1.35}",
  ".value-groups{display:grid;gap:10px}",
  ".value-group{border:1px solid #e1e7ea;background:#fbfcfd;border-radius:8px;overflow:hidden}",
  ".value-group-title{display:flex;justify-content:space-between;gap:10px;align-items:center;",
  "padding:9px 10px;font-size:12px;font-weight:700;color:var(--ink);",
  "border-bottom:1px solid #e1e7ea;background:#fff}",
  ".value-table-wrap{overflow:auto;border:1px solid #e1e7ea;border-radius:8px}",
  ".value-group .value-table-wrap{border:0;border-radius:0}",
  ".value-table{width:100%;border-collapse:collapse;font-size:12px;min-width:0}",
  ".value-table th{background:#f8fafb;color:var(--muted);text-align:left;",
  "font-size:11px;text-transform:uppercase;letter-spacing:.04em;padding:8px}",
  ".value-table td{border-top:1px solid #edf1f3;padding:8px;vertical-align:top}",
  ".value-table td:first-child{width:30%}",
  ".value-table td:nth-child(2),.value-table td:nth-child(3){width:16%}",
  ".value-table td:nth-child(4){width:16%}",
  ".value-table td:nth-child(5){width:22%}",
  ".value-table code{font-family:Consolas,Menlo,monospace;font-size:11px;color:var(--ink);",
  "white-space:normal;overflow-wrap:normal;word-break:normal}",
  ".value-context{display:block;color:var(--muted);font-size:11px;margin-top:2px}",
  ".status-pill{display:inline-block;border-radius:999px;padding:3px 7px;",
  "font-size:11px;background:#edf2f4;color:var(--muted);white-space:nowrap}",
  ".status-default{background:#eef7f4;color:var(--green)}",
  ".status-changed{background:#fff4df;color:#7a5a12}",
  ".status-not-in-defaults{background:#f8fafb;color:var(--muted)}",
  ".definition-chip{position:relative;cursor:help;display:inline-flex;",
  "align-items:center;gap:4px}",
  ".definition-chip .info-mark{font-size:11px;line-height:1;color:var(--teal)}",
  ".chip-tooltip{position:absolute;left:0;top:calc(100% + 6px);z-index:20;",
  "visibility:hidden;opacity:0;min-width:220px;max-width:330px;white-space:normal;",
  "background:#fff;border:1px solid #ccd8dd;border-radius:7px;",
  "box-shadow:0 8px 20px rgba(33,63,75,.16);padding:8px 10px;",
  "color:var(--ink);line-height:1.35;transition:opacity .12s ease,visibility .12s ease}",
  ".definition-chip:hover .chip-tooltip,.definition-chip:focus .chip-tooltip{",
  "visibility:visible;opacity:1}",
  ".definition-toggle{margin-top:10px;border:1px solid #ccd8dd;background:#fff;",
  "border-radius:999px;padding:5px 9px;font-size:12px;color:var(--teal);cursor:pointer}",
  ".field-box-list{display:grid;gap:8px}",
  ".definition-toggle+.field-box-list{margin-top:10px}",
  ".field-box{border:1px solid #e1e7ea;background:#fbfcfd;border-radius:8px;",
  "padding:9px 10px}",
  ".field-box-term{font-family:Consolas,Menlo,monospace;font-weight:700;",
  "font-size:12px;color:var(--ink);overflow-wrap:anywhere}",
  ".field-box-definition{color:var(--muted);font-size:12px;line-height:1.4;",
  "margin-top:6px}",
  ".definition-list{display:grid;gap:8px;margin-top:10px}",
  ".definition-list[hidden]{display:none}",
  ".definition-item{border:1px solid #e1e7ea;background:#fbfcfd;border-radius:7px;",
  "padding:8px 10px}",
  ".definition-term{font-weight:700;color:var(--ink);font-size:12px;margin-bottom:3px}",
  ".definition-text{color:var(--muted);font-size:12px;line-height:1.4}",
  ".equation-box{font-family:Consolas,Menlo,monospace;font-size:12px;line-height:1.35;",
  "background:#f6faf8;border:1px solid #d6e6d1;border-radius:7px;padding:10px;",
  "white-space:pre-wrap;overflow:auto;color:#213f4b;margin-bottom:8px}",
  ".connection-list{list-style:none;margin:0;padding:0;display:grid;gap:6px}",
  ".connection-list li{border:1px solid #e1e7ea;border-radius:7px;padding:7px 8px;",
  "background:#fbfcfd;font-size:13px;color:var(--ink)}",
  ".connection-list button{border:0;background:none;color:var(--teal);font:inherit;",
  "padding:0;cursor:pointer;text-decoration:underline}",
  ".assumption-list{margin:0;padding-left:18px;display:grid;gap:7px}",
  ".assumption-list li{line-height:1.38}",
  ".empty{color:var(--muted);font-style:italic}",
  ".link-list a{display:block;color:var(--teal);word-break:break-all;margin-bottom:6px}",
  ".developer-list{display:grid;gap:7px}",
  ".developer-row{display:grid;grid-template-columns:120px minmax(0,1fr);gap:8px;",
  "font-size:12px;align-items:start}",
  ".developer-row strong{color:var(--muted);text-transform:uppercase;",
  "letter-spacing:.04em;font-size:11px}",
  ".developer-row code{font-family:Consolas,Menlo,monospace;font-size:12px;",
  "white-space:normal;overflow-wrap:anywhere;color:var(--ink)}",
  "@media (max-width:1200px){.shell{grid-template-columns:1fr}.inspector{min-height:auto}.graph-panel{min-height:680px}.graph-wrap{height:600px}}",
  "@media (max-width:860px){.page{padding:12px}.header{display:block}.summary-pills{justify-content:flex-start;margin-top:10px}.graph-grid{grid-template-columns:1fr;overflow:auto}.stage-modules{grid-template-columns:1fr}.graph-wrap{height:900px}.shell{grid-template-columns:1fr;min-width:0}.graph-panel{min-height:auto}}",
  "</style>\n",
  "</head>\n",
  "<body>\n",
  "<div class='page'>\n",
  "<header class='header'>\n",
  "<div><h1>", html_escape(model_title), "</h1>",
  "<p>Compact model overview with details shown in a persistent inspector. ",
  "This keeps the graph readable while still exposing equations, assumptions, ",
  "parameters, source files, and Doxygen links.</p></div>",
  "<div class='summary-pills'>",
  "<span class='pill'>", html_escape(model_label), "</span>",
  "<span class='pill'>", configured_module_count, " configured modules</span>",
  "<span class='pill'>Click a node for details</span>",
  "</div>",
  "</header>\n",
  "<main class='shell'>\n",
  "<section class='graph-panel'>\n",
  "<div class='toolbar'>",
  "<div class='toolbar-left'>",
  "<select id='focusSelect' aria-label='Focus node'></select>",
  "</div>",
  "<div class='legend'>",
  "<span><i class='swatch' style='background:var(--blue)'></i>Input</span>",
  "<span><i class='swatch' style='background:var(--teal)'></i>Module</span>",
  "<span><i class='swatch' style='background:var(--green)'></i>Output</span>",
  "</div>",
  "</div>",
  "<div class='graph-wrap' id='graphWrap'>",
  "<svg id='edgeLayer' aria-hidden='true'>",
  "<defs><marker id='arrow' markerWidth='7' markerHeight='7' refX='6.2' refY='2.5' orient='auto' markerUnits='strokeWidth'><path d='M0,0 L0,5 L6.5,2.5 z' fill='#9bb8c2'></path></marker></defs>",
  "<g id='edgePaths'></g>",
  "</svg>",
  "<div class='graph-grid'>",
  render_stage("inputs", "Inputs", nodes),
  render_stage("modules", "Configured Model", nodes),
  render_stage("outputs", "Expected Outputs", nodes),
  "</div>",
  "</div>",
  "</section>\n",
  "<aside class='inspector' aria-live='polite'>",
  "<div class='inspector-header'>",
  "<div class='inspector-kicker' id='inspectorRole'>Selected node</div>",
  "<h2 id='inspectorTitle'>Select a node</h2>",
  "<div class='inspector-subtitle' id='inspectorSubtitle'></div>",
  "</div>",
  "<div class='inspector-body' id='inspectorBody'></div>",
  "</aside>\n",
  "</main>\n",
  "</div>\n",
  "<script>\n",
  "const nodes = ", nodes_json, ";\n",
  "const edges = ", edges_json, ";\n",
  "const fieldDefinitionMode = '", field_definition_mode, "';\n",
  "const nodeById = Object.fromEntries(nodes.map((node) => [node.id, node]));\n",
  "let selectedNodeId = ", initial_focus_json, ";\n",
  "let activeInspectorTab = 'overview';\n",
  "const graphWrap = document.getElementById('graphWrap');\n",
  "const edgePaths = document.getElementById('edgePaths');\n",
  "const focusSelect = document.getElementById('focusSelect');\n",
  "function esc(value){return String(value ?? '').replace(/[&<>\\\"]/g,(char)=>({'&':'&amp;','<':'&lt;','>':'&gt;','\\\"':'&quot;'}[char]));}\n",
  "function arrayValue(value){if(!value){return [];} return Array.isArray(value) ? value : [value];}\n",
  "function neighbors(id){const ids = new Set([id]); edges.forEach((edge)=>{if(edge.from===id){ids.add(edge.to);} if(edge.to===id){ids.add(edge.from);}}); return ids;}\n",
  "function isVisibleInView(node){return true;}\n",
  "function populateSelect(){nodes.forEach((node)=>{const option=document.createElement('option'); option.value=node.id; option.textContent=`${node.role}: ${node.label}`; focusSelect.appendChild(option);}); focusSelect.value=selectedNodeId;}\n",
  "function renderList(values, emptyText='None detected'){values=arrayValue(values).filter(Boolean); if(values.length===0){return `<span class='empty'>${esc(emptyText)}</span>`;} return `<div class='chip-list'>${values.map((value)=>`<span class='chip'>${esc(value)}</span>`).join('')}</div>`;}\n",
  "function fieldDefinitionMap(node){const labels=arrayValue(node.field_definition_labels); const text=arrayValue(node.field_definition_text); const definitions={}; labels.forEach((label,index)=>{if(label){definitions[label]=text[index] || 'No definition found yet.';}}); return definitions;}\n",
  "function renderFieldChips(values,node,emptyText='None detected'){values=arrayValue(values).filter(Boolean); if(values.length===0){return `<span class='empty'>${esc(emptyText)}</span>`;} const definitions=fieldDefinitionMap(node); return `<div class='chip-list'>${values.map((value)=>{const definition=definitions[value]; if(fieldDefinitionMode==='hover' && definition){return `<span class='chip definition-chip' tabindex='0'>${esc(value)} <span class='info-mark' aria-hidden='true'>&#9432;</span><span class='chip-tooltip' role='tooltip'>${esc(definition)}</span></span>`;} return `<span class='chip'>${esc(value)}</span>`;}).join('')}</div>`;}\n",
  "function renderFieldBoxes(values,node,emptyText='None detected'){values=arrayValue(values).filter(Boolean); if(values.length===0){return `<span class='empty'>${esc(emptyText)}</span>`;} const definitions=fieldDefinitionMap(node); return `<div class='field-box-list'>${values.map((value)=>`<div class='field-box'><div class='field-box-term'>${esc(value)}</div><div class='field-box-definition' data-field-definition hidden>${esc(definitions[value] || 'No definition found yet.')}</div></div>`).join('')}</div>`;}\n",
  "function renderFieldDisplay(values,node,emptyText='None detected'){if(fieldDefinitionMode==='hover'){return renderFieldChips(values,node,emptyText);} return renderFieldBoxes(values,node,emptyText);}\n",
  "function renderDefinitionToggle(node){const labels=arrayValue(node.field_definition_labels).filter(Boolean); if(fieldDefinitionMode==='hover' || labels.length===0){return '';} return `<button type='button' class='definition-toggle' data-toggle-definitions>Show definitions</button>`;}\n",
  "function renderFields(node){const notes=arrayValue(node.field_notes).filter(Boolean); const inactive=arrayValue(node.inactive_fields).filter(Boolean); let html=''; if(notes.length>0){html += notes.map((note)=>`<p class='field-note'>${esc(note)}</p>`).join('');} html += renderDefinitionToggle(node); if(notes.length===0 && inactive.length===0){html += renderFieldDisplay(node.fields,node);} else {html += `<div class='field-subheading'>Active for this configuration</div>${renderFieldDisplay(node.fields,node)}`; if(inactive.length>0){html += `<div class='field-subheading'>Registered delta-method fields not active here</div>${renderFieldDisplay(inactive,node)}`;}} return html;}\n",
  "function renderAssumptions(values){values=arrayValue(values).filter(Boolean); if(values.length===0){return `<span class='empty'>No tagged assumptions found yet.</span>`;} return `<ul class='assumption-list'>${values.map((value)=>`<li>${esc(value)}</li>`).join('')}</ul>`;}\n",
  "function renderEquations(node){const equations=arrayValue(node.equations).filter(Boolean); if(equations.length===0){return `<span class='empty'>No equation extracted yet.</span>`;} return equations.slice(0,3).map((eq)=>`<div class='equation-box'>${esc(eq)}</div>`).join('') + `<div class='link-list'><a href='../package-level-explorer/fims_equation_catalog.html' target='_blank'>Open equation catalog</a></div>`;}\n",
  "function renderLinks(node){let links=''; if(node.url){links += `<a href='${esc(node.url)}' target='_blank'>Open Doxygen page</a>`;} if(node.source){links += `<a href='../${esc(node.source)}' target='_blank'>${esc(node.source)}</a>`;} return links || `<span class='empty'>No link available for this node yet.</span>`;}\n",
  "function renderDeveloperRows(rows){rows=rows.filter((row)=>row && row[1]); if(rows.length===0){return '';} return `<div class='developer-list'>${rows.map(([label,value])=>`<div class='developer-row'><strong>${esc(label)}</strong><code>${esc(value)}</code></div>`).join('')}</div>`;}\n",
  "function renderDeveloperTab(node){const dev=node.developer || {}; let html=''; if(dev.setup_helper){html += `<section class='section'><h3>R setup helper</h3><p><code>${esc(dev.setup_helper)}()</code></p></section>`;} const optionLabels=arrayValue(dev.valid_module_option_labels).filter(Boolean); if(optionLabels.length>0){html += `<section class='section'><h3>Valid module options</h3>${renderList(optionLabels,'No module options detected')}</section>`;} const rows=renderDeveloperRows([['metadata type',dev.metadata_type],['module_name',dev.module_name],['selected module_type',dev.selected_module_type],['option source',dev.option_source],['Rcpp class',dev.rcpp_class],['C++ interface',dev.cpp_interface],['source file',dev.source_file],['source',dev.source],['note',dev.note]]); if(rows){html += `<section class='section'><h3>Technical mapping</h3>${rows}</section>`;} return html || `<section class='section'><h3>Developer metadata</h3><span class='empty'>No developer metadata available for this node yet.</span></section>`;}\n",
  "function renderOptionContext(node){const selected=node.selected_option_label || node.selected_module_type || ''; const labels=arrayValue(node.available_option_labels).filter(Boolean); if(!selected && labels.length===0){return '';} const others=labels.filter((label)=>label!==selected); const otherHtml=others.length>0 ? renderList(others,'No alternatives listed') : `<span class='empty'>No alternatives listed.</span>`; const roleLabel=(node.role || 'Module').replace(/ module$/i,''); return `<section class='section'><h3>${esc(roleLabel)} option</h3><div class='option-context'><div><strong>Selected</strong><div class='chip-list'><span class='chip'>${esc(selected || node.component || node.label)}</span></div></div><div><strong>Available alternatives</strong>${otherHtml}</div></div></section>`;}\n",
  "function renderFleetGroups(node,title='Fleets'){const groups=arrayValue(node.fleet_groups).filter((group)=>group && arrayValue(group.fleets).filter(Boolean).length>0); if(groups.length===0){return '';} const groupHtml=groups.map((group)=>{const fleets=arrayValue(group.fleets).filter(Boolean); const chips=fleets.map((fleet)=>`<span class='chip'>${esc(fleet)}</span>`).join(''); const count=group.count_label || `${fleets.length} ${fleets.length===1 ? 'fleet' : 'fleets'}`; return `<div class='fleet-group'><div class='fleet-group-title'><span>${esc(group.label || 'Fleets')}</span><span class='fleet-count'>${esc(count)}</span></div><div class='chip-list'>${chips}</div></div>`;}).join(''); return `<section class='section'><h3>${esc(title)}</h3><div class='fleet-groups'>${groupHtml}</div></section>`;}\n",
  "function renderRelatedValueLinks(node){const links=arrayValue(node.related_value_links).filter((link)=>link && link.target_id && nodeById[link.target_id]); if(links.length===0){return '';} const buttons=links.map((link)=>`<button type='button' class='related-link-button' data-related-node='${esc(link.target_id)}' data-related-tab='${esc(link.target_tab || 'values')}'>${esc(link.label)}</button>`).join(''); return `<section class='section'><h3>Related composition data</h3><div class='related-link-list'>${buttons}</div></section>`;}\n",
  "function renderCompositionSummaries(node){const summaries=arrayValue(node.composition_summaries).filter((summary)=>summary && summary.fleet); if(summaries.length===0){return '';} const cards=summaries.map((summary,index)=>{const rows=[['Related input',summary.related_input],['Years',summary.year_range],['Observed years',summary.observed_years],['Missing years',summary.missing_years],[summary.bin_label || 'Observed bins',summary.bin_range],['Sample size',summary.sample_size]].filter((row)=>row[1]); const rowHtml=rows.map(([label,value])=>`<div><dt>${esc(label)}</dt><dd>${esc(value)}</dd></div>`).join(''); const open=summaries.length<=3 || index===0 ? ' open' : ''; return `<details class='composition-summary'${open}><summary><span>${esc(summary.fleet)}</span></summary><dl class='composition-summary-list'>${rowHtml}</dl></details>`;}).join(''); return `<section class='section'><div class='composition-section-header'><h3>Composition by fleet</h3><div class='composition-actions'><button type='button' class='composition-toggle-button' data-composition-open='true'>Expand all</button><button type='button' class='composition-toggle-button' data-composition-open='false'>Collapse all</button></div></div><div class='composition-summaries'>${cards}</div></section>`;}\n",
  "function statusClass(status){return `status-${String(status || '').toLowerCase().replace(/[^a-z0-9]+/g,'-').replace(/^-|-$/g,'')}`;}\n",
  "function formatDisplayValue(value){return String(value ?? '').replace(/_/g,' ');}\n",
  "function formatParameterName(value){return esc(value).replace(/_/g,'_<wbr>');}\n",
  "function fleetFromContext(context){const match=String(context || '').match(/(?:^|;\\s*)fleet\\s+([^;]+)/i); return match ? match[1].trim() : '';}\n",
  "function contextWithoutFleet(context){return String(context || '').split(';').map((part)=>part.trim()).filter((part)=>part && !/^fleet\\s+/i.test(part)).join('; ');}\n",
  "function fleetOptionLabel(node,fleet){const groups=arrayValue(node.fleet_groups); for(const group of groups){const fleets=arrayValue(group && group.fleets).filter(Boolean); if(fleets.includes(fleet)){return group.label || '';}} return '';}\n",
  "function parameterGroupLabel(node,row){const fleet=fleetFromContext(row.context); if(!fleet){return '';} const option=fleetOptionLabel(node,fleet); return option ? `${option}: ${fleet}` : fleet;}\n",
  "function renderParameterTable(rows,stripFleetContext=false){const body=rows.map((row)=>{const context=stripFleetContext ? contextWithoutFleet(row.context) : row.context; return `<tr><td><code>${formatParameterName(row.parameter)}</code>${context ? `<span class='value-context'>${esc(context)}</span>` : ''}</td><td>${esc(row.user_value)}</td><td>${esc(row.default_value)}</td><td><span class='status-pill ${statusClass(row.status)}'>${esc(formatDisplayValue(row.status))}</span></td><td>${esc(formatDisplayValue(row.estimation_type))}</td></tr>`;}).join(''); return `<div class='value-table-wrap'><table class='value-table'><thead><tr><th>Parameter</th><th>Your value</th><th>Default</th><th>Status</th><th>Estimation</th></tr></thead><tbody>${body}</tbody></table></div>`;}\n",
  "function renderGroupedParameterValues(node,rows){const groups=new Map(); rows.forEach((row)=>{const label=parameterGroupLabel(node,row); if(!label){return;} if(!groups.has(label)){groups.set(label,[]);} groups.get(label).push(row);}); if(groups.size===0){return renderParameterTable(rows,false);} const groupedRows=new Set(Array.from(groups.values()).flat()); const otherRows=rows.filter((row)=>!groupedRows.has(row)); let html=`<div class='value-groups'>${Array.from(groups.entries()).map(([label,groupRows])=>`<div class='value-group'><div class='value-group-title'><span>${esc(label)}</span></div>${renderParameterTable(groupRows,true)}</div>`).join('')}</div>`; if(otherRows.length>0){html += renderParameterTable(otherRows,false);} return html;}\n",
  "function renderParameterValues(node){const rows=arrayValue(node.parameter_values).filter((row)=>row && row.parameter); if(rows.length===0){return `<span class='empty'>No explicit pre-run parameter values are available for this node.</span>`;} return renderGroupedParameterValues(node,rows);}\n",
  "function renderOverviewTab(node){return `<section class='section'><h3>What this shows</h3><p>${esc(node.description)}</p></section>${renderOptionContext(node)}<section class='section'><h3>Equation preview</h3>${renderEquations(node)}</section><section class='section'><h3>Assumptions</h3>${renderAssumptions(node.assumptions)}</section>`;}\n",
  "function renderValuesTab(node){const rows=arrayValue(node.parameter_values).filter((row)=>row && row.parameter); const compositionHtml=renderCompositionSummaries(node); const hideFleetSummary=compositionHtml || (rows.length>0 && /selectivity/i.test(node.role || '')); const fleetTitle=/observation/i.test(node.role || '') ? 'Fleet inputs' : 'Fleets'; const fleetHtml=hideFleetSummary ? '' : renderFleetGroups(node,fleetTitle); let html=compositionHtml + fleetHtml + renderRelatedValueLinks(node); if(rows.length>0 || !html){html += `<section class='section'><h3>Configured values</h3>${renderParameterValues(node)}</section>`;} return html;}\n",
  "function renderDocumentationTab(node){return `<section class='section'><h3>Documentation and source</h3><div class='link-list'>${renderLinks(node)}</div></section><section class='section'><h3>Parameters or fields</h3>${renderFields(node)}</section>`;}\n",
  "function renderTabContent(node){if(activeInspectorTab==='values'){return renderValuesTab(node);} if(activeInspectorTab==='documentation'){return renderDocumentationTab(node);} if(activeInspectorTab==='developer'){return renderDeveloperTab(node);} return renderOverviewTab(node);}\n",
  "function renderTabButtons(){const tabs=[['overview','Overview'],['values','Values'],['documentation','Documentation'],['developer','Power User']]; return `<div class='inspector-tabs'>${tabs.map(([id,label])=>`<button type='button' class='tab-button ${activeInspectorTab===id ? 'active' : ''}' data-inspector-tab='${id}'>${label}</button>`).join('')}</div>`;}\n",
  "function updateInspector(node){document.getElementById('inspectorRole').textContent=node.role; document.getElementById('inspectorTitle').textContent=node.label; document.getElementById('inspectorSubtitle').textContent=node.subtitle || node.component || ''; const body=document.getElementById('inspectorBody'); body.innerHTML = `${renderTabButtons()}<div class='tab-content'>${renderTabContent(node)}</div>`; body.querySelectorAll('[data-inspector-tab]').forEach((button)=>button.addEventListener('click',()=>{activeInspectorTab=button.dataset.inspectorTab; updateInspector(node);})); body.querySelectorAll('[data-related-node]').forEach((button)=>button.addEventListener('click',()=>{activeInspectorTab=button.dataset.relatedTab || 'values'; selectNode(button.dataset.relatedNode);})); body.querySelectorAll('[data-composition-open]').forEach((button)=>button.addEventListener('click',()=>{const open=button.dataset.compositionOpen==='true'; body.querySelectorAll('.composition-summary').forEach((summary)=>{summary.open=open;});})); const definitionButton=body.querySelector('[data-toggle-definitions]'); if(definitionButton){definitionButton.addEventListener('click',()=>{const definitions=Array.from(body.querySelectorAll('[data-field-definition]')); const opening=definitions.some((definition)=>definition.hasAttribute('hidden')); definitions.forEach((definition)=>{if(opening){definition.removeAttribute('hidden');} else {definition.setAttribute('hidden','');}}); definitionButton.textContent=opening ? 'Hide definitions' : 'Show definitions';});}}\n",
  "function updateNodeClasses(){const near=neighbors(selectedNodeId); document.querySelectorAll('.node-card').forEach((card)=>{const node=nodeById[card.dataset.nodeId]; card.classList.toggle('active', card.dataset.nodeId===selectedNodeId); card.classList.toggle('related', near.has(card.dataset.nodeId) && card.dataset.nodeId!==selectedNodeId); card.classList.toggle('dimmed', !near.has(card.dataset.nodeId)); card.classList.toggle('filtered-dim', !isVisibleInView(node));});}\n",
  "function selectNode(id){selectedNodeId=id; focusSelect.value=id; updateInspector(nodeById[id]); updateNodeClasses(); drawEdges();}\n",
  "function cardRect(card){const wrap=graphWrap.getBoundingClientRect(); const box=card.getBoundingClientRect(); const scrollX=graphWrap.scrollLeft; const scrollY=graphWrap.scrollTop; return {left:box.left-wrap.left+scrollX,right:box.right-wrap.left+scrollX,top:box.top-wrap.top+scrollY,bottom:box.bottom-wrap.top+scrollY,width:box.width,height:box.height};}\n",
  "function cardCenter(card){const rect=cardRect(card); return {x:rect.left+rect.width/2,y:rect.top+rect.height/2};}\n",
  "function sidePoint(card, side, offset=0){const rect=cardRect(card); if(side==='top' || side==='bottom'){const x=Math.max(rect.left+24, Math.min(rect.right-24, rect.left+rect.width/2+offset)); const y=side==='top' ? rect.top : rect.bottom; return {x,y};} const y=Math.max(rect.top+18, Math.min(rect.bottom-18, rect.top+rect.height/2+offset)); const x=side==='left' ? rect.left : rect.right; return {x,y};}\n",
  "function selectedSideFor(edge){const selectedIsSource=edge.from===selectedNodeId; if(!selectedIsSource && edge.kind==='data'){return 'left';} if(edge.kind==='process'){const selectedCard=cardFor(selectedNodeId); const otherCard=cardFor(selectedIsSource ? edge.to : edge.from); if(selectedCard && otherCard){const selectedCenter=cardCenter(selectedCard); const otherCenter=cardCenter(otherCard); const dx=otherCenter.x-selectedCenter.x; const dy=otherCenter.y-selectedCenter.y; if(Math.abs(dx)>Math.abs(dy)*1.08){return dx>0 ? 'right' : 'left';} if(Math.abs(dy)>20){return dy>0 ? 'bottom' : 'top';} return dx>=0 ? 'right' : 'left';} return selectedIsSource ? 'right' : 'top';} if(selectedIsSource && edge.kind==='output'){return 'right';} const selectedCard=cardFor(selectedNodeId); const otherId=selectedIsSource ? edge.to : edge.from; const otherCard=cardFor(otherId); if(!selectedCard || !otherCard){return 'right';} const selectedCenter=cardCenter(selectedCard); const otherCenter=cardCenter(otherCard); return otherCenter.x >= selectedCenter.x ? 'right' : 'left';}\n",
  "function oppositeSide(side){return {left:'right',right:'left',top:'bottom',bottom:'top'}[side] || 'left';}\n",
  "function controlPoint(point, side, distance){if(side==='left'){return {x:point.x-distance,y:point.y};} if(side==='right'){return {x:point.x+distance,y:point.y};} if(side==='top'){return {x:point.x,y:point.y-distance};} return {x:point.x,y:point.y+distance};}\n",
  "function curvePath(p1,p2,fromSide,toSide){const span=Math.hypot(p2.x-p1.x,p2.y-p1.y); const distance=Math.max(42, Math.min(130, span*0.38)); const c1=controlPoint(p1,fromSide,distance); const c2=controlPoint(p2,toSide,distance); return `M ${p1.x} ${p1.y} C ${c1.x} ${c1.y}, ${c2.x} ${c2.y}, ${p2.x} ${p2.y}`;}\n",
  "function cardFor(id){return document.querySelector(`[data-node-id='${id}']`);}\n",
  "function addPortDot(point, kind){const dot=document.createElementNS('http://www.w3.org/2000/svg','circle'); dot.setAttribute('cx',point.x); dot.setAttribute('cy',point.y); dot.setAttribute('r',4.5); dot.classList.add('port-dot', kind); edgePaths.appendChild(dot);}\n",
  "function drawPath(p1,p2,fromSide,toSide,kind,withArrow,label){const path=document.createElementNS('http://www.w3.org/2000/svg','path'); path.setAttribute('d',curvePath(p1,p2,fromSide,toSide)); if(withArrow){path.setAttribute('marker-end','url(#arrow)');} path.classList.add('edge-path', kind, 'active'); const title=document.createElementNS('http://www.w3.org/2000/svg','title'); title.textContent=label || ''; path.appendChild(title); edgePaths.appendChild(path);}\n",
  "function edgeSpacing(side, edgeCount){if(side==='top' || side==='bottom'){return 42;} return edgeCount > 1 ? 44 : 28;}\n",
  "function edgeSortValue(edge,side){const selectedIsSource=edge.from===selectedNodeId; const otherCard=cardFor(selectedIsSource ? edge.to : edge.from); if(!otherCard){return 0;} const center=cardCenter(otherCard); return (side==='left' || side==='right') ? center.y : center.x;}\n",
  "function orderedEdges(edgesForSide,side){return [...edgesForSide].sort((a,b)=>edgeSortValue(a,side)-edgeSortValue(b,side));}\n",
  "function drawEdges(){edgePaths.innerHTML=''; const relatedEdges=edges.filter((edge)=>edge.from===selectedNodeId || edge.to===selectedNodeId).filter((edge)=>{const fromNode=nodeById[edge.from]; const toNode=nodeById[edge.to]; return isVisibleInView(fromNode) || isVisibleInView(toNode);}); const grouped={left:[],right:[],top:[],bottom:[]}; relatedEdges.forEach((edge)=>grouped[selectedSideFor(edge)].push(edge)); ['left','right','top','bottom'].forEach((side)=>{const edgesForSide=orderedEdges(grouped[side],side); edgesForSide.forEach((edge,index)=>{const fromCard=cardFor(edge.from); const toCard=cardFor(edge.to); if(!fromCard || !toCard){return;} const spacing=edgeSpacing(side,edgesForSide.length); const offset=(index-(edgesForSide.length-1)/2)*spacing; const selectedIsSource=edge.from===selectedNodeId; const fromSide=selectedIsSource ? side : oppositeSide(side); const toSide=selectedIsSource ? oppositeSide(side) : side; const p1=selectedIsSource ? sidePoint(fromCard,fromSide,offset) : sidePoint(fromCard,fromSide,0); const p2=selectedIsSource ? sidePoint(toCard,toSide,0) : sidePoint(toCard,toSide,offset); const selectedPort=selectedIsSource ? p1 : p2; drawPath(p1,p2,fromSide,toSide,edge.kind,true,edge.label); addPortDot(selectedPort, edge.kind);});});}\n",
  "document.querySelectorAll('.node-card').forEach((card)=>card.addEventListener('click',()=>selectNode(card.dataset.nodeId)));\n",
  "focusSelect.addEventListener('change',(event)=>selectNode(event.target.value));\n",
  "populateSelect();\n",
  "graphWrap.addEventListener('scroll',()=>window.requestAnimationFrame(drawEdges));\n",
  "window.addEventListener('resize',()=>window.requestAnimationFrame(drawEdges));\n",
  "selectNode(selectedNodeId);\n",
  "setTimeout(drawEdges, 50);\n",
  "</script>\n",
  "</body>\n",
  "</html>\n"
)

  output_dir <- dirname(output)
  dir.create(output_dir, showWarnings = FALSE, recursive = TRUE)

  writeLines(html, output, useBytes = TRUE)

  output_path <- normalizePath(output)

  if (isTRUE(open)) {
    utils::browseURL(output_path)
  }

  list(
    model_label = model_spec$metadata$model_label,
    selected_components = model_spec$modules[, c("model_role", "component")],
    n_nodes = length(nodes),
    n_edges = length(edges),
    output = output_path
  )
}

create_model_inspector <- function(
    parameters = NULL,
    data = NULL,
    output = NULL,
    open = interactive(),
    spec = NULL,
    model_graph = NULL,
    title = "FIMS User Model Explorer Prototype",
    model_label = "User model",
    setup_default_if_missing = TRUE,
    field_definition_mode = c("toggle", "hover")) {
  field_definition_mode <- match.arg(field_definition_mode)

  if (!is.null(spec) && !is.null(model_graph)) {
    stop("Provide either `spec` or `model_graph`, not both.")
  }

  if (is.null(output)) {
    output <- default_model_inspector_output(model_label = model_label)
  }

  if (is.null(spec)) {
    if (!is.null(model_graph)) {
      spec <- build_model_inspector_spec_from_graph(
        model_graph = model_graph,
        title = title,
        model_label = model_label
      )
    } else {
      spec <- build_model_inspector_spec(
        parameters = parameters,
        data = data,
        title = title,
        model_label = model_label,
        setup_default_if_missing = setup_default_if_missing
      )
    }
  }

  render_model_inspector(
    model_spec = spec,
    output = output,
    open = open,
    field_definition_mode = field_definition_mode
  )
}

inspect_fims_model <- function(
    data,
    parameters = NULL,
    model_label = "User model",
    output = NULL,
    open = interactive(),
    title = "FIMS User Model Explorer Prototype",
    setup_default_if_missing = TRUE,
    field_definition_mode = c("toggle", "hover")) {
  field_definition_mode <- match.arg(field_definition_mode)

  create_model_inspector(
    data = data,
    parameters = parameters,
    output = output,
    open = open,
    title = title,
    model_label = model_label,
    setup_default_if_missing = setup_default_if_missing,
    field_definition_mode = field_definition_mode
  )
}

make_demo_parameters <- function(
    growth_type = "VonBertalanffySchnute",
    include_growth = TRUE,
    include_maturity = TRUE,
    include_recruitment = TRUE,
    include_selectivity = TRUE) {
  module_names <- c("Growth", "Maturity", "Recruitment", "Selectivity")
  module_types <- c(growth_type, "Logistic", "BevertonHolt", "Logistic")
  include_module <- c(
    include_growth,
    include_maturity,
    include_recruitment,
    include_selectivity
  )

  data.frame(
    module_name = module_names[include_module],
    module_type = module_types[include_module],
    stringsAsFactors = FALSE
  )
}

make_demo_data <- function(data_types) {
  data.frame(
    type = data_types,
    stringsAsFactors = FALSE
  )
}

model_inspector_demo_configurations <- function() {
  list(
    full_age_length_index = list(
      label = "Full age-length-index example",
      file = "01_full_age_length_index.html",
      description = paste(
        "Includes catch, age composition, length composition, and index data.",
        "This shows the broadest current prototype graph."
      ),
      parameters = make_demo_parameters(),
      data = make_demo_data(c("catch", "age_comp", "length_comp", "index"))
    ),
    no_index_age_length = list(
      label = "No-index age-length example",
      file = "02_no_index_age_length.html",
      description = paste(
        "Removes the index data stream while keeping the main biological",
        "and observation modules."
      ),
      parameters = make_demo_parameters(),
      data = make_demo_data(c("catch", "age_comp", "length_comp"))
    ),
    empirical_weight_at_age = list(
      label = "Empirical weight-at-age example",
      file = "03_empirical_weight_at_age.html",
      description = paste(
        "Switches Growth to EWAA and uses empirical weight-at-age data",
        "instead of length-composition-driven growth."
      ),
      parameters = make_demo_parameters(growth_type = "EWAA"),
      data = make_demo_data(c("catch", "age_comp", "weight_at_age"))
    ),
    growth_only = list(
      label = "Growth-only example",
      file = "04_growth_only.html",
      description = paste(
        "Keeps only a growth component and its empirical weight-at-age input.",
        "This is the smallest example for showing how the graph shrinks."
      ),
      parameters = make_demo_parameters(
        growth_type = "EWAA",
        include_maturity = FALSE,
        include_recruitment = FALSE,
        include_selectivity = FALSE
      ),
      data = make_demo_data("weight_at_age")
    )
  )
}

write_model_inspector_example_index <- function(example_summary, output) {
  cards <- vapply(seq_len(nrow(example_summary)), function(i) {
    paste0(
      "<article class='example-card'>",
      "<h2><a href='", html_escape(example_summary$file[[i]]), "'>",
      html_escape(example_summary$label[[i]]),
      "</a></h2>",
      "<p>", html_escape(example_summary$description[[i]]), "</p>",
      "<dl>",
      "<dt>Data</dt><dd>", html_escape(example_summary$data_types[[i]]), "</dd>",
      "<dt>Modules</dt><dd>", html_escape(example_summary$modules[[i]]), "</dd>",
      "<dt>Graph size</dt><dd>",
      html_escape(example_summary$n_nodes[[i]]),
      " nodes, ",
      html_escape(example_summary$n_edges[[i]]),
      " edges</dd>",
      "</dl>",
      "</article>"
    )
  }, character(1))

  html <- paste0(
    "<!doctype html>\n",
    "<html lang='en'>\n",
    "<head>\n",
    "<meta charset='utf-8'>\n",
    "<meta name='viewport' content='width=device-width, initial-scale=1'>\n",
    "<title>FIMS model inspector examples</title>\n",
    "<style>\n",
    ":root{color-scheme:light;--ink:#153847;--muted:#637985;",
    "--line:#d6e1e6;--teal:#0c7280;--wash:#f6fafb;}\n",
    "body{margin:0;font-family:Segoe UI,Arial,sans-serif;color:var(--ink);",
    "background:linear-gradient(180deg,#f8fbfc,#eef6f8);}\n",
    "main{max-width:1180px;margin:0 auto;padding:40px 28px;}\n",
    "h1{margin:0 0 8px;font-size:42px;color:var(--teal);}\n",
    ".lead{margin:0 0 28px;color:var(--muted);font-size:19px;line-height:1.45;}\n",
    ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(270px,1fr));gap:18px;}\n",
    ".example-card{background:white;border:1px solid var(--line);border-radius:10px;",
    "padding:22px;box-shadow:0 14px 34px rgba(21,56,71,.07);}\n",
    ".example-card h2{margin:0 0 10px;font-size:22px;line-height:1.2;}\n",
    "a{color:var(--teal);text-decoration-thickness:2px;text-underline-offset:3px;}\n",
    "p{color:var(--muted);line-height:1.45;}\n",
    "dl{display:grid;grid-template-columns:82px 1fr;gap:8px 12px;margin:18px 0 0;}\n",
    "dt{font-weight:700;color:#4f6874;}dd{margin:0;color:var(--ink);}\n",
    "</style>\n",
    "</head>\n",
    "<body>\n",
    "<main>\n",
    "<h1>FIMS model inspector examples</h1>\n",
    "<p class='lead'>These examples use the same prototype inspector with different ",
    "mock model configurations so the graph can be compared across scenarios.</p>\n",
    "<section class='grid'>\n",
    paste(cards, collapse = "\n"),
    "\n</section>\n",
    "</main>\n",
    "</body>\n",
    "</html>\n"
  )

  writeLines(html, output, useBytes = TRUE)
  normalizePath(output)
}

create_model_inspector_demo_examples <- function(
    output_dir = file.path(
      "visualization-prototypes",
      "examples",
      "user-model-inspector"
    ),
    open_index = interactive()) {
  dir.create(output_dir, showWarnings = FALSE, recursive = TRUE)
  configurations <- model_inspector_demo_configurations()
  configuration_names <- names(configurations)

  example_summary <- do.call(rbind, lapply(configuration_names, function(name) {
    configuration <- configurations[[name]]
    output_path <- file.path(output_dir, configuration$file)
    inspector_summary <- create_model_inspector(
      parameters = configuration$parameters,
      data = configuration$data,
      output = output_path,
      open = FALSE,
      model_label = configuration$label
    )

    data.frame(
      name = name,
      label = configuration$label,
      description = configuration$description,
      data_types = paste(configuration$data$type, collapse = ", "),
      modules = paste(
        paste(
          configuration$parameters$module_name,
          configuration$parameters$module_type,
          sep = ": "
        ),
        collapse = ", "
      ),
      n_nodes = inspector_summary$n_nodes,
      n_edges = inspector_summary$n_edges,
      file = configuration$file,
      output = inspector_summary$output,
      stringsAsFactors = FALSE
    )
  }))

  index_path <- write_model_inspector_example_index(
    example_summary = example_summary,
    output = file.path(output_dir, "index.html")
  )

  if (isTRUE(open_index)) {
    utils::browseURL(index_path)
  }

  list(
    index = index_path,
    examples = example_summary
  )
}

script_was_sourced <- function() {
  any(vapply(sys.calls(), function(call) {
    if (!is.call(call)) {
      return(FALSE)
    }

    identical(as.character(call[[1]])[[1]], "source")
  }, logical(1)))
}

if (!script_was_sourced()) {
  inspector_summary <- create_model_inspector(
    spec = example_model_spec,
    output = file.path(
      "visualization-prototypes",
      "examples",
      "user-model-inspector",
      "fims_user_model_inspector_example.html"
    ),
    open = FALSE
  )

  print(inspector_summary)

  invisible(inspector_summary)
}
