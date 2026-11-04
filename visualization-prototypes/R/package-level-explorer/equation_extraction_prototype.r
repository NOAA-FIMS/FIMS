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
  trimws(x)
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

    paste(collected, collapse = "\n")
  })

  unique(unlist(equations))
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

    equation_format <- "latex"
    if (length(math_matches) == 0) {
      plain_equations <- extract_plain_equations(comment_text)

      if (length(plain_equations) > 0) {
        math_matches <- plain_equations
        equation_format <- "plain"
      }
    }

    if (length(math_matches) == 0) {
      next
    }

    entity_info <- infer_entity(lines, block$end_line)

    # Keep array-storage indexing in source documentation, not the
    # scientific equation catalogue.
    if (basename(path) == "growth_products.hpp" &&
        identical(entity_info$entity, "GrowthProducts")) {
      next
    }

    enclosing_class <- infer_enclosing_class(lines, block$start_line)
    r_visible_name <- guess_r_visible_name(entity_info$entity, path)

    if (is.na(r_visible_name) &&
        !is.na(enclosing_class) &&
        identical(entity_info$entity_type, "function")) {
      r_visible_name <- guess_r_visible_name(enclosing_class, path)
    }

    # Reuse the documented brief; do not invent descriptions for untagged blocks.
    brief_match <- regexec(
      "(?s)@brief\\s+(.*?)(?=\\n\\s*\\n|\\n\\s*@|$)",
      comment_text,
      perl = TRUE
    )
    brief_parts <- regmatches(comment_text, brief_match)[[1]]
    description <- if (length(brief_parts) > 1) {
      trimws(gsub("\\s+", " ", brief_parts[[2]]))
    } else {
      ""
    }

    # Preserve an explicitly documented "where:" definition list.
    definitions_match <- regexec(
      "(?s)(?:^|\\n)where:[ \\t]*\\n(.*?)(?=\\n\\s*\\n|\\n\\s*@|$)",
      comment_text,
      perl = TRUE
    )
    definitions_parts <- regmatches(comment_text, definitions_match)[[1]]
    symbol_definitions <- if (length(definitions_parts) > 1) {
      trimws(definitions_parts[[2]])
    } else {
      ""
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
        equation_format = equation_format,
        description = description,
        symbol_definitions = symbol_definitions,
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
      equation_format = character(),
      description = character(),
      symbol_definitions = character(),
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

    # Change display labels only; keep the original identifiers in the footer.
    card_title <- gsub("([A-Z]+)([A-Z][a-z])", "\\1 \\2", card_title)
    card_title <- gsub("([a-z0-9])([A-Z])", "\\1 \\2", card_title)
    card_title <- gsub("_", " ", card_title, fixed = TRUE)
    if (identical(row[["display_name"]], "AgeSpecificSelectivity") &&
        identical(entity, "AgeSpecificSelectivity")) {
      card_title <- "Age-specific selectivity"
    }
    description <- if ("description" %in% names(row)) row[["description"]] else ""
    description_html <- if (!is.na(description) && nzchar(description)) {
      paste0("<p class='meaning'>", html_escape(description), "</p>")
    } else {
      ""
    }
    source_url <- paste0(
      "https://github.com/NOAA-FIMS/FIMS/blob/main/",
      row[["source_file"]], "#L", trimws(row[["line_start"]])
    )

    equation <- row[["equation"]]
    # Only explicitly extracted LaTeX is typeset; plain formulas stay literal.
    equation_format <- if ("equation_format" %in% names(row)) {
      row[["equation_format"]]
    } else {
      "plain"
    }
    equation_html <- if (identical(equation_format, "latex")) {
      paste0("\\[", html_escape(equation), "\\]")
    } else {
      paste0("<pre><code>", html_escape(equation), "</code></pre>")
    }
    symbol_definitions <- if ("symbol_definitions" %in% names(row)) {
      row[["symbol_definitions"]]
    } else {
      ""
    }

    definitions_html <- ""
    if (!is.na(symbol_definitions) && nzchar(symbol_definitions)) {
      definition_items <- strsplit(
        trimws(symbol_definitions),
        "\\n(?=\\s*- )",
        perl = TRUE
      )[[1]]

      # Join wrapped source lines and remove the source bullet markers.
      definition_items <- trimws(gsub("\\s+", " ", definition_items))
      definition_items <- sub("^-\\s+", "", definition_items)

      # Escape source text before adding the supported display markup.
      definition_items <- html_escape(definition_items)
      definition_items <- gsub(
        "\\\\f\\$(.*?)\\\\f\\$",
        "\\\\(\\1\\\\)",
        definition_items,
        perl = TRUE
      )
      definition_items <- gsub(
        "`([^`]+)`",
        "<code>\\1</code>",
        definition_items,
        perl = TRUE
      )

      definitions_html <- paste0(
        "<ul class='meaning definitions'>",
        paste0("<li>", definition_items, "</li>", collapse = ""),
        "</ul>"
      )
    }

    # Search the source text rather than MathJax's generated markup.
    search_values <- row[c(
      "display_name", "module", "entity", "enclosing_class", "r_visible_name",
      "equation", "source_file"
    )]
    search_values <- c(
      search_values, card_title, description, symbol_definitions
    )
    search_text <- paste(search_values[!is.na(search_values)], collapse = " ")

    paste0(
      '<article class="equation-card" data-search="',
      html_escape(search_text), '" data-module="',
      html_escape(ifelse(is.na(row[["module"]]), "", row[["module"]])), '">',
      "<div class='equation-meta'>",
      '<button class="module-badge" type="button" aria-label="Filter by module: ',
      html_escape(row[["module"]]), '">',
      html_escape(row[["module"]]), "</button>",
      "</div>",
      "<h2>", html_escape(card_title), "</h2>",
      description_html,
      "<div class='equation'>", equation_html, "</div>",
      "<footer class='source'>",
      '<a title="View current main-branch source; line locations may differ" href="',
      html_escape(source_url), '">',
      html_escape(basename(row[["source_file"]])), " &middot; line ",
      html_escape(trimws(row[["line_start"]])), "</a>",
      "<details><summary>Source details</summary>",
      "<p><code>", html_escape(row[["source_file"]]), "</code><br>",
      html_escape(row[["source_kind"]]), "<br><code>",
      html_escape(entity_label), "</code></p>",
      "<p>Equation source text</p><pre><code>",
      html_escape(equation), "</code></pre></details>",
      "</footer>",
      "</article>"
    )
  })

  html <- paste0(
    "<!doctype html>",
    "<html lang='en'>",
    "<head>",
    "<meta charset='utf-8'>",
    "<meta name='viewport' content='width=device-width, initial-scale=1'>",
    "<title>Equation catalog | FIMS</title>",
    "<link rel='stylesheet' href='https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&amp;display=swap'>",
    "<script>",
    "window.MathJax = { tex: { inlineMath: [['\\\\(','\\\\)']], displayMath: [['\\\\[','\\\\]']] }, chtml: { scale: 1, matchFontHeight: false } };",
    "</script>",
    "<script async src='https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js'></script>",
    "<style>",
    "body{font-family:Inter,system-ui,-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;",
    "margin:0;padding:2rem;color:#213f4b;background:#ffffff;}",
    "a{color:#0e6876;}",
    "h1{color:#0e6876;font-size:2.25rem;margin:0 0 .5rem 0;}",
    ".product-name{color:#0e6876;font-size:.85rem;font-weight:700;letter-spacing:.12em;margin:0 0 .35rem;}",
    ".catalog-header{border-bottom:1px solid #cfd5d8;padding-bottom:1rem;margin-bottom:1rem;}",
    ".subtitle{color:#4a636c;margin:0 0 .75rem;}",
    ".catalog-info{color:#4a636c;margin:.4rem 0;font-size:.95rem;}",
    ".catalog-info h2{color:#0e6876;font-size:.95rem;font-weight:600;margin:0 0 .2rem;}",
    ".catalog-info p{max-width:900px;line-height:1.5;margin:0 0 .5rem;}",
    ".version-notice{width:fit-content;max-width:100%;box-sizing:border-box;background:#EFF4F6;border-left:3px solid #0E6876;padding:.75rem 1rem;margin:1rem 0;color:#213F4B;font-size:.85rem;line-height:1.5;overflow-wrap:anywhere;}",
    ".version-notice p{margin:0;}",
    ".version-notice strong{display:block;margin-bottom:.15rem;}",
    ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(360px,1fr));grid-auto-rows:1fr;gap:1rem;align-items:stretch;}",
    ".equation-card{display:flex;flex-direction:column;min-width:0;border:1px solid #cfd5d8;border-radius:8px;padding:1rem;background:#fff;}",
    ".equation-card[hidden]{display:none;}",
    ".equation-card h2{font-size:1.1rem;line-height:1.35;color:#0e6876;margin:.4rem 0 .4rem;}",
    ".equation-meta{display:flex;gap:.5rem;flex-wrap:wrap;}",
    ".module-badge{display:inline-flex;align-items:center;padding:.2rem .6rem;border:0;border-radius:999px;font:inherit;font-size:.8rem;font-weight:600;line-height:1.4;color:#FFFFFF;background:var(--module-badge-color,#213F4B);cursor:pointer;}",
    ".module-badge:hover{text-decoration:underline;}",
    ".module-badge:focus-visible{outline:3px solid #213F4B;outline-offset:3px;}",
    ".equation-card[data-module='Selectivity']{--module-badge-color:#0072B2;}",
    ".equation-card[data-module='Recruitment']{--module-badge-color:#875D00;}",
    ".equation-card[data-module='Model']{--module-badge-color:#007653;}",
    ".equation-card[data-module='Distribution']{--module-badge-color:#B34D00;}",
    ".equation-card[data-module='Common math']{--module-badge-color:#236C91;}",
    ".equation-card[data-module='Growth']{--module-badge-color:#9B4678;}",
    ".equation-card[data-module='Maturity']{--module-badge-color:#716800;}",
    ".meaning{color:#213f4b;font-size:.95rem;line-height:1.45;margin:.35rem 0 .65rem;}",
    ".definitions code{overflow-wrap:anywhere;}",
    ".equation{overflow-x:auto;padding:.35rem .65rem;background:#FFFFFF;border-radius:6px;font-size:1rem;}",
    ".equation mjx-container[display='true']{margin:.5em 0 !important;}",
    ".equation pre{margin:.5em 0;font-size:.9rem;line-height:1.5;white-space:pre;}",
    ".source{font-size:.8rem;color:#60757e;margin-top:auto;padding-top:.65rem;overflow-wrap:anywhere;}",
    ".source details{margin-top:.3rem;}.source summary{cursor:pointer;width:fit-content;}",
    ".source p{margin:.35rem 0 0;line-height:1.45;}",
    ".source pre{white-space:pre-wrap;overflow-wrap:anywhere;margin:.35rem 0 0;}",
    ".source :focus-visible{outline:3px solid #0e6876;outline-offset:2px;}",
    ".search-controls{margin-bottom:1.25rem;}",
    ".search-controls label{display:block;font-weight:600;margin-bottom:.5rem;}",
    ".search-row{display:grid;grid-template-columns:minmax(0,12fr) minmax(0,5fr) auto;gap:.75rem;align-items:end;}",
    ".search-field{min-width:0;}",
    ".search-row input,.search-row select{box-sizing:border-box;width:100%;height:2.75rem;padding:.6rem;font:inherit;",
    "border:1px solid #60757e;border-radius:4px;color:inherit;background:#ffffff;}",
    ".search-row :focus-visible{outline:3px solid #0e6876;outline-offset:2px;}",
    ".search-row button{height:2.75rem;padding:.6rem .25rem;font:inherit;cursor:pointer;border:0;background:transparent;color:#0e6876;text-decoration:underline;}",
    ".search-controls p{color:#4a636c;margin:.5rem 0;}",
    "@media(max-width:640px){body{padding:1rem;}.search-row{grid-template-columns:1fr;}.search-row button{justify-self:start;}.grid{grid-template-columns:1fr;grid-auto-rows:auto;}}",
    "</style>",
    "</head>",
    "<body>",
    "<header class='catalog-header'>",
    "<h1>Equation Catalog</h1>",
    "<p class='subtitle'>Browse equations by module and explore their source documentation.</p>",

    "<section class='catalog-info'><h2>About this catalog</h2>",
    "<p>Equations are extracted from Doxygen-style source comments in the FIMS repository ",
    "and associated with components using nearby class and function names. ",
    "Each card includes its source file and line location.</p></section>",

    "<section class='catalog-info'><h2>Why do some equations appear more than once?</h2>",
    "<p>Some formulas appear more than once because the same ",
    "mathematical relationship can be documented at multiple levels, such as ",
    "a generic math helper and the module that calls it, or as scalar and ",
    "time-varying overloads of the same method.</p></section>",

    "<aside class='version-notice' aria-labelledby='version-notice-label'>",
    "<strong id='version-notice-label'>Version information</strong>",
    "<p>This catalog corresponds to the FIMS version shown above. ",
    "GitHub’s <code>main</code> branch may differ. ",
    "Check your installed version with <code>packageVersion(\"FIMS\")</code>. ",
    "<a href='https://noaa-fims.github.io/FIMS/#installation'>",
    "Installation instructions</a>.</p>",
    "</aside>",

    "</header>",
      "<div class='search-controls'>",
      "<div class='search-row'>",
      "<div class='search-field'>",
      "<label for='equation-search'>Search equations</label>",
      "<input id='equation-search' type='search' ",
      "placeholder='Name, module, or equation text'>",
      "</div>",
      "<div class='search-field'>",
      "<label for='module-filter'>Module</label>",
      "<select id='module-filter'><option value=''>All modules</option></select>",
      "</div>",
      "<button id='clear-search' type='button'>Reset filters</button>",
      "</div>",
      "<p id='search-count' role='status' aria-live='polite'></p>",
      "</div>",
      "<p id='search-empty' hidden>No matching equations. Try another search or module.</p>",
      "<section class='grid'>",
      paste(rows_html, collapse = "\n"),
      "</section>",
      "<script>\n",
      "const searchInput = document.getElementById('equation-search');\n",
      "const moduleFilter = document.getElementById('module-filter');\n",
      "const cards = Array.from(document.querySelectorAll('.equation-card'));\n",
      "const modules = [...new Set(cards.map(card => card.dataset.module).filter(Boolean))].sort((a,b) => a.localeCompare(b));\n",
      "modules.forEach(module => {\n",
      "  const option = document.createElement('option');\n",
      "  option.value = module; option.textContent = module; moduleFilter.appendChild(option);\n",
      "});\n",
      "const searchRecords = cards.map(card => ({card, text: card.dataset.search.toLowerCase()}));\n",
      "function filterEquations(){\n",
      "  const terms = searchInput.value.trim().toLowerCase().split(/\\s+/).filter(Boolean);\n",
      "  let matches = 0;\n",
      "  searchRecords.forEach(({card, text}) => {\n",
      "    const visible = (!moduleFilter.value || card.dataset.module === moduleFilter.value) && terms.every(term => text.includes(term));\n",
      "    card.hidden = !visible;\n",
      "    if(visible) matches++;\n",
      "  });\n",
      "  document.getElementById('search-count').textContent = (terms.length || moduleFilter.value) ? `${matches} of ${cards.length} equations` : `${cards.length} equations`;\n",
      "  document.getElementById('search-empty').hidden = matches !== 0;\n",
      "}\n",
      "searchInput.addEventListener('input', filterEquations);\n",
      "moduleFilter.addEventListener('change', filterEquations);\n",
      "cards.forEach(card => {\n",
      "  card.querySelector('.module-badge').addEventListener('click', () => {\n",
      "    moduleFilter.value = card.dataset.module; filterEquations();\n",
      "  });\n",
      "});\n",
      "document.getElementById('clear-search').addEventListener('click', () => {\n",
      "  searchInput.value = ''; moduleFilter.value = ''; filterEquations(); searchInput.focus();\n",
      "});\n",
      "filterEquations();\n",
      "</script>",
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
# 2. Save outputs (configurable for website builds)
# -------------------------------------------------------------------------

default_data_output_dir <- file.path("visualization-prototypes", "data")
default_example_output_dir <- file.path(
  "visualization-prototypes",
  "examples",
  "package-level-explorer"
)

csv_path <- Sys.getenv(
  "FIMS_EQ_CSV_OUT",
  unset = file.path(default_data_output_dir, "fims_equation_catalog.csv")
)

html_path <- Sys.getenv(
  "FIMS_EQ_HTML_OUT",
  unset = file.path(default_example_output_dir, "fims_equation_catalog.html")
)

dir.create(dirname(csv_path), showWarnings = FALSE, recursive = TRUE)
dir.create(dirname(html_path), showWarnings = FALSE, recursive = TRUE)

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
