/*
 * File:   rcpp_interface.hpp
 *
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file for reuse
 * information.
 *
 *
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP

#include "rcpp_objects/rcpp_data.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
//#include "rcpp_objects/rcpp_nll.hpp"
#include "../../common/model.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"
#include "rcpp_objects/rcpp_tmb_distribution.hpp"
/**
 * @brief Create the TMB model object and add interface objects to it.
 */
bool CreateTMBModel() {
  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
       i++) {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  d0->CreateModel();

  // first-order derivative
  std::shared_ptr<fims_info::Information<TMB_FIMS_FIRST_ORDER>> d1 =
      fims_info::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
  d1->CreateModel();

  // second-order derivative
  std::shared_ptr<fims_info::Information<TMB_FIMS_SECOND_ORDER>> d2 =
      fims_info::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
  d2->CreateModel();

  // third-order derivative
  std::shared_ptr<fims_info::Information<TMB_FIMS_THIRD_ORDER>> d3 =
      fims_info::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
  d3->CreateModel();

  return true;
}

Rcpp::NumericVector get_fixed_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->fixed_effects_parameters.size(); i++) {
    p.push_back(*d0->fixed_effects_parameters[i]);
  }

  return p;
}

Rcpp::NumericVector get_random_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->random_effects_parameters.size(); i++) {
    p.push_back(*d0->random_effects_parameters[i]);
  }

  return p;
}

/**
 * Clears the contents of info log file.
 */
void clear_info_log() {
  // First flush the output stream to make sure nothing
  // is left in the stream memory bufffer.
  INFO_LOG.flush();

  // Next an new stream is opened and closed to
  // overwrite the file.
  std::ofstream CLEAR_LOG("logs/info.log");
  CLEAR_LOG.close();

  // Finally the stream output location is reset back to the start
  // of the file.
  INFO_LOG.seekp(0);
}

/**
 * Clears the contents of fims log file.
 */
void clear_fims_log() {
  FIMS_LOG.flush();
  std::ofstream CLEAR_LOG("logs/fims.log");
  CLEAR_LOG.close();
  FIMS_LOG.seekp(0);
}

/**
 * Clears the contents of data log file.
 */
void clear_data_log() {
  DATA_LOG.flush();
  std::ofstream CLEAR_LOG("logs/data.log");
  CLEAR_LOG.close();
  DATA_LOG.seekp(0);
}

/**
 * Clears the contents of error log file.
 */
void clear_error_log() {
  ERROR_LOG.flush();
  std::ofstream CLEAR_LOG("logs/error.log");
  CLEAR_LOG.close();
  ERROR_LOG.seekp(0);
}

/**
 * Clears the contents of model log file.
 */
void clear_model_log() {
  MODEL_LOG.flush();
  std::ofstream CLEAR_LOG("logs/model.log");
  CLEAR_LOG.close();
  MODEL_LOG.seekp(0);
}

/**
 * Clears the contents of fleet log file.
 */
void clear_fleet_log() {
  FLEET_LOG.flush();
  std::ofstream CLEAR_LOG("logs/fleet.log");
  CLEAR_LOG.close();
  FLEET_LOG.seekp(0);
}

/**
 * Clears the contents of population log file.
 */
void clear_population_log() {
  POPULATION_LOG.flush();
  std::ofstream CLEAR_LOG("logs/population.log");
  CLEAR_LOG.close();
  POPULATION_LOG.seekp(0);
}

/**
 * Clears the contents of maturity log file.
 */
void clear_maturity_log() {
  MATURITY_LOG.flush();
  std::ofstream CLEAR_LOG("logs/maturity.log");
  CLEAR_LOG.close();
  MATURITY_LOG.seekp(0);
}

/**
 * Clears the contents of recruitment log file.
 */
void clear_recruitment_log() {
  RECRUITMENT_LOG.flush();
  std::ofstream CLEAR_LOG("logs/recruitment.log");
  CLEAR_LOG.close();
  RECRUITMENT_LOG.seekp(0);
}

/**
 * Clears the contents of growth log file.
 */
void clear_growth_log() {
  GROWTH_LOG.flush();
  std::ofstream CLEAR_LOG("logs/growth.log");
  CLEAR_LOG.close();
  GROWTH_LOG.seekp(0);
}

/**
 * Clears the contents of selectivity log file.
 */
void clear_selectivity_log() {
  SELECTIVITY_LOG.flush();
  std::ofstream CLEAR_LOG("logs/selectivity.log");
  CLEAR_LOG.close();
  SELECTIVITY_LOG.seekp(0);
}

/**
 * Clears the contents of debug log file.
 */
void clear_debug_log() {
  DEBUG_LOG.flush();
  std::ofstream CLEAR_LOG("logs/debug/debug.log");
  CLEAR_LOG.close();
  DEBUG_LOG.seekp(0);
}

/**
 * Clears the contents of log files.
 */
void clear_logs() {
  clear_fims_log();
  clear_info_log();
  clear_data_log();
  clear_error_log();
  clear_model_log();
  clear_fleet_log();
  clear_population_log();
  clear_recruitment_log();
  clear_growth_log();
  clear_maturity_log();
  clear_selectivity_log();
  clear_debug_log();
}

template <typename Type>
void clear_internal() {
  std::shared_ptr<fims_info::Information<Type>> d0 =
      fims_info::Information<Type>::GetInstance();
  d0->fixed_effects_parameters.clear();
  d0->random_effects_parameters.clear();
}
/**
 * Clears the vector of independent variables.
 */
void clear() {
  // rcpp_interface_base.hpp
  FIMSRcppInterfaceBase::fims_interface_objects.clear();

  // rcpp_data.hpp
  DataInterfaceBase::id_g = 1;
  DataInterfaceBase::live_objects.clear();

  AgeCompDataInterface::id_g = 1;
  AgeCompDataInterface::live_objects.clear();

  IndexDataInterface::id_g = 1;
  IndexDataInterface::live_objects.clear();

  // rcpp_fleets.hpp
  FleetInterfaceBase::id_g = 1;
  FleetInterfaceBase::live_objects.clear();

  FleetInterface::id_g = 1;
  FleetInterface::live_objects.clear();

  // rcpp_growth.hpp
  GrowthInterfaceBase::id_g = 1;
  GrowthInterfaceBase::live_objects.clear();

  EWAAGrowthInterface::id_g = 1;
  EWAAGrowthInterface::live_objects.clear();

  // rcpp_maturity.hpp
  MaturityInterfaceBase::id_g = 1;
  MaturityInterfaceBase::live_objects.clear();

  LogisticMaturityInterface::id_g = 1;
  LogisticMaturityInterface::live_objects.clear();

  // rcpp_population.hpp
  PopulationInterfaceBase::id_g = 1;
  PopulationInterfaceBase::live_objects.clear();

  PopulationInterface::id_g = 1;
  PopulationInterface::live_objects.clear();

  // rcpp_recruitment.hpp
  RecruitmentInterfaceBase::id_g = 1;
  RecruitmentInterfaceBase::live_objects.clear();

  BevertonHoltRecruitmentInterface::id_g = 1;
  BevertonHoltRecruitmentInterface::live_objects.clear();

  // rcpp_selectivity.hpp
  SelectivityInterfaceBase::id_g = 1;
  SelectivityInterfaceBase::live_objects.clear();

  LogisticSelectivityInterface::id_g = 1;
  LogisticSelectivityInterface::live_objects.clear();

  DoubleLogisticSelectivityInterface::id_g = 1;
  DoubleLogisticSelectivityInterface::live_objects.clear();

  // rcpp_tmb_distribution.hpp
  DistributionsInterfaceBase::id_g = 1;
  DistributionsInterfaceBase::live_objects.clear();

  DnormDistributionsInterface::id_g = 1;
  DnormDistributionsInterface::live_objects.clear();

  DlnormDistributionsInterface::id_g = 1;
  DlnormDistributionsInterface::live_objects.clear();

  DmultinomDistributionsInterface::id_g = 1;
  DmultinomDistributionsInterface::live_objects.clear();

  clear_internal<TMB_FIMS_REAL_TYPE>();
  clear_internal<TMB_FIMS_FIRST_ORDER>();
  clear_internal<TMB_FIMS_SECOND_ORDER>();
  clear_internal<TMB_FIMS_THIRD_ORDER>();
}

RCPP_EXPOSED_CLASS(Parameter)
RCPP_MODULE(fims) {
  Rcpp::function("CreateTMBModel", &CreateTMBModel,
                 "Create the TMB model object and add interface objects to it. No inputs are required.");
  Rcpp::function("get_fixed", &get_fixed_parameters_vector,
                 "Get fixed parameters vector. No inputs are required.");
  Rcpp::function("get_random", &get_random_parameters_vector,
                 "Get random paramters vector. No inputs are required.");
  Rcpp::function("clear", clear,
                 "Clears the vector of independent variables. No inputs are required.");
  Rcpp::function("clear_logs", clear_logs,
                 "Clears the contents of log files. No inputs are required.");
  Rcpp::function("clear_fims_log", clear_fims_log,
                 "Clears the contents of fims log file. No inputs are required.");
  Rcpp::function("clear_info_log", clear_info_log,
                 "Clears the contents of info log file. No inputs are required.");
  Rcpp::function("clear_error_log", clear_error_log,
                 "Clears the contents of error log file. No inputs are required.");
  Rcpp::function("clear_data_log", clear_data_log,
                 "Clears the contents of data log file. No inputs are required.");
  Rcpp::function("clear_population_log", clear_population_log,
                 "Clears the contents of population log file. No inputs are required.");
  Rcpp::function("clear_model_log", clear_model_log,
                 "Clears the contents of model log file. No inputs are required.");
  Rcpp::function("clear_recruitment_log", clear_recruitment_log);
  Rcpp::function("clear_fleet_log", clear_fleet_log,
                 "Clears the contents of fleet log file. No inputs are required.");
  Rcpp::function("clear_growth_log", clear_growth_log);
  Rcpp::function("clear_maturity_log", clear_maturity_log,
                 "Clears the contents of maturity log file. No inputs are required.");
  Rcpp::function("clear_selectivity_log", clear_selectivity_log,
                 "Clears the contents of selectivity log file. No inputs are required.");
  Rcpp::function("clear_debug_log", clear_debug_log,
                 "Clears the contents of debug log file. No inputs are required.");

  Rcpp::class_<Parameter>("Parameter")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::value_m,
             "Initial value of the parameter.")
      .field("min", &Parameter::min_m,
             "Min value of the parameter. Default value is -Infinity.")
      .field("max", &Parameter::max_m,
             "Max value of the parameter. Default value is Infinity.")
      .field("is_random_effect", &Parameter::is_random_effect_m,
             "Is the parameter a random effect parameter? Default value is false.")
      .field("estimated", &Parameter::estimated_m,
             "Is the parameter estimated? Default value is false.");

  Rcpp::class_<BevertonHoltRecruitmentInterface>("BevertonHoltRecruitment")
      .constructor()
      .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep,
             "Recruitment relative to unfished recruitment at 20 percent of unfished spawning biomass. Steepness is subject to a logit transformation.")
      .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero,
             "Natural log of unexploited recruitment. Unit: ln(number).")
      .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs,
             "A vector of recruitment deviations in natural log space.")
      .field("estimate_log_devs",
             &BevertonHoltRecruitmentInterface::estimate_log_devs,
             "A flag to indicate if recruitment deviations are estimated or not. Default is true.")
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id,
             "Get the ID of the interface object. No inputs are required.")
      .field("log_sigma_recruit",
             &BevertonHoltRecruitmentInterface::log_sigma_recruit,
             "Natural log of standard deviation of natural log recruitment.")
      .method("evaluate", &BevertonHoltRecruitmentInterface::evaluate,
             "Calculates the expected recruitment for a given spawning input. Requires inputs of spawning output and number of spawners per recruit of an unfished population.")
      .method("evaluate_nll", &BevertonHoltRecruitmentInterface::evaluate_nll,
             "Calculates the negative log likelihood of recruitment deviations. No inputs are required.");

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .field("is_survey", &FleetInterface::is_survey,
            "Is this fleet object a fishing fleet or a survey? Default is false.")
      .field("log_q", &FleetInterface::log_q,
            "Estimated parameter: natural log of catchability for the fleet.")
      .field("log_Fmort", &FleetInterface::log_Fmort,
            "Estimated parameter: natural log of fishing mortality.")
      .field("nages", &FleetInterface::nages,
            "The number of ages in the model.")
      .field("nyears", &FleetInterface::nyears,
            "The number of years in the model.")
      .field("estimate_F", &FleetInterface::estimate_F,
            "Whether the parameter F should be estimated. Default is false.")
      .field("estimate_q", &FleetInterface::estimate_q,
            "Whether the parameter q should be estimated. Default is false.")
      .field("estimate_obs_error", &FleetInterface::estimate_obs_error,
            "Whether the parameter log_obs_error should be estimated. Default is false.")
      .field("random_q", &FleetInterface::random_q,
            "Whether q should be a random effect. Default is false.")
      .field("random_F", &FleetInterface::random_F,
            "Whether F should be a random effect. Default is false.")
      .field("log_obs_error", &FleetInterface::log_obs_error,
            "Natural log of the observation error.")
      .method("SetAgeCompLikelihood", &FleetInterface::SetAgeCompLikelihood,
            "Set the unique id for the Age Comp Likelihood object.")
      .method("SetIndexLikelihood", &FleetInterface::SetIndexLikelihood,
            "Set the unique id for the Index Likelihood object.")
      .method("SetObservedAgeCompData", &FleetInterface::SetObservedAgeCompData,
            "Set the unique id for the Observed Age Comp Data object.")
      .method("SetObservedIndexData", &FleetInterface::SetObservedIndexData,
            "Set the unique id for the Observed Index Data object.")
      .method("SetSelectivity", &FleetInterface::SetSelectivity,
            "Set the unique id for the Selectivity object.");

  Rcpp::class_<AgeCompDataInterface>("AgeComp")
      .constructor<int, int>("The first dimension of the data is the maximum year, and the second dimension is the maximum age.")
      .field("age_comp_data", &AgeCompDataInterface::age_comp_data,
            "The age composition data. Unit: number at age; proportion at age also works.")
      .method("get_id", &AgeCompDataInterface::get_id,
             "Get the ID of the interface object. No inputs are required.");

  Rcpp::class_<IndexDataInterface>("Index")
      .constructor<int>("The dimension of the data is the maximum year.")
      .field("index_data", &IndexDataInterface::index_data,
            "The index data. Unit: mt; it's possible to use other units as long as the survey index is assumed to be proportional to biomass.")
      .method("get_id", &IndexDataInterface::get_id,
            "Get the ID of the interface object. No inputs are required.");

  Rcpp::class_<PopulationInterface>("Population")
      .constructor()
      .method("get_id", &PopulationInterface::get_id,
              "Get the ID of the interface object. No inputs are required.")
      .field("nages", &PopulationInterface::nages,
             "Number of ages.")
      .field("nfleets", &PopulationInterface::nfleets,
             "Number of fleets.")
      .field("nseasons", &PopulationInterface::nseasons,
             "Number of seasons.")
      .field("nyears", &PopulationInterface::nyears,
             "Number of years.")
      .field("log_M", &PopulationInterface::log_M,
             "Natural log of the natural mortality of the population.")
      .field("log_init_naa", &PopulationInterface::log_init_naa,
             "Natural log of the initial numbers at age. Unit: in number.")
      .field("proportion_female", &PopulationInterface::proportion_female,
             "The proportion of female individuals.")
      .field("ages", &PopulationInterface::ages,
             "Vector of ages in the population; length nages.")
      .field("estimate_M", &PopulationInterface::estimate_M,
             "Whether parameter should be estimated.")
      .field("estimate_init_naa", &PopulationInterface::estimate_initNAA,
             "Whether parameter should be estimated.")
      .field("estimate_prop_female", &PopulationInterface::estimate_prop_female,
             "Whether proportion female should be estimated.")
      .method("evaluate", &PopulationInterface::evaluate,
             "Evaluate the population function.")
      .method("SetMaturity", &PopulationInterface::SetMaturity,
             "Set the unique id for the Maturity object.")
      .method("SetGrowth", &PopulationInterface::SetGrowth,
             "Set the unique id for the growth object.")
      .method("SetRecruitment", &PopulationInterface::SetRecruitment,
             "Set the unique id for the recruitment object.");

  Rcpp::class_<DnormDistributionsInterface>("TMBDnormDistribution")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id,
              "Get the id of the Dnorm distributions interface class object.")
      .method("evaluate", &DnormDistributionsInterface::evaluate,
              "Evaluate normal probability density function. It requires input for do_log to determine whether to return the natural log of the pdf.")
      .field("x", &DnormDistributionsInterface::x,
              "Observed data x.")
      .field("mean", &DnormDistributionsInterface::mean,
              "Mean of x for the normal distribution.")
      .field("sd", &DnormDistributionsInterface::sd,
              "Standard deviation of x for the normal distribution.");

  Rcpp::class_<LogisticMaturityInterface>("LogisticMaturity")
      .constructor()
      .field("inflection_point", &LogisticMaturityInterface::inflection_point,
            "The index value at which the response reaches 0.5.")
      .field("slope", &LogisticMaturityInterface::slope,
            "The width of the curve at the inflection_point.")
      .method("get_id", &LogisticMaturityInterface::get_id,
            "Get the ID of the interface object.")
      .method("evaluate", &LogisticMaturityInterface::evaluate,
            "Evaluate the logistic maturity function. The function requires input of the independent variable in the logistic function (e.g., age or size in maturity).");

  Rcpp::class_<LogisticSelectivityInterface>("LogisticSelectivity")
      .constructor()
      .field("inflection_point",
             &LogisticSelectivityInterface::inflection_point,
             "The index value at which the response reaches 0.5.")
      .field("slope", &LogisticSelectivityInterface::slope,
             "The width of the curve at the inflection_point.")
      .method("get_id", &LogisticSelectivityInterface::get_id,
             "Get the ID of the interface object.")
      .method("evaluate", &LogisticSelectivityInterface::evaluate,
             "Evaluate the logistic selectivity function. The function requires input of the independent variable in the logistic function (e.g., age or size in selectivity).");

  Rcpp::class_<DoubleLogisticSelectivityInterface>("DoubleLogisticSelectivity")
      .constructor()
      .field("inflection_point_asc",
             &DoubleLogisticSelectivityInterface::inflection_point_asc,
             "The index value at which the response reaches 0.5 on the ascending limb of the function.")
      .field("slope_asc", &DoubleLogisticSelectivityInterface::slope_asc,
             "The width of the curve at the inflection point of the ascending limb of the function.")
      .field("inflection_point_desc",
             &DoubleLogisticSelectivityInterface::inflection_point_desc,
             "The index value at which the response reaches 0.5 on the descending limb of the function.")
      .field("slope_desc", &DoubleLogisticSelectivityInterface::slope_desc,
             "The width of the curve at the inflection point of the descending limb of the function.")
      .method("get_id", &DoubleLogisticSelectivityInterface::get_id,
             "Get the id for the double logistic selectivity interface.")
      .method("evaluate", &DoubleLogisticSelectivityInterface::evaluate,
             "Evaluate the double logistic selectivity function. The function requires input of the independent variable in the logistic function (e.g., age or size in selectivity).");

  Rcpp::class_<EWAAGrowthInterface>("EWAAgrowth")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages,
             "Ages for each age class.")
      .field("weights", &EWAAGrowthInterface::weights,
             "Weights for each age class. Unit: mt.")
      .method("get_id", &EWAAGrowthInterface::get_id,
             "Get the id of the interface object.")
      .method("evaluate", &EWAAGrowthInterface::evaluate,
             "Rcpp interface to the EWAAgrowth evaluate method. The function requires input of an age.");

  Rcpp::class_<DlnormDistributionsInterface>("TMBDlnormDistribution")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id,
              "Get the id of the Dlnorm distributions interface class object.")
      .method("evaluate", &DlnormDistributionsInterface::evaluate,
              "Evaluate lognormal probability density function. It requires input for do_log to determine whether to return the natural log of the pdf.")
      .field("x", &DlnormDistributionsInterface::x,
              "Observation x.")
      .field("meanlog", &DlnormDistributionsInterface::meanlog,
              "Mean of the distribution of log(x).")
      .field("sdlog", &DlnormDistributionsInterface::sdlog,
              "Standard deviation of the distribution of log(x).");

  Rcpp::class_<DmultinomDistributionsInterface>("TMBDmultinomDistribution")
      .constructor()
      .method("evaluate", &DmultinomDistributionsInterface::evaluate,
              "Evaluate multinom probability density function. It requires input for do_log to determine whether to return the natural log of the pdf.")
      .method("get_id", &DmultinomDistributionsInterface::get_id,
              "Get the id of the Dmultinom distributions interface class object.")
      .field("x", &DmultinomDistributionsInterface::x,
              "Vector of length K of integers.")
      .field("p", &DmultinomDistributionsInterface::p,
              "Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1).");
}

#endif /* RCPP_INTERFACE_HPP */
