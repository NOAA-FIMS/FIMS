#' C++ Classes Exported via Rcpp
#'
#' The following classes are exported from C++ via Rcpp. Their documentation is
#' provided in the rendered C++ documentation. For detailed usage, see the C++
#' docs.
#'
#' @section C++ Documentation:
#' [NOAA-FIMS C++ Documentation](https://noaa-fims.github.io/FIMS/doxygen/)
#'
#' @name Cpp_classes
#' @aliases AgeComp BevertonHoltRecruitment CatchAtAge DlnormDistribution DmultinomDistribution DnormDistribution DoubleLogisticSelectivity EWAAGrowth VonBertalanffySchnuteGrowth Fleet Index Catch LengthComp LogDevsRecruitmentProcess LogRRecruitmentProcess LogisticMaturity LogisticSelectivity Variable VariableVector Population RealVector SharedInt SharedReal SharedString
#'
#' @details
#' - [AgeComp](https://noaa-fims.github.io/FIMS/doxygen/classAgeCompDataInterface.html)
#' - [BevertonHoltRecruitment](https://noaa-fims.github.io/FIMS/doxygen/classBevertonHoltRecruitmentInterface.html)
#' - [CatchAtAge](https://noaa-fims.github.io/FIMS/doxygen/classCatchAtAgeInterface.html)
#' - [DlnormDistribution](https://noaa-fims.github.io/FIMS/doxygen/classDlnormDistributionsInterface.html)
#' - [DmultinomDistribution](https://noaa-fims.github.io/FIMS/doxygen/classDmultinomDistributionsInterface.html)
#' - [DnormDistribution](https://noaa-fims.github.io/FIMS/doxygen/classDnormDistributionsInterface.html)
#' - [DoubleLogisticSelectivity](https://noaa-fims.github.io/FIMS/doxygen/classDoubleLogisticSelectivityInterface.html)
#' - [EWAAGrowth](https://noaa-fims.github.io/FIMS/doxygen/classEWAAGrowthInterface.html)
#' - [VonBertalanffySchnuteGrowth](https://noaa-fims.github.io/FIMS/doxygen/classVonBertalanffySchnuteGrowthInterface.html)
#' - [Fleet](https://noaa-fims.github.io/FIMS/doxygen/classFleetInterface.html)
#' - [Index](https://noaa-fims.github.io/FIMS/doxygen/classIndexDataInterface.html)
#' - [Catch](https://noaa-fims.github.io/FIMS/doxygen/classCatchDataInterface.html)
#' - [LengthComp](https://noaa-fims.github.io/FIMS/doxygen/classLengthCompDataInterface.html)
#' - [LogDevsRecruitmentProcess](https://noaa-fims.github.io/FIMS/doxygen/classLogDevsRecruitmentInterface.html)
#' - [LogRRecruitmentProcess](https://noaa-fims.github.io/FIMS/doxygen/classLogRRecruitmentInterface.html)
#' - [LogisticMaturity](https://noaa-fims.github.io/FIMS/doxygen/classLogisticMaturityInterface.html)
#' - [LogisticSelectivity](https://noaa-fims.github.io/FIMS/doxygen/classLogisticSelectivityInterface.html)
#' - [Population](https://noaa-fims.github.io/FIMS/doxygen/classPopulationInterface.html)
#' - [RealVector](https://noaa-fims.github.io/FIMS/doxygen/classRealVector.html)
#' - [SharedInt](https://noaa-fims.github.io/FIMS/doxygen/classSharedInt.html)
#' - [SharedReal](https://noaa-fims.github.io/FIMS/doxygen/classSharedReal.html)
#' - [SharedString](https://noaa-fims.github.io/FIMS/doxygen/classSharedString.html)
#' - [Variable](https://noaa-fims.github.io/FIMS/doxygen/classVariable.html)
#' - [VariableVector](https://noaa-fims.github.io/FIMS/doxygen/classVariableVector.html)
NULL

#' Shared parameter starting values
#'
#' Retrieve the data-independent starting values defined beside their core C++
#' modules, in each module's header. Core C++ classes, Rcpp constructors, and
#' [setup_default_parameters()] use these same R-independent definitions.
#'
#' @details
#' Developers should define recommended values in the corresponding core module
#' header, reuse them in that module's Rcpp constructor, and expose them here
#' when adding R setup support. This function only collects existing definitions;
#' it does not define a second set of defaults.
#'
#' These are suggested starting values, not a complete model configuration or
#' defaults appropriate for every stock. Direct Rcpp users must still supply
#' dimensions, data, module links, estimation types, initial numbers at age,
#' and growth reference ages. Fleet constructors use fishing-fleet mortality;
#' use `survey_log_Fmort` for surveys.
#'
#' Growth variability inputs remain absent in Rcpp constructors because the
#' interpolation and delta-method paths are mutually exclusive. R setup selects
#' interpolation and assigns `length_sd_young` and `length_sd_old` at the
#' data-derived reference ages.
#'
#' `Dnorm` supplies `log_sd` directly. `Dlnorm` supplies both `sd`, the input
#' scale of its R setup helper, and `log_sd`, the parameter scale.
#' Rcpp estimation types remain `"constant"` unless explicitly configured;
#' R setup assigns its existing module-specific estimation settings.
#'
#' Calling this function does not construct or register model objects. Each
#' call returns a fresh list; modifying it does not change future defaults.
#'
#' @return A named list of named numeric vectors for `Logistic`,
#'   `DoubleLogistic`, `BevertonHolt`, `VonBertalanffySchnute`, `Population`,
#'   `Fleet`, `Dnorm`, and `Dlnorm`.
#' @name get_parameter_defaults
#' @examples
#' get_parameter_defaults()[["Logistic"]]
#' get_parameter_defaults()[["Population"]][["log_M"]]
NULL

#' C++ Functions Exported via Rcpp
#'
#' The following functions are exported from C++ via Rcpp. Their documentation
#' is provided in the rendered C++ documentation. For detailed usage, see the
#' C++ docs.
#'
#' @section C++ Documentation:
#' [NOAA-FIMS C++ Documentation](https://noaa-fims.github.io/FIMS/doxygen/)
#'
#' @name Cpp_functions
#' @aliases clear get_fixed get_log get_log_errors get_log_warnings get_parameter_names get_random get_random_names inv_logit log_error log_info log_warning logit set_fixed set_log_throw_on_error set_random CreateTMBModel
#'
#' @details
#' - [clear](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_fixed](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_log](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_log_errors](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_log_warnings](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_parameter_names](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_random](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [get_random_names](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [inv_logit](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [log_error](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [log_info](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [log_warning](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [logit](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [set_fixed](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [set_log_throw_on_error](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [set_random](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
#' - [CreateTMBModel](https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html)
NULL
