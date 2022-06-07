
# VALIDATION FUNCTIONS --------------------------------------------------------
# These validate functions must be the first entries of this file otherwise
# the documentation via [document()] will not work.
# Another option is to use @include in the documentation of the class and put
# the validation functions in another file.
#' @export
validFIMSFrame <- function(object) {
 # validation code goes here
 TRUE
}

#' @export
validFIMSFrameAge <- function(object) {
 # validation code goes here
 TRUE
}

# DATA CLASSES ----------------------------------------------------------------

# Class to hold the dataframe

# example: https://github.com/rbchan/unmarked/blob/master/R/unmarkedFrame.R
# Note that it's a little outdated. Check Advanced R and R pkgs.

#' FIMSframe Class
#'
#' Some details about this class and my plans for it in the body.
#'
#' @slot data Data for FIMS
setClass("FIMSFrame",
    slots = list(
        data = "data.frame"
    ),
    validity = validFIMSFrame
)
#' The title for my S4 class that extends \code{"character"} class.
#'
#' Some details about this class and my plans for it in the body.
#' @name FIMSFrameAge-class
#' @rdname FIMSFrameAge-class
#' @exportClass FIMSFrameAge
setClass("FIMSFrameAge",
    slots = list(
        weightatage = "vector",
        ages = "vector"
    ),
    contains = "FIMSFrame",
    validity = validFIMSFrameAge
)


# CONSTRUCTORS ----------------------------------------------------------------
#' FIMSFrame base class constructor
#' @export
FIMSFrame <- function(data) {
  stopifnot(inherits(data, "data.frame"))

  # Calculate information based on input data
  out <- new("FIMSFrame",
    data = data
  )

  return(out)
}
#' FIMSFrameAge
#'
#' @export
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

# SELECTORS -------------------------------------------------------------------
# these can define for e.g., subsetting functions. This could 
# be helpful for example if you want to take out all data for a fleet
# across multiple slots in the object.

# Accessors -----
# Methods for accessing info in the slots
# e.g., something like name.

# Show and plot methods ----

# show method
setMethod("show", "FIMSFrame", function(object) 
{
  dat_types <- unique(object@data[["type"]])
  beg_of_obj <- head(object@data)
  cat("This is a FIMSFrameAge data.frame", "\n", "\n",
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

