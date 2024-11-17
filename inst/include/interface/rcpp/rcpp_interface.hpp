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
#include "../../common/model.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"
#include "rcpp_objects/rcpp_distributions.hpp"


SEXP FIMS_objective_function;
SEXP FIMS_gradient_function;
double FIMS_function_value = 0;
Rcpp::NumericVector FIMS_function_gradient;
double FIMS_mgc_value = 0;
bool FIMS_finalized = false;

/**
 * @brief Create the TMB model object and add interface objects to it.
 */
bool CreateTMBModel() {
  FIMS_INFO_LOG("adding FIMS objects to TMB");
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

void SetFIMSFunctions(SEXP fn, SEXP gr) {
    FIMS_objective_function = fn;
    FIMS_gradient_function = gr;
}

/**
 * @brief Extracts derived quantities from model objects.
 */
void Finalize(Rcpp::NumericVector p) {
    FIMS_finalized = true;
    std::shared_ptr<fims_info::Information < double>> information =
            fims_info::Information<double>::GetInstance();

    std::shared_ptr<fims_model::Model < double>> model =
            fims_model::Model<double>::GetInstance();

    for (size_t i = 0; i < information->fixed_effects_parameters.size(); i++) {
        *information->fixed_effects_parameters[i] = p[i];
    }

    model->Evaluate();

    Rcpp::Function f = Rcpp::as<Rcpp::Function>(FIMS_objective_function);
    Rcpp::Function g = Rcpp::as<Rcpp::Function>(FIMS_gradient_function);
    double ret = Rcpp::as<double>(f(p));
    Rcpp::NumericVector grad = Rcpp::as<Rcpp::NumericVector>(g(p));

    FIMS_function_value = ret;
    FIMS_function_gradient = grad;
    std::cout << "Final value = " << FIMS_function_value << "\nGradient: \n";
    double maxgc = -9999;
    for (size_t i = 0; i < FIMS_function_gradient.size(); i++) {
        if (std::fabs(FIMS_function_gradient[i]) > maxgc) {
            maxgc = std::fabs(FIMS_function_gradient[i]);
        }
    }
    FIMS_mgc_value = maxgc;
    //        std::cout<<"mgc =  "<<maxgc<<"\n";


    //    fims_info::Information < double>::population_iterator pit;
    //    for (pit = information->populations.begin(); pit != information->populations.end(); ++pit) {
    //        pit->second->Prepare();
    //        pit->second->Evaluate();
    //    }
    //
    //    fims_info::Information < double>::fleet_iterator fit;
    //    for (fit = information->fleets.begin(); fit != information->fleets.end(); ++fit) {
    //        fit->second->Prepare();
    //        fit->second->Evaluate();
    ////        double ac = fit->second->evaluate_age_comp_nll();
    ////        double i = fit->second->evaluate_index_nll();
    //    }


    for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
            i++) {
        FIMSRcppInterfaceBase::fims_interface_objects[i]->finalize();
    }
}

/**
 * @brief Extracts derived quantities from model objects.
 */
std::string ToJSON() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string ctime_no_newline = strtok(ctime(&now_time), "\n");
    std::shared_ptr<fims_info::Information < double>> info =
            fims_info::Information<double>::GetInstance();
    std::stringstream ss;
    ss << "{\n";
    ss << "\"timestamp\": \"" << ctime_no_newline << "\",\n";
    ss << "\"nyears\":" << info->nyears << ",\n";
    ss << "\"nseasons\":" << info->nseasons << ",\n";
    ss << "\"nages\":" << info->nages << ",\n";
    ss << "\"finalized\":" << FIMS_finalized << ",\n";
    ss << "\"objective_function_value\": " << FIMS_function_value << ",\n";
    ss << "\"max_gradient_component\": " << FIMS_mgc_value << ",\n";
    ss << "\"final_gradient\": [";
    if (FIMS_function_gradient.size() > 0) {
        for (size_t i = 0; i < FIMS_function_gradient.size() - 1; i++) {
            ss << FIMS_function_gradient[i] << ", ";
        }
        ss << FIMS_function_gradient[FIMS_function_gradient.size() - 1] << "],\n";
    } else {
        ss << "],";
    }
    size_t length = FIMSRcppInterfaceBase::fims_interface_objects.size();
    for (size_t i = 0; i < length - 1; i++) {
        ss << FIMSRcppInterfaceBase::fims_interface_objects[i]->to_json() << ",\n";
    }

    ss << FIMSRcppInterfaceBase::fims_interface_objects[length - 1]->to_json() << "\n}";
    return ss.str();
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

Rcpp::List get_parameter_names(Rcpp::List pars) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->parameter_names;


  return pars;
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
  FIMS_LOG_OLD.flush();
  std::ofstream CLEAR_LOG("logs/fims.log");
  CLEAR_LOG.close();
  FIMS_LOG_OLD.seekp(0);
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
 * Clears the contents of distributions log file.
 */
void clear_distributions_log() {
  DISTRIBUTIONS_LOG.flush();
  std::ofstream CLEAR_LOG("logs/distributions.log");
  CLEAR_LOG.close();
  DISTRIBUTIONS_LOG.seekp(0);
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
  clear_distributions_log();
}

template <typename Type>
void clear_internal() {
  std::shared_ptr<fims_info::Information<Type>> d0 =
      fims_info::Information<Type>::GetInstance();
  d0->fixed_effects_parameters.clear();
  d0->random_effects_parameters.clear();
  d0->variable_map.clear();
}
/**
 * Clears the vector of independent variables.
 */
void clear() {
  // rcpp_interface_base.hpp
  FIMSRcppInterfaceBase::fims_interface_objects.clear();

  //Parameter and ParameterVector
  Parameter::id_g = 1;
  ParameterVector::id_g = 1;
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

    FIMS_finalized = false;
}

/**
 * Returns the entire log as a string in JSON format.
 */
std::string get_log() {
    return fims::FIMSLog::fims_log->get_log();
}

/**
 * Returns only error entries from log as a string in JSON format.
 */
std::string get_log_errors() {
    return fims::FIMSLog::fims_log->get_errors();
}

/**
 * Returns only warning entries from log as a string in JSON format.
 */
std::string get_log_warnings() {
    return fims::FIMSLog::fims_log->get_warnings();
}

/**
 * Returns only info entries from log as a string in JSON format.
 */
std::string get_log_info() {
    return fims::FIMSLog::fims_log->get_info();
}

/**
 * Returns log entries by module as a string in JSON format.
 */
std::string get_log_module(const std::string& module) {
    return fims::FIMSLog::fims_log->get_module(module);
}

/**
 * If true, writes the log on exit .
 */
void write_log(bool write) {
    FIMS_INFO_LOG("Setting FIMS write log: " + fims::to_string(write));
    fims::FIMSLog::fims_log->write_on_exit = write;
}

/**
 * Sets the path for the log file to written.
 */
void set_log_path(const std::string& path) {
    FIMS_INFO_LOG("Setting FIMS log path: " + path);
    fims::FIMSLog::fims_log->set_path(path);
}

/**
 * If true, throws a  runtime exception when an error is logged .
 */
void set_log_throw_on_error(bool throw_on_error) {
    fims::FIMSLog::fims_log->throw_on_error = throw_on_error;
}

/**
 * Initializes the logging system, sets all signal handling.
 */
void init_logging() {

    FIMS_INFO_LOG("Initializing FIMS logging system.");
    std::signal(SIGSEGV, &fims::WriteAtExit);
    std::signal(SIGINT, &fims::WriteAtExit);
    std::signal(SIGABRT, &fims::WriteAtExit);
    std::signal(SIGFPE, &fims::WriteAtExit);
    std::signal(SIGILL, &fims::WriteAtExit);
    std::signal(SIGTERM, &fims::WriteAtExit);
}

/**
 * Add log info entry from R.
 */
void log_info(std::string log_entry) {
    fims::FIMSLog::fims_log->info_message(log_entry, -1, "R_env", "R_script_entry");
}

/**
 * Add log warning entry from R.
 */
void log_warning(std::string log_entry) {
    fims::FIMSLog::fims_log->warning_message(log_entry, -1, "R_env", "R_script_entry");
}

/**
 * Add log error entry from R.
 */
void log_error(std::string log_entry) {

    std::stringstream ss;
    ss << "capture.output(traceback(4))";
    SEXP expression, result;
    ParseStatus status;

    PROTECT(expression = R_ParseVector(Rf_mkString(ss.str().c_str()), 1, &status, R_NilValue));
    if (status != PARSE_OK) {
        std::cout << "Error parsing expression" << std::endl;
        UNPROTECT(1);
    }
    Rcpp::Rcout << "before call.";
    PROTECT(result = Rf_eval(VECTOR_ELT(expression, 0), R_GlobalEnv));
    Rcpp::Rcout << "after call.";
    UNPROTECT(2);
    std::stringstream ss_ret;
    ss_ret << "traceback:\n";
    for (int j = 0; j < LENGTH(result); j++) {
        std::string str(CHAR(STRING_ELT(result, j)));
        ss_ret << str << "\n";
    }

    std::string ret = ss_ret.str(); //"find error";//Rcpp::as<std::string>(result);


    //    Rcpp::Environment base = Rcpp::Environment::global_env();
    //    Rcpp::Function f  = base["traceback"];
    //    std::string ret = Rcpp::as<std::string>(f());
    fims::FIMSLog::fims_log->error_message(log_entry, -1, "R_env", ret.c_str());
}

RCPP_EXPOSED_CLASS(Parameter)
RCPP_EXPOSED_CLASS(ParameterVector)

RCPP_MODULE(fims) {
  Rcpp::function("CreateTMBModel", &CreateTMBModel);
  Rcpp::function("SetFIMSFunctions", &SetFIMSFunctions);
  Rcpp::function("Finalize", &Finalize);
  Rcpp::function("ToJSON", &ToJSON);
  Rcpp::function("get_fixed", &get_fixed_parameters_vector);
  Rcpp::function("get_random", &get_random_parameters_vector);
  Rcpp::function("get_parameter_names", &get_parameter_names);
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
  Rcpp::function("get_log", get_log);
  Rcpp::function("get_log_errors", get_log_errors);
  Rcpp::function("get_log_warnings", get_log_warnings);
  Rcpp::function("get_log_info", get_log_info);
  Rcpp::function("get_log_module", get_log_module);
  Rcpp::function("write_log", write_log);
  Rcpp::function("set_log_path", set_log_path);
  Rcpp::function("init_logging", init_logging);
  Rcpp::function("set_log_throw_on_error", set_log_throw_on_error);
  Rcpp::function("log_info", log_info);
  Rcpp::function("log_warning", log_warning);
  Rcpp::function("log_error", log_error);
  Rcpp::class_<Parameter>("Parameter", "FIMS Parameter Class")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::initial_value_m, "numeric parameter value")
      .field("value", &Parameter::final_value_m, "numeric estimated parameter value")
      .field("min", &Parameter::min_m, "minimum parameter value")
      .field("max", &Parameter::max_m, "maximum parameter value")
      .field("id", &Parameter::id_m, "unique id for parameter class")
      .field("is_random_effect", &Parameter::is_random_effect_m, "boolean indicating whether or not parameter is a random effect; default value is FALSE")
      .field("estimated", &Parameter::estimated_m, "boolean indicating whether or not parameter is estimated; default value is FALSE");

   Rcpp::class_<ParameterVector>("ParameterVector")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("get", &ParameterVector::get)
      .method("set", &ParameterVector::set)
      .method("show", &ParameterVector::show)
      //            .field("data", &ParameterVector::storage_m, "list where each element is a Parameter class")
      .method("at", &ParameterVector::at, "returns a Parameter at the indicated position given the index argument")
      .method("size", &ParameterVector::size, "returns the size of the Parameter Vector")
      .method("resize", &ParameterVector::resize, "resizes the Parameter Vector given the provided length argument")
      .method("set_all_estimable", &ParameterVector::set_all_estimable, "sets all Parameters within vector as estimable")
      .method("set_all_random", &ParameterVector::set_all_random, "sets all Parameters within vector as estimable")
      .method("fill", &ParameterVector::fill, "sets the value of all Parameters in the vector with the provided value")
      .method("get_id", &ParameterVector::get_id, "get the ID of the interface base object.");

  Rcpp::class_<BevertonHoltRecruitmentInterface>("BevertonHoltRecruitment")
      .constructor()
      .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep)
      .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero)
      .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs)
      .field("estimate_log_devs",
             &BevertonHoltRecruitmentInterface::estimate_log_devs)
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
      .method("evaluate", &BevertonHoltRecruitmentInterface::evaluate);

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .field("is_survey", &FleetInterface::is_survey)
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("nages", &FleetInterface::nages)
      .field("nyears", &FleetInterface::nyears)
      .field("estimate_q", &FleetInterface::estimate_q)
      .field("random_q", &FleetInterface::random_q)
      .field("log_expected_index", &FleetInterface::log_expected_index)
      .field("proportion_catch_numbers_at_age", &FleetInterface::proportion_catch_numbers_at_age)
      .method("SetObservedAgeCompData", &FleetInterface::SetObservedAgeCompData)
      .method("GetObservedAgeCompDataID", &FleetInterface::GetObservedAgeCompDataID)
      .method("SetObservedIndexData", &FleetInterface::SetObservedIndexData)
      .method("GetObservedIndexDataID", &FleetInterface::GetObservedIndexDataID)
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
      .field("estimate_prop_female", &PopulationInterface::estimate_prop_female)
      .method("evaluate", &PopulationInterface::evaluate)
      .method("SetMaturity", &PopulationInterface::SetMaturity)
      .method("SetGrowth", &PopulationInterface::SetGrowth)
      .method("SetRecruitment", &PopulationInterface::SetRecruitment)
      .method("evaluate", &PopulationInterface::evaluate);

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

  Rcpp::class_<DnormDistributionsInterface>("DnormDistribution")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
      .method("evaluate", &DnormDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
      .method("set_observed_data", &DnormDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
      .method("set_distribution_links", &DnormDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
      .field("x", &DnormDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect")
      .field("expected_values", &DnormDistributionsInterface::expected_values, "Mean of the distribution.")
      .field("log_sd", &DnormDistributionsInterface::log_sd, "The natural log of the standard deviation.");

  Rcpp::class_<DlnormDistributionsInterface>("DlnormDistribution")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
      .method("evaluate", &DlnormDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
      .method("set_observed_data", &DlnormDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
      .method("set_distribution_links", &DlnormDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
      .field("x", &DlnormDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect")
      .field("expected_values", &DlnormDistributionsInterface::expected_values, "Mean of the distribution on the log scale.")
      .field("log_sd", &DlnormDistributionsInterface::log_sd, "The natural log of the standard deviation of the distribution on the log scale.");

  Rcpp::class_<DmultinomDistributionsInterface>("DmultinomDistribution")
      .constructor()
      .method("get_id", &DmultinomDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
      .method("evaluate", &DmultinomDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
      .method("set_observed_data", &DmultinomDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
      .method("set_distribution_links", &DmultinomDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
      .field("x", &DmultinomDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect")
      .field("expected_values", &DmultinomDistributionsInterface::expected_values, "numeric non-negative vector of length K, specifying the probability for the K classes.")
      .field("dims", &DmultinomDistributionsInterface::dims, "dimension of the multivariate input, e.g., c(num rows, num cols).");
}

#endif /* RCPP_INTERFACE_HPP */
