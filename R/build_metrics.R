#' Parse build log and write metrics to metrics.json
#'
#' Reads build.log from the build-metrics workflow and pulls out
#' build health metrics, then saves them to metrics.json.
#'
#' @keywords developer
#' @examples
#' \dontrun{
#' parse_build_metrics()
#' }
parse_build_metrics <- function(){
  lines <- readLines("build.log", warn = FALSE)
  metrics <- list(commit_sha = Sys.getenv("GITHUB_SHA", unset = "unknown"),
    timestamp = format(Sys.time(), "%Y-%m-%dT%H:%M:%SZ", tz = "UTC"),
    wall_clock_time_sec = parse_wall_clock(lines),
    peak_ram_mb = parse_peak_ram_mb(lines), warning_count = parse_warning_count(lines),
    error_count = parse_error_count(lines), binary_sizes = parse_binary_sizes(lines),
    template_time_sec = parse_template_time(lines))
  jsonlite::write_json(metrics, "metrics.json", pretty = TRUE, auto_unbox = TRUE)
  invisible(metrics)
}
#' Compare current build metrics against the previous run
#'
#' Reads metrics.json and previous_metrics.json, compares them,
#' and writes the results to metrics_comparison.log. Flags any regressions.
#'
#' @keywords developer
#' @examples
#' \dontrun{
#' compare_build_metrics()
#' }
compare_build_metrics <- function(){
  current <- jsonlite::read_json("metrics.json", simplifyVector = TRUE)
  previous <- if (file.exists("previous_metrics.json")){
    jsonlite::read_json("previous_metrics.json", simplifyVector = TRUE)
  }
  else{
    NULL
  }
  lines <- c("BUILD METRICS COMPARISON:", "")
  if (is.null(previous)){
    lines <- c(lines, "No previous metrics found. This is the first run.")
  }
  else{
    lines <- c(
      lines, paste0("Previous commit : ", previous$commit_sha),
      paste0("Current commit : ", current$commit_sha), "",
      paste0("Wall clock time : ", format_change(current$wall_clock_time_sec, previous$wall_clock_time_sec, "s")),
      paste0("Peak RAM : ", format_change(current$peak_ram_mb, previous$peak_ram_mb, "MB")),
      paste0("Warnings : ", format_change(current$warning_count, previous$warning_count)),
      paste0("Errors : ", format_change(current$error_count, previous$error_count)),
      paste0("Template time : ", format_change(current$template_time_sec, previous$template_time_sec, "s")),
      ""
    )
    regressions <- c()

    if (!is.na(current$warning_count) && !is.null(previous$warning_count) && current$warning_count > previous$warning_count){
      regressions <- c(regressions, paste0("WARNING: warning count increased from ",
        previous$warning_count,
         " to ", current$warning_count))
    }
    if (!is.na(current$error_count) && !is.null(previous$error_count) && current$error_count > previous$error_count){
      regressions <- c(regressions, paste0("WARNING: error count increased from ", 
      previous$error_count, " to ", current$error_count
      ))
    }
    if (!is.na(current$peak_ram_mb) && !is.null(previous$peak_ram_mb) &&!is.na(previous$peak_ram_mb)){
      pct_change <- (current$peak_ram_mb - previous$peak_ram_mb)/previous$peak_ram_mb*100
      if (pct_change > 10){
        regressions <- c(regressions, paste0("WARNING: peak RAM increased by ", round(pct_change, 1), "%"
        ))
      }
    }

    if (!is.na(current$wall_clock_time_sec) && !is.null(previous$wall_clock_time_sec) && !is.na(previous$wall_clock_time_sec)){
      pct_change <- (current$wall_clock_time_sec - previous$wall_clock_time_sec)/
        previous$wall_clock_time_sec*100
      if (pct_change > 10){
        regressions <- c(regressions, paste0("WARNING: compile time increased by ", round(pct_change, 1), "%"
        ))
      }
    }
    if (length(regressions) > 0){
      lines <- c(lines, "REGRESSIONS DETECTED:", regressions)
    }
    else{
      lines <- c(lines, "No regressions detected.")
    }
  }
  writeLines(lines, "metrics_comparison.log")
  invisible(lines)
}

parse_wall_clock <- function(lines){
  raw <- extract_first(lines, "Elapsed \\(wall clock\\) time.*: ([0-9:]+\\.?[0-9]*)")
  if (is.na(raw)){
    return(NA_real_)
  }
  parts <- as.numeric(strsplit(raw, ":")[[1]])
  total <- if (length(parts) == 3){
    parts[1]*3600 + parts[2]*60 + parts[3]
  }
  else if (length(parts) == 2){
    parts[1]*60 + parts[2]
  }
  else{
    parts[1]
  }
  round(total, 2)
}
parse_peak_ram_mb <- function(lines){
  raw <- extract_first(lines, "Maximum resident set size \\(kbytes\\): ([0-9]+)")
  if (is.na(raw)){
    return(NA_real_)
  }
  round(as.numeric(raw)/1024, 2)
}
parse_warning_count <- function(lines){
  sum(grepl("warning:", lines, ignore.case = TRUE))}

parse_error_count <- function(lines){
  error_lines <- grepl("error:", lines, ignore.case = TRUE) &
    !grepl("[0-9]+ error(s)? generated", lines, ignore.case = TRUE)
  sum(error_lines)}
parse_binary_sizes <- function(lines){
  start <- which(grepl("=== Binary and object file sizes ===", lines))
  end <- which(grepl("=== du summary", lines))
  if (length(start) == 0){
    return(list())}
  section <- if (length(end) > 0){
    lines[(start[1]+1):(end[1]-1)]
  }
  else{
    lines[(start[1]+1):length(lines)]
  }
  results <- list()
  for (line in section){
    m <- regmatches(line, regexec(
      "^[-d][rwx-]+\\s+\\d+\\s+\\S+\\s+\\S+\\s+(\\S+)\\s+.*\\s(\\S+)$",
      line ))[[1]]
    if (length(m) == 3){
      results[[basename(m[3])]] <- m[2]
    }
  }
  results
}

parse_template_time <- function(lines){
  matches <- regmatches(lines, regexec("^\\s*TOTAL\\s*:\\s*[0-9.]+\\s+[0-9.]+\\s+([0-9.]+)",
    lines))
  totals <- sapply(matches, function(m){
    if (length(m) >= 2) as.numeric(m[[2]]){else NA_real_}
  })
  totals <- totals[!is.na(totals)]
  if (length(totals) == 0){
    return(NA_real_)
  }
  round(sum(totals), 2)
}
extract_first <- function(lines, pattern){
  matches <- regmatches(lines, regexec(pattern, lines))
  for (m in matches){
    if (length(m) >= 2){
      return(m[[2]])
    }
  }
  return(NA_character_)
}

format_change <- function(current, previous, unit = ""){
  if (is.null(previous) || is.na(previous) || is.na(current)){
    return("no previous data")
  }
  diff <- current - previous
  pct <- round((diff/previous)*100, 1)
  direction <- if (diff > 0) "+" else ""
  paste0(current, unit, " (", direction, diff, unit, ", ", direction, pct, "%)")
}