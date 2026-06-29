#' Smart FIMS Documentation Finder
#'
#' @param query A string representing the function or class to search for.
#'
#' @importFrom utils adist download.file
#' @importFrom xml2 read_xml xml_find_all xml_find_first xml_text
#' @importFrom htmltools browsable tags
#'
#' @export
#' @examples
#' \dontrun{
#' # Search for an R function's documentation in the pkgdown site
#' fims_help("FIMSFrame")
#'
#' # Search for a C++ class's documentation in the Doxygen site
#' fims_help("FIMSLog")
#'
#' # The function also handles partial, case-insensitive matches for C++ entities
#' fims_help("bevertonholt")
#' }
fims_help <- function(query) {
  # Do not run in non-interactive sessions (e.g., R CMD check)
  if (!interactive()) {
    message("fims_help() is designed for interactive use only.")
    return(invisible(NULL))
  }
  target_url <- NULL

  doxygen_base_url <- "https://noaa-fims.github.io/FIMS/doxygen/"
  r_base_url <- "https://noaa-fims.github.io/FIMS/" # Adjust to your pkgdown path

  # Check native R exports first
  if (query %in% getNamespaceExports("FIMS")) {
    target_url <- paste0(r_base_url, "reference/", query, ".html")
  } else {
    # Setup caching for the Doxygen Tagfile
    tagfile_url <- paste0(doxygen_base_url, "FIMS.tag")
    cache_dir <- tempdir()
    local_tagfile <- file.path(cache_dir, "FIMS.tag")

    # Download the tagfile only if we haven't already in this R session
    if (!file.exists(local_tagfile)) {
      message("Fetching FIMS C++ documentation index...")
      tryCatch(
        utils::download.file(tagfile_url, destfile = local_tagfile, quiet = TRUE),
        error = function(e) stop("Could not download FIMS.tag. Check internet connection.")
      )
    }

    tag_data <- xml2::read_xml(local_tagfile)

    # Extract ALL names from the XML
    compound_names_nodes <- xml2::xml_find_all(tag_data, "//compound/name")
    compound_names <- xml2::xml_text(compound_names_nodes)

    # Extract unique names and find partial, case-insensitive matches
    unique_names <- unique(compound_names)
    matched_names <- unique_names[grep(query, unique_names, ignore.case = TRUE)]

    if (length(matched_names) == 0) {
      # If no direct matches, try to find close matches for a "Did you mean?" feature
      distances <- utils::adist(query, unique_names, ignore.case = TRUE)
      # Find suggestions within a reasonable distance (e.g., distance of 2)
      suggestions <- unique_names[which(distances <= 2)]

      message(sprintf("No C++ match found for '%s'.", query))
      if (length(suggestions) > 0) {
        message("Did you mean one of these?")
        for (suggestion in suggestions) {
          message(paste("  -", suggestion))
        }
      }
    } else {
      # If multiple unique matches exist, let the user know!
      if (length(matched_names) > 1) {
        message("Multiple unique C++ matches found:")
        for (name in matched_names) {
          message(paste("  -", name))
        }
        message(sprintf("\nLoading the first match ('%s'). Refine your query to view others.", matched_names[1]))
      } else {
        message(sprintf("Found C++ match: '%s'. Loading...", matched_names[1]))
      }

      best_match_name <- matched_names[1]

      # Now use the exact unique name to find the parent node and extract the filename
      xpath_query <- sprintf("//compound[name[text()='%s']]", best_match_name)
      match_node <- xml2::xml_find_first(tag_data, xpath_query)

      filename <- xml2::xml_text(xml2::xml_find_first(match_node, ".//filename"))
      target_url <- paste0(doxygen_base_url, filename)
    }
  }

  if (!is.null(target_url)) {
    # Render in the Viewer Pane
    viewer_html <- htmltools::browsable(
      htmltools::tags$iframe(src = target_url, width = "100%", height = "600px", style = "border:none;")
    )

    print(viewer_html)
    invisible(target_url)
  }
}
