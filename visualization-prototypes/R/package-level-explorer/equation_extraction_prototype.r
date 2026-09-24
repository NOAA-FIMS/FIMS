# -------------------------------------------------------------------------
# 0. Setup
# -------------------------------------------------------------------------
# Purpose:
# Extract equations from FIMS source/documentation comments and render a small
# equation catalog that can later be connected to the package-level explorer.
#
# This is a prototype script. It does not modify FIMS source code.
#
# Run from the FIMS repository root.

if (!file.exists("DESCRIPTION") ||
    !dir.exists("inst") ||
    !dir.exists("R")) {
  stop("Run this script from the FIMS repository root.")
}

html_escape <- function(x) {
  x <- gsub("&", "&amp;", x, fixed = TRUE)
  x <- gsub("<", "&lt;", x, fixed = TRUE)
  x <- gsub(">", "&gt;", x, fixed = TRUE)
  x <- gsub('"', "&quot;", x, fixed = TRUE)
  x
}

clean_comment_text <- function(lines) {
  lines <- gsub("^\\s*/\\*\\*?", "", lines)
  lines <- gsub("\\*/\\s*$", "", lines)
  lines <- gsub("^\\s*\\*\\s?", "", lines)
  lines <- gsub("^\\s*#'\\s?", "", lines)
  lines <- gsub("^\\s*//\\s?", "", lines)
  paste(lines, collapse = "\n")
}

extract_regex_matches <- function(text, pattern) {
  matches <- gregexpr(pattern, text, perl = TRUE)[[1]]

  if (identical(matches, -1L)) {
    return(character())
  }

  regmatches(text, list(matches))[[1]]
}

is_equation_like <- function(x) {
  x <- strip_math_delimiters(x)
  nchar(x) >= 12 &
    grepl(
      paste(
        c(
          "=",
          "\\\\frac",
          "\\\\sum",
          "\\\\exp",
          "\\\\log",
          "\\\\sqrt",
          "\\\\left",
          "\\\\begin",
          "\\\\mathrel",
          "\\\\times",
          "/"
        ),
        collapse = "|"
      ),
      x,
      perl = TRUE
    )
}

strip_math_delimiters <- function(x) {
  x <- gsub("^\\\\f\\[\\s*", "", x, perl = TRUE)
  x <- gsub("\\s*\\\\f\\]$", "", x, perl = TRUE)
  x <- gsub("^\\\\f\\$\\s*", "", x, perl = TRUE)
  x <- gsub("\\s*\\\\f\\$$", "", x, perl = TRUE)
  x <- gsub("^\\$\\$\\s*", "", x, perl = TRUE)
  x <- gsub("\\s*\\$\\$$", "", x, perl = TRUE)
  trimws(gsub("\\s+", " ", x))
}

escape_plain_identifier_underscores <- function(equation) {
  # Plain C++/R identifier names like mean_length_young are easier to read as
  # literal identifiers than as accidental TeX subscripts.
  pattern <- "[A-Za-z][A-Za-z0-9]*(?:_[A-Za-z0-9]+)+"
  matches <- gregexpr(pattern, equation, perl = TRUE)[[1]]

  if (identical(matches, -1L)) {
    return(equation)
  }

  identifiers <- regmatches(equation, list(matches))[[1]]
  escaped_identifiers <- gsub("_", "\\_", identifiers, fixed = TRUE)
  regmatches(equation, list(matches)) <- list(escaped_identifiers)
  equation
}

extract_plain_equations <- function(comment_text) {
  lines <- unlist(strsplit(comment_text, "\n", fixed = TRUE))
  starts <- which(
    grepl("=", lines, fixed = TRUE) &
      grepl("[A-Za-z][A-Za-z0-9_]*\\s*\\(|exp\\(|log\\(|\\^|\\*|/", lines)
  )

  starts <- starts[
    !grepl("^\\s*@", lines[starts]) &
      !grepl("^\\s*#include", lines[starts]) &
      !grepl("^\\s*-", lines[starts]) &
      !grepl("^\\s*(and|this|for|where|users)\\b", lines[starts], ignore.case = TRUE) &
      !grepl("target=|href=|<a\\s", lines[starts]) &
      !grepl("with Type =", lines[starts], fixed = TRUE) &
      !grepl("`", lines[starts], fixed = TRUE)
  ]

  if (length(starts) == 0) {
    return(character())
  }

  equations <- lapply(starts, function(start) {
    collected <- character()

    for (i in start:length(lines)) {
      line <- trimws(lines[[i]])

      if (nchar(line) == 0 ||
          grepl("^@", line) ||
          grepl("target=|href=|<a\\s", line) ||
          grepl("`", line, fixed = TRUE)) {
        break
      }

      collected <- c(collected, line)
    }

    paste(collected, collapse = " ")
  })

  unique(vapply(
    unlist(equations),
    escape_plain_identifier_underscores,
    character(1)
  ))
}

infer_module <- function(path) {
  path <- gsub("\\\\", "/", path)

  if (grepl("/growth/", path)) {
    return("Growth")
  }
  if (grepl("/maturity/", path)) {
    return("Maturity")
  }
  if (grepl("/selectivity/", path)) {
    return("Selectivity")
  }
  if (grepl("/recruitment/", path)) {
    return("Recruitment")
  }
  if (grepl("/distributions/", path)) {
    return("Distribution")
  }
  if (grepl("/common/", path)) {
    return("Common math")
  }
  if (grepl("/models/", path)) {
    return("Model")
  }
  if (grepl("^vignettes/", path)) {
    return("Vignette")
  }
  if (grepl("^R/", path)) {
    return("R")
  }

  "Other"
}

infer_source_kind <- function(path) {
  extension <- tools::file_ext(path)

  if (extension %in% c("hpp", "cpp", "h")) {
    return("C++ source comment")
  }
  if (extension %in% c("R", "r")) {
    return("R/Roxygen comment")
  }
  if (extension %in% c("Rmd", "qmd", "md")) {
    return("Markdown/vignette")
  }

  "Other"
}

normalize_signature <- function(lines) {
  lines <- trimws(lines)
  lines <- lines[nchar(lines) > 0]
  lines <- lines[!grepl("^//", lines)]
  lines <- lines[!grepl("^/\\*", lines)]
  lines <- lines[!grepl("^\\*", lines)]
  lines <- lines[!grepl("^template\\s*<", lines)]

  if (length(lines) == 0) {
    return(NA_character_)
  }

  signature <- paste(lines[seq_len(min(6, length(lines)))], collapse = " ")
  trimws(signature)
}

infer_entity <- function(lines, block_end) {
  lookahead <- lines[(block_end + 1):min(length(lines), block_end + 12)]
  signature <- normalize_signature(lookahead)

  if (is.na(signature)) {
    return(list(entity = NA_character_, entity_type = NA_character_))
  }

  class_match <- regexec("\\b(struct|class)\\s+([A-Za-z_][A-Za-z0-9_]*)", signature)
  class_hit <- regmatches(signature, class_match)[[1]]

  if (length(class_hit) == 3) {
    return(list(entity = class_hit[3], entity_type = class_hit[2]))
  }

  function_match <- regexec(
    "([A-Za-z_][A-Za-z0-9_:<>]*)\\s*\\(",
    signature,
    perl = TRUE
  )
  function_hit <- regmatches(signature, function_match)[[1]]

  if (length(function_hit) == 2) {
    ignored <- c("if", "for", "while", "switch", "catch")

    if (!(function_hit[2] %in% ignored)) {
      return(list(entity = function_hit[2], entity_type = "function"))
    }
  }

  list(entity = NA_character_, entity_type = NA_character_)
}

infer_enclosing_class <- function(lines, block_start) {
  preceding_lines <- lines[seq_len(max(1, block_start - 1))]
  preceding_lines <- trimws(preceding_lines)
  preceding_lines <- preceding_lines[!grepl("^\\s*template\\s*<", preceding_lines)]
  preceding_lines <- preceding_lines[!grepl("^(/|\\*)", preceding_lines)]

  class_lines <- grep(
    "\\b(struct|class)\\s+([A-Za-z_][A-Za-z0-9_]*)",
    preceding_lines,
    perl = TRUE
  )

  if (length(class_lines) == 0) {
    return(NA_character_)
  }

  last_class_line <- preceding_lines[[utils::tail(class_lines, 1)]]
  class_match <- regexec(
    "\\b(struct|class)\\s+([A-Za-z_][A-Za-z0-9_]*)",
    last_class_line,
    perl = TRUE
  )
  class_hit <- regmatches(last_class_line, class_match)[[1]]

  if (length(class_hit) == 3) {
    return(class_hit[[3]])
  }

  NA_character_
}

guess_r_visible_name <- function(entity, path) {
  entity_lookup <- c(
    SRBevertonHolt = "BevertonHoltRecruitment",
    LogDevsRecruitment = "LogDevsRecruitmentProcess",
    LogRRecruitment = "LogRRecruitmentProcess",
    NormalLPDF = "DnormDistribution",
    LogNormalLPDF = "DlnormDistribution",
    MultinomialLPMF = "DmultinomDistribution"
  )

  if (!is.na(entity) && entity %in% names(entity_lookup)) {
    return(unname(entity_lookup[entity]))
  }

  visible_names <- c(
    "AgeSpecificSelectivity",
    "BevertonHoltRecruitment",
    "DoubleLogisticSelectivity",
    "EWAAGrowth",
    "VonBertalanffySchnuteGrowth",
    "LogisticMaturity",
    "LogisticSelectivity"
  )

  if (!is.na(entity) && entity %in% visible_names) {
    return(entity)
  }

  if (grepl("fims_math\\.hpp$", path) && !is.na(entity)) {
    return(entity)
  }

  NA_character_
}

parse_comment_blocks <- function(lines) {
  start_lines <- grep("^\\s*/\\*\\*", lines)
  blocks <- list()

  for (start_line in start_lines) {
    end_candidates <- grep("\\*/", lines[start_line:length(lines)])

    if (length(end_candidates) == 0) {
      next
    }

    end_line <- start_line + end_candidates[[1]] - 1

    blocks[[length(blocks) + 1]] <- list(
      start_line = start_line,
      end_line = end_line,
      lines = lines[start_line:end_line]
    )
  }

  blocks
}

extract_equations_from_file <- function(path) {
  lines <- readLines(path, warn = FALSE)
  comment_blocks <- parse_comment_blocks(lines)
  rows <- list()

  for (block in comment_blocks) {
    comment_text <- clean_comment_text(block$lines)

    math_matches <- c(
      extract_regex_matches(comment_text, "(?s)\\\\f\\[.*?\\\\f\\]"),
      extract_regex_matches(comment_text, "(?s)\\\\f\\$.*?\\\\f\\$"),
      extract_regex_matches(comment_text, "(?s)\\$\\$.*?\\$\\$")
    )

    math_matches <- math_matches[
      vapply(math_matches, is_equation_like, logical(1))
    ]

    if (length(math_matches) == 0) {
      plain_equations <- extract_plain_equations(comment_text)

      if (length(plain_equations) > 0) {
        math_matches <- plain_equations
      }
    }

    if (length(math_matches) == 0) {
      next
    }

    entity_info <- infer_entity(lines, block$end_line)
    enclosing_class <- infer_enclosing_class(lines, block$start_line)
    r_visible_name <- guess_r_visible_name(entity_info$entity, path)

    if (is.na(r_visible_name) &&
        !is.na(enclosing_class) &&
        identical(entity_info$entity_type, "function")) {
      r_visible_name <- guess_r_visible_name(enclosing_class, path)
    }

    for (math_match in math_matches) {
      equation <- strip_math_delimiters(math_match)

      rows[[length(rows) + 1]] <- data.frame(
        source_file = path,
        line_start = block$start_line,
        line_end = block$end_line,
        source_kind = infer_source_kind(path),
        module = infer_module(path),
        entity = entity_info$entity,
        entity_type = entity_info$entity_type,
        enclosing_class = enclosing_class,
        r_visible_name = r_visible_name,
        equation = equation,
        stringsAsFactors = FALSE
      )
    }
  }

  if (length(rows) == 0) {
    return(data.frame(
      source_file = character(),
      line_start = integer(),
      line_end = integer(),
      source_kind = character(),
      module = character(),
      entity = character(),
      entity_type = character(),
      enclosing_class = character(),
      r_visible_name = character(),
      equation = character(),
      stringsAsFactors = FALSE
    ))
  }

  do.call(rbind, rows)
}

render_equation_catalog <- function(equation_catalog, path) {
  rows_html <- apply(equation_catalog, 1, function(row) {
    card_title <- row[["display_name"]]
    entity <- row[["entity"]]
    entity_type <- row[["entity_type"]]

    if (!is.na(entity) &&
        entity != "" &&
        !is.na(card_title) &&
        card_title != "" &&
        entity != card_title) {
      card_title <- paste0(card_title, ": ", entity)
    }

    entity_label <- ifelse(
      !is.na(entity_type) && entity_type != "",
      paste0(entity_type, ": ", entity),
      entity
    )

    equation <- row[["equation"]]
    is_display <- grepl("\\\\frac|\\\\Big|\\\\left|=", equation)
    equation_html <- if (is_display) {
      paste0("\\[", equation, "\\]")
    } else {
      paste0("\\(", equation, "\\)")
    }

    paste0(
      "<article class='equation-card'>",
      "<div class='equation-meta'>",
      "<span>", html_escape(row[["module"]]), "</span>",
      "<span>", html_escape(row[["source_kind"]]), "</span>",
      "</div>",
      "<h2>", html_escape(card_title), "</h2>",
      "<div class='entity'>",
      html_escape(entity_label),
      "</div>",
      "<div class='equation'>", equation_html, "</div>",
      "<div class='source'>",
      html_escape(row[["source_file"]]),
      ":",
      html_escape(row[["line_start"]]),
      "</div>",
      "</article>"
    )
  })

  html <- paste0(
    "<!doctype html>",
    "<html lang='en'>",
    "<head>",
    "<meta charset='utf-8'>",
    "<meta name='viewport' content='width=device-width, initial-scale=1'>",
    "<title>FIMS Equation Catalog Prototype</title>",
    "<script>",
    "window.MathJax = { tex: { inlineMath: [['\\\\(','\\\\)']], displayMath: [['\\\\[','\\\\]']] } };",
    "</script>",
    "<script async src='https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js'></script>",
    "<style>",
    "body{font-family:Outfit,system-ui,-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;",
    "margin:0;padding:2rem;color:#213f4b;background:#ffffff;}",
    "h1{color:#0e6876;font-size:2.25rem;margin:0 0 .5rem 0;}",
    ".subtitle{max-width:900px;color:#4a636c;margin-bottom:1.5rem;}",
    ".summary{display:flex;gap:.75rem;flex-wrap:wrap;margin:0 0 1.5rem 0;}",
    ".pill{border:1px solid #cfd5d8;border-radius:999px;padding:.25rem .75rem;background:#ebeeef;}",
    ".note{max-width:900px;color:#4a636c;background:#f8fafb;border-left:4px solid #0e6876;",
    "padding:.75rem 1rem;margin:0 0 1.5rem 0;}",
    ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(360px,1fr));gap:1rem;}",
    ".equation-card{border:1px solid #cfd5d8;border-radius:8px;padding:1rem;background:#fff;}",
    ".equation-card h2{font-size:1.25rem;color:#0e6876;margin:.5rem 0 .25rem 0;}",
    ".equation-meta{display:flex;gap:.5rem;flex-wrap:wrap;}",
    ".equation-meta span{font-size:.8rem;border-radius:999px;background:#ebeeef;padding:.15rem .55rem;}",
    ".entity{color:#4a636c;font-size:.95rem;margin-bottom:.75rem;}",
    ".equation{overflow-x:auto;padding:.75rem;background:#f8fafb;border-radius:6px;}",
    ".source{font-size:.85rem;color:#60757e;margin-top:.75rem;word-break:break-all;}",
    "</style>",
    "</head>",
    "<body>",
    "<h1>FIMS Equation Catalog Prototype</h1>",
    "<p class='subtitle'>Equations extracted from Doxygen-style source comments. ",
    "This is a first pass for connecting equations to the package-level explorer.",
    "</p>",
    "<div class='summary'>",
      "<span class='pill'>Equations: ", nrow(equation_catalog), "</span>",
      "<span class='pill'>Modules: ", length(unique(equation_catalog$module)), "</span>",
      "<span class='pill'>Files: ", length(unique(equation_catalog$source_file)), "</span>",
      "<span class='pill'>Sorted alphabetically</span>",
      "</div>",
      "<p class='note'>Some formulas appear more than once because the same ",
      "mathematical relationship can be documented at multiple levels, such as ",
      "a generic math helper and the module that calls it, or as scalar and ",
      "time-varying overloads of the same method.</p>",
      "<section class='grid'>",
      paste(rows_html, collapse = "\n"),
      "</section>",
    "</body></html>"
  )

  writeLines(html, path)
}

# -------------------------------------------------------------------------
# 1. Extract equation records
# -------------------------------------------------------------------------

source_files <- list.files(
  c("inst/include", "src", "R", "vignettes"),
  pattern = "\\.(hpp|h|cpp|R|Rmd)$",
  recursive = TRUE,
  full.names = TRUE
)

equation_catalog <- do.call(
  rbind,
  lapply(source_files, extract_equations_from_file)
)

equation_catalog <- equation_catalog[
  nchar(equation_catalog$equation) > 0,
]

equation_catalog$display_name <- ifelse(
  !is.na(equation_catalog$r_visible_name) &
    equation_catalog$r_visible_name != "",
  equation_catalog$r_visible_name,
  ifelse(
    !is.na(equation_catalog$enclosing_class) &
      equation_catalog$enclosing_class != "",
    equation_catalog$enclosing_class,
    equation_catalog$entity
  )
)

equation_catalog$display_name <- ifelse(
  is.na(equation_catalog$display_name) |
    equation_catalog$display_name == "",
  equation_catalog$module,
  equation_catalog$display_name
)

equation_catalog <- equation_catalog[
  order(
    tolower(equation_catalog$display_name),
    tolower(equation_catalog$module),
    equation_catalog$source_file,
    equation_catalog$line_start,
    equation_catalog$equation
  ),
]

row.names(equation_catalog) <- NULL

# -------------------------------------------------------------------------
# 2. Save outputs
# -------------------------------------------------------------------------

data_output_dir <- file.path("visualization-prototypes", "data")
example_output_dir <- file.path(
  "visualization-prototypes",
  "examples",
  "package-level-explorer"
)
dir.create(data_output_dir, showWarnings = FALSE, recursive = TRUE)
dir.create(example_output_dir, showWarnings = FALSE, recursive = TRUE)

csv_path <- file.path(data_output_dir, "fims_equation_catalog.csv")
html_path <- file.path(example_output_dir, "fims_equation_catalog.html")

utils::write.csv(equation_catalog, csv_path, row.names = FALSE)
render_equation_catalog(equation_catalog, html_path)

message("Saved equation CSV: ", normalizePath(csv_path))
message("Saved equation HTML: ", normalizePath(html_path))

equation_summary <- list(
  n_equations = nrow(equation_catalog),
  n_modules = length(unique(equation_catalog$module)),
  n_files = length(unique(equation_catalog$source_file)),
  equations_by_module = as.list(table(equation_catalog$module))
)

print(equation_summary)

invisible(equation_catalog)
