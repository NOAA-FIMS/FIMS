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
RCPP_EXPOSED_CLASS(ParameterVector)
RCPP_MODULE(fims) {
  Rcpp::function("CreateTMBModel", &CreateTMBModel);
  Rcpp::function("get_fixed", &get_fixed_parameters_vector);
  Rcpp::function("get_random", &get_random_parameters_vector);
  Rcpp::function("clear", clear);
  Rcpp::function("clear_logs", clear_logs);
  Rcpp::function("clear_fims_log", clear_fims_log);
  Rcpp::function("clear_info_log", clear_info_log);
  Rcpp::function("clear_error_log", clear_error_log);
  Rcpp::function("clear_data_log", clear_data_log);
  Rcpp::function("clear_population_log", clear_population_log);
  Rcpp::function("clear_model_log", clear_model_log);
  Rcpp::function("clear_recruitment_log", clear_recruitment_log);
  Rcpp::function("clear_fleet_log", clear_fleet_log);
  Rcpp::function("clear_growth_log", clear_growth_log);
  Rcpp::function("clear_maturity_log", clear_maturity_log);
  Rcpp::function("clear_selectivity_log", clear_selectivity_log);
  Rcpp::function("clear_debug_log", clear_debug_log);
  Rcpp::function("create_vector", CreateVector);

  Rcpp::class_<Parameter>("Parameter")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::value_m)
      .field("min", &Parameter::min_m)
      .field("max", &Parameter::max_m)
      .field("id", &Parameter::id_m)
      .field("is_random_effect", &Parameter::is_random_effect_m)
      .field("estimated", &Parameter::estimated_m);

   Rcpp::class_<ParameterVector>("ParameterVector")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .field("data", &ParameterVector::storage_m)
      .method("at", &ParameterVector::at)
      .method("size", &ParameterVector::size)
      .method("resize", &ParameterVector::resize)
      .method("set_all_estimable", &ParameterVector::set_all_estimable)
      .method("set_all_random", &ParameterVector::set_all_random)
      .method("fill", &ParameterVector::fill);

  Rcpp::class_<BevertonHoltRecruitmentInterface>("BevertonHoltRecruitment")
      .constructor()
      .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep)
      .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero)
      .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs)
      .field("estimate_log_devs",
             &BevertonHoltRecruitmentInterface::estimate_log_devs)
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
      .field("log_sigma_recruit",
             &BevertonHoltRecruitmentInterface::log_sigma_recruit)
      .method("evaluate", &BevertonHoltRecruitmentInterface::evaluate)
      .method("evaluate_nll", &BevertonHoltRecruitmentInterface::evaluate_nll);

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .field("is_survey", &FleetInterface::is_survey)
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("nages", &FleetInterface::nages)
      .field("nyears", &FleetInterface::nyears)
      .field("estimate_F", &FleetInterface::estimate_F)
      .field("estimate_q", &FleetInterface::estimate_q)
      .field("estimate_obs_error", &FleetInterface::estimate_obs_error)
      .field("random_q", &FleetInterface::random_q)
      .field("random_F", &FleetInterface::random_F)
      .field("log_obs_error", &FleetInterface::log_obs_error)
      .method("SetAgeCompLikelihood", &FleetInterface::SetAgeCompLikelihood)
      .method("SetIndexLikelihood", &FleetInterface::SetIndexLikelihood)
      .method("SetObservedAgeCompData", &FleetInterface::SetObservedAgeCompData)
      .method("SetObservedIndexData", &FleetInterface::SetObservedIndexData)
      .method("SetSelectivity", &FleetInterface::SetSelectivity);

  Rcpp::class_<AgeCompDataInterface>("AgeComp")
      .constructor<int, int>()
      .field("age_comp_data", &AgeCompDataInterface::age_comp_data)
      .method("get_id", &AgeCompDataInterface::get_id);

  Rcpp::class_<IndexDataInterface>("Index")
      .constructor<int>()
      .field("index_data", &IndexDataInterface::index_data)
      .method("get_id", &IndexDataInterface::get_id);

  Rcpp::class_<PopulationInterface>("Population")
      .constructor()
      .method("get_id", &PopulationInterface::get_id)
      .field("nages", &PopulationInterface::nages)
      .field("nfleets", &PopulationInterface::nfleets)
      .field("nseasons", &PopulationInterface::nseasons)
      .field("nyears", &PopulationInterface::nyears)
      .field("log_M", &PopulationInterface::log_M)
      .field("log_init_naa", &PopulationInterface::log_init_naa)
      .field("proportion_female", &PopulationInterface::proportion_female)
      .field("ages", &PopulationInterface::ages)
      .field("estimate_M", &PopulationInterface::estimate_M)
      .field("estimate_init_naa", &PopulationInterface::estimate_initNAA)
      .field("estimate_prop_female", &PopulationInterface::estimate_prop_female)
      .method("evaluate", &PopulationInterface::evaluate)
      .method("SetMaturity", &PopulationInterface::SetMaturity)
      .method("SetGrowth", &PopulationInterface::SetGrowth)
      .method("SetRecruitment", &PopulationInterface::SetRecruitment)
      .method("evaluate", &PopulationInterface::evaluate);

  Rcpp::class_<DnormDistributionsInterface>("TMBDnormDistribution")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id)
      .method("evaluate", &DnormDistributionsInterface::evaluate)
      .field("x", &DnormDistributionsInterface::x)
      .field("mean", &DnormDistributionsInterface::mean)
      .field("sd", &DnormDistributionsInterface::sd);

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
             &DoubleLogisticSelectivityInterface::inflection_point_asc)
      .field("slope_asc", &DoubleLogisticSelectivityInterface::slope_asc)
      .field("inflection_point_desc",
             &DoubleLogisticSelectivityInterface::inflection_point_desc)
      .field("slope_desc", &DoubleLogisticSelectivityInterface::slope_desc)
      .method("get_id", &DoubleLogisticSelectivityInterface::get_id)
      .method("evaluate", &DoubleLogisticSelectivityInterface::evaluate);

  Rcpp::class_<EWAAGrowthInterface>("EWAAgrowth")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages)
      .field("weights", &EWAAGrowthInterface::weights)
      .method("get_id", &EWAAGrowthInterface::get_id)
      .method("evaluate", &EWAAGrowthInterface::evaluate);

  Rcpp::class_<DlnormDistributionsInterface>("TMBDlnormDistribution")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id)
      .method("evaluate", &DlnormDistributionsInterface::evaluate)
      .field("x", &DlnormDistributionsInterface::x)
      .field("meanlog", &DlnormDistributionsInterface::meanlog)
      .field("sdlog", &DlnormDistributionsInterface::sdlog);

  Rcpp::class_<DmultinomDistributionsInterface>("TMBDmultinomDistribution")
      .constructor()
      .method("evaluate", &DmultinomDistributionsInterface::evaluate)
      .method("get_id", &DmultinomDistributionsInterface::get_id)
      .field("x", &DmultinomDistributionsInterface::x)
      .field("p", &DmultinomDistributionsInterface::p);
}

#endif /* RCPP_INTERFACE_HPP */
