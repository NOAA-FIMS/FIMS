# Developers: ----

# This file defines the parent class FIMSFrame and its potential children. The
# class is an S4 class with accessors and validators but no setters.
#
# The top of this file contains the declaration of the FIMSFrame class, which
# is the controller of everything. Then the function FIMSFrame() is how objects
# of that class are created, i.e., the constructor, and how users will interact
# with the class the most. When the returned object from that constructor are
# changed, the call to methods::setClass() that defines the class must also be changed.
# The remainder of the file is set up to help you easily augment this class.
# Follow the step-by-step instructions in order or at least know that the
# functions are present in this order:
#
# 1. Add or remove the slot of interest in the call to `methods::setClass()`, e.g.,
#    if you are adding a new slot you must declare the slot and the type of
#    object that should be expected in that slot; to remove an object from the
#    FIMSFrame class you must remove the slot here.
# 2. Add an accessor function, e.g., get_*(), to allow users to access the
#    object stored in the new slot; or, remove the accessor function if you
#    remove a slot. Some internal accessors are also available, e.g., m_*(),
#    and should be used to provide data to a model but should not be used by
#    average users.
# 3. If we had setter functions for FIMSFrame, you would add or delete the
#    appropriate setter functions next but we do not. Instead, we want users to
#    re-run FIMSFrame() when they make any changes to their data, that way all
#    of the slots will be updated simultaneously. @nathanvaughan-NOAA mentioned
#    during Code club 2024-12-17 that this may be a problem for future use of
#    FIMSFrame objects, especially when doing MSE or simulation when there is a
#    large overhead in running FIMSFrame and you just want to change a small,
#    simple thing in your data and re-run the model. We will cross that bridge
#    later. @msupernaw also informed us about the ability to lock an R object
#    so it cannot be altered. See https://rdrr.io/r/base/bindenv.html.
# 4. Augment the validator functions to ensure that users do not pass
#    incompatible information to FIMSFrame().
# 5. Augment FIMSFrame() to ensure that the slot is created if you are adding a
#    new object or remove the object from the returned object if you are
#    removing a slot.

# TODO: ----

# TODO: change @data to return a tibble
# TODO: remove or change get_fleets to return fleet names in alphabetized order
# TODO: n_fleets should store total number of fleets, i.e., fishing + survey
# TODO: determine how to dplyr::arrange() the data and pass necessary info
# TODO: implement sorting of information in terms of alphabetized fleet order

# methods::setClass: ----

# Classes are not currently exported, and therefore, do not need documentation.
# See the following link if we do want to document them in the future:
# https://stackoverflow.com/questions/7368262/how-to-properly-document-s4-class-slots-using-roxygen2

methods::setClass(
  Class = "FIMSFrame",
  slots = c(
    data = "data.frame", # can use c( ) or list here.
    fleets = "numeric",
    n_years = "integer",
    ages = "numeric",
    n_ages = "integer",
    lengths = "numeric",
    n_lengths = "integer",
    start_year = "integer",
    end_year = "integer"
  )
)

# methods::setMethod: accessors ----

# Methods for accessing info in the slots using get_*() or m_*()

#' Get a slot in a FIMSFrame object
#'
#' There is an accessor function for each slot in the S4 class `FIMSFrame`,
#' where the function is named `get_*()` and the star can be replaced with the
#' slot name, e.g., [get_data()]. These accessor functions are the preferred
#' way to access objects stored in the available slots.
#'
#' @param x An object returned from [FIMSFrame()].
#' @name get_FIMSFrame
NULL

#' @return
#' [get_data()] returns a data frame of the class of `data.frame` containing
#' data for a FIMS model in a long format. The data frame will have the
#' following columns:
#' `r glue::glue_collapse(colnames(data1), sep = ", ", last = ", and ")`.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_data", function(x) standardGeneric("get_data"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_data", "FIMSFrame", function(x) x@data)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_data",
  "data.frame",
  function(x) FIMSFrame(x)@data
)

#' @return
#' [get_fleets()] returns a vector of integer values specifying which fleets in
#' the model are fishing fleets.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_fleets", function(x) standardGeneric("get_fleets"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_fleets", "FIMSFrame", function(x) x@fleets)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_fleets",
  "data.frame",
  function(x) FIMSFrame(x)@fleets
)

#' @return
#' [get_n_years()] returns an integer specifying the number of years in the
#' model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_n_years", function(x) standardGeneric("get_n_years"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_n_years", "FIMSFrame", function(x) x@n_years)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_n_years",
  "data.frame",
  function(x) FIMSFrame(x)@n_years
)

#' @return
#' [get_start_year()] returns an integer specifying the start year of the
#' model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_start_year", function(x) standardGeneric("get_start_year"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_start_year", "FIMSFrame", function(x) x@start_year)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_start_year",
  "data.frame",
  function(x) FIMSFrame(x)@start_year
)

#' @return
#' [get_end_year()] returns an integer specifying the end year of the
#' model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_end_year", function(x) standardGeneric("get_end_year"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_end_year", "FIMSFrame", function(x) x@end_year)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_end_year",
  "data.frame",
  function(x) FIMSFrame(x)@end_year
)

#' @return
#' [get_ages()] returns a vector of age bins used in the model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_ages", function(x) standardGeneric("get_ages"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_ages", "FIMSFrame", function(x) x@ages)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_ages",
  "data.frame",
  function(x) FIMSFrame(x)@ages
)

#' @return
#' [get_n_ages()] returns an integer specifying the number of age bins used in
#' the model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_n_ages", function(x) standardGeneric("get_n_ages"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_n_ages", "FIMSFrame", function(x) x@n_ages)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_n_ages",
  "data.frame",
  function(x) FIMSFrame(x)@n_ages
)

#' @return
#' [get_lengths()] returns a vector of length bins used in the model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric("get_lengths", function(x) standardGeneric("get_lengths"))
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_lengths", "FIMSFrame", function(x) x@lengths)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_lengths",
  "data.frame",
  function(x) FIMSFrame(x)@lengths
)

#' @return
#' [get_n_lengths()] returns an integer specifying the number of length bins
#' used in the model.
#' @export
#' @rdname get_FIMSFrame
methods::setGeneric(
  "get_n_lengths",
  function(x) standardGeneric("get_n_lengths")
)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod("get_n_lengths", "FIMSFrame", function(x) x@n_lengths)
#' @export
#' @rdname get_FIMSFrame
methods::setMethod(
  "get_n_lengths",
  "data.frame",
  function(x) FIMSFrame(x)@n_lengths
)

#' Get a vector of data to be passed to a FIMS module from a FIMSFrame object
#'
#' There is an accessor function for each data type needed to run a FIMS model.
#' A FIMS model accepts vectors of data and thus each of the `m_*()` functions,
#' where the star can be replaced with the data type separated by underscores,
#' e.g., weight_at_age. These accessor functions are the preferred way to pass
#' data to a FIMS module because the data will have the appropriate indexing.
#'
#' @details
#' Age-to-length-conversion data, i.e., the proportion of age "a" that are
#' length "l", are used to convert lengths (input data) to ages (modeled) as
#' a way to fit length data without estimating growth.
#'
#' @inheritParams get_data
#' @param fleet_name A string, or vector of strings, specifying the name of the
#'   fleet(s) of interest that you want landings data for. The strings must
#'   exactly match strings in the column `"name"` of `get_data(x)`.
#' @return
#' All of the `m_*()` functions return vectors of data. Currently, the order of
#' the data is the same order as the data frame because no arranging is done in
#' [FIMSFrame()] and the function just extracts the appropriate column.
#' @name m_
NULL

#' @export
#' @rdname m_
methods::setGeneric(
  "m_landings",
  function(x, fleet_name) standardGeneric("m_landings")
)
#' @export
#' @rdname m_
methods::setMethod(
  "m_landings", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "landings",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @export
#' @rdname m_
methods::setMethod(
  "m_landings",
  "data.frame",
  function(x, fleet_name) m_landings(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname m_
methods::setGeneric(
  "m_index",
  function(x, fleet_name) standardGeneric("m_index")
)
#' @export
#' @rdname m_
methods::setMethod(
  "m_index", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "index",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @export
#' @rdname m_
methods::setMethod(
  "m_index",
  "data.frame",
  function(x, fleet_name) m_index(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname m_
methods::setGeneric("m_agecomp", function(x, fleet_name) standardGeneric("m_agecomp"))
#' @export
#' @rdname m_
methods::setMethod(
  "m_agecomp", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "age",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @export
#' @rdname m_
methods::setMethod(
  "m_agecomp",
  "data.frame",
  function(x, fleet_name) m_agecomp(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname m_
methods::setGeneric(
  "m_lengthcomp",
  function(x, fleet_name) standardGeneric("m_lengthcomp")
)
#' @export
#' @rdname m_
methods::setMethod(
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
#' @export
#' @rdname m_
methods::setMethod(
  "m_lengthcomp",
  "data.frame",
  function(x, fleet_name) m_lengthcomp(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname m_
methods::setGeneric(
  "m_weight_at_age",
  function(x) standardGeneric("m_weight_at_age")
)
#' @export
#' @rdname m_
methods::setMethod(
  "m_weight_at_age",
  "FIMSFrame",
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
#' @export
#' @rdname m_
methods::setMethod(
  "m_weight_at_age",
  "data.frame",
  function(x) {
    m_weight_at_age(FIMSFrame(x))
  }
)

#' @export
#' @rdname m_
methods::setGeneric(
  "m_age_to_length_conversion",
  function(x, fleet_name) standardGeneric("m_age_to_length_conversion")
)
#' @export
#' @rdname m_
methods::setMethod(
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
#' @export
#' @rdname m_
methods::setMethod(
  "m_age_to_length_conversion",
  "data.frame",
  function(x, fleet_name) m_age_to_length_conversion(FIMSFrame(x), fleet_name)
)

# methods::setMethod: initialize ----

# Not currently using methods::setMethod(f = "initialize")
# because @kellijohnson-NOAA did not quite understand how they actually work.

# methods::setMethod: plot ----

methods::setMethod(
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

# methods::setMethod: show ----

methods::setMethod(
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

# methods::setValidity ----

methods::setValidity(
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
      object@data[["datestart"]]
    )
    grepl_dateend <- grepl(
      "[0-9]{1,4}-[0-9]{1,2}-[0-9]{1-2}",
      object@data[["dateend"]]
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
  if (!any(c("age", "length") %in% the_column_names)) {
    errors <- c(errors, "data must contain 'ages' and/or 'lengths'")
  }
  return(errors)
}

# Constructors ----

# All constructors in this file are documented in 1 roxygen file via @rdname.

#' Class constructors for `FIMSFrame` and associated child classes
#'
#' All constructor functions take a single input and build an object specific
#' to the needs of each model type within \pkg{FIMS}. `FIMSFrame` is the parent
#' class. Future, associated child classes will have the additional slots
#' needed for different types of models.
#'
#' @rdname FIMSFrame
#'
#' @param data A `data.frame` that contains the necessary columns to construct
#'   a `FIMSFrame-class` object. Currently, those columns are
#'   `r glue::glue_collapse(colnames(data1), sep = ", ", last = ", and ")`. See
#'   the data1 object in FIMS, e.g., `data(data1, package = "FIMS")`.
#'
#' @return
#' An object of the S4 class `FIMSFrame` class, or one of its child classes, is
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
    # ages <- sort(unique(data[["age"]]))
    # ages <- ages[!is.na(ages)]
  } else {
    ages <- numeric()
  }
  n_ages <- length(ages)

  if ("length" %in% colnames(data)) {
    lengths <- sort(unique(data[["length"]]))
    lengths <- lengths[!is.na(lengths)]
  } else {
    lengths <- numeric()
  }
  n_lengths <- length(lengths)

  # Fill the empty data frames with data extracted from the data file
  out <- methods::new("FIMSFrame",
    data = data,
    fleets = fleets,
    n_years = n_years,
    start_year = start_year,
    end_year = end_year,
    ages = ages,
    n_ages = n_ages,
    lengths = lengths,
    n_lengths = n_lengths
  )
  return(out)
}
