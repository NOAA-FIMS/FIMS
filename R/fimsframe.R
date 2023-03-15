# This file defines a parent class and its children by
# (1) setting the class;
# (2) defining methods, using setMethod();
# (3) setting the validators; and
# (4) establishing the constructors (i.e., functions called by users)
# where only the constructors are documented using roxygen.

# setClass: ----
# Classes are not currently exported, and therefore, do not need documentation.
# See the following link if we do want to document them in the future:
# https://stackoverflow.com/questions/7368262/how-to-properly-document-s4-class-slots-using-roxygen2

setClass(
  Class = "FIMSFrame",
  slots = c(data = "data.frame")
)

setClass("FIMSFrameAge",
  slots = list(
    weightatage = "data.frame",
    ages = "numeric"
  ),
  contains = "FIMSFrame"
)

# setMethod: accessors ----
# Methods for accessing info in the slots

# setMethod: initialize ----
# Not currently using setMethod(f = "initialize")
# because @kellijohnson-NOAA did not quite understand how they actually work.

# setMethod: plot ----
setMethod(
  f = "plot",
  signature = "FIMSFrame",
  definition = function(x) {
    ggplot2::ggplot(
      data = x@data,
      mapping = ggplot2::aes(
        x = as.Date(.data[["datestart"]]),
        y = .data[["value"]],
        col = .data[["name"]]
      )
    ) +
      # Using Set3 b/c it is the palette with the largest number of colors
      # and not {nmfspalette} b/c didn't want to depend on GitHub package
      ggplot2::scale_color_brewer(palette = "Set3") +
      ggplot2::facet_wrap("type", scales = "free_y") +
      ggplot2::geom_point() +
      ggplot2::scale_x_date(labels = scales::date_format("%Y-%m-%d")) +
      ggplot2::xlab("Start date (Year-Month-Day)") +
      ggplot2::ylab("Value") +
      ggplot2::theme(
        axis.text.x = ggplot2::element_text(angle = 15)
      )
  }
)

setMethod(
  f = "plot",
  signature = "FIMSFrameAge",
  definition = function(x) {
    y <- x@weightatage[["value"]]
    x_axis <- x@weightatage[["age"]]
    plot(x_axis, y, xlab = "Age", ylab = "Weight")
  }
)

# setMethod: show ----
setMethod(
  f = "show",
  signature = "FIMSFrame",
  definition = function(object) {
    message("data.frame of class '", class(object), "'")
    if (length(object@data) == 0) {
      return()
    }
    dat_types <- unique(object@data[[which(colnames(object@data) == "type")]])
    message("with the following 'types': ", paste0(dat_types, collapse = ", "))
    snames <- slotNames(object)
    ordinnames <- !snames %in% c(
      "data",
      ".S3Class",
      "row.names",
      "names"
    )
    print(head(object@data))
    for (nm in snames[ordinnames]) {
      cat("+@", nm, ":\n", sep = "")
      print(slot(object, nm))
    }
  }
)

# setValidity ----
setValidity(
  Class = "FIMSFrame",
  method = function(object) {
    errors <- character()

    if (NROW(object@data) == 0) {
      errors <- c(errors, "data must have at least one row")
    }

    # Check columns
    if (!"type" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'type'")
    }
    if (!"datestart" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'datestart'")
    }
    if (!"dateend" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'dateend'")
    }
    if (!"dateend" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'value'")
    }
    if (!"dateend" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'unit'")
    }
    if (!"dateend" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'uncertainty'")
    }

    # Return
    if (length(errors) == 0) {
      return(TRUE)
    } else {
      return(errors)
    }
  }
)

setValidity(
  Class = "FIMSFrameAge",
  method = function(object) {
    errors <- character()

    # Check columns
    if (!"age" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'age'")
    }

    # Return
    if (length(errors) == 0) {
      return(TRUE)
    } else {
      return(errors)
    }
  }
)

# Constructors ----
# All constructors in this file are documented in 1 roxygen file via @rdname.

#' Class constructors for `FIMSFrame` and associated child classes
#'
#' All constructor functions take a single input and build an object specific to
#' the needs of each model type within \pkg{FIMS}. `FIMSFrame` is the
#' parent class and the associated child classes have additional slots needed
#' for each model type.
#' @export
#' @rdname FIMSFrame
#' @param data A `data.frame` that contains the necessary columns
#'   to construct a data frame of a given `FIMSFrame-class`.
#' @return An object of the S4 class `FIMSFrame` or one of its child classes
#' is validated and then returned. All objects will at a minimum have a slot
#' called `data` to store the input data frame. Additional slots are dependent
#' on the child class. Use [showClass()] to see all available slots.
FIMSFrame <- function(data) {
  #Get the earliest and latest year of data and use to calculate n years for population simulation
  start_yr <- as.numeric(strsplit(min(data[["datestart"]],na.rm=TRUE),"-")[[1]][1])
  end_yr <- as.numeric(strsplit(max(data[["dateend"]],na.rm=TRUE),"-")[[1]][1])
  nyrs <- end_yr-start_yr+1
  years <- start_yr:end_yr
  #Get the range of ages displayed in the data to use to specify population simulation range
  #with one extra year added to act as a plus group
  ages <- 0:(max(data[["age"]],na.rm=TRUE)+1)
  #Get the fleets represented in the data
  fleets <- unique(data[["name"]])[grep("fleet",unique(data[["name"]]))]
  fleets <- as.numeric(unlist(lapply(strsplit(fleets,"fleet"),function(x)x[2])))
  
  #Make empty NA data frames in the format needed to pass to FIMS
  
  
  
  #Fill the empty data frames with data extracted from the data file
  
  
  out <- new("FIMSFrame", data = data)
  return(out)
}
#' FIMSFrameAge
#' @export
#' @rdname FIMSFrame
FIMSFrameAge <- function(data) {
  # Calculate information based on input data
  ages <- 0:max(data[["age"]], na.rm = TRUE)
  weightatage <- dplyr::filter(
    data,
    .data[["type"]] == "weight-at-age"
  )
  # TODO: decide if weightatage info should be removed
  #       from data because it is in weightatage?
  out <- new("FIMSFrameAge",
    data = data,
    ages = ages,
    weightatage = weightatage
  )
  return(out)
}
