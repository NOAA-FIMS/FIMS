# scripts/build_vignette_index.R
# Generates docs/vignettes.json during the pkgdown workflow

library(fs)
library(jsonlite)
library(yaml)
library(purrr)
library(stringr)

read_front_matter <- function(path) {
  lines <- readLines(path, warn = FALSE)

  if (length(lines) < 3 || trimws(lines[1]) != "---") {
    return(list())
  }

  end_idx <- which(trimws(lines[-1]) == "---")[1] + 1
  if (is.na(end_idx)) {
    return(list())
  }

  yaml_text <- paste(lines[2:(end_idx - 1)], collapse = "\n")

  tryCatch(
    yaml::yaml.load(yaml_text),
    error = function(e) list()
  ) |> (\(x) if (is.null(x)) list() else x)()
}

first_nonempty <- function(...) {
  vals <- list(...)
  for (x in vals) {
    if (!is.null(x) && length(x) > 0) {
      x <- as.character(x[1])
      if (nzchar(x)) return(x)
    }
  }
  NULL
}

slug_from_file <- function(path) {
  path_ext_remove(path_file(path))
}

article_url_for <- function(rel_path) {
  slug <- slug_from_file(rel_path)
  paste0("https://noaa-fims.github.io/FIMS/articles/", slug, ".html")
}

vignette_files <- dir_ls("vignettes", recurse = TRUE, type = "file") |>
  keep(~ str_detect(path_file(.x), "\\.(qmd|Rmd)$"))

records <- map(vignette_files, function(file) {
  fm <- read_front_matter(file)

  title <- first_nonempty(fm$title) %||% slug_from_file(file)
  summary <- first_nonempty(fm$summary, fm$description) %||% ""

  level <- first_nonempty(
    fm$level
  ) %||% "new-user"
  
  list(
    title = title,
    summary = summary,
    level = level,
    page = article_url_for(file),
    source = as.character(file)
  )
})

jsonlite::write_json(records, "docs/vignettes.json", auto_unbox = TRUE, pretty = TRUE, null = "null")