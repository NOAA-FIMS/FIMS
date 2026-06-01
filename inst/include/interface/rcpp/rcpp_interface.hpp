/**
 * @file rcpp_interface.hpp
 * @brief The Rcpp interface to declare things.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP
#include "../../common/model.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/types.hpp"
#include "../../utilities/fims_json.hpp"
#include "rcpp_objects/rcpp_data.hpp"
#include "rcpp_objects/rcpp_distribution.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_interface_base.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_models.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"

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
 * @brief Initialize and construct the FIMS model using TMB.
 *
 * @details
 * This function sets up the core C++ objects required for building the
 * objective function with TMB before optimizing a FIMS model. The main steps
 * of the function are as follows:
 * - The logging system is initialized and any existing model structures are
 *   cleared, ensuring a clean slate for a new model.
 * - It resets and prepares the main model information objects
 *   (fims_info::Information singletons), ensuring all internal data and
 *   settings are cleared and ready for a new model run. This step is essential
 *   for both initializing the model structure and avoiding conflicts from
 *   previous runs.
 * - It iterates over all registered FIMS interface objects and adds them to
 *   the TMB model context.
 * - After all of the objects are registered, it calls
 *   fims_info::Information::CreateModel() and
 *   fims_info::Information::CheckModel() on the base fims_info::Information
 *   object.
 * - It instantiates the singleton fims_model::Model object which represents
 *   the constructed TMB model.
 *
 * Typically the average user does not interact with this function because it
 * is called within <a href =
 * "https://noaa-fims.github.io/FIMS/reference/initialize_fims.html">`initialize_fims`</a>.
 *
 * @see init_logging()
 * @see fims_info::Information::Clear()
 * @see fims_info::Information::CreateModel()
 * @see fims_info::Information::CheckModel()
 * @see fims_info::Information::GetInstance()
 * @see <a href =
 * "https://noaa-fims.github.io/FIMS/reference/initialize_fims.html"
 * target="_blank">`initialize_fims()`</a>
 * @return A boolean is returned, where true indicates that the model was
 * successfully created.
 */
bool CreateTMBModel() {
  init_logging();

  // clear first
  //  base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  info0->Clear();

  std::shared_ptr<fims_info::Information<TMBAD_FIMS_TYPE>> info =
      fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance();
  info->Clear();

  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
       i++) {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  // base model
  info0->CreateModel();
  info0->CheckModel();

  info->CreateModel();
  /*
    // instantiate the model? TODO: Ask Matthew what this does
    std::shared_ptr<fims_model::Model<TMB_FIMS_REAL_TYPE>> m0 =
        fims_model::Model<TMB_FIMS_REAL_TYPE>::GetInstance();
  */
  return true;
}

/* Dictionary block for shared documentation.
  [details_set_x_parameters]
  Updates the internal parameter values for the model base of type
  TMB_FIMS_REAL_TYPE. It is typically called before finalize() or
  @ref CatchAtAgeInterface::to_json "`get_output()`" to ensure the correct
  values are used because TMB doesn't always keep the updated parameters in
  the "double" version of the tape. So we need to update those first.
  \n\n
  Usage example in R:
  \code{.R}
  set_fixed_parameters(c(1, 2, 3))
  set_random_parameters(c(1, 2, 3))
  catch_at_age$get_output()
  \endcode
  [details_set_x_parameters]
*/
/* Dictionary block for shared documentation.
  [param_par]
  @param par A vector of parameter values.
  [param_par]
 */

/**
 * @brief Update fixed parameters in the tape, so the output is correct.
 * @details @snippet{doc} this details_set_x_parameters
 * @snippet{doc} this param_par
 * @see set_random_parameters()
 */
void set_fixed_parameters(Rcpp::NumericVector par) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++) {
    *info0->fixed_effects_parameters[i] = par[i];
  }
}

/**
 * @brief Gets the fixed parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_fixed_parameters_vector() {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++) {
    p.push_back(*info0->fixed_effects_parameters[i]);
  }

  return p;
}

/**
 * @brief Update random effect parameters in the tape, so the output is correct.
 * @details @snippet{doc} this details_set_x_parameters
 * @snippet{doc} this param_par
 * @see set_fixed_parameters()
 */
void set_random_parameters(Rcpp::NumericVector par) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  for (size_t i = 0; i < info0->random_effects_parameters.size(); i++) {
    *info0->random_effects_parameters[i] = par[i];
  }
}

/**
 * @brief Gets the random parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
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
 * @brief Gets the parameter names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_parameter_names(Rcpp::List pars) {
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
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
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->random_effects_names;

  return pars;
}

/**
 * @brief Classifies an R formula's Left-Hand Side (LHS) and extracts the
 * transformation.
 * * @details This helper function inspects the Abstract Syntax Tree (AST) of
 * the raw R expression pointer (SEXP) passed from the formula's left-hand side.
 * It maps recognized R mathematical operations (like `log()`, `exp()`, or
 * exponentiation `^2`) to their respective domain-specific strongly typed enums
 * defined in the fims namespace.
 * * Supported AST structural mappings include:
 * - \b Symbol (e.g., \code{sd}): Maps to \c Label::identity
 * - \b Language Call (e.g., \code{log(sd)}): Maps to \c Label::log
 * - \b Language Call (e.g., \code{exp(sd)}): Maps to \c Label::exp
 * - \b Language Call (e.g., \code{logit(sd)}): Maps to \c Label::logit
 * - \b Language Call (e.g., \code{sqrt(sd)}): Maps to \c Label::sqrt
 * - \b Infix Exponent (e.g., \code{sd^2}): Maps to \c Label::square
 *
 * @param lhs_raw A raw \code{SEXP} representing the left-hand side node of an R
 * formula.
 * * @throws std::runtime_error (via \code{Rcpp::stop}) if the expression tree
 * structure is unrecognizable or contains an unmapped transformation function.
 * * @return A \code{fims::Transformation::Label} with the enumeration token.
 */
fims::Transformation classify_and_extract_transformation(SEXP lhs_raw) {
  fims::Transformation trans_config;

  if (Rcpp::is<Rcpp::Symbol>(lhs_raw)) {
    trans_config.label = fims::Transformation::Label::identity;
    return trans_config;
  }

  if (Rcpp::is<Rcpp::Language>(lhs_raw)) {
    Rcpp::Language lhs_lang = Rcpp::as<Rcpp::Language>(lhs_raw);
    Rcpp::CharacterVector op_name =
        Rcpp::as<Rcpp::CharacterVector>(Rcpp::as<Rcpp::Symbol>(lhs_lang[0]));

    // Match R operators/functions to your fims::Transformation::Label enums
    if (op_name[0] == "log") {
      trans_config.label = fims::Transformation::Label::log;
      return trans_config;
    } else if (op_name[0] == "exp") {
      trans_config.label = fims::Transformation::Label::exp;
      return trans_config;
    } else if (op_name[0] == "logit") {
      trans_config.label = fims::Transformation::Label::logit;
      // Extract bounds if provided, otherwise default to 0 and 1
      if (lhs_lang.size() >= 3) {
        trans_config.args.lower = Rcpp::as<double>(lhs_lang[2]);
      }
      if (lhs_lang.size() >= 4) {
        trans_config.args.upper = Rcpp::as<double>(lhs_lang[3]);
      }
      // Validate bounds
      if (trans_config.args.lower >= trans_config.args.upper) {
        Rcpp::stop("Logit transformation requires lower < upper.");
      }
      return trans_config;
    } else if (op_name[0] == "sqrt") {
      trans_config.label = fims::Transformation::Label::sqrt;
      return trans_config;
    } else if (op_name[0] == "^") {
      int power = Rcpp::as<int>(lhs_lang[2]);
      if (power == 2) {
        trans_config.label = fims::Transformation::Label::square;
        return trans_config;
      }
    }
    // Add reciprocal parsing here if you still want to route 1/x variants
  }

  // Fallback/Error state if formula layout isn't supported
  Rcpp::stop("Unsupported Left-Hand Side (LHS) transformation operator.");
}

/**
 * @brief A structure to hold the parsed components of a distribution formula.
 */
struct FormulaComponents {
  fims::Transformation transformation;
  fims::Distribution::Label distribution;
  std::vector<double> hyperparameters;
};

//' Parse a Distributional Formula
//'
//' Parses an R formula specifying a target variable and its prior/likelihood
// distribution ' along with its parameters (e.g., \code{y ~ dnorm(0, 1)}).
//'
//' @param f A standard R \code{Formula} object. It must follow the structure
//'   \code{variable ~ distribution(param1, param2, ...)}.
//'
//' @details
//' The function unpacks the R formula by treating it as an abstract syntax tree
//(AST) ' via the \code{Rcpp::Language} class: ' \itemize{ '   \item
//\strong{Operator (\code{[0]}):} The tilde (\code{~}) operator. '   \item
//\strong{LHS (\code{[1]}):} Extracted as a \code{Symbol} and converted to a
// character vector representing the target variable name. '   \item \strong{RHS
//(\code{[2]}):} Treated as a nested \code{Language} call where the head
//(\code{[0]}) is the distribution name, and subsequent elements are the numeric
// parameters. ' }
//'
//' @return A named \code{Rcpp::List} containing three elements:
//' \itemize{
//'   \item \code{variable}: A character vector holding the name of the LHS
// variable. '   \item \code{distribution}: A character vector holding the name
// of the RHS distribution function. '   \item \code{hyperparameters}: A numeric
// vector containing the extracted hyperparameter values. ' }
FormulaComponents parse_distribution_formula(Rcpp::Formula f) {
  // Convert the Formula to a standard standard R language object (Call)
  Rcpp::Language formula = Rcpp::as<Rcpp::Language>(f);

  // Parse the formula
  // R formulas are structured as a tree where element 0 is the operator `~`
  // element 1 is the Left-Hand Side (LHS), and element 2 is the Right-Hand Side
  // (RHS)

  SEXP lhs_raw = formula[1];

  // Extract the transformation configuration and variable string
  fims::Transformation transform = classify_and_extract_transformation(lhs_raw);

  // 2. Parse Right-Hand Side (RHS)
  if (!Rcpp::is<Rcpp::Language>(formula[2])) {
    Rcpp::stop("Right-Hand Side (RHS) must be a distribution function call.");
  }
  Rcpp::Language rhs = Rcpp::as<Rcpp::Language>(formula[2]);

  // The first element of a function call language object is the function name
  // itself Convert formula->Symbol->CharacterVector->enum
  Rcpp::Symbol distribution_symbol = Rcpp::as<Rcpp::Symbol>(rhs[0]);
  Rcpp::CharacterVector distribution_string =
      Rcpp::as<Rcpp::CharacterVector>(distribution_symbol);

  // 3. Extract the numeric parameters from the function arguments
  int num_args =
      rhs.size() - 1;  // Subtract 1 because element 0 is the function name
  std::vector<double> hyperparameters(num_args);

  for (int i = 0; i < num_args; ++i) {
    SEXP arg = rhs[i + 1];
    if (Rcpp::is<Rcpp::Language>(arg) || Rcpp::is<Rcpp::Symbol>(arg)) {
      // Evaluate the expression to get a numeric value
      Rcpp::Environment base_env = Rcpp::Environment::base_env();
      SEXP result = Rcpp::Rcpp_eval(arg, base_env);
      hyperparameters[i] = Rcpp::as<double>(result);
    } else {
      hyperparameters[i] = Rcpp::as<double>(arg);
    }
  }

  return FormulaComponents{transform,
                           fims::StringToDistributionLabel(
                               Rcpp::as<std::string>(distribution_string)),
                           hyperparameters};
}

void setup_prior(Rcpp::Formula f, Rcpp::List parameter_vectors) {
  Rcpp::IntegerVector ids(parameter_vectors.size());

  // Parse formula into components
  FormulaComponents formula_parts = parse_distribution_formula(f);
  fims::Distribution::Label distribution_name = formula_parts.distribution;
  std::vector<double> hyperparameters = formula_parts.hyperparameters;
  fims::Transformation prior_transformation = formula_parts.transformation;

  // Pass prior transformation to ParameterVector
  // Extract the raw pointer from the XPtr
  for (int i = 0; i < parameter_vectors.size(); i++) {
    ParameterVector pv = Rcpp::as<ParameterVector>(parameter_vectors[i]);

    // Modifies shared Transformation object
    *pv.prior_transformation_m = prior_transformation;
    ids[i] = pv.id_m;
  }

  switch (distribution_name) {
    case fims::Distribution::Label::Normal: {
      auto prior = std::make_shared<DnormDistributionsInterface>();
      prior->expected_values[0].initial_value_m = hyperparameters[0];
      prior->log_sd[0].initial_value_m = fims_math::log(hyperparameters[1]);
      prior->set_distribution_links("prior", ids);
      break;
    }

    case fims::Distribution::Label::Lognormal: {
      auto prior = std::make_shared<DlnormDistributionsInterface>();
      prior->expected_values[0].initial_value_m = hyperparameters[0];
      prior->log_sd[0].initial_value_m = fims_math::log(hyperparameters[1]);
      prior->set_distribution_links("prior", ids);
      break;
    }

    default:
      throw std::invalid_argument(
          "Unsupported distribution type in add_prior.");
  }
}

// Define setup_prior_function in ParameterVector class
inline void register_prior_functions() {
  ParameterVector::setup_prior_function = setup_prior;
}

void add_shared_prior(Rcpp::Formula f, Rcpp::List parameter_vectors) {
  setup_prior(f, parameter_vectors);
}

/**
 * @brief Clears the internal objects.
 *
 * @tparam Type
 */
template <typename Type>
void clear_internal() {
  std::shared_ptr<fims_info::Information<Type>> d0 =
      fims_info::Information<Type>::GetInstance();
  d0->Clear();
}

/**
 * @brief Clears the vector of independent variables.
 */
void clear() {
  // rcpp_interface_base.hpp
  FIMSRcppInterfaceBase::fims_interface_objects.clear();

  // Parameter and ParameterVector
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

  FisheryModelInterfaceBase::id_g = 1;
  FisheryModelInterfaceBase::live_objects.clear();

  clear_internal<TMB_FIMS_REAL_TYPE>();
  clear_internal<TMBAD_FIMS_TYPE>();

  fims::FIMSLog::fims_log->clear();
}

/**
 * @brief Gets the log entries as a string in JSON format.
 */
std::string get_log() { return fims::FIMSLog::fims_log->get_log(); }

/**
 * @brief Gets the error entries from the log as a string in JSON format.
 */
std::string get_log_errors() { return fims::FIMSLog::fims_log->get_errors(); }

/**
 * @brief Gets the warning entries from the log as a string in JSON format.
 */
std::string get_log_warnings() {
  return fims::FIMSLog::fims_log->get_warnings();
}

/**
 * @brief Gets the info entries from the log as a string in JSON format.
 */
std::string get_log_info() { return fims::FIMSLog::fims_log->get_info(); }

/**
 * @brief If true, writes the log on exit.
 */
void write_log(bool write) { fims::FIMSLog::fims_log->write_on_exit = write; }

/**
 * @brief Sets the path for the log file to be written to.
 */
void set_log_path(const std::string &path) {
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
  fims::FIMSLog::fims_log->info_message(log_entry, -1, "R_env",
                                        "R_script_entry");
}

/**
 * @brief Adds a warning entry to the log from the R environment.
 */
void log_warning(std::string log_entry) {
  fims::FIMSLog::fims_log->warning_message(log_entry, -1, "R_env",
                                           "R_script_entry");
}

/**
 * @brief Escapes quotations.
 *
 * @param input A string.
 * @return std::string
 */
std::string escapeQuotes(const std::string &input) {
  std::string result = input;
  std::string search = "\"";
  std::string replace = "\\\"";

  // Find each occurrence of `"` and replace it with `\"`
  size_t pos = result.find(search);
  while (pos != std::string::npos) {
    result.replace(pos, search.size(), replace);
    pos = result.find(search,
                      pos + replace.size());  // Move past the replaced position
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

  PROTECT(expression = R_ParseVector(Rf_mkString(ss.str().c_str()), 1, &status,
                                     R_NilValue));
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

  std::string ret =
      ss_ret.str();  //"find error";//Rcpp::as<std::string>(result);

  fims::FIMSLog::fims_log->error_message(log_entry, -1, "R_env", ret.c_str());
}
#endif  // FIMS_INTERFACE_RCPP_INTERFACE_HPP
