#' Create tests/google_benchmark/benchmark_*.cpp and register it in CMakeLists.txt
#'
#' This helper function generates a Google Benchmark C++ template file for a
#' given workload and appends lines to `tests/google_benchmark/CMakeLists.txt`
#' to register the benchmark as a separate executable.
#'
#' @param name A string representing the name for the C++ benchmark file and
#'   CMake executable target. It must follow the format
#'   `FileName_ClassName_FunctionName`, where
#'   - `FileName` is the C++ source file name
#'   - `ClassName` is the C++ class name (or a placeholder such as `ClassName`)
#'   - `FunctionName` is the C++ function name
#'   and underscores separate each component. For example:
#'   `Population_CatchAtAge_CatchNumbersAtAge`.
#'
#'   The generated file will be
#'   `tests/google_benchmark/benchmark_<name>.cpp` and the CMake target will be
#'   `benchmark_<name>`.
#' @param type Benchmark template type. Use `"simple"` for direct benchmarking
#'   without gtest fixtures, or `"fixture"` to generate a fixture-based template
#'   that includes gtest.
#'
#' @return
#' If successful, this function invisibly returns `TRUE` to allow for the
#' chaining of commands. If the function is unsuccessful, an error message is
#' returned.
#'
#' Messages from the usethis package and cli indicate where the new file was
#' created and that the benchmark was registered in
#' `tests/google_benchmark/CMakeLists.txt`.
#'
#' @examples
#' \dontrun{
#' # Create a new benchmark file and register it in CMake:
#' FIMS:::use_google_benchmark_template(
#'   name = "Population_CatchAtAge_CatchNumbersAtAge"
#' )
#'
#' # Create a fixture-based benchmark template:
#' FIMS:::use_google_benchmark_template(
#'   name = "Population_CatchAtAge_CatchNumbersAtAge",
#'   type = "fixture"
#' )
#' }
#'
#' @keywords developer
use_google_benchmark_template <- function(name = "FileName_ClassName_FunctionName",
                                          type = c("simple", "fixture")) {
  type <- match.arg(type)
  bm_dir <- file.path("tests", "google_benchmark")
  cmakelist_path <- file.path(bm_dir, "CMakeLists.txt")

  if (!file.exists(cmakelist_path)) {
    cli::cli_abort(
      c("{.file {cmakelist_path}} does not exist.",
        "i" = "Please ensure that the CMakeLists.txt file is present in the
        {.file tests/google_benchmark} directory."
      )
    )
  }

  # Validate name format
  if (!nzchar(name)) {
    cli::cli_abort(
      c("Invalid {.var name}.",
        "i" = "{.var name} must be a non-empty string."
      )
    )
  }
  name_parts_list <- strsplit(x = name, split = "_")[[1]]
  if (length(name_parts_list) != 3) {
    cli::cli_abort(
      c("Invalid {.var name} format.",
        "i" = "Expected format: {.val FileName_ClassName_FunctionName}.",
        "x" = "Received name: {.val {name}}."
      )
    )
  }
  file_name <- name_parts_list[[1]]
  class_name <- name_parts_list[[2]]
  function_name <- name_parts_list[[3]]

  path <- file.path(bm_dir, paste0("benchmark_", name, ".cpp"))
  if (file.exists(path)) {
    cli::cli_abort("{.file {path}} already exists.")
  }

  template_file <- if (identical(type, "fixture")) {
    "google_benchmark_template.cpp"
  } else {
    "google_benchmark_simple_template.cpp"
  }

  usethis::use_template(
    template = template_file,
    save_as = path,
    data = list(
      benchmark_name = name,
      file_name = file_name,
      class_name = class_name,
      function_name = function_name
    ),
    package = "FIMS"
  )

  CON <- file(cmakelist_path, "a")
  on.exit(close(CON), add = TRUE)
  cmake_lines <- c(
    "",
    glue::glue("# benchmark_{name}.cpp"),
    glue::glue("add_executable(benchmark_{name}"),
    glue::glue("  benchmark_{name}.cpp"),
    ")",
    "",
    glue::glue("target_link_libraries(benchmark_{name}"),
    "  benchmark::benchmark_main",
    "  fims_test"
  )
  if (identical(type, "fixture")) {
    cmake_lines <- c(cmake_lines, "  GTest::gtest")
  }
  cmake_lines <- c(cmake_lines, ")")
  writeLines(cmake_lines, CON)
  cli::cli_alert_success(
    "Registering benchmark {.val benchmark_{name}} in {.file {cmakelist_path}}."
  )

  invisible(TRUE)
}
