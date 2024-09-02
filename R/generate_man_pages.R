#' Format RD Block Content for .Rd Files
#'
#' This function takes an RD block extracted from a C++ or a header file, parses its 
#' components, and formats it into a string suitable for inclusion in an .Rd
#' documentation file.
#'
#' @param rd_block A string containing the raw RD block extracted from a C++ or a header file.
#'
#' @return A formatted string representing the content of the .Rd file, 
#' including sections such as title, description, param, return, 
#' and examples.
format_rd_content <- function(rd_block) {
  rd_content <- ""
  
  # Extract different components
  title <- stringr::str_extract(rd_block, "(?<=@title ).*")
  description <- stringr::str_extract(rd_block, "(?<=@description ).*")
  params <- stringr::str_extract_all(rd_block, "(?<=@param )[^@]+")[[1]]
  return_val <- stringr::str_extract(rd_block, "(?<=@return ).*")
  examples <- stringr::str_extract(rd_block, "(?<=@examples).*")
  
  # Construct the Rd content
  if (!is.na(title)) {
    rd_content <- paste0(rd_content, "\\name{", title, "}\n")
    rd_content <- paste0(rd_content, "\\title{", title, "}\n")
  }
  
  if (!is.na(description)) {
    rd_content <- paste0(rd_content, "\\description{\n", description, "\n}\n")
  }
  
  if (length(params) > 0) {
    rd_content <- paste0(rd_content, "\\arguments{\n")
    for (param in params) {
      param_name <- stringr::str_extract(param, "\\S+")
      param_desc <- stringr::str_trim(stringr::str_replace(param, param_name, ""))
      rd_content <- paste0(rd_content, "\\item{", param_name, "}{", param_desc, "}\n")
    }
    rd_content <- paste0(rd_content, "}\n")
  }
  
  if (!is.na(return_val)) {
    rd_content <- paste0(rd_content, "\\value{\n", return_val, "\n}\n")
  }
  
  if (!is.na(examples)) {
    examples <- stringr::str_replace_all(examples, "^\\s*//[']", "")
    examples <- stringr::str_replace_all(examples, "^\\s*//", "")
    examples <- stringr::str_trim(examples)
    rd_content <- paste0(rd_content, "\\examples{\n", examples, "\n}\n")
  }
  
  return(rd_content)
}

#' Process a C++ File to Extract RD Blocks
#'
#' This function reads a specified C++ file, searches for RD blocks
#' (documentation comments) within the file, and writes the extracted
#' and formatted RD content to an .Rd file in the man/ directory.
#'
#' @param file_path A string specifying the path to the C++ file to be processed.
process_cpp_file <- function(file_path) {
  cpp_content <- readr::read_lines(file_path)
  cpp_content <- paste(cpp_content, collapse = "\n")
  
  # Regular expressions to match RD blocks
  block_rd_blocks <- stringr::str_extract_all(cpp_content, "(?s)/\\*\\*\\s*@rd.*?\\*/")[[1]]
  line_rd_blocks <- stringr::str_extract_all(cpp_content, "(?m)//\\s*@rd.*?$(\n.*//\\s*@[a-zA-Z0-9]+.*?$)*")[[1]]
  line_squote_rd_blocks <- stringr::str_extract_all(cpp_content, "(?m)//'\\s*@rd.*?$(\n.*//'\\s*@[a-zA-Z0-9]+.*?$)*")[[1]]
  
  # Combine all found blocks
  rd_blocks <- c(block_rd_blocks, line_rd_blocks, line_squote_rd_blocks)
  
  if (length(rd_blocks) == 0) {
    message("No RD tags found in ", file_path)
    return()
  }
  
  for (block in rd_blocks) {
    function_name <- stringr::str_extract(block, "(?<=@name )\\w+")
    rd_block_clean <- stringr::str_replace_all(block, "(^/\\*\\*\\s*@rd|^//\\s*@rd|^//'\\s*@rd|\\*/$)", "")
    rd_block_clean <- stringr::str_replace_all(rd_block_clean, "^//\\s*", "")
    rd_block_clean <- stringr::str_replace_all(rd_block_clean, "^//'\\s*", "")
    rd_block_clean <- stringr::str_trim(rd_block_clean)
    
    rd_content <- format_rd_content(rd_block_clean)
    
    if (!is.na(function_name)) {
      output_file <- paste0("man/",paste0(function_name, ".Rd"))
      readr::write_lines(rd_content, output_file)
      message("Created: ", output_file)
    } else {
      warning("Could not extract function name from block in ", file_path)
    }
  }
}

#' Process C++ and Header Files in a Directory
#'
#' This function processes all `.cpp` and `.hpp` files within a specified
#' directory, including subdirectories, by applying the `process_cpp_file`
#' function to each file. It searches for RD blocks in the files and generates
#' corresponding `.Rd` files.
#'
#' @param dir_path A string specifying the path to the directory containing C++ 
#' and header files.
process_directory <- function(dir_path) {
  cpp_files <- fs::dir_ls(dir_path, recurse = TRUE, glob = "*.cpp")
  
  for (cpp_file in cpp_files) {
    message("Processing file: ", cpp_file)
    process_cpp_file(cpp_file)
  }
  
   hpp_files <- fs::dir_ls(dir_path, recurse = TRUE, glob = "*.hpp")
  
  for (hpp_file in hpp_files) {
    message("Processing file: ", hpp_file)
    process_cpp_file(hpp_file)
  }
}


# process_directory("inst/include/interface/rcpp")
