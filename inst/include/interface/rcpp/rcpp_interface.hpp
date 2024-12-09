/**
 * @file rcpp_interface.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
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
#include "rcpp_objects/rcpp_distribution.hpp"
#include "../../utilities/fims_json.hpp"

/**
 * @brief TODO: provide a brief description.
 * 
 */
SEXP FIMS_objective_function;
/**
 * @brief TODO: provide a brief description.
 * 
 */
SEXP FIMS_gradient_function;
/**
 * @brief TODO: provide a brief description.
 * 
 */
double FIMS_function_value = 0;
/**
 * @brief TODO: provide a brief description.
 * 
 */
Rcpp::NumericVector FIMS_function_parameters;
/**
 * @brief TODO: provide a brief description.
 * 
 */
Rcpp::NumericVector FIMS_function_gradient;
/**
 * @brief TODO: provide a brief description.
 * 
 */
double FIMS_mgc_value = 0;
/**
 * @brief Sets FIMS_finalized to false as the default.
 * 
 */
bool FIMS_finalized = false;

/**
 * Initializes the logging system, sets all signal handling.
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
 * @brief Create the TMB model object and add interface objects to it.
 */
bool CreateTMBModel() {

    init_logging();

    FIMS_INFO_LOG("adding FIMS objects to TMB");
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

    return true;
}

/**
 * @brief TODO: provide a brief description.
 * 
 * @param fn 
 * @param gr 
 */
void SetFIMSFunctions(SEXP fn, SEXP gr) {
    FIMS_objective_function = fn;
    FIMS_gradient_function = gr;
}

/**
 * @brief Loops through the Rcpp Interface objects and extracts derived
 * quantities.Updates parameter estimates from model core objects.
 */
void finalize_objects(Rcpp::NumericVector p) {
    FIMS_function_parameters = p;



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
    double maxgc = -999;
    for (size_t i = 0; i < FIMS_function_gradient.size(); i++) {
        if (std::fabs(FIMS_function_gradient[i]) > maxgc) {
            maxgc = std::fabs(FIMS_function_gradient[i]);
        }
    }
    FIMS_mgc_value = maxgc;

    for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
            i++) {
        FIMSRcppInterfaceBase::fims_interface_objects[i]->finalize();
    }
}

/**
 * @brief Finalizes a FIMS model by updating the parameter set. This function
 *        evaluates the objective function and the gradient with the given
 *        parameter set.
 *
 * @param obj - Either a list containing \"fn\" and \"gr\", or a list containing
 *              two seperate lists \"obj\" and \"opt\", \"obj\" should contain
 *              \"fn\" and \"gr\", \"opt\" should contain \"par\". In the second
 *              case, the second function argument is expected to be null and
 *              ignored.
 *
 * @param opt - A list containing \"par\".
 */
void finalize_fims(Rcpp::Nullable< Rcpp::List> obj = R_NilValue,
        Rcpp::Nullable< Rcpp::List> opt = R_NilValue) {

    bool has_obj = false;
    bool has_opt = false;
    bool is_single_argument = false;
    bool valid_list = true;
    Rcpp::NumericVector parameters;

    //check and handle the first argument.
    if (!Rf_isNull(obj.get())) {
        Rcpp::List input_list = Rcpp::as<Rcpp::List>(obj);
        if (input_list.containsElementNamed("obj")
                && input_list.containsElementNamed("opt")) {
            Rcpp::List obj_list = input_list["obj"];
            Rcpp::List opt_list = input_list["opt"];

            if (obj_list.containsElementNamed("fn")) {
                FIMS_objective_function = obj_list["fn"];
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"fn\" not found in argument list.");
            }

            if (obj_list.containsElementNamed("gr")) {
                FIMS_gradient_function = obj_list["gr"];
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"gr\" not found in argument list.");
            }

            if (opt_list.containsElementNamed("par")) {
                parameters = Rcpp::as<Rcpp::NumericVector>(opt_list["par"]);
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"par\" not found in argument list.");

            }

            //if we are here, a single argument was used. if it contains the
            //expected elements, the list is valid and objects can be finalize.
            if (valid_list) {
                finalize_objects(parameters);
                FIMS_finalized = true;
                return;
            } else {
                return;
            }


        } else {//two arguments?
            if (input_list.containsElementNamed("fn")) {
                FIMS_objective_function = input_list["fn"];
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"fn\" not found in argument list.");
            }

            if (input_list.containsElementNamed("gr")) {
                FIMS_gradient_function = input_list["gr"];
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"gr\" not found in argument list.");
            }
        }
    }


    //check second argument.
    if (!Rf_isNull(opt.get())) {

        Rcpp::List input_list = Rcpp::as<Rcpp::List>(opt);

        if (input_list.containsElementNamed("par")) {
            parameters = Rcpp::as<Rcpp::NumericVector>(input_list["par"]);
        } else {
            valid_list = false;
            FIMS_ERROR_LOG("Invalid call, \"par\" not found in argument list.");

        }
    } else {
        valid_list = false;
    }

    //if we're here, two arguments were given. If they contain the expected
    //elements, the lists are valid and objects can be finalized.
    if (valid_list) {
        finalize_objects(parameters);
        FIMS_finalized = true;
    }


}

/**
 * @brief Finalizes a FIMS run.
 *
 * @param tmb_obj is list containing an objective function \"fn\",
 * a gradient function \"gr\" and a parameter set \"par\".
 *
 */
void finalize_fims_deprecated(SEXP tmb_obj = R_NilValue) {

    Rcpp::NumericVector parameters;
    bool valid_list = true;

    if (!Rf_isNull(tmb_obj)) {

        Rcpp::List tmb_list = Rcpp::as<Rcpp::List>(tmb_obj);

        if (tmb_list.containsElementNamed("obj")) {
            Rcpp::List tmb_obj_list = tmb_list["obj"];

            if (tmb_obj_list.containsElementNamed("fn")) {
                FIMS_objective_function = tmb_obj_list["fn"];
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"fn\" not found in argument list.");
            }

            if (tmb_obj_list.containsElementNamed("gr")) {
                FIMS_gradient_function = tmb_obj_list["gr"];
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"gr\" not found in argument list.");
            }
        } else {
            valid_list = false;
            FIMS_ERROR_LOG("Invalid call, \"obj\" not found in argument list.");

        }


        if (tmb_list.containsElementNamed("opt")) {
            Rcpp::List tmb_opt_list = tmb_list["opt"];
            if (tmb_opt_list.containsElementNamed("par")) {
                parameters = Rcpp::as<Rcpp::NumericVector>(tmb_opt_list["par"]);
            } else {
                valid_list = false;
                FIMS_ERROR_LOG("Invalid call, \"par\" not found in argument list.");

            }
        } else {
            valid_list = false;
            FIMS_ERROR_LOG("Invalid call, \"opt\" not found in argument list.");

        }

        if (valid_list) {
            Rcpp::Rcout << "finalizing objects:)";
            finalize_objects(parameters);
        } else {
            Rcpp::Rcout << "Invalid list argument for Finalize(List).";
            FIMS_ERROR_LOG("Unable to finalize objects.");

        }
    }
}

/**
 * @brief Extracts derived quantities from model objects.
 */
std::string get_output() {
    std::string ret;
    if (FIMS_finalized) {
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

        ret = fims::JsonParser::PrettyFormatJSON(ss.str());
    } else {
        Rcpp::Rcout << "Invalid request to \"get_output\". Please call finalize first.";
    }
    return ret;
}

/**
 * @brief Get the fixed parameters vector object
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
 * @brief Get the random parameters vector object
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
 * @brief Get the parameter names object
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

    LengthCompDataInterface::id_g = 1;
    LengthCompDataInterface::live_objects.clear();

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
 * If true, writes the log on exit.
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
 * @brief Escape quotations.
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
    ss_ret << "traceback: ";
    for (int j = 0; j < LENGTH(result); j++) {
        std::string str(CHAR(STRING_ELT(result, j)));
        ss_ret << escapeQuotes(str) << "\\n";
    }

    std::string ret = ss_ret.str(); //"find error";//Rcpp::as<std::string>(result);


    //    Rcpp::Environment base = Rcpp::Environment::global_env();
    //    Rcpp::Function f  = base["traceback"];
    //    std::string ret = Rcpp::as<std::string>(f());
    fims::FIMSLog::fims_log->error_message(log_entry, -1, "R_env", ret.c_str());
}

RCPP_EXPOSED_CLASS(Parameter)
RCPP_EXPOSED_CLASS(ParameterVector)

/**
 * @brief Construct a new rcpp module object
 * 
 */
RCPP_MODULE(fims) {
    Rcpp::function("CreateTMBModel", &CreateTMBModel);
    Rcpp::function("SetFIMSFunctions", &SetFIMSFunctions);
    Rcpp::function("finalize", &finalize_fims);
    Rcpp::function("get_output", &get_output);
    Rcpp::function("get_fixed", &get_fixed_parameters_vector);
    Rcpp::function("get_random", &get_random_parameters_vector);
    Rcpp::function("get_parameter_names", &get_parameter_names);
    Rcpp::function("clear", clear);
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
            .field("is_random_effect", &Parameter::is_random_effect_m, "A boolean indicating whether or not parameter is a random effect; default value is FALSE.")
            .field("estimated", &Parameter::estimated_m, "A boolean indicating whether or not parameter is estimated; default value is FALSE.");

    Rcpp::class_<ParameterVector>("ParameterVector")
            .constructor()
            .constructor<size_t>()
            .constructor<Rcpp::NumericVector, size_t>()
            .method("get", &ParameterVector::get)
            .method("set", &ParameterVector::set)
            .method("show", &ParameterVector::show)
            //            .field("data", &ParameterVector::storage_m, "list where each element is a Parameter class")
            .method("at", &ParameterVector::at, "Returns a Parameter at the indicated position given the index argument.")
            .method("size", &ParameterVector::size, "Returns the size of the Parameter Vector.")
            .method("resize", &ParameterVector::resize, "Resizes the Parameter Vector given the provided length argument.")
            .method("set_all_estimable", &ParameterVector::set_all_estimable, "Sets all Parameters within vector as estimable.")
            .method("set_all_random", &ParameterVector::set_all_random, "Sets all Parameters within vector as random.")
            .method("fill", &ParameterVector::fill, "Sets the value of all Parameters in the vector with the provided value.")
            .method("get_id", &ParameterVector::get_id, "Get the ID of the interface base object.");

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
      .field("nlengths", &FleetInterface::nlengths)
      .field("estimate_q", &FleetInterface::estimate_q)
      .field("random_q", &FleetInterface::random_q)
      .field("log_expected_index", &FleetInterface::log_expected_index)
      .field("proportion_catch_numbers_at_age", &FleetInterface::proportion_catch_numbers_at_age)
      .field("proportion_catch_numbers_at_length", &FleetInterface::proportion_catch_numbers_at_length)
      .field("age_length_conversion_matrix", &FleetInterface::age_length_conversion_matrix)
      .method("SetObservedAgeCompData", &FleetInterface::SetObservedAgeCompData)
      .method("GetObservedAgeCompDataID", &FleetInterface::GetObservedAgeCompDataID)
      .method("SetObservedLengthCompData", &FleetInterface::SetObservedLengthCompData)
      .method("GetObservedLengthCompDataID", &FleetInterface::GetObservedLengthCompDataID)
      .method("SetObservedIndexData", &FleetInterface::SetObservedIndexData)
      .method("GetObservedIndexDataID", &FleetInterface::GetObservedIndexDataID)
      .method("SetSelectivity", &FleetInterface::SetSelectivity);

    Rcpp::class_<AgeCompDataInterface>("AgeComp")
            .constructor<int, int>()
            .field("age_comp_data", &AgeCompDataInterface::age_comp_data)
            .method("get_id", &AgeCompDataInterface::get_id);

    Rcpp::class_<LengthCompDataInterface>("LengthComp")
            .constructor<int, int>()
            .field("length_comp_data", &LengthCompDataInterface::length_comp_data)
            .method("get_id", &LengthCompDataInterface::get_id);

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
            .field("nlengths", &PopulationInterface::nlengths)
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
        .field("expected_values", &DlnormDistributionsInterface::expected_values, "Mean of the distribution on the log scale.")
        .field("log_sd", &DlnormDistributionsInterface::log_sd, "The natural log of the standard deviation of the distribution on the log scale.");

    Rcpp::class_<DmultinomDistributionsInterface>("DmultinomDistribution")
        .constructor()
        .method("get_id", &DmultinomDistributionsInterface::get_id, "Returns a unique ID for the Dnorm distribution class.")
        .method("evaluate", &DmultinomDistributionsInterface::evaluate, "Evaluates the normal distribution given input data and parameter values.")
        .method("set_observed_data", &DmultinomDistributionsInterface::set_observed_data, "Accepts a unique ID for a given Data Object class to link the data with the distribution.")
        .method("set_distribution_links", &DmultinomDistributionsInterface::set_distribution_links, "Accepts a unique ID for a given parameter to link the parameter with the distribution.")
        .field("x", &DmultinomDistributionsInterface::x, "Input for distribution when not observations, e.g., prior or random effect.")
        .field("expected_values", &DmultinomDistributionsInterface::expected_values, "numeric non-negative vector of length K, specifying the probability for the K classes.")
        .field("dims", &DmultinomDistributionsInterface::dims, "dimension of the multivariate input, e.g., c(num rows, num cols).");
}



#endif /* RCPP_INTERFACE_HPP */
