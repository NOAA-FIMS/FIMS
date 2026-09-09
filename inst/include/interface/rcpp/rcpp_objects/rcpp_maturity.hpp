/**
 * @file rcpp_maturity.hpp
 * @brief The Rcpp interface to declare different maturity options, e.g.,
 * logistic. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP

#include "../../../population_dynamics/maturity/maturity.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief The maturity forms FIMS can build.
 *
 * @details The create_maturity_() function takes one of these names from R and
 * builds the matching class: "Logistic" builds a LogisticMaturityInterface.
 * MaturityInterfaceBase is never built on its own; it only holds what all
 * maturity forms have in common.
 *
 * These are an enum rather than plain strings so that every place in the C++
 * code that acts on a maturity form has to name one of these values, which
 * makes it harder to add a form and forget to handle it somewhere.
 */
enum class MaturityType : uint8_t {
  logistic = 0
};

/**
 * @brief Convert a type name supplied from R to a MaturityType.
 */
inline MaturityType MaturityTypeFromString(const std::string &name) {
  if (name == "Logistic") return MaturityType::logistic;
  throw std::invalid_argument(
      "Invalid type: '" + name +
      "'. Valid options are: Logistic.");
}

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp maturity
 * interfaces. This type should be inherited and not called from R directly.
 */
class MaturityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the MaturityInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the MaturityInterfaceBase object.
   */
  uint32_t id;

  /**
   * @brief The constructor.
   */
  MaturityInterfaceBase() { this->id = MaturityInterfaceBase::id_g++; }

  /**
   * @brief Interface objects are not copyable.
   */
  MaturityInterfaceBase(const MaturityInterfaceBase &) = delete;
  MaturityInterfaceBase &operator=(const MaturityInterfaceBase &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~MaturityInterfaceBase() {}

    
  /**
   * @brief A method for each child maturity interface object to inherit so
   * each maturity option can have an evaluate() function.
   */
  virtual double evaluate(double x) = 0;
};

/**
 * @brief Rcpp interface for logistic maturity to instantiate the object from R:
 * logistic_maturity <- methods::new(logistic_maturity).
 */
class LogisticMaturityInterface : public MaturityInterfaceBase {
 public:
  /**
   * @brief The index value at which the response reaches 0.5.
   */
  VariableVector inflection_point;
  /**
   * @brief The width of the curve at the inflection point.
   */
  VariableVector slope;

  /**
   * @brief The constructor.
   */
  LogisticMaturityInterface() : MaturityInterfaceBase() {}

  /**
   * @brief Interface objects are not copyable.
   */
  LogisticMaturityInterface(const LogisticMaturityInterface &) = delete;
  LogisticMaturityInterface &operator=(const LogisticMaturityInterface &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~LogisticMaturityInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @copydoc FIMSRcppInterfaceBase::get_variable_vector
   */
  virtual VariableVector *get_variable_vector(const std::string &name) {
    if (name == "inflection_point") return &this->inflection_point;
    if (name == "slope") return &this->slope;
    return nullptr;
  }

  /**
   * @brief Evaluate maturity using the logistic function.
   * @param x The independent variable in the logistic function (e.g., age or
   * size in maturity).
   */
  virtual double evaluate(double x) {
    fims_popdy::LogisticMaturity<double> LogisticMat;
    LogisticMat.inflection_point.resize(1);
    LogisticMat.inflection_point[0] = this->inflection_point[0].initial_value_m;
    LogisticMat.slope.resize(1);
    LogisticMat.slope[0] = this->slope[0].initial_value_m;
    return LogisticMat.evaluate(x);
  }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Logistic Maturity  " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::maturity_models_iterator it;

    // search for maturity in Information
    it = info->maturity_models.find(this->id);
    // if not found, just return
    if (it == info->maturity_models.end()) {
      FIMS_WARNING_LOG("Logistic Maturity " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::LogisticMaturity<double>> mat =
          std::dynamic_pointer_cast<fims_popdy::LogisticMaturity<double>>(
              it->second);

      for (size_t i = 0; i < inflection_point.size(); i++) {
        set_final_value_by_estimation_status(this->inflection_point[i],
                                             mat->inflection_point[i]);
      }

      for (size_t i = 0; i < slope.size(); i++) {
        set_final_value_by_estimation_status(this->slope[i], mat->slope[i]);
      }
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * maturity interface with logistic maturity. It also returns the ID and the
   * parameters. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"Maturity\",\n";
    ss << " \"module_type\": \"Logistic\",\n";
    ss << " \"module_id\": " << this->id << ",\n";

    ss << " \"parameters\": [\n{\n";
    ss << "   \"name\": \"inflection_point\",\n";
    ss << "   \"id\":" << this->inflection_point.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->inflection_point << "},\n ";

    ss << "{\n";
    ss << "   \"name\": \"slope\",\n";
    ss << "   \"id\":" << this->slope.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->slope << "}]\n";

    ss << "}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::LogisticMaturity<Type>> maturity =
        std::make_shared<fims_popdy::LogisticMaturity<Type>>();

    // set relative info
    maturity->id = this->id;
    std::stringstream ss;
    maturity->inflection_point.resize(this->inflection_point.size());
    for (size_t i = 0; i < this->inflection_point.size(); i++) {
      maturity->inflection_point[i] = this->inflection_point[i].initial_value_m;
      ss.str("");
      ss << "Maturity." << this->id << ".inflection_point."
         << this->inflection_point[i].id_m;
      register_parameter_if_estimable(
          maturity->inflection_point[i],
          this->inflection_point[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->inflection_point.id_m] =
        &(maturity)->inflection_point;

    maturity->slope.resize(this->slope.size());
    for (size_t i = 0; i < this->slope.size(); i++) {
      maturity->slope[i] = this->slope[i].initial_value_m;
      ss.str("");
      ss << "Maturity." << this->id << ".slope." << this->slope[i].id_m;
      register_parameter_if_estimable(
          maturity->slope[i], this->slope[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->slope.id_m] = &(maturity)->slope;

    // add to Information
    info->maturity_models[maturity->id] = maturity;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

#endif
