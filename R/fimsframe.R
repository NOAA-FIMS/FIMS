
# VALIDATION FUNCTIONS ----
# These validate functions must be the first entries of this file otherwise
# [document()] will not work.
# Another option is to use @include in the documentation of the class and put
# the validation functions in another file.
validFIMSFrame <- function(object) {
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

  # Return
  if (length(errors) == 0) {
    return(TRUE)
  } else {
    return(errors)
  }
}

validFIMSFrameAge <- function(object) {
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

# DATA CLASSES ----
# example: https://github.com/rbchan/unmarked/blob/master/R/unmarkedFrame.R
# Note that the reference is a little outdated. Check Advanced R and R pkgs.

#' FIMSFrame class
#'
#' The FIMSFrame class has just one slot for a `data.frame`.
#' The class is extended by other classes specific to certain model types`.
#'
#' @aliases FIMSFrame
#' @rdname FIMSFrame
#' @slot data A `data.frame` that stores data used to fit a FIMS model.
#'   `data` can contain as many columns as you want and in any order.
#'   But, the following columns are mandatory:
#'   `type`, `datestart`, `dateend`, value, unit, and uncertainty.

setClass("FIMSFrame",
    slots = list(
        data = "data.frame"
    ),
    validity = validFIMSFrame
)
#' `FIMSFrameAge` is an S4 class that extends the `FIMSFrame` class
#'
#' Some details about this class and my plans for it in the body.
#' @name FIMSFrameAge-class
#' @rdname FIMSFrame-class
#' @exportClass FIMSFrameAge
setClass("FIMSFrameAge",
    slots = list(
        weightatage = "data.frame",
        ages = "numeric"
    ),
    contains = "FIMSFrame",
    validity = validFIMSFrameAge
)


# CONSTRUCTORS ----
#' Base class constructor for FIMSFrame
#'
#' @param data A `data.frame` that contains the necessary columns
#'   to construct a data frame of a given `FIMSFrame-class`, which are useable
#'   by models supported within the \pkg{FIMS} package.
#' @export
#' @rdname FIMSFrame
FIMSFrame <- function(data) {
  stopifnot(inherits(data, "data.frame"))

  # Calculate information based on input data
  out <- new("FIMSFrame",
    data = data
  )

  return(out)
}
#' FIMSFrameAge
#' @export
#' @rdname FIMSFrame
FIMSFrameAge <- function(data) {
  stopifnot(inherits(data, "data.frame"))

  # Calculate information based on input data
  ages <- 0:max(data[["age"]], na.rm = TRUE)
  weightatage <- dplyr::filter(
    data,
    type == "weight-at-age"
  )
  out <- new("FIMSFrameAge",
    data = data,
    ages = ages,
    weightatage = weightatage
  )
  return(out)
}

# SELECTORS ----
# these can define for e.g., subsetting functions. This could 
# be helpful for example if you want to take out all data for a fleet
# across multiple slots in the object.

# Accessors ----
# Methods for accessing info in the slots
# e.g., something like name.

# Show and plot methods ----

# show method
setMethod("show", "FIMSFrame", function(object) 
{
  dat_types <- unique(object@data[["type"]])
  beg_of_obj <- head(object@data)
  cat("This is a FIMSFrame data.frame", "\n", "\n",
      "Data includes types: ", paste0(dat_types, collapse = ", "),
      "\n", "\n"
      )
      print(beg_of_obj)
})

# plot method
setMethod("plot", "FIMSFrameAge", function(x) {
  y <- x@weightatage[["value"]]
  x_axis <- x@weightatage[["age"]]
  plot(x_axis, y, xlab = "Age", ylab = "Weight")
})

