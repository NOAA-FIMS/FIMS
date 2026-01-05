/**
 * @file fims_modules.hpp
 * @brief Rcpp module definitions. Allows for the use of
 * methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef SRC_FIMS_MODULES_HPP
#define SRC_FIMS_MODULES_HPP

#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_data.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_fleet.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_growth.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_natural_mortality.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_recruitment.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"

RCPP_EXPOSED_CLASS(Parameter)
RCPP_EXPOSED_CLASS(ParameterVector)
RCPP_EXPOSED_CLASS(RealVector)
RCPP_EXPOSED_CLASS(SharedInt)
RCPP_EXPOSED_CLASS(SharedString)
RCPP_EXPOSED_CLASS(SharedReal)
RCPP_EXPOSED_CLASS(SharedBoolean)

/**
 * @brief The `fims` Rcpp module construct, providing declarative code of what
 * the module exposes to R.
 *
 * @details Each element included in the module should have a name, a pointer,
 * and a description separated by commas in that order. Both the name and the
 * description should be wrapped in quotes. The description is printed to the
 * screen when the R function `methods::show()` is used on the object. The
 * available description should exactly match the information found in the
 * brief tag where the function, class, etc. is documented. See the Rcpp
 * vignette for more information on documenting modules, particularly how to
 * include lists for parameters to a function. Each of the functions included
 * in this module should be exported by manually exporting them in
 * R/FIMS-package.R.
 *
 */
RCPP_MODULE(fims) {
  Rcpp::function(
      "CreateTMBModel", &CreateTMBModel,
      "Creates the TMB model object and adds interface objects to it.");
  Rcpp::function("set_fixed", &set_fixed_parameters,
                 "Sets the fixed parameters vector object.");
  Rcpp::function("get_fixed", &get_fixed_parameters_vector,
                 "Gets the fixed parameters vector object.");
  Rcpp::function("set_random", &set_random_parameters,
                 "Sets the random parameters vector object.");
  Rcpp::function("get_random", &get_random_parameters_vector,
                 "Gets the random parameters vector object.");
  Rcpp::function("get_parameter_names", &get_parameter_names,
                 "Gets the parameter names object.");
  Rcpp::function("get_random_names", &get_random_names,
                 "Gets the random effects names object.");
  Rcpp::function("clear", clear,
                 "Clears all pointers/references of a FIMS model.");
  Rcpp::function("get_log", get_log,
                 "Gets the log entries as a string in JSON format.");
  Rcpp::function(
      "get_log_errors", get_log_errors,
      "Gets the error entries from the log as a string in JSON format.");
  Rcpp::function(
      "get_log_warnings", get_log_warnings,
      "Gets the warning entries from the log as a string in JSON format.");
  Rcpp::function(
      "get_log_info", get_log_info,
      "Gets the info entries from the log as a string in JSON format.");
  Rcpp::function("get_log_module", get_log_module,
                 "Gets log entries by module as a string in JSON format.");
  Rcpp::function("write_log", write_log, "If true, writes the log on exit.");
  Rcpp::function("set_log_path", set_log_path,
                 "Sets the path for the log file to be written to.");
  Rcpp::function(
      "init_logging", init_logging,
      "Initializes the logging system, setting all signal handling.");
  Rcpp::function(
      "set_log_throw_on_error", set_log_throw_on_error,
      "If true, throws a runtime exception when an error is logged.");
  Rcpp::function("log_info", log_info,
                 "Adds an info entry to the log from the R environment.");
  Rcpp::function("log_warning", log_warning,
                 "Adds a warning entry to the log from the R environment.");
  Rcpp::function("log_error", log_error,
                 "Adds a error entry to the log from the R environment.");
  Rcpp::function("logit", logit_rcpp,
                 "Applies the logit transformation: -log(b - x) + log(x - a).");
  Rcpp::function(
      "inv_logit", inv_logit_rcpp,
      "Applies the inverse of the logit transformation to a bounded space.");
  Rcpp::class_<Parameter>(
      "Parameter", "An RcppInterface class that defines the Parameter class.")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::initial_value_m,
             "A numeric value specifying the initial value of the parameter.")
      .field("value", &Parameter::final_value_m,
             "A numeric value specifying the final value of the parameter.")
      .field("min", &Parameter::min_m,
             "A numeric value specifying the minimum possible parameter value, "
             "where the default is negative infinity.")
      .field("max", &Parameter::max_m,
             "A numeric value specifying the maximum possible parameter value, "
             "where the default is positive infinity.")
      .field("id", &Parameter::id_m, "unique id for parameter class")
      .field("estimation_type", &Parameter::estimation_type_m,
             "A string that takes three arguments: constant, indicating a "
             "parameter is not estimated; fixed_effects, indicating a "
             "parameter is estimated; and random_effects, indicating a "
             "parameter is estimated; the default is constant.");

  Rcpp::class_<ParameterVector>(
      "ParameterVector",
      "An RcppInterface class that defines the ParameterVector class.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("get", &ParameterVector::get,
              "An internal accessor for calling a position of a "
              "ParameterVector from R.")
      .method("set", &ParameterVector::set,
              "An internal setter for setting a position of a ParameterVector "
              "from R.")
      .method("show", &ParameterVector::show,
              "The printing methods for a ParameterVector.")
      .method("at", &ParameterVector::at,
              "Returns a Parameter at the indicated position given the index "
              "argument.")
      .method("size", &ParameterVector::size,
              "Returns the size of a ParameterVector.")
      .method("resize", &ParameterVector::resize,
              "Resizes a ParameterVector to the desired length.")
      .method("set_all_estimable", &ParameterVector::set_all_estimable,
              "Sets all Parameters within a ParameterVector as estimable.")
      .method("set_all_random", &ParameterVector::set_all_random,
              "Sets all Parameters within a ParameterVector as random effects.")
      .method("fill", &ParameterVector::fill,
              "Sets the value of all Parameters in the ParameterVector to the "
              "provided value.")
      .method("get_id", &ParameterVector::get_id,
              "Gets the ID of the ParameterVector object.");
  Rcpp::class_<RealVector>(
      "RealVector", "An RcppInterface class that defines the RealVector class.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method(
          "get", &RealVector::get,
          "An internal accessor for calling a position of a RealVector from R.")
      .method(
          "set", &RealVector::set,
          "An internal setter for setting a position of a RealVector from R.")
      .method("fromRVector", &RealVector::fromRVector,
              "Initializes the RealVector from the values of a R vector.")
      .method("toRVector", &RealVector::toRVector,
              "Returns values as a R vector.")
      .method("show", &RealVector::show,
              "The printing methods for a RealVector.")
      .method("at", &RealVector::at,
              "Returns a double at the indicated position given the index "
              "argument.")
      .method("size", &RealVector::size, "Returns the size of a RealVector.")
      .method("resize", &RealVector::resize,
              "Resizes a RealVector to the desired length.")
      .method("get_id", &RealVector::get_id,
              "Gets the ID of the RealVector object.");

  Rcpp::class_<SharedInt>(
      "SharedInt", "An RcppInterface class that defines the SharedInt class.")
      .constructor()
      .constructor<int>()
      .method("get", &SharedInt::get)
      .method("set", &SharedInt::set);

  Rcpp::class_<SharedString>(
      "SharedString",
      "An RcppInterface class that defines the SharedString class.")
      .constructor()
      .constructor<std::string>()
      .method("get", &SharedString::get)
      .method("set", &SharedString::set);

  Rcpp::class_<SharedBoolean>(
      "SharedBoolean",
      "An RcppInterface class that defines the SharedBoolean class.")
      .constructor()
      .constructor<bool>()
      .method("get", &SharedBoolean::get)
      .method("set", &SharedBoolean::set);

  Rcpp::class_<SharedReal>(
      "SharedReal", "An RcppInterface class that defines the SharedReal class.")
      .constructor()
      .constructor<double>()
      .method("get", &SharedReal::get)
      .method("set", &SharedReal::set);

  Rcpp::class_<BevertonHoltRecruitmentInterface>("BevertonHoltRecruitment")
      .constructor()
      .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep)
      .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero)
      .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs)
      .field("log_r", &BevertonHoltRecruitmentInterface::log_r,
             "recruitment as a random effect on the natural log scale")
      .field("log_expected_recruitment",
             &BevertonHoltRecruitmentInterface::log_expected_recruitment,
             "expected recruitment as a random effect on the natural log scale")
      .field("n_years", &BevertonHoltRecruitmentInterface::n_years,
             "Number of years")
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
      .method("SetRecruitmentProcessID",
              &BevertonHoltRecruitmentInterface::SetRecruitmentProcessID,
              "Set unique ID for recruitment process")
      .method("evaluate_mean",
              &BevertonHoltRecruitmentInterface::evaluate_mean);

  Rcpp::class_<LogDevsRecruitmentInterface>("LogDevsRecruitmentProcess")
      .constructor()
      .method("get_id", &LogDevsRecruitmentInterface::get_id)
      .method("evaluate_process",
              &LogDevsRecruitmentInterface::evaluate_process);

  Rcpp::class_<LogRRecruitmentInterface>("LogRRecruitmentProcess")
      .constructor()
      .method("get_id", &LogRRecruitmentInterface::get_id)
      .method("evaluate_process", &LogRRecruitmentInterface::evaluate_process);

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("n_ages", &FleetInterface::n_ages)
      .field("n_years", &FleetInterface::n_years)
      .field("n_lengths", &FleetInterface::n_lengths)
      .field("observed_landings_units",
             &FleetInterface::observed_landings_units)
      .field("observed_index_units", &FleetInterface::observed_index_units)
      .field("index_expected", &FleetInterface::derived_index_expected)
      .field("landings_expected", &FleetInterface::derived_landings_expected)
      .field("log_index_expected", &FleetInterface::log_index_expected)
      .field("log_landings_expected", &FleetInterface::log_landings_expected)
      .field("agecomp_expected", &FleetInterface::agecomp_expected)
      .field("lengthcomp_expected", &FleetInterface::lengthcomp_expected)
      .field("agecomp_proportion", &FleetInterface::agecomp_proportion)
      .field("lengthcomp_proportion", &FleetInterface::lengthcomp_proportion)
      .field("age_to_length_conversion",
             &FleetInterface::age_to_length_conversion)
      .method("get_id", &FleetInterface::get_id)
      .method("SetName", &FleetInterface::SetName)
      .method("GetName", &FleetInterface::GetName)
      .method("SetObservedAgeCompDataID",
              &FleetInterface::SetObservedAgeCompDataID)
      .method("GetObservedAgeCompDataID",
              &FleetInterface::GetObservedAgeCompDataID)
      .method("SetObservedLengthCompDataID",
              &FleetInterface::SetObservedLengthCompDataID)
      .method("GetObservedLengthCompDataID",
              &FleetInterface::GetObservedLengthCompDataID)
      .method("SetObservedIndexDataID", &FleetInterface::SetObservedIndexDataID)
      .method("GetObservedIndexDataID", &FleetInterface::GetObservedIndexDataID)
      .method("SetObservedLandingsDataID",
              &FleetInterface::SetObservedLandingsDataID)
      .method("GetObservedLandingsDataID",
              &FleetInterface::GetObservedLandingsDataID)
      .method("SetSelectivityID", &FleetInterface::SetSelectivityID);

  Rcpp::class_<AgeCompDataInterface>("AgeComp")
      .constructor<int, int>()
      .field("age_comp_data", &AgeCompDataInterface::age_comp_data)
      .method("get_id", &AgeCompDataInterface::get_id);

  Rcpp::class_<LengthCompDataInterface>("LengthComp")
      .constructor<int, int>()
      .field("length_comp_data", &LengthCompDataInterface::length_comp_data)
      .method("get_id", &LengthCompDataInterface::get_id);

  Rcpp::class_<LandingsDataInterface>("Landings")
      .constructor<int>()
      .field("landings_data", &LandingsDataInterface::landings_data)
      .method("get_id", &LandingsDataInterface::get_id);

  Rcpp::class_<IndexDataInterface>("Index")
      .constructor<int>()
      .field("index_data", &IndexDataInterface::index_data)
      .method("get_id", &IndexDataInterface::get_id);

  Rcpp::class_<PopulationInterface>("Population")
      .constructor()
      .method("get_id", &PopulationInterface::get_id, "get population ID")
      .field("n_ages", &PopulationInterface::n_ages, "number of ages")
      .field("n_fleets", &PopulationInterface::n_fleets, "number of fleets")
      .field("n_years", &PopulationInterface::n_years, "number of years")
      .field("n_lengths", &PopulationInterface::n_lengths, "number of lengths")
      .field("log_M", &PopulationInterface::log_M,
             "natural log of the natural mortality of the population")
      .field("log_f_multiplier", &PopulationInterface::log_f_multiplier,
             "natural log of the annual fishing mortality multiplier of the "
             "population")
      .field("spawning_biomass_ratio",
             &PopulationInterface::spawning_biomass_ratio,
             "population spawning biomass ratio for each year")
      .field("log_init_naa", &PopulationInterface::log_init_naa,
             "natural log of the initial numbers at age")
      .field("ages", &PopulationInterface::ages,
             "vector of ages in the population; length n_ages")
      .method("SetMaturityID", &PopulationInterface::SetMaturityID,
              "Set the unique id for the Maturity object")
      .method("SetGrowthID", &PopulationInterface::SetGrowthID,
              "Set the unique id for the growth object")
      .method("SetRecruitmentID", &PopulationInterface::SetRecruitmentID,
              "Set the unique id for the Recruitment object")
      .method("AddFleet", &PopulationInterface::AddFleet,
              "Set a unique fleet id to the list of fleets operating on this "
              "population")
      .method("SetName", &PopulationInterface::SetName,
              "Set the name of the population")
      .method("GetName", &PopulationInterface::GetName,
              "Get the name of the population");

  Rcpp::class_<LogisticMaturityInterface>("LogisticMaturity")
      .constructor()
      .field("inflection_point", &LogisticMaturityInterface::inflection_point)
      .field("slope", &LogisticMaturityInterface::slope)
      .method("get_id", &LogisticMaturityInterface::get_id)
      .method("evaluate", &LogisticMaturityInterface::evaluate);

  Rcpp::class_<LogisticSelectivityInterface>("LogisticSelectivity")
      .constructor()
      .field("inflection_point",
             &LogisticSelectivityInterface::inflection_point)
      .field("slope", &LogisticSelectivityInterface::slope)
      .method("get_id", &LogisticSelectivityInterface::get_id)
      .method("evaluate", &LogisticSelectivityInterface::evaluate);

  Rcpp::class_<DoubleLogisticSelectivityInterface>("DoubleLogisticSelectivity")
      .constructor()
      .field("inflection_point_asc",
             &DoubleLogisticSelectivityInterface::inflection_point_asc,
             "50 percent quantile of the value of the quantity of interest (x) "
             " on the ascending limb of the double logistic curve; e.g., age "
             "at  which 50 percent of the fish are selected.")
      .field("slope_asc", &DoubleLogisticSelectivityInterface::slope_asc,
             "Scalar multiplier of difference between quantity of interest "
             "value (x) and inflection_point on the ascending limb of the "
             "double logistic curve.")
      .field("inflection_point_desc",
             &DoubleLogisticSelectivityInterface::inflection_point_desc,
             "50 percent quantile of the value of the quantity of interest (x) "
             "on the descending limb of the double logistic curve; e.g. age at "
             "which 50 percent of the fish are selected.")
      .field("slope_desc", &DoubleLogisticSelectivityInterface::slope_desc,
             "Scalar multiplier of difference between quantity of interest  "
             "value (x) and inflection_point on the descending limb of the "
             "double  logistic  curve.")
      .method("get_id", &DoubleLogisticSelectivityInterface::get_id,
              "Returns a unique ID for the selectivity class.")
      .method("evaluate", &DoubleLogisticSelectivityInterface::evaluate,
              "Evaluates the double logistic selectivity given input value "
              "(e.g., age or size in selectivity).");

  Rcpp::class_<EWAAGrowthInterface>("EWAAGrowth")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages, "Ages for each age class.")
      .field("weights", &EWAAGrowthInterface::weights,
             "Weights for each age class.")
      .method("get_id", &EWAAGrowthInterface::get_id)
      .method("evaluate", &EWAAGrowthInterface::evaluate);

  Rcpp::class_<DnormDistributionsInterface>("DnormDistribution")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id,
              "Returns a unique ID for the Dnorm distribution class.")
      .method("evaluate", &DnormDistributionsInterface::evaluate,
              "Evaluates the normal distribution given input data and "
              "parameter values.")
      .method("set_observed_data",
              &DnormDistributionsInterface::set_observed_data,
              "Accepts a unique ID for a given Data Object class to link the "
              "data with the distribution.")
      .method("set_distribution_mean",
              &DnormDistributionsInterface::set_distribution_mean,
              "Accepts a value for the expected distribution mean "
              "creates a fixed effect estimated mean.")
      .method("set_distribution_links",
              &DnormDistributionsInterface::set_distribution_links,
              "Accepts a unique ID for a given parameter to link the parameter "
              "with the distribution.")
      .field("x", &DnormDistributionsInterface::x,
             "Input for distribution when not observations, e.g., prior or "
             "random effect.")
      .field("expected_values", &DnormDistributionsInterface::expected_values,
             "Mean of the distribution.")
      .field("expected_mean", &DnormDistributionsInterface::expected_mean,
             "The expected mean of the distribution.")
      .field("log_sd", &DnormDistributionsInterface::log_sd,
             "The natural log of the standard deviation.");

  Rcpp::class_<DlnormDistributionsInterface>("DlnormDistribution")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id,
              "Returns a unique ID for the Dnorm distribution class.")
      .method("evaluate", &DlnormDistributionsInterface::evaluate,
              "Evaluates the normal distribution given input data and "
              "parameter values.")
      .method("set_observed_data",
              &DlnormDistributionsInterface::set_observed_data,
              "Accepts a unique ID for a given Data Object class to link the "
              "data with the distribution.")
      .method("set_distribution_links",
              &DlnormDistributionsInterface::set_distribution_links,
              "Accepts a unique ID for a given parameter to link the parameter "
              "with the distribution.")
      .field("x", &DlnormDistributionsInterface::x,
             "Input for distribution when not observations, e.g., prior or "
             "random effect.")
      .field("expected_values", &DlnormDistributionsInterface::expected_values,
             "Mean of the distribution on the natural log scale.")
      .field("log_sd", &DlnormDistributionsInterface::log_sd,
             "The natural log of the standard deviation of the distribution on "
             "the natural log scale.");

  Rcpp::class_<DmultinomDistributionsInterface>("DmultinomDistribution")
      .constructor()
      .method("get_id", &DmultinomDistributionsInterface::get_id,
              "Returns a unique ID for the Dnorm distribution class.")
      .method("evaluate", &DmultinomDistributionsInterface::evaluate,
              "Evaluates the normal distribution given input data and "
              "parameter values.")
      .method("set_observed_data",
              &DmultinomDistributionsInterface::set_observed_data,
              "Accepts a unique ID for a given Data Object class to link the "
              "data with the distribution.")
      .method("set_distribution_links",
              &DmultinomDistributionsInterface::set_distribution_links,
              "Accepts a unique ID for a given parameter to link the parameter "
              "with the distribution.")
      .method("set_note", &DmultinomDistributionsInterface::set_note)
      .field("x", &DmultinomDistributionsInterface::x,
             "Input for distribution when not observations, e.g., prior or "
             "random effect.")
      .field("expected_values",
             &DmultinomDistributionsInterface::expected_values,
             "numeric non-negative vector of length K, specifying the "
             "probability for the K classes.")
      .field(
          "dims", &DmultinomDistributionsInterface::dims,
          "dimension of the multivariate input, e.g., c(num rows, num cols).");

  Rcpp::class_<CatchAtAgeInterface>("CatchAtAge")
      .constructor()
      .method("AddPopulation", &CatchAtAgeInterface::AddPopulation)
      .method("get_output", &CatchAtAgeInterface::to_json)
      .method("GetReport", &CatchAtAgeInterface::get_report)
      .method("GetId", &CatchAtAgeInterface::get_id)
      .method("DoReporting", &CatchAtAgeInterface::DoReporting)
      .method("IsReporting", &CatchAtAgeInterface::IsReporting);
}

#endif /* SRC_FIMS_MODULES_HPP */
