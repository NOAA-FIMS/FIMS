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
    n_years = "integer",
    ages = "numeric",
    n_ages = "integer",
    lengths = "numeric",
    n_lengths = "integer",
    weight_at_age = "data.frame",
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
setMethod(
  "get_data",
  "data.frame",
  function(x) FIMSFrame(x)@data
)

# example: so we can call fleets(obj) instead of obj@fleets
setGeneric("fleets", function(x) standardGeneric("fleets"))
setMethod("fleets", "FIMSFrame", function(x) x@fleets)
setMethod(
  "fleets",
  "data.frame",
  function(x) FIMSFrame(x)@fleets
)

setGeneric("n_years", function(x) standardGeneric("n_years"))
setMethod("n_years", "FIMSFrame", function(x) x@n_years)
setMethod(
  "n_years",
  "data.frame",
  function(x) FIMSFrame(x)@n_years
)

setGeneric("start_year", function(x) standardGeneric("start_year"))
setMethod("start_year", "FIMSFrame", function(x) x@start_year)
setMethod(
  "start_year",
  "data.frame",
  function(x) FIMSFrame(x)@start_year
)

setGeneric("end_year", function(x) standardGeneric("end_year"))
setMethod("end_year", "FIMSFrame", function(x) x@end_year)
setMethod(
  "end_year",
  "data.frame",
  function(x) FIMSFrame(x)@end_year
)

setGeneric("ages", function(x) standardGeneric("ages"))
setMethod("ages", "FIMSFrame", function(x) x@ages)
setMethod(
  "ages",
  "data.frame",
  function(x) FIMSFrame(x)@ages
)

setGeneric("n_ages", function(x) standardGeneric("n_ages"))
setMethod("n_ages", "FIMSFrame", function(x) x@n_ages)
setMethod(
  "n_ages",
  "data.frame",
  function(x) FIMSFrame(x)@n_ages
)

setGeneric("get_lengths", function(x) standardGeneric("get_lengths"))
setMethod("get_lengths", "FIMSFrame", function(x) x@lengths)
setMethod(
  "get_lengths",
  "data.frame",
  function(x) FIMSFrame(x)@lengths
)

setGeneric("n_lengths", function(x) standardGeneric("n_lengths"))
setMethod("n_lengths", "FIMSFrame", function(x) x@n_lengths)
setMethod(
  "n_lengths",
  "data.frame",
  function(x) FIMSFrame(x)@n_lengths
)

setGeneric("weight_at_age", function(x) standardGeneric("weight_at_age"))
setMethod("weight_at_age", "FIMSFrame", function(x) x@weight_at_age)
setMethod(
  "weight_at_age",
  "data.frame",
  function(x) FIMSFrame(x)@weight_at_age
)

# TODO: create accessors for all of the model, i.e., m_*, functions that allow
# things to work if you pass it a data frame rather than a FIMSFrame, i.e.,
# first convert it to a FIMSFrame and then pass it to the other generic.

#' Get the weight at age data to be used in the model
#'
#' @param x The object containing weight at age data.
#' @export
setGeneric("m_weight_at_age", function(x) standardGeneric("m_weight_at_age"))
setMethod(
  "m_weight_at_age", "FIMSFrame",
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

# TODO: do we need m_ages()? ages can be extracted using data@ages.
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

#' Get the length-composition data from a FIMSFrame object
#'
#' @param x A FIMSFrame object with length-composition data.
#' @param fleet_name A string providing the name of the fleet for which you
#'   want the length-composition data for. Technically, you can pass it a
#'   vector of strings but it is more common to pass it a single string.
#' @return
#' A vector of length-composition data. The order of the vector is the order
#' the data frame was in before this function was called because it is just
#' extracting a column.
#' @export
#' @rdname m_lengthcomp
setGeneric(
  "m_lengthcomp",
  function(x, fleet_name) standardGeneric("m_lengthcomp")
)
#' @export
#' @rdname m_lengthcomp
setMethod(
  "m_lengthcomp",
  "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "length",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)

#' Get the age-to-length-conversion data from a FIMSFrame object
#'
#' @param x A FIMSFrame object with age-to-length-conversion data (i.e.,
#'   the proportion of age "a" that are length "l").
#' @inheritParams m_lengthcomp
#' @return
#' A vector of age-to-length-conversion data. The order of the vector is the
#' order the data frame was in before this function was called because it is
#' just extracting a column.
#' @export
#' @rdname m_age_to_length_conversion
setGeneric(
  "m_age_to_length_conversion",
  function(x, fleet_name) standardGeneric("m_age_to_length_conversion")
)
#' @export
#' @rdname m_age_to_length_conversion
setMethod(
  "m_age_to_length_conversion",
  "FIMSFrame",
  function(x, fleet_name) {
    if ("length" %in% colnames(x@data)) {
      dplyr::filter(
        .data = as.data.frame(x@data),
        .data[["type"]] == "age-to-length-conversion",
        .data[["name"]] %in% fleet_name
      ) |>
        dplyr::group_by(.data[["age"]], .data[["length"]]) |>
        dplyr::summarize(
          mean_value = mean(as.numeric(.data[["value"]]), na.rm = TRUE)
        ) |>
        dplyr::pull(as.numeric(.data[["mean_value"]]))
    }
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
  definition = function(x, y, ...) {
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

is.FIMSFrame <- function(x) {
  inherits(x, "FIMSFrame")
}

# setValidity ----
setValidity(
  Class = "FIMSFrame",
  method = function(object) {
    errors <- character()

    if (NROW(object@data) == 0) {
      errors <- c(errors, "data must have at least one row")
    }

    errors <- c(errors, validate_data_colnames(object@data))

    # Add checks for other slots
    # Check the format for acceptable variants of the ideal yyyy-mm-dd
    grepl_datestart <- grepl(
      "[0-9]{1,4}-[0-9]{1,2}-[0-9]{1-2}",
      data_mile1[["datestart"]]
    )
    grepl_dateend <- grepl(
      "[0-9]{1,4}-[0-9]{1,2}-[0-9]{1-2}",
      data_mile1[["dateend"]]
    )
    if (!all(grepl_datestart)) {
      errors <- c(errors, "datestart must be in 'yyyy-mm-dd' format")
    }
    if (!all(grepl_dateend)) {
      errors <- c(errors, "dateend must be in 'yyyy-mm-dd' format")
    }

    # Return
    if (length(errors) == 0) {
      return(TRUE)
    } else {
      return(errors)
    }
  }
)

validate_data_colnames <- function(data) {
  the_column_names <- colnames(data)
  errors <- character()
  if (!"type" %in% the_column_names) {
    errors <- c(errors, "data must contain 'type'")
  }
  if (!"name" %in% the_column_names) {
    errors <- c(errors, "data must contain 'name'")
  }
  if (!"datestart" %in% the_column_names) {
    errors <- c(errors, "data must contain 'datestart'")
  }
  if (!"dateend" %in% the_column_names) {
    errors <- c(errors, "data must contain 'dateend'")
  }
  if (!"dateend" %in% the_column_names) {
    errors <- c(errors, "data must contain 'value'")
  }
  if (!"dateend" %in% the_column_names) {
    errors <- c(errors, "data must contain 'unit'")
  }
  if (!"dateend" %in% the_column_names) {
    errors <- c(errors, "data must contain 'uncertainty'")
  }
  if (!"age" %in% the_column_names) {
    errors <- c(errors, "data must contain 'age'")
  }
  return(errors)
}

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
  errors <- validate_data_colnames(data)
  if (length(errors) > 0) {
    stop(
      "Check the columns of your data, the following are missing:\n",
      paste(errors, sep = "\n", collapse = "\n")
    )
  }
  # datestart and dateend need to be date classes so leading zeros are present
  # but writing and reading from csv file removes the classes so they must be
  # enforced here
  # e.g., 0004-01-01 for January 01 0004
  date_formats <- c("%Y-%m-%d")
  data[["datestart"]] <- as.Date(data[["datestart"]], tryFormats = date_formats)
  data[["dateend"]] <- as.Date(data[["dateend"]], tryFormats = date_formats)

  # Get the earliest and latest year formatted as a string of 4 integers
  start_year <- as.integer(format(
    as.Date(min(data[["datestart"]], na.rm = TRUE), tryFormats = date_formats),
    "%Y"
  ))
  end_year <- as.integer(format(
    as.Date(max(data[["dateend"]], na.rm = TRUE), tryFormats = date_formats),
    "%Y"
  ))
  n_years <- as.integer(end_year - start_year + 1)
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
  if ("age" %in% colnames(data)) {
    ages <- min(data[["age"]], na.rm = TRUE):max(data[["age"]], na.rm = TRUE)
  } else {
    ages <- numeric()
  }
  n_ages <- length(ages)
  if ("length" %in% colnames(data)) {
    lengths <- min(data[["length"]], na.rm = TRUE):
      max(data[["length"]], na.rm = TRUE)
  } else {
    lengths <- numeric()
  }
  n_lengths <- length(lengths)

  weight_at_age <- dplyr::filter(
    data,
    .data[["type"]] == "weight-at-age"
  )

  # Fill the empty data frames with data extracted from the data file
  out <- new("FIMSFrame",
    data = data,
    fleets = fleets,
    n_years = n_years,
    start_year = start_year,
    end_year = end_year,
    ages = ages,
    n_ages = n_ages,
    lengths = lengths,
    n_lengths = n_lengths,
    weight_at_age = weight_at_age
  )
  return(out)
}
