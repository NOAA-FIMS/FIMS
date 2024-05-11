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
  slots = c(
    data = "data.frame", # can use c( ) or list here.
    fleets = "numeric",
    n_yrs = "integer",
    ages = "numeric",
    n_ages = "integer",
    weightatage = "data.frame",
    start_year = "integer",
    end_year = "integer"
  )
)

# setMethod: accessors ----
# Methods for accessing info in the slots

# for now, only getters are included, not setters.
# setter example where ages is the slot and Person is the class
# setGeneric("age<-", function(x, value) standardGeneric("age<-"))
# setMethod("age<-", "Person", function(x, value) {
#   x@age <- value
#   x
# })


# is it problematic to set the generic for data? not sure...
# but it will not work without set generic
# can't call this data because there is already a generic
setGeneric("get_data", function(x) standardGeneric("get_data"))
setMethod("get_data", "FIMSFrame", function(x) x@data)

# example: so we can call fleets(obj) instead of obj@fleets
setGeneric("fleets", function(x) standardGeneric("fleets"))
setMethod("fleets", "FIMSFrame", function(x) x@fleets)

setGeneric("n_yrs", function(x) standardGeneric("n_yrs"))
setMethod("n_yrs", "FIMSFrame", function(x) x@n_yrs)

setGeneric("start_year", function(x) standardGeneric("start_year"))
setMethod("start_year", "FIMSFrame", function(x) x@start_year)

setGeneric("end_year", function(x) standardGeneric("end_year"))
setMethod("end_year", "FIMSFrame", function(x) x@end_year)

setGeneric("ages", function(x) standardGeneric("ages"))
setMethod("ages", "FIMSFrame", function(x) x@ages)

setGeneric("n_ages", function(x) standardGeneric("n_ages"))
setMethod("n_ages", "FIMSFrame", function(x) x@n_ages)

setGeneric("weightatage", function(x) standardGeneric("weightatage"))
setMethod("weightatage", "FIMSFrame", function(x) x@weightatage)

setGeneric("m_weightatage", function(x) standardGeneric("m_weightatage"))
setMethod(
  "m_weightatage", "FIMSFrame",
  function(x) {
    dplyr::filter(
      .data = as.data.frame(x@data),
      .data[["type"]] == "weight-at-age"
    ) |>
      dplyr::group_by(.data[["age"]]) |>
      dplyr::summarize(mean_value = mean(.data[["value"]])) |>
      dplyr::pull(.data[["mean_value"]])
  }
)

setGeneric("m_ages", function(x) standardGeneric("m_ages"))
setMethod("m_ages", "FIMSFrame", function(x) {
  x@ages
})

#' Get the landings data to be used in the model
#'
#' @param x The object containing landings.
#' @export
setGeneric("m_landings", function(x) standardGeneric("m_landings"))

#' Get the landings data to be used in the model
#'
#' @param x The FIMSFrame object containing landings.
#' @export
setMethod(
  "m_landings", "FIMSFrame",
  function(x) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "landings"
    ) |>
      dplyr::pull(.data[["value"]])
  }
)

#' Get the index data to be used in the model
#'
#' @param x The object containing index.
#' @param fleet_name The name of the fleet for the index data.
#' @export
setGeneric("m_index", function(x, fleet_name) standardGeneric("m_index"))

#' Get the index data to be used in the model
#'
#' @param x The FIMSFrame object containing index.
#' @param fleet_name The name of the fleet for the index data.
#' @export
setMethod(
  "m_index", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "index",
      .data[["name"]] == fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)


#' Get the age-composition data to be used in the model
#'
#' @param x The object containing the age-composition data.
#' @param fleet_name The name of the fleet for the age-composition data.
#' @export
setGeneric("m_agecomp", function(x, fleet_name) standardGeneric("m_agecomp"))
# Should we add name as an argument here?

#' Get the age-composition data data to be used in the model
#'
#' @param x  The FIMSFrame containing age-composition data.
#' @param fleet_name  The name of the fleet for the age-composition data.
#' @export
setMethod(
  "m_agecomp", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "age",
      .data[["name"]] == fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)

# Note: don't include setters, because for right now, we don't want users to be
# setting ages, fleets, etc. However, we could allow it in the future, if there
# is away to update the object based on changing the fleets?

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
    print(utils::head(object@data))
    for (nm in snames[ordinnames]) {
      cat("additional slots: ", nm, ":\n", sep = "")
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
    if (!"age" %in% colnames(object@data)) {
      errors <- c(errors, "data must contain 'age'")
    }

    # TODO: Add checks for other slots

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
#'
#' @rdname FIMSFrame
#'
#' @param data A `data.frame` that contains the necessary columns to construct
#'   a data frame of a given `FIMSFrame-class`.
#'
#' @return
#' An object of the S4 class `FIMSFrame` or one of its child classes is 
#' validated and then returned. All objects will at a minimum have a slot
#' called `data` to store the input data frame. Additional slots are dependent
#' on the child class. Use [showClass()] to see all available slots.
#' @export
FIMSFrame <- function(data) {
  # Get the earliest and latest year of data and use to calculate n years for
  # population simulation
  start_year <- as.integer(
    strsplit(min(data[["datestart"]], na.rm = TRUE), "-")[[1]][1]
  )
  end_year <- as.integer(
    strsplit(max(data[["dateend"]], na.rm = TRUE), "-")[[1]][1]
  )
  n_yrs <- as.integer(end_year - start_year + 1)
  years <- start_year:end_year

  # Get the fleets represented in the data
  fleets <- unique(data[["name"]])[grep("fleet", unique(data[["name"]]))]
  fleets <- as.numeric(
    unlist(lapply(strsplit(fleets, "fleet"), function(x) x[2]))
  )
  n_fleets <- length(fleets)
  # Make empty NA data frames in the format needed to pass to FIMS
  # Get the range of ages displayed in the data to use to specify population
  # simulation range
  ages <- min(data[["age"]], na.rm = TRUE):max(data[["age"]], na.rm = TRUE)
  n_ages <- length(ages)
  weightatage <- dplyr::filter(
    data,
    .data[["type"]] == "weight-at-age"
  )

  # Fill the empty data frames with data extracted from the data file
  out <- new("FIMSFrame",
    data = data,
    fleets = fleets,
    n_yrs = n_yrs,
    start_year = start_year,
    end_year = end_year,
    ages = ages,
    n_ages = n_ages,
    weightatage = weightatage
  )
  return(out)
}
