generate_fims <- function() {

  pkg_root     <- normalizePath(".")
  include_root <- file.path(pkg_root, "inst", "include")
  output_dir   <- file.path(pkg_root, "src")

  cat("Package root:", pkg_root, "\n")
  cat("Include root:", include_root, "\n")

  if (!dir.exists(include_root)) {
    stop("Cannot find inst/include directory.")
  }

  if (!dir.exists(output_dir)) {
    dir.create(output_dir, recursive = TRUE)
  }

  # ------------------------------------------------------------
  # 1. Collect all .hpp files
  # ------------------------------------------------------------
  files <- list.files(
    include_root,
    pattern = "\\.hpp$",
    recursive = TRUE,
    full.names = TRUE
  )

  if (length(files) == 0) {
    stop("No .hpp files found.")
  }

  rel_paths <- sub(paste0("^", include_root, "/?"), "", files)

  # Deterministic ordering
  ord <- order(rel_paths)
  files <- files[ord]
  rel_paths <- rel_paths[ord]

  # Map basename -> full relative path(s)
  basename_map <- split(rel_paths, basename(rel_paths))

  # ------------------------------------------------------------
  # 2. Helper: Encode filename
  # ------------------------------------------------------------
  encode_name <- function(rel_path) {
    no_ext <- sub("\\.hpp$", "", rel_path)
    paste0("F_", gsub("/", "_", no_ext), ".h")
  }

  # ------------------------------------------------------------
  # 3. Resolve dependency
  # ------------------------------------------------------------
  resolve_dependency <- function(dep_raw) {

    dep_raw <- trimws(dep_raw)
    dep_file <- paste0(dep_raw, ".hpp")

    # Full path case
    if (grepl("/", dep_raw)) {
      if (!dep_file %in% rel_paths) {
        stop("Cannot resolve dependency: ", dep_raw)
      }
      return(dep_raw)
    }

    # Basename case
    if (!dep_file %in% names(basename_map)) {
      stop("Cannot resolve dependency: ", dep_raw)
    }

    matches <- basename_map[[dep_file]]

    if (length(matches) > 1) {
      stop(
        "Ambiguous dependency: ", dep_raw,
        "\nFound in:\n  ",
        paste(matches, collapse = "\n  ")
      )
    }

    sub("\\.hpp$", "", matches)
  }

  # ------------------------------------------------------------
  # 4. Generate individual headers
  # ------------------------------------------------------------
  generated_headers <- character(length(rel_paths))

  for (i in seq_along(files)) {

    input_file  <- files[i]
    rel_path    <- rel_paths[i]
    rel_no_ext  <- sub("\\.hpp$", "", rel_path)
    output_name <- encode_name(rel_path)
    output_file <- file.path(output_dir, output_name)

    cat("Generating", output_name, "\n")

    lines <- readLines(input_file, warn = FALSE)

    dep_pattern <- "FIMS_DEPENDS\\(([^)]+)\\);"

    for (j in seq_along(lines)) {

      if (grepl(dep_pattern, lines[j])) {

        match <- regmatches(
          lines[j],
          regexec(dep_pattern, lines[j])
        )[[1]]

        if (length(match) < 2) {
          stop("Malformed FIMS_DEPENDS in ", rel_path)
        }

        dep_raw <- match[2]
        dep_resolved <- resolve_dependency(dep_raw)
        dep_header <- encode_name(dep_resolved)

        lines[j] <- paste0("#include \"", dep_header, "\"")
      }
    }

    # Include guard
    guard <- toupper(gsub("[^A-Za-z0-9]", "_", rel_no_ext))
    guard <- paste0("FIMS_", guard, "_H")

    wrapped <- c(
      paste0("#ifndef ", guard),
      paste0("#define ", guard),
      "",
      lines,
      "",
      "#endif"
    )

    writeLines(wrapped, output_file)

    generated_headers[i] <- output_name
  }

  # ------------------------------------------------------------
  # 5. Generate FIMS_all.hpp (master include)
  # ------------------------------------------------------------
  master_file <- file.path(output_dir, "FIMS_all.hpp")

  master_contents <- c(
    "#ifndef FIMS_ALL_HPP",
    "#define FIMS_ALL_HPP",
    "",
    "// Auto-generated master include. Do not edit manually.",
    "",
    paste0('#include "', generated_headers, '"'),
    "",
    "#endif"
  )

  writeLines(master_contents, master_file)

  cat("Generated FIMS_all.hpp\n")
  cat("Header generation complete.\n")
}

# Automatically run when sourced
generate_fims()



# include_root <- "inst/include"
# src_dir      <- "src"

# dir.create(src_dir, showWarnings = FALSE)

# # Recursively find all .hpp files
# files <- list.files(include_root,
#                     pattern = "\\.hpp$",
#                     recursive = TRUE,
#                     full.names = TRUE)

# # Encode full relative path into unique filename
# encode_name <- function(path) {
#   rel <- sub(paste0("^", include_root, "/"), "", path)
#   rel <- sub("\\.hpp$", "", rel)
#   rel <- gsub("/", "_", rel)
#   paste0("F_", rel)
# }

# generated_headers <- character()

# for (file in files) {

#   encoded_base <- encode_name(file)
#   out_h   <- file.path(src_dir, paste0(encoded_base, ".h"))
#   out_cpp <- file.path(src_dir, paste0(encoded_base, ".cpp"))

#   cat("Generating", out_h, "\n")

#   txt <- readLines(file, warn = FALSE)

#   # Rewrite FIMS_DEPENDS(x/y/z.hpp)
# pattern <- "FIMS_DEPENDS\\(([^)]+)\\.hpp\\);"

# for (i in seq_along(txt)) {

#   if (grepl(pattern, txt[i])) {

#     match <- regmatches(txt[i], regexec(pattern, txt[i]))[[1]]
#     dep_name <- match[2]          # e.g. "model" or "path/to/model"

#     # If no slash, search for it
#     if (!grepl("/", dep_name)) {

#       candidates <- files[basename(files) == paste0(dep_name, ".hpp")]

#       if (length(candidates) == 0) {
#         stop("Cannot resolve dependency: ", dep_name)
#       }

#       if (length(candidates) > 1) {
#         cat("Ambiguous dependency: ", dep_name, " — multiple matches found.\n")
#       }

#       dep_full <- sub(paste0("^", include_root, "/"), "", candidates)

#     } else {
#       dep_full <- dep_name
#     }
#     dep_full <- stringr::str_remove(dep_full, "\\.hpp")
#     dep_encoded <- paste0("F_", gsub("/", "_", dep_full), ".h")

#     txt[i] <- paste0("#include \"", dep_encoded, "\"")
#   }
# }

# #   # Write generated header
#   writeLines(
#     c(
#       "// Autogenerated",
#       "#define WITH_LIBTMB",
#       "//#include <TMB.h>",
#       txt
#     ),
#     out_h
#   )

#   # Write corresponding cpp
#   writeLines(
#     c(
#       "// Autogenerated",
#       "#define WITH_LIBTMB",
#       paste0("#include \"", basename(out_h), "\"")
#     ),
#     out_cpp
#   )

#   generated_headers <- c(generated_headers, basename(out_h))
# }

# # Generate umbrella header
# writeLines(
#   c("// Autogenerated FIMS umbrella",
#     paste0("#include \"", generated_headers, "\"")),
#   file.path(src_dir, "FIMS.h")
# )


