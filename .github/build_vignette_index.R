# scripts/build_vignette_index.R
# Generates docs/vignettes.json during the pkgdown workflow

first_nonempty <- function(..., default) {
  for (value in list(...)) {
    if (length(value) > 0L) {
      text <- as.character(value[[1L]])
      if (!is.na(text) && nzchar(text)) {
        return(text)
      }
    }
  }
  default
}

vignette_files <- list.files(
  "vignettes",
  pattern = "\\.(qmd|Rmd)$",
  recursive = TRUE,
  full.names = TRUE
)

front_matter <- purrr::map(vignette_files, rmarkdown::yaml_front_matter)

records <- purrr::map2(vignette_files, front_matter, function(file, metadata) {
  slug <- tools::file_path_sans_ext(basename(file))

  list(
    title = first_nonempty(metadata[["title"]], default = slug),
    summary = first_nonempty(
      metadata[["summary"]], metadata[["description"]], default = ""
    ),
    level = first_nonempty(metadata[["level"]], default = "new-user"),
    page = paste0("https://noaa-fims.github.io/FIMS/articles/", slug, ".html"),
    source = file
  )
})

jsonlite::write_json(
  records,
  "docs/vignettes.json",
  auto_unbox = TRUE,
  pretty = TRUE,
  null = "null"
)