generate_fims <- function() {

  include_root <- "inst/include"
  output_dir   <- "src"

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
    stop("No .hpp files found in inst/include")
  }

  # Precompute relative paths
  rel_paths <- sub(paste0("^", include_root, "/?"), "", files)

  # Map basename -> full relative path(s)
  basename_map <- split(rel_paths, basename(rel_paths))

  # ------------------------------------------------------------
  # 2. Helper: Encode filename to generated header name
  # ------------------------------------------------------------
  encode_name <- function(rel_path) {
    no_ext <- sub("\\.hpp$", "", rel_path)
    paste0("F_", gsub("/", "_", no_ext), ".h")
  }

  # ------------------------------------------------------------
  # 3. Helper: Resolve dependency
  # ------------------------------------------------------------
  resolve_dependency <- function(dep_raw) {

    dep_raw <- trimws(dep_raw)
    dep_file <- paste0(dep_raw, ".hpp")

    # Case 1: full path provided
    if (grepl("/", dep_raw)) {

      if (!dep_file %in% rel_paths) {
        stop("Cannot resolve dependency: ", dep_raw)
      }

      return(dep_raw)
    }

    # Case 2: basename only
    if (!dep_file %in% names(basename_map)) {
      stop("Cannot resolve dependency: ", dep_raw)
    }

    matches <- basename_map[[dep_file]]

    if (length(matches) > 1) {
      stop(
        "Ambiguous dependency: ", dep_raw,
        " found in multiple locations:\n  ",
        paste(matches, collapse = "\n  ")
      )
    }

    sub("\\.hpp$", "", matches)
  }

  # ------------------------------------------------------------
  # 4. Process each file
  # ------------------------------------------------------------
  for (i in seq_along(files)) {

    input_file  <- files[i]
    rel_path    <- rel_paths[i]
    rel_no_ext  <- sub("\\.hpp$", "", rel_path)
    output_file <- file.path(output_dir, encode_name(rel_path))

    message("Generating ", output_file)

    lines <- readLines(input_file, warn = FALSE)

    # Replace FIMS_DEPENDS(...) with #include
    dep_pattern <- "FIMS_DEPENDS\\(([^)]+)\\);"

    for (j in seq_along(lines)) {

      if (grepl(dep_pattern, lines[j])) {

        match <- regmatches(
          lines[j],
          regexec(dep_pattern, lines[j])
        )[[1]]

        if (length(match) < 2) {
          stop("Malformed FIMS_DEPENDS in ", input_file)
        }

        dep_raw <- match[2]
        dep_resolved <- resolve_dependency(dep_raw)
        dep_header <- encode_name(dep_resolved)

        lines[j] <- paste0("#include \"", dep_header, "\"")
      }
    }

    # Add include guard
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
  }

  message("FIMS header generation complete.")
}