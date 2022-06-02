# --------------------------- DATA CLASSES ------------------------------------

# Class to hold the dataframe

# example: https://github.com/rbchan/unmarked/blob/master/R/unmarkedFrame.R
setClass("FIMSFrame",
    representation(
        data = "data.frame",
        weightatage = "vector"
    ),
    validity = validFIMSFrame
)

# ------------------------ VALIDATION FUNCTIONS -------------------------------
validFIMSFrame <- function(object) {
 # validation code goes here
}

# ------------------------------- CONSTRUCTORS --------------------------------


# ----------------------- SELECTORS -------------------------------------------