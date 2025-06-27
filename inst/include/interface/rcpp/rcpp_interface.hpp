/**
 * @file rcpp_interface.hpp
 * @brief The Rcpp interface to declare things. Allows for the use of
 * methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP

#include "rcpp_objects/rcpp_data.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_math.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "../../common/model.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"
#include "rcpp_objects/rcpp_distribution.hpp"
#include "../../utilities/fims_json.hpp"
#include "../rcpp/rcpp_objects/rcpp_interface_base.hpp"

/**
 * Initializes the logging system, setting all signal handling.
 */
void init_logging() {
  std::signal(SIGSEGV, &fims::WriteAtExit);
  std::signal(SIGINT, &fims::WriteAtExit);
  std::signal(SIGABRT, &fims::WriteAtExit);
  std::signal(SIGFPE, &fims::WriteAtExit);
  std::signal(SIGILL, &fims::WriteAtExit);
  std::signal(SIGTERM, &fims::WriteAtExit);
}

/**
 * @brief Creates the TMB model object and adds interface objects to it.
 *
 * @details
 * This function is called within `initialize_fims()` from R and is not
 * typically called by the user directly.
 */
bool CreateTMBModel() {
  init_logging();

  FIMS_INFO_LOG("Adding FIMS objects to TMB, " +
    fims::to_string(FIMSRcppInterfaceBase::fims_interface_objects.size()) +
    " objects");
  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
      i++) {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  // base model
  std::shared_ptr<fims_info::Information < TMB_FIMS_REAL_TYPE>> d0 =
    fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  d0->CreateModel();

  // first-order derivative
  std::shared_ptr<fims_info::Information < TMB_FIMS_FIRST_ORDER>> d1 =
    fims_info::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
  d1->CreateModel();

  // second-order derivative
  std::shared_ptr<fims_info::Information < TMB_FIMS_SECOND_ORDER>> d2 =
    fims_info::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
  d2->CreateModel();

  // third-order derivative
  std::shared_ptr<fims_info::Information < TMB_FIMS_THIRD_ORDER>> d3 =
    fims_info::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
  d3->CreateModel();

  // instantiate the model? TODO: Ask Matthew what this does
  std::shared_ptr<fims_model::Model < TMB_FIMS_REAL_TYPE>> m0 =
  fims_model::Model<TMB_FIMS_REAL_TYPE>::GetInstance();

  return true;
}

/**
 * Finalize a model run by populating derived quantities into the Rcpp interface
 * objects and return the output as a JSON string.
 *
 * @param par A vector of parameter values.
 * @param fn The objective function.
 * @param gr The gradient function.
 *
 * @return A JSON output string is returned.
 */
std::string finalize_fims(Rcpp::NumericVector par, Rcpp::Function fn, Rcpp::Function gr) {

    std::shared_ptr<fims_info::Information < double>> information =
      fims_info::Information<double>::GetInstance();

    std::shared_ptr<fims_model::Model < double>> model =
      fims_model::Model<double>::GetInstance();
    for (size_t i = 0; i < information->fixed_effects_parameters.size(); i++) {
        *information->fixed_effects_parameters[i] = par[i];
    }

    bool reporting = model->do_tmb_reporting;
    model->do_tmb_reporting = false;
    model->Evaluate();

    Rcpp::Function f = Rcpp::as<Rcpp::Function>(fn);
    Rcpp::Function g = Rcpp::as<Rcpp::Function>(gr);
    double val = Rcpp::as<double>(f(par));
    Rcpp::NumericVector grad = Rcpp::as<Rcpp::NumericVector>(g(par));
    double maxgc = -999;
    for (R_xlen_t i = 0; i < grad.size(); i++) {
      if (std::fabs(grad[i]) > maxgc) {
        maxgc = std::fabs(grad[i]);
      }
    }

    for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
        i++) {
      FIMSRcppInterfaceBase::fims_interface_objects[i]->finalize();
    }
  std::string ret;
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
    ss << "\"objective_function_value\": " << val << ",\n";
    ss << "\"max_gradient_component\": " << maxgc << ",\n";
    ss << "\"final_gradient\": [";
    if (grad.size() > 0) {
      for (R_xlen_t i = 0; i < grad.size() - 1; i++) {
        ss << grad[i] << ", ";
      }
      ss << grad[grad.size() - 1] << "],\n";
    } else {
      ss << "],";
    }
    ss << "\"modules\" : [\n";
    size_t length = FIMSRcppInterfaceBase::fims_interface_objects.size();
    if (length > 0) {
        for (size_t i = 0; i < length - 1; i++) {
            ss << FIMSRcppInterfaceBase::fims_interface_objects[i]->to_json() << ",\n";
        }

        ss << FIMSRcppInterfaceBase::fims_interface_objects[length - 1]->to_json() << "\n]\n}";
    } else {
        ss << "\n]\n}";
    }

    ret = fims::JsonParser::PrettyFormatJSON(ss.str());
    model->do_tmb_reporting = reporting;
    return ret;
}

/**
 * @brief Gets the fixed parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_fixed_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information < TMB_FIMS_REAL_TYPE>> d0 =
    fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->fixed_effects_parameters.size(); i++) {
    p.push_back(*d0->fixed_effects_parameters[i]);
  }

  return p;
}

/**
 * @brief Gets the random parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_random_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information < TMB_FIMS_REAL_TYPE>> d0 =
    fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->random_effects_parameters.size(); i++) {
    p.push_back(*d0->random_effects_parameters[i]);
  }

  return p;
}

/**
 * @brief Gets the parameter names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_parameter_names(Rcpp::List pars) {
  // base model
  std::shared_ptr<fims_info::Information < TMB_FIMS_REAL_TYPE>> d0 =
    fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->parameter_names;

  return pars;
}

/**
 * @brief Gets the random effects names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_random_names(Rcpp::List pars) {
  // base model
  std::shared_ptr<fims_info::Information < TMB_FIMS_REAL_TYPE>> d0 =
    fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->random_effects_names;

  return pars;
}

/**
 * @brief Clears the internal objects.
 *
 * @tparam Type
 */
template <typename Type>
void clear_internal() {
  std::shared_ptr<fims_info::Information < Type>> d0 =
    fims_info::Information<Type>::GetInstance();
  d0->Clear();
}

/**
 * @brief Clears the vector of independent variables.
 */
void clear() {
  FIMS_INFO_LOG("Clearing FIMS objects from interface stack");
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

  LengthCompDataInterface::id_g = 1;
  LengthCompDataInterface::live_objects.clear();

  LandingsDataInterface::id_g = 1;
  LandingsDataInterface::live_objects.clear();

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

  // rcpp_distribution.hpp
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

  fims::FIMSLog::fims_log->clear();

}

/**
 * @brief Gets the log entries as a string in JSON format.
 */
std::string get_log() {
  return fims::FIMSLog::fims_log->get_log();
}

/**
 * @brief Gets the error entries from the log as a string in JSON format.
 */
std::string get_log_errors() {
  return fims::FIMSLog::fims_log->get_errors();
}

/**
 * @brief Gets the warning entries from the log as a string in JSON format.
 */
std::string get_log_warnings() {
    return fims::FIMSLog::fims_log->get_warnings();
}

/**
 * @brief Gets the info entries from the log as a string in JSON format.
 */
std::string get_log_info() {
    return fims::FIMSLog::fims_log->get_info();
}

/**
 * @brief Gets log entries by module as a string in JSON format.
 */
std::string get_log_module(const std::string& module) {
  return fims::FIMSLog::fims_log->get_module(module);
}

/**
 * @brief If true, writes the log on exit.
 */
void write_log(bool write) {
  FIMS_INFO_LOG("Setting FIMS write log: " + fims::to_string(write));
  fims::FIMSLog::fims_log->write_on_exit = write;
}

/**
 * @brief Sets the path for the log file to be written to.
 */
void set_log_path(const std::string& path) {
  FIMS_INFO_LOG("Setting FIMS log path: " + path);
  fims::FIMSLog::fims_log->set_path(path);
}

/**
 * @brief If true, throws a runtime exception when an error is logged.
 */
void set_log_throw_on_error(bool throw_on_error) {
  fims::FIMSLog::fims_log->throw_on_error = throw_on_error;
}

/**
 * @brief Adds an info entry to the log from the R environment.
 */
void log_info(std::string log_entry) {
  fims::FIMSLog::fims_log->info_message(log_entry, -1, "R_env", "R_script_entry");
}

/**
 * @brief Adds a warning entry to the log from the R environment.
 */
void log_warning(std::string log_entry) {
  fims::FIMSLog::fims_log->warning_message(log_entry, -1, "R_env", "R_script_entry");
}

/**
 * @brief Escapes quotations.
 *
 * @param input A string.
 * @return std::string
 */
std::string escapeQuotes(const std::string& input) {
  std::string result = input;
  std::string search = "\"";
  std::string replace = "\\\"";

  // Find each occurrence of `"` and replace it with `\"`
  size_t pos = result.find(search);
  while (pos != std::string::npos) {
    result.replace(pos, search.size(), replace);
    pos = result.find(search, pos + replace.size()); // Move past the replaced position
  }
  return result;
}

/**
 * @brief Adds a error entry to the log from the R environment.
 */
void log_error(std::string log_entry) {
  std::stringstream ss;
  ss << "capture.output(traceback(4))";
  SEXP expression, result;
  ParseStatus status;

  PROTECT(expression = R_ParseVector(Rf_mkString(ss.str().c_str()), 1, &status, R_NilValue));
  if (status != PARSE_OK) {
    Rcpp::Rcout << "Error parsing expression" << std::endl;
    UNPROTECT(1);
  }
  Rcpp::Rcout << "before call.";
  PROTECT(result = Rf_eval(VECTOR_ELT(expression, 0), R_GlobalEnv));
  Rcpp::Rcout << "after call.";
  UNPROTECT(2);
  std::stringstream ss_ret;
  ss_ret << "traceback: ";
  for (int j = 0; j < LENGTH(result); j++) {
    std::string str(CHAR(STRING_ELT(result, j)));
    ss_ret << escapeQuotes(str) << "\\n";
  }

  std::string ret = ss_ret.str(); //"find error";//Rcpp::as<std::string>(result);

  fims::FIMSLog::fims_log->error_message(log_entry, -1, "R_env", ret.c_str());
}

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
    Rcpp::function(
        "finalize", &finalize_fims,
        "Extracts the derived quantities from `Information` to the Rcpp object and returns a JSON string as the output.");
    Rcpp::function(
        "get_fixed", &get_fixed_parameters_vector,
        "Gets the fixed parameters vector object.");
    Rcpp::function(
        "get_random", &get_random_parameters_vector,
        "Gets the random parameters vector object.");
    Rcpp::function(
        "get_parameter_names", &get_parameter_names,
        "Gets the parameter names object.");
    Rcpp::function(
        "get_random_names", &get_random_names,
        "Gets the random effects names object.");
    Rcpp::function(
        "clear", clear,
        "Clears all pointers/references of a FIMS model");
    Rcpp::function(
        "get_log", get_log,
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
    Rcpp::function(
        "get_log_module", get_log_module,
        "Gets log entries by module as a string in JSON format.");
    Rcpp::function(
        "write_log", write_log,
        "If true, writes the log on exit.");
    Rcpp::function(
        "set_log_path", set_log_path,
        "Sets the path for the log file to be written to.");
    Rcpp::function(
        "init_logging", init_logging,
        "Initializes the logging system, setting all signal handling.");
    Rcpp::function(
        "set_log_throw_on_error", set_log_throw_on_error,
        "If true, throws a runtime exception when an error is logged.");
    Rcpp::function(
        "log_info", log_info,
        "Adds an info entry to the log from the R environment.");
    Rcpp::function(
        "log_warning", log_warning,
        "Adds a warning entry to the log from the R environment.");
    Rcpp::function(
        "log_error", log_error,
        "Adds a error entry to the log from the R environment.");
    Rcpp::function(
        "logit", logit_rcpp,
        "Applies the logit transformation: -log(b - x) + log(x - a).");
    Rcpp::function(
        "inv_logit", inv_logit_rcpp,
        "Applies the inverse of the logit transformation to a bounded space.");
    Rcpp::class_<Parameter>(
        "Parameter",
        "An RcppInterface class that defines the Parameter class.")
            .constructor()
            .constructor<double>()
            .constructor<Parameter>()
            .field(
                "value", &Parameter::initial_value_m,
                "A numeric value specifying the initial value of the parameter.")
            .field(
                "value", &Parameter::final_value_m,
                "A numeric value specifying the final value of the parameter.")
            .field(
                "min", &Parameter::min_m,
                "A numeric value specifying the minimum possible parameter value, where the default is negative infinity.")
            .field(
                "max", &Parameter::max_m,
                "A numeric value specifying the maximum possible parameter value, where the default is positive infinity.")
            .field(
                "id", &Parameter::id_m,
                "unique id for parameter class")
            .field(
                "estimation_type", &Parameter::estimation_type_m,
                "A string that takes three arguments: constant, indicating a parameter is not estimated; fixed_effects, indicating a parameter is estimated; and random_effects, indicating a parameter is estimated; the default is constant.");

    Rcpp::class_<ParameterVector>(
      "ParameterVector",
      "An RcppInterface class that defines the ParameterVector class.")
            .constructor()
            .constructor<size_t>()
            .constructor<Rcpp::NumericVector, size_t>()
            .method("get", &ParameterVector::get,
                "An internal accessor for calling a position of a ParameterVector from R.")
            .method("set", &ParameterVector::set,
                "An internal setter for setting a position of a ParameterVector from R.")
            .method("show", &ParameterVector::show,
                "The printing methods for a ParameterVector.")
            .method("at", &ParameterVector::at,
                "Returns a Parameter at the indicated position given the index argument.")
            .method("size", &ParameterVector::size,
                "Returns the size of a ParameterVector.")
            .method("resize", &ParameterVector::resize,
                "Resizes a ParameterVector to the desired length.")
            .method("set_all_estimable", &ParameterVector::set_all_estimable,
                "Sets all Parameters within a ParameterVector as estimable.")
            .method("set_all_random", &ParameterVector::set_all_random,
                "Sets all Parameters within a ParameterVector as random effects.")
            .method("fill", &ParameterVector::fill,
                "Sets the value of all Parameters in the ParameterVector to the provided value.")
            .method("get_id", &ParameterVector::get_id,
                "Gets the ID of the ParameterVector object.");
          Rcpp::class_<RealVector>(
            "RealVector",
            "An RcppInterface class that defines the RealVector class.")
            .constructor()
            .constructor<size_t>()
            .constructor<Rcpp::NumericVector, size_t>()
            .method("get", &RealVector::get,
            "An internal accessor for calling a position of a RealVector from R.")
            .method("set", &RealVector::set,
            "An internal setter for setting a position of a RealVector from R.")
            .method("fromRVector", &RealVector::fromRVector,
            "Initializes the RealVector from the values of a R vector.")
            .method("toRVector", &RealVector::toRVector,
            "Returns values as a R vector.")
            .method("show", &RealVector::show,
            "The printing methods for a RealVector.")
            .method("at", &RealVector::at,
            "Returns a double at the indicated position given the index argument.")
            .method("size", &RealVector::size,
            "Returns the size of a RealVector.")
            .method("resize", &RealVector::resize,
            "Resizes a RealVector to the desired length.")
            .method("get_id", &RealVector::get_id,
            "Gets the ID of the RealVector object.");

    Rcpp::class_<SharedInt>(
            "SharedInt",
            "An RcppInterface class that defines the SharedInt class.")
            .constructor()
            .constructor<int>()
            .method("get", &SharedInt::get)
            .method("set", &SharedInt::set);

    Rcpp::class_<SharedBoolean>(
            "SharedBoolean",
            "An RcppInterface class that defines the SharedBoolean class.")
            .constructor()
            .constructor<bool>()
            .method("get", &SharedBoolean::get)
            .method("set", &SharedBoolean::set);

    Rcpp::class_<SharedReal>(
            "SharedReal",
            "An RcppInterface class that defines the SharedReal class.")
            .constructor()
            .constructor<double>()
            .method("get", &SharedReal::get)
            .method("set", &SharedReal::set);

    Rcpp::class_<BevertonHoltRecruitmentInterface>("BevertonHoltRecruitment")
            .constructor()
            .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep)
            .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero)
            .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs)
            .field("log_r", &BevertonHoltRecruitmentInterface::log_r, "recruitment as a random effect on the natural log scale")
            .field("nyears", &BevertonHoltRecruitmentInterface::nyears, "Number of years")
            .field("log_expected_recruitment", &BevertonHoltRecruitmentInterface::log_expected_recruitment, "Log expectation of the recruitment process")
            .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
            .method("SetRecruitmentProcessID", &BevertonHoltRecruitmentInterface::SetRecruitmentProcessID, "Set unique ID for recruitment process")
            .method("evaluate_mean", &BevertonHoltRecruitmentInterface::evaluate_mean);

  Rcpp::class_<LogDevsRecruitmentInterface>("LogDevsRecruitmentProcess")
      .constructor()
      .method("get_id", &LogDevsRecruitmentInterface::get_id)
      .method("evaluate_process", &LogDevsRecruitmentInterface::evaluate_process);

  Rcpp::class_<LogRRecruitmentInterface>("LogRRecruitmentProcess")
      .constructor()
      .method("get_id", &LogRRecruitmentInterface::get_id)
      .method("evaluate_process", &LogRRecruitmentInterface::evaluate_process);

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("nages", &FleetInterface::nages)
      .field("nyears", &FleetInterface::nyears)
      .field("nlengths", &FleetInterface::nlengths)
      .field("observed_landings_units", &FleetInterface::observed_landings_units)
      .field("observed_index_units", &FleetInterface::observed_index_units)
      .field("index_expected", &FleetInterface::derived_index_expected)
      .field("landings_expected", &FleetInterface::derived_landings_expected)
      .field("log_index_expected", &FleetInterface::log_index_expected)
      .field("log_landings_expected", &FleetInterface::log_landings_expected)
      .field("agecomp_expected", &FleetInterface::agecomp_expected)
      .field("lengthcomp_expected", &FleetInterface::lengthcomp_expected)
      .field("agecomp_proportion", &FleetInterface::agecomp_proportion)
      .field("lengthcomp_proportion", &FleetInterface::lengthcomp_proportion)
      .field("age_to_length_conversion", &FleetInterface::age_to_length_conversion)
      .method("SetObservedAgeCompDataID", &FleetInterface::SetObservedAgeCompDataID)
      .method("GetObservedAgeCompDataID", &FleetInterface::GetObservedAgeCompDataID)
      .method("SetObservedLengthCompDataID", &FleetInterface::SetObservedLengthCompDataID)
      .method("GetObservedLengthCompDataID", &FleetInterface::GetObservedLengthCompDataID)
      .method("SetObservedIndexDataID", &FleetInterface::SetObservedIndexDataID)
      .method("GetObservedIndexDataID", &FleetInterface::GetObservedIndexDataID)
      .method("SetObservedLandingsDataID", &FleetInterface::SetObservedLandingsDataID)
      .method("GetObservedLandingsDataID", &FleetInterface::GetObservedLandingsDataID)
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
            .field("nages", &PopulationInterface::nages, "number of ages")
            .field("nfleets", &PopulationInterface::nfleets, "number of fleets")
            .field("nseasons", &PopulationInterface::nseasons, "number of seasons")
            .field("nyears", &PopulationInterface::nyears, "number of years")
            .field("nlengths", &PopulationInterface::nlengths, "number of lengths")
            .field("log_M", &PopulationInterface::log_M, "natural log of the natural mortality of the population")
            .field("log_init_naa", &PopulationInterface::log_init_naa, "natural log of the initial numbers at age")
            .field("ages", &PopulationInterface::ages, "vector of ages in the population; length nages")
            .method("evaluate", &PopulationInterface::evaluate, "evaluate the population function")
            .method("SetMaturityID", &PopulationInterface::SetMaturityID, "Set the unique id for the Maturity object")
            .method("SetGrowthID", &PopulationInterface::SetGrowthID, "Set the unique id for the growth object")
            .method("SetRecruitmentID", &PopulationInterface::SetRecruitmentID, "Set the unique id for the Recruitment object")
            .method("evaluate", &PopulationInterface::evaluate, "evaluate the population function");

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
            .field(
                "inflection_point_asc",
                &DoubleLogisticSelectivityInterface::inflection_point_asc,
                "50 percent quantile of the value of the quantity of interest (x)  on the ascending limb of the double logistic curve; e.g., age at  which 50 percent of the fish are selected.")
            .field(
                "slope_asc",
                &DoubleLogisticSelectivityInterface::slope_asc,
                "Scalar multiplier of difference between quantity of interest value (x) and inflection_point on the ascending limb of the double logistic curve.")
            .field(
                "inflection_point_desc",
                &DoubleLogisticSelectivityInterface::inflection_point_desc,
                "50 percent quantile of the value of the quantity of interest (x) on the descending limb of the double logistic curve; e.g. age at which 50 percent of the fish are selected.")
            .field(
                "slope_desc",
                &DoubleLogisticSelectivityInterface::slope_desc,
                "Scalar multiplier of difference between quantity of interest  value (x) and inflection_point on the descending limb of the double  logistic  curve.")
            .method(
                "get_id",
                &DoubleLogisticSelectivityInterface::get_id,
                "Returns a unique ID for the selectivity class.")
            .method(
                "evaluate",
                &DoubleLogisticSelectivityInterface::evaluate,
                "Evaluates the double logistic selectivity given input value (e.g., age or size in selectivity).");

    Rcpp::class_<EWAAGrowthInterface>("EWAAgrowth")
            .constructor()
            .field("ages", &EWAAGrowthInterface::ages, "Ages for each age class.")
            .field("weights", &EWAAGrowthInterface::weights, "Weights for each age class.")
            .method("get_id", &EWAAGrowthInterface::get_id)
            .method("evaluate", &EWAAGrowthInterface::evaluate);

    Rcpp::class_<DnormDistributionsInterface>("DnormDistribution")
        .constructor()
        .method("get_id", &DnormDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
        .method("evaluate", &DnormDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
        .method("set_observed_data", &DnormDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
        .method("set_distribution_links", &DnormDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
        .field("x", &DnormDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect.")
        .field("expected_values", &DnormDistributionsInterface::expected_values, "Mean of the distribution.")
        .field("log_sd", &DnormDistributionsInterface::log_sd, "The natural log of the standard deviation.");

    Rcpp::class_<DlnormDistributionsInterface>("DlnormDistribution")
        .constructor()
        .method("get_id", &DlnormDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
        .method("evaluate", &DlnormDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
        .method("set_observed_data", &DlnormDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
        .method("set_distribution_links", &DlnormDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
        .field("x", &DlnormDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect.")
        .field("expected_values", &DlnormDistributionsInterface::expected_values, "Mean of the distribution on the natural log scale.")
        .field("log_sd", &DlnormDistributionsInterface::log_sd, "The natural log of the standard deviation of the distribution on the natural log scale.");

    Rcpp::class_<DmultinomDistributionsInterface>("DmultinomDistribution")
        .constructor()
        .method("get_id", &DmultinomDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
        .method("evaluate", &DmultinomDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
        .method("set_observed_data", &DmultinomDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
        .method("set_distribution_links", &DmultinomDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
        .method("set_note",&DmultinomDistributionsInterface::set_note)
        .field("x", &DmultinomDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect.")
        .field("expected_values", &DmultinomDistributionsInterface::expected_values, "numeric non-negative vector of length K, specifying the probability for the K classes.")
        .field("dims", &DmultinomDistributionsInterface::dims, "dimension of the multivariate input, e.g., c(num rows, num cols).");
}

#endif /* RCPP_INTERFACE_HPP */
