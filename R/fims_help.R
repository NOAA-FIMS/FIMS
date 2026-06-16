#' Smart FIMS Documentation Finder
#'
#' @param query A string representing the function or class to search for.
#' @export
fims_help <- function(query) {
  
  doxygen_base_url <- "https://e-perl-noaa.github.io/FIMS/doxygen/"
  r_base_url <- "https://e-perl-noaa.github.io/FIMS/" # Adjust to your pkgdown path
  
  # 1. Check native R exports first
  if (query %in% getNamespaceExports("FIMS")) {
    target_url <- paste0(r_base_url, "reference/", query, ".html")
  } else {
    
    # 2. Setup caching for the Doxygen Tagfile
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
    
    # 3. Parse the XML and find the matching HTML file
    tag_data <- xml2::read_xml(local_tagfile)
    xpath_query <- sprintf("//name[text()='%s']/..", query)
    match_node <- xml2::xml_find_first(tag_data, xpath_query)
    
    if (length(match_node) == 0 || is.na(match_node)) {
      message("Exact C++ match not found. Opening Doxygen index...")
      target_url <- paste0(doxygen_base_url, "index.html")
    } else {
      filename <- xml2::xml_text(xml2::xml_find_first(match_node, ".//filename"))
      target_url <- paste0(doxygen_base_url, filename)
    }
  }
  
  # 4. Render in the Viewer Pane
  viewer_html <- htmltools::browsable(
    htmltools::tags$iframe(src = target_url, width = "100%", height = "600px", style = "border:none;")
  )
  
  print(viewer_html)
  invisible(target_url)
}