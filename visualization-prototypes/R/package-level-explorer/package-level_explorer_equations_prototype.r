# -------------------------------------------------------------------------
# 0. Setup
# -------------------------------------------------------------------------
# Purpose:
# Build a package-level FIMS explorer that combines:
# - R-side function dependencies from pkgnet
# - R-to-Doxygen links from R/Rcpp_exports.R
# - Rcpp module registration information from src/rcpp_*.cpp
#
# This is a prototype script. It does not modify FIMS source code.
#
# Run from the FIMS repository root.

if (!file.exists("DESCRIPTION") ||
    !file.exists(file.path("R", "Rcpp_exports.R")) ||
    !dir.exists("src")) {
  stop("Run this script from the FIMS repository root.")
}

library(pkgnet)
library(visNetwork)

if (!requireNamespace("htmlwidgets", quietly = TRUE)) {
  stop("Install the htmlwidgets package to export the graph to HTML.")
}

safe_extract_pkgnet_edges <- function(function_reporter, pkg_nodes, pkg_env) {
  pkgnet_edges <- tryCatch(
    as.data.frame(function_reporter$edges),
    error = function(error) error
  )

  if (!inherits(pkgnet_edges, "error")) {
    return(pkgnet_edges)
  }

  message(
    "pkgnet edge extraction failed: ",
    conditionMessage(pkgnet_edges)
  )
  message("Falling back to per-function edge extraction.")

  called_by <- getFromNamespace(".called_by", "pkgnet")
  all_functions <- pkg_nodes$node
  skipped_functions <- character()

  edge_rows <- lapply(all_functions, function(function_name) {
    result <- tryCatch(
      called_by(
        fname = function_name,
        all_functions = all_functions,
        pkg_env = pkg_env
      ),
      error = function(error) {
        skipped_functions <<- c(skipped_functions, function_name)
        NULL
      }
    )

    if (is.null(result)) {
      return(data.frame(
        SOURCE = character(),
        TARGET = character(),
        stringsAsFactors = FALSE
      ))
    }

    as.data.frame(result[, c("SOURCE", "TARGET")])
  })

  if (length(skipped_functions) > 0) {
    message(
      "Skipped ",
      length(unique(skipped_functions)),
      " function(s) that pkgnet could not parse: ",
      paste(unique(skipped_functions), collapse = ", ")
    )
  }

  pkgnet_edges <- do.call(rbind, edge_rows)

  if (nrow(pkgnet_edges) == 0) {
    return(data.frame(
      SOURCE = character(),
      TARGET = character(),
      stringsAsFactors = FALSE
    ))
  }

  unique(pkgnet_edges)
}

save_interactive_widget <- function(widget, path) {
  export_result <- tryCatch(
    {
      htmlwidgets::saveWidget(
        widget = widget,
        file = path,
        selfcontained = TRUE
      )
      "self-contained"
    },
    error = function(error) {
      message(
        "Self-contained HTML export failed: ",
        conditionMessage(error)
      )
      message("Retrying with selfcontained = FALSE.")

      htmlwidgets::saveWidget(
        widget = widget,
        file = path,
        selfcontained = FALSE
      )
      "HTML with supporting files"
    }
  )

  message("Saved ", export_result, " file: ", normalizePath(path))
}

html_escape <- function(x) {
  x <- gsub("&", "&amp;", x, fixed = TRUE)
  x <- gsub("<", "&lt;", x, fixed = TRUE)
  x <- gsub(">", "&gt;", x, fixed = TRUE)
  x <- gsub('"', "&quot;", x, fixed = TRUE)
  x
}

collapse_html_list <- function(x, empty = "None detected") {
  x <- unique(stats::na.omit(x))

  if (length(x) == 0) {
    return(empty)
  }

  paste(html_escape(x), collapse = "<br>- ")
}

fims_graph_colors <- list(
  r = "#2e75b6",
  rcpp = "#0e6876",
  cpp = "#548235",
  neutral = "#213f4b",
  documented_border = "#213f4b",
  edge = "#9bb8c2",
  muted = "#8c8c8c",
  pale = "#ebeeef"
)

documented_border_width <- 1

graph_group_labels <- c(
  r_function = "R function",
  cpp_function = "Rcpp function",
  cpp_class = "C++ class",
  cpp_interface = "Rcpp interface",
  cpp_field = "Rcpp field",
  cpp_method = "Rcpp method",
  r_function_documented = "R function with docs",
  cpp_function_documented = "Rcpp function with docs",
  cpp_class_documented = "C++ class with docs",
  cpp_interface_documented = "Rcpp interface with docs"
)

make_graph_group <- function(node_type, has_documentation) {
  group_key <- ifelse(
    has_documentation,
    paste0(node_type, "_documented"),
    node_type
  )

  unname(graph_group_labels[group_key])
}

make_layer_label <- function(node_type) {
  layer_lookup <- c(
    r_function = "R layer",
    cpp_function = "Rcpp layer",
    cpp_class = "C++ layer",
    cpp_interface = "Rcpp layer",
    cpp_field = "Rcpp fields/methods",
    cpp_method = "Rcpp fields/methods"
  )

  layer <- unname(layer_lookup[node_type])
  ifelse(is.na(layer), "Other layer", layer)
}

add_focus_metadata <- function(nodes, edges) {
  edge_node_ids <- unique(c(edges$from, edges$to))

  nodes$is_isolated <- !(nodes$id %in% edge_node_ids)
  nodes$size <- ifelse(nodes$is_isolated, 8, 12)
  nodes$size <- ifelse(nodes$has_equation, nodes$size + 2, nodes$size)
  nodes$borderWidth <- ifelse(nodes$has_equation, 3, 1)
  nodes$borderWidthSelected <- ifelse(nodes$has_equation, 4, 2)
  nodes$connectivity <- ifelse(
    nodes$is_isolated,
    "Currently isolated nodes",
    "Connected nodes"
  )

  nodes$filter_tags <- mapply(
    FUN = function(layer, documentation_status, equation_status, connectivity, group) {
      paste(
        c(layer, documentation_status, equation_status, connectivity, group),
        collapse = ", "
      )
    },
    layer = nodes$layer,
    documentation_status = nodes$documentation_status,
    equation_status = nodes$equation_status,
    connectivity = nodes$connectivity,
    group = nodes$group,
    USE.NAMES = FALSE
  )

  nodes
}

filter_graph_data <- function(nodes, edges, keep_node_ids) {
  keep_node_ids <- unique(keep_node_ids)

  filtered_nodes <- nodes[nodes$id %in% keep_node_ids, ]
  filtered_edges <- edges[
    edges$from %in% filtered_nodes$id &
      edges$to %in% filtered_nodes$id,
  ]

  list(
    nodes = add_focus_metadata(filtered_nodes, filtered_edges),
    edges = filtered_edges
  )
}

get_neighbor_ids <- function(edges, seed_ids, degree = 1) {
  neighborhood <- unique(seed_ids)

  for (i in seq_len(degree)) {
    edge_subset <- edges[
      edges$from %in% neighborhood |
        edges$to %in% neighborhood,
    ]

    neighborhood <- unique(c(
      neighborhood,
      edge_subset$from,
      edge_subset$to
    ))
  }

  neighborhood
}

focus_filter_values <- c(
  "Connected nodes",
  "Currently isolated nodes",
  "Has documentation link",
  "No documentation link",
  "Equations available",
  "No equation found",
  "R layer",
  "Rcpp layer",
  "C++ layer",
  "Rcpp fields/methods",
  "R function",
  "Rcpp function",
  "Rcpp interface",
  "C++ class with docs"
)

# -------------------------------------------------------------------------
# 1. Parse FIMS R-to-C++ documentation links
# -------------------------------------------------------------------------
# What it creates:
# A table of R-visible Rcpp/C++ names and their Doxygen URLs.
#
# Why it matters:
# R/Rcpp_exports.R already contains curated links from R-facing names to
# Doxygen pages. This gives the graph a documentation layer.

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

cpp_doc_nodes <- do.call(rbind, doc_rows)

# -------------------------------------------------------------------------
# 2. Parse Rcpp module registration blocks
# -------------------------------------------------------------------------
# What it creates:
# A table connecting the R-facing class names to their C++ interface classes,
# exposed fields, and exposed methods.
#
# Why it matters:
# The previous graph had many isolated C++ class documentation nodes. Those
# nodes were not necessarily isolated in the code; the graph simply did not
# know how Rcpp classes were registered. This section extracts that layer.

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

rcpp_registration$interface_id <- paste0(
  "cpp_interface::",
  rcpp_registration$cpp_interface
)

# -------------------------------------------------------------------------
# 3. Build the pkgnet R-side function dependency graph
# -------------------------------------------------------------------------
# What it creates:
# The R-side function dependency network. This is useful, but it does not
# inspect internal C++ function-to-function dependencies.

fr <- pkgnet::FunctionReporter$new()
fr$set_package("FIMS")

pkg_nodes <- as.data.frame(fr$nodes)
ns <- asNamespace("FIMS")
pkg_edges <- safe_extract_pkgnet_edges(fr, pkg_nodes, ns)

pkg_nodes$is_cpp_function <- vapply(pkg_nodes$node, function(x) {
  inherits(get(x, envir = ns), "C++Function")
}, logical(1))

pkg_nodes$node_type <- ifelse(
  pkg_nodes$is_cpp_function,
  "cpp_function",
  "r_function"
)

pkg_nodes <- merge(
  pkg_nodes,
  cpp_doc_nodes,
  by.x = "node",
  by.y = "name",
  all.x = TRUE,
  suffixes = c("", "_doc")
)

pkg_nodes$node_type <- ifelse(
  !is.na(pkg_nodes$node_type_doc),
  pkg_nodes$node_type_doc,
  pkg_nodes$node_type
)

# -------------------------------------------------------------------------
# 4. Add Rcpp class and C++ interface nodes
# -------------------------------------------------------------------------
# What it creates:
# - R-facing C++ class nodes, e.g., EWAAGrowth
# - C++ interface nodes, e.g., EWAAGrowthInterface
#
# Why it matters:
# This makes the graph more like the Doxygen class pages. A C++ class node can
# now point to the interface object that provides fields and methods to R.

cpp_class_nodes <- subset(cpp_doc_nodes, node_type == "cpp_class")

cpp_class_nodes <- data.frame(
  node = cpp_class_nodes$name,
  type = "class",
  isExported = TRUE,
  node_type = "cpp_class",
  url = cpp_class_nodes$url,
  stringsAsFactors = FALSE
)

interface_nodes <- data.frame(
  node = unique(rcpp_registration$interface_id),
  type = "interface",
  isExported = FALSE,
  node_type = "cpp_interface",
  url = NA_character_,
  stringsAsFactors = FALSE
)

interface_nodes$label <- sub("^cpp_interface::", "", interface_nodes$node)

combined_nodes <- rbind(
  pkg_nodes[, c("node", "type", "isExported", "node_type", "url")],
  cpp_class_nodes[, c("node", "type", "isExported", "node_type", "url")],
  interface_nodes[, c("node", "type", "isExported", "node_type", "url")]
)

combined_nodes <- combined_nodes[!duplicated(combined_nodes$node), ]

# -------------------------------------------------------------------------
# 5. Add Rcpp registration edges
# -------------------------------------------------------------------------
# What it creates:
# Edges from the R-facing class name to the C++ interface class registered
# through Rcpp::class_<...>("...").

rcpp_registration_edges <- data.frame(
  from = rcpp_registration$r_name,
  to = rcpp_registration$interface_id,
  label = "registered_interface",
  arrows = "to",
  stringsAsFactors = FALSE
)

pkgnet_edges <- data.frame(
  from = pkg_edges$SOURCE,
  to = pkg_edges$TARGET,
  label = "r_dependency",
  arrows = "to",
  stringsAsFactors = FALSE
)

viz_edges_main <- rbind(
  pkgnet_edges,
  rcpp_registration_edges
)

# -------------------------------------------------------------------------
# 6. Add optional field and method detail nodes
# -------------------------------------------------------------------------
# What it creates:
# A more detailed graph variant where interface nodes point to exposed fields
# and methods.
#
# The default graph below does not include these nodes because they can make
# the figure busy. The detailed graph is still created as graph_with_members.

make_member_nodes_and_edges <- function(registration) {
  member_nodes <- list()
  member_edges <- list()

  for (i in seq_len(nrow(registration))) {
    row <- registration[i, ]

    for (field in row$fields[[1]]) {
      member_id <- paste("cpp_field", row$cpp_interface, field, sep = "::")
      member_nodes[[length(member_nodes) + 1]] <- data.frame(
        node = member_id,
        label = field,
        type = "field",
        isExported = FALSE,
        node_type = "cpp_field",
        url = NA_character_,
        stringsAsFactors = FALSE
      )
      member_edges[[length(member_edges) + 1]] <- data.frame(
        from = row$interface_id,
        to = member_id,
        label = "field",
        arrows = "to",
        stringsAsFactors = FALSE
      )
    }

    for (method in row$methods[[1]]) {
      member_id <- paste("cpp_method", row$cpp_interface, method, sep = "::")
      member_nodes[[length(member_nodes) + 1]] <- data.frame(
        node = member_id,
        label = paste0(method, "()"),
        type = "method",
        isExported = FALSE,
        node_type = "cpp_method",
        url = NA_character_,
        stringsAsFactors = FALSE
      )
      member_edges[[length(member_edges) + 1]] <- data.frame(
        from = row$interface_id,
        to = member_id,
        label = "method",
        arrows = "to",
        stringsAsFactors = FALSE
      )
    }
  }

  list(
    nodes = if (length(member_nodes) == 0) {
      data.frame(
        node = character(),
        label = character(),
        type = character(),
        isExported = logical(),
        node_type = character(),
        url = character(),
        stringsAsFactors = FALSE
      )
    } else {
      do.call(rbind, member_nodes)
    },
    edges = if (length(member_edges) == 0) {
      data.frame(
        from = character(),
        to = character(),
        label = character(),
        arrows = character(),
        stringsAsFactors = FALSE
      )
    } else {
      do.call(rbind, member_edges)
    }
  )
}

member_graph_data <- make_member_nodes_and_edges(rcpp_registration)

combined_nodes_with_members <- rbind(
  combined_nodes,
  member_graph_data$nodes[, c("node", "type", "isExported", "node_type", "url")]
)

combined_nodes_with_members <- combined_nodes_with_members[
  !duplicated(combined_nodes_with_members$node),
]

viz_edges_with_members <- rbind(
  viz_edges_main,
  member_graph_data$edges
)

# -------------------------------------------------------------------------
# 7. Build hover text
# -------------------------------------------------------------------------
# What it creates:
# Doxygen-style summaries for Rcpp class and interface nodes.

class_title_lookup <- setNames(
  paste0(
    "<b>", html_escape(rcpp_registration$r_name), "</b><br>",
    "Type: Rcpp class<br>",
    "C++ interface: ", html_escape(rcpp_registration$cpp_interface), "<br>",
    "Source: ", html_escape(rcpp_registration$source_file), "<br>",
    "<br><b>Fields</b><br>- ",
    vapply(rcpp_registration$fields, collapse_html_list, character(1)),
    "<br><br><b>Methods</b><br>- ",
    vapply(rcpp_registration$methods, collapse_html_list, character(1)),
    ifelse(
      is.na(rcpp_registration$url),
      "",
      paste0(
        "<br><br><a href='",
        rcpp_registration$url,
        "' target='_blank'>Open documentation</a>"
      )
    )
  ),
  rcpp_registration$r_name
)

interface_title_lookup <- setNames(
  paste0(
    "<b>", html_escape(rcpp_registration$cpp_interface), "</b><br>",
    "Type: C++ interface registered with Rcpp<br>",
    "R-facing name: ", html_escape(rcpp_registration$r_name), "<br>",
    "Source: ", html_escape(rcpp_registration$source_file), "<br>",
    "<br><b>Fields exposed to R</b><br>- ",
    vapply(rcpp_registration$fields, collapse_html_list, character(1)),
    "<br><br><b>Methods exposed to R</b><br>- ",
    vapply(rcpp_registration$methods, collapse_html_list, character(1)),
    ifelse(
      is.na(rcpp_registration$url),
      "",
      paste0(
        "<br><br><a href='",
        rcpp_registration$url,
        "' target='_blank'>Open documentation</a>"
      )
    )
  ),
  rcpp_registration$interface_id
)

equation_catalog_path <- file.path(
  "visualization-prototypes",
  "data",
  "fims_equation_catalog.csv"
)

if (!file.exists(equation_catalog_path)) {
  source(file.path(
    "visualization-prototypes",
    "R",
    "package-level-explorer",
    "equation_extraction_prototype.r"
  ))
}

equation_catalog <- utils::read.csv(
  equation_catalog_path,
  stringsAsFactors = FALSE
)

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

  equation <- gsub("\\s+", " ", equation)
  trimws(equation)
}

get_equation_rows_for_node <- function(node_id) {
  r_visible_name <- equation_catalog$r_visible_name
  entity <- equation_catalog$entity
  enclosing_class <- equation_catalog$enclosing_class

  r_visible_name[is.na(r_visible_name)] <- ""
  entity[is.na(entity)] <- ""
  enclosing_class[is.na(enclosing_class)] <- ""

  equation_rows <- equation_catalog[
    r_visible_name %in% node_id |
      entity %in% node_id |
      enclosing_class %in% node_id,
  ]

  equation_rows <- equation_rows[
    !is.na(equation_rows$equation) &
      equation_rows$equation != "" &
      equation_rows$equation != "NA",
  ]

  equation_rows[!duplicated(equation_rows$equation), ]
}

node_has_equation <- function(node_id) {
  nrow(get_equation_rows_for_node(node_id)) > 0
}

format_equation_for_tooltip <- function(equation) {
  paste0(
    "<div style='font-family: Consolas, monospace; font-size: 0.92em; ",
    "background: #f8fafb; border: 1px solid #cfd5d8; border-radius: 4px; ",
    "padding: 4px 6px; margin-top: 4px; white-space: normal;'>",
    html_escape(make_readable_equation(equation)),
    "</div>"
  )
}

make_equation_tooltip <- function(node_id) {
  equation_rows <- get_equation_rows_for_node(node_id)

  if (nrow(equation_rows) == 0) {
    return("")
  }

  equation_rows <- equation_rows[seq_len(min(3, nrow(equation_rows))), ]

  paste0(
    "<br><br><b>Equations</b><br>",
    paste(
      format_equation_for_tooltip(equation_rows$equation),
      collapse = "<br>"
    ),
    "<br><a href='fims_equation_catalog.html' ",
    "target='_blank'>Open equation catalog</a>"
  )
}

build_viz_nodes <- function(nodes) {
  labels <- ifelse(
    grepl("^cpp_interface::", nodes$node),
    sub("^cpp_interface::", "", nodes$node),
    nodes$node
  )

  titles <- ifelse(
    nodes$node %in% names(class_title_lookup),
    class_title_lookup[nodes$node],
    ifelse(
      nodes$node %in% names(interface_title_lookup),
      interface_title_lookup[nodes$node],
      ifelse(
        is.na(nodes$url),
        paste0(
          "<b>", html_escape(labels), "</b><br>",
          "Type: ", html_escape(nodes$node_type), "<br>",
          "Exported: ", nodes$isExported
        ),
        paste0(
          "<b>", html_escape(labels), "</b><br>",
          "Type: ", html_escape(nodes$node_type), "<br>",
          "Exported: ", nodes$isExported, "<br>",
          "<a href='", nodes$url, "' target='_blank'>Open documentation</a>"
        )
      )
    )
  )

  equation_tooltips <- vapply(
    nodes$node,
    make_equation_tooltip,
    character(1)
  )

  has_equation <- vapply(
    nodes$node,
    node_has_equation,
    logical(1)
  )

  titles <- paste0(titles, equation_tooltips)

  data.frame(
    id = nodes$node,
    label = labels,
    group = make_graph_group(
      node_type = nodes$node_type,
      has_documentation = !is.na(nodes$url)
    ),
    node_type = nodes$node_type,
    layer = make_layer_label(nodes$node_type),
    has_documentation = !is.na(nodes$url),
    documentation_status = ifelse(
      !is.na(nodes$url),
      "Has documentation link",
      "No documentation link"
    ),
    has_equation = has_equation,
    equation_status = ifelse(
      has_equation,
      "Equations available",
      "No equation found"
    ),
    title = titles,
    stringsAsFactors = FALSE
  )
}

viz_nodes_main <- build_viz_nodes(combined_nodes)
viz_nodes_with_members <- build_viz_nodes(combined_nodes_with_members)

# -------------------------------------------------------------------------
# 8. Render main graph
# -------------------------------------------------------------------------
# What it shows:
# - R-side function dependency edges from pkgnet
# - Rcpp-backed documented functions/classes
# - Rcpp class registration edges to C++ interface classes
# - Doxygen links and Doxygen-style hover summaries
#
# This is the recommended default view.

graph_main <- visNetwork(
  viz_nodes_main,
  viz_edges_main,
  height = "850px",
  width = "100%"
) |>
  visGroups(
    groupname = "R function",
    color = list(background = fims_graph_colors$r, border = fims_graph_colors$r),
    shape = "dot"
  ) |>
  visGroups(
    groupname = "Rcpp function",
    color = list(background = fims_graph_colors$rcpp, border = fims_graph_colors$rcpp),
    shape = "dot"
  ) |>
  visGroups(
    groupname = "C++ class",
    color = list(background = fims_graph_colors$cpp, border = fims_graph_colors$cpp),
    shape = "box"
  ) |>
  visGroups(
    groupname = "Rcpp interface",
    color = list(background = fims_graph_colors$rcpp, border = fims_graph_colors$rcpp),
    shape = "diamond"
  ) |>
  visGroups(
    groupname = "R function with docs",
    color = list(
      background = fims_graph_colors$r,
      border = fims_graph_colors$documented_border
    ),
    shape = "dot",
    borderWidth = documented_border_width
  ) |>
  visGroups(
    groupname = "Rcpp function with docs",
    color = list(
      background = fims_graph_colors$rcpp,
      border = fims_graph_colors$documented_border
    ),
    shape = "dot",
    borderWidth = documented_border_width
  ) |>
  visGroups(
    groupname = "C++ class with docs",
    color = list(
      background = fims_graph_colors$cpp,
      border = fims_graph_colors$documented_border
    ),
    shape = "box",
    borderWidth = documented_border_width
  ) |>
  visGroups(
    groupname = "Rcpp interface with docs",
    color = list(
      background = fims_graph_colors$rcpp,
      border = fims_graph_colors$documented_border
    ),
    shape = "diamond",
    borderWidth = documented_border_width
  ) |>
  visLegend() |>
  visOptions(
    highlightNearest = TRUE,
    nodesIdSelection = TRUE
  ) |>
  visInteraction(
    hover = TRUE,
    navigationButtons = TRUE
  ) |>
  visEdges(
    arrows = "to",
    color = list(
      color = fims_graph_colors$edge,
      highlight = fims_graph_colors$neutral,
      hover = fims_graph_colors$neutral
    ),
    width = 0.8,
    selectionWidth = 1.6,
    hoverWidth = 1.2,
    smooth = TRUE
  ) |>
  visPhysics(stabilization = TRUE)

# -------------------------------------------------------------------------
# 9. Render detailed graph with fields and methods
# -------------------------------------------------------------------------
# What it shows:
# The same graph as graph_main, plus field and method nodes for the Rcpp
# interface classes. This can be busy, but it is useful for inspecting one
# interface neighborhood.

graph_with_members <- visNetwork(
  viz_nodes_with_members,
  viz_edges_with_members,
  height = "850px",
  width = "100%"
) |>
  visGroups(
    groupname = "R function",
    color = list(background = fims_graph_colors$r, border = fims_graph_colors$r),
    shape = "dot"
  ) |>
  visGroups(
    groupname = "Rcpp function",
    color = list(background = fims_graph_colors$rcpp, border = fims_graph_colors$rcpp),
    shape = "dot"
  ) |>
  visGroups(
    groupname = "C++ class",
    color = list(background = fims_graph_colors$cpp, border = fims_graph_colors$cpp),
    shape = "box"
  ) |>
  visGroups(
    groupname = "Rcpp interface",
    color = list(background = fims_graph_colors$rcpp, border = fims_graph_colors$rcpp),
    shape = "diamond"
  ) |>
  visGroups(
    groupname = "Rcpp field",
    color = list(background = fims_graph_colors$muted, border = fims_graph_colors$muted),
    shape = "square"
  ) |>
  visGroups(
    groupname = "Rcpp method",
    color = list(background = fims_graph_colors$neutral, border = fims_graph_colors$neutral),
    shape = "triangle"
  ) |>
  visGroups(
    groupname = "R function with docs",
    color = list(
      background = fims_graph_colors$r,
      border = fims_graph_colors$documented_border
    ),
    shape = "dot",
    borderWidth = documented_border_width
  ) |>
  visGroups(
    groupname = "Rcpp function with docs",
    color = list(
      background = fims_graph_colors$rcpp,
      border = fims_graph_colors$documented_border
    ),
    shape = "dot",
    borderWidth = documented_border_width
  ) |>
  visGroups(
    groupname = "C++ class with docs",
    color = list(
      background = fims_graph_colors$cpp,
      border = fims_graph_colors$documented_border
    ),
    shape = "box",
    borderWidth = documented_border_width
  ) |>
  visGroups(
    groupname = "Rcpp interface with docs",
    color = list(
      background = fims_graph_colors$rcpp,
      border = fims_graph_colors$documented_border
    ),
    shape = "diamond",
    borderWidth = documented_border_width
  ) |>
  visLegend() |>
  visOptions(
    highlightNearest = TRUE,
    nodesIdSelection = TRUE
  ) |>
  visInteraction(
    hover = TRUE,
    navigationButtons = TRUE
  ) |>
  visEdges(
    arrows = "to",
    color = list(
      color = fims_graph_colors$edge,
      highlight = fims_graph_colors$neutral,
      hover = fims_graph_colors$neutral
    ),
    width = 0.8,
    selectionWidth = 1.6,
    hoverWidth = 1.2,
    smooth = TRUE
  ) |>
  visPhysics(stabilization = TRUE)

# -------------------------------------------------------------------------
# 10. Render focus/filter graph variants
# -------------------------------------------------------------------------
# What these show:
# - graph_focus_all keeps all package-level nodes but adds a highlight dropdown.
# - graph_focus_connected hides nodes that are isolated in the current parser.
# - graph_focus_documentation_context keeps documented nodes plus 1-step neighbors.
# - graph_focus_with_members adds field/method details with the same controls.
#
# These are the recommended comparison views for the next prototype.

render_focus_graph <- function(nodes, edges) {
  visNetwork(
    nodes,
    edges,
    height = "850px",
    width = "100%"
  ) |>
    visGroups(
      groupname = "R function",
      color = list(background = fims_graph_colors$r, border = fims_graph_colors$r),
      shape = "dot"
    ) |>
    visGroups(
      groupname = "Rcpp function",
      color = list(background = fims_graph_colors$rcpp, border = fims_graph_colors$rcpp),
      shape = "dot"
    ) |>
    visGroups(
      groupname = "C++ class",
      color = list(background = fims_graph_colors$cpp, border = fims_graph_colors$cpp),
      shape = "box"
    ) |>
    visGroups(
      groupname = "Rcpp interface",
      color = list(background = fims_graph_colors$rcpp, border = fims_graph_colors$rcpp),
      shape = "diamond"
    ) |>
    visGroups(
      groupname = "Rcpp field",
      color = list(background = fims_graph_colors$muted, border = fims_graph_colors$muted),
      shape = "square"
    ) |>
    visGroups(
      groupname = "Rcpp method",
      color = list(background = fims_graph_colors$neutral, border = fims_graph_colors$neutral),
      shape = "triangle"
    ) |>
    visGroups(
      groupname = "R function with docs",
      color = list(
        background = fims_graph_colors$r,
        border = fims_graph_colors$documented_border
      ),
      shape = "dot",
      borderWidth = documented_border_width
    ) |>
    visGroups(
      groupname = "Rcpp function with docs",
      color = list(
        background = fims_graph_colors$rcpp,
        border = fims_graph_colors$documented_border
      ),
      shape = "dot",
      borderWidth = documented_border_width
    ) |>
    visGroups(
      groupname = "C++ class with docs",
      color = list(
        background = fims_graph_colors$cpp,
        border = fims_graph_colors$documented_border
      ),
      shape = "box",
      borderWidth = documented_border_width
    ) |>
    visGroups(
      groupname = "Rcpp interface with docs",
      color = list(
        background = fims_graph_colors$rcpp,
        border = fims_graph_colors$documented_border
      ),
      shape = "diamond",
      borderWidth = documented_border_width
    ) |>
    visLegend() |>
    visOptions(
      highlightNearest = list(
        enabled = TRUE,
        degree = 2,
        hover = TRUE,
        hideColor = "rgba(220,220,220,0.16)",
        labelOnly = FALSE
      ),
      nodesIdSelection = list(
        enabled = TRUE,
        main = "Focus node",
        style = "width: 220px; height: 28px"
      ),
      selectedBy = list(
        variable = "filter_tags",
        values = focus_filter_values,
        multiple = TRUE,
        main = "Highlight nodes by tag",
        highlight = TRUE,
        hideColor = "rgba(220,220,220,0.12)",
        style = "width: 220px; height: 28px"
      )
    ) |>
    visInteraction(
      hover = TRUE,
      navigationButtons = TRUE
    ) |>
    visEdges(
      arrows = "to",
      color = list(
        color = fims_graph_colors$edge,
        highlight = fims_graph_colors$neutral,
        hover = fims_graph_colors$neutral
      ),
      width = 0.8,
      selectionWidth = 1.6,
      hoverWidth = 1.2,
      smooth = TRUE
    ) |>
    visPhysics(stabilization = TRUE)
}

viz_nodes_focus_all <- add_focus_metadata(viz_nodes_main, viz_edges_main)
graph_focus_all <- render_focus_graph(viz_nodes_focus_all, viz_edges_main)

connected_node_ids <- viz_nodes_focus_all$id[!viz_nodes_focus_all$is_isolated]
focus_connected_data <- filter_graph_data(
  nodes = viz_nodes_focus_all,
  edges = viz_edges_main,
  keep_node_ids = connected_node_ids
)
graph_focus_connected <- render_focus_graph(
  focus_connected_data$nodes,
  focus_connected_data$edges
)

documented_node_ids <- viz_nodes_focus_all$id[
  viz_nodes_focus_all$has_documentation
]
documentation_context_ids <- get_neighbor_ids(
  edges = viz_edges_main,
  seed_ids = documented_node_ids,
  degree = 1
)
focus_documentation_context_data <- filter_graph_data(
  nodes = viz_nodes_focus_all,
  edges = viz_edges_main,
  keep_node_ids = documentation_context_ids
)
graph_focus_documentation_context <- render_focus_graph(
  focus_documentation_context_data$nodes,
  focus_documentation_context_data$edges
)

viz_nodes_focus_with_members <- add_focus_metadata(
  viz_nodes_with_members,
  viz_edges_with_members
)
graph_focus_with_members <- render_focus_graph(
  viz_nodes_focus_with_members,
  viz_edges_with_members
)

# -------------------------------------------------------------------------
# 11. Inspect extracted metadata
# -------------------------------------------------------------------------
# Useful tables while troubleshooting:
# - cpp_doc_nodes
# - rcpp_registration
# - combined_nodes
# - viz_edges_main

output_dir <- file.path(
  "visualization-prototypes",
  "examples",
  "package-level-explorer"
)
dir.create(output_dir, showWarnings = FALSE, recursive = TRUE)

focus_connected_html_path <- file.path(
  output_dir,
  "fims_package_level_explorer_equations_connected_only.html"
)

save_interactive_widget(graph_focus_connected, focus_connected_html_path)

list(
  n_pkgnet_nodes = nrow(pkg_nodes),
  n_pkgnet_edges = nrow(pkg_edges),
  n_documented_cpp_names = nrow(cpp_doc_nodes),
  n_rcpp_registered_classes = nrow(rcpp_registration),
  n_main_nodes = nrow(viz_nodes_main),
  n_main_edges = nrow(viz_edges_main),
  n_focus_connected_nodes = nrow(focus_connected_data$nodes),
  n_focus_connected_edges = nrow(focus_connected_data$edges),
  n_documentation_context_nodes = nrow(focus_documentation_context_data$nodes),
  n_documentation_context_edges = nrow(focus_documentation_context_data$edges),
  n_member_detail_nodes = nrow(member_graph_data$nodes),
  n_member_detail_edges = nrow(member_graph_data$edges)
)

graph_focus_connected

# To compare the other focus views, run:
# graph_focus_all
# graph_focus_documentation_context
# graph_focus_with_members
