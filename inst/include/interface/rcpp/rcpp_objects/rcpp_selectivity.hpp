/**
 * @file rcpp_selectivity.hpp
 * @brief The Rcpp interface to declare different types of selectivity, e.g.,
 * logistic and double logistic. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_SELECTIVITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_SELECTIVITY_HPP

#include "../../../population_dynamics/selectivity/selectivity.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief The selectivity forms FIMS can build.
 *
 * @details The create_selectivity_() function takes one of these names from R
 * and builds the matching class: "logistic" builds a
 * LogisticSelectivityInterface, "double_logistic" builds a
 * DoubleLogisticSelectivityInterface. SelectivityInterfaceBase is never built
 * on its own; it only holds what all selectivity forms have in common.
 *
 * These are an enum rather than plain strings so that every place in the C++
 * code that acts on a selectivity form has to name one of these values, which
 * makes it harder to add a form and forget to handle it somewhere.
 */
enum class SelectivityType : uint8_t {
  logistic = 0,
  double_logistic = 1
};

/**
 * @brief Convert a type name supplied from R to a SelectivityType.
 */
inline SelectivityType SelectivityTypeFromString(const std::string &name) {
  if (name == "logistic") return SelectivityType::logistic;
  if (name == "double_logistic") return SelectivityType::double_logistic;
  throw std::invalid_argument(
      "Invalid type: '" + name +
      "'. Valid options are: logistic, double_logistic.");
}

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp selectivity
 * interfaces. This type should be inherited and not called from R directly.
 */
class SelectivityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the SelectivityInterfaceBase.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the SelectivityInterfaceBase object.
   */
  uint32_t id;

  /**
   * @brief The constructor.
   */
  SelectivityInterfaceBase() { this->id = SelectivityInterfaceBase::id_g++; }

  /**
   * @brief Interface objects are not copyable.
   */
  SelectivityInterfaceBase(const SelectivityInterfaceBase &) = delete;
  SelectivityInterfaceBase &operator=(const SelectivityInterfaceBase &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~SelectivityInterfaceBase() {}

    
  /**
   * @brief A method for each child selectivity interface object to inherit so
   * each selectivity option can have an evaluate() function.
   */
  virtual double evaluate(double x) = 0;
};

/**
 * @brief Rcpp interface for logistic selectivity to instantiate the object
 * from R:
 * logistic_selectivity <- methods::new(logistic_selectivity).
 */
class LogisticSelectivityInterface : public SelectivityInterfaceBase {
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
  LogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  /**
   * @brief Interface objects are not copyable.
   */
  LogisticSelectivityInterface(const LogisticSelectivityInterface &) = delete;
  LogisticSelectivityInterface &operator=(const LogisticSelectivityInterface &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~LogisticSelectivityInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @copydoc FIMSRcppInterfaceBase::get_parameter
   */
  virtual VariableVector *get_parameter(const std::string &name) {
    if (name == "inflection_point") return &this->inflection_point;
    if (name == "slope") return &this->slope;
    return nullptr;
  }

  /**
   * @brief Evaluate selectivity using the logistic function.
   * @param x The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims_popdy::LogisticSelectivity<double> LogisticSel;
    LogisticSel.inflection_point.resize(1);
    LogisticSel.inflection_point[0] = this->inflection_point[0].initial_value_m;
    LogisticSel.slope.resize(1);
    LogisticSel.slope[0] = this->slope[0].initial_value_m;
    return LogisticSel.evaluate(x);
  }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Logistic Selectivity  " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::selectivity_models_iterator it;

    // search for maturity in Information
    it = info->selectivity_models.find(this->id);
    // if not found, just return
    if (it == info->selectivity_models.end()) {
      FIMS_WARNING_LOG("Logistic Selectivity " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::LogisticSelectivity<double>> sel =
          std::dynamic_pointer_cast<fims_popdy::LogisticSelectivity<double>>(
              it->second);

      for (size_t i = 0; i < inflection_point.size(); i++) {
        set_final_value_by_estimation_status(this->inflection_point[i],
                                             sel->inflection_point[i]);
      }

      for (size_t i = 0; i < slope.size(); i++) {
        set_final_value_by_estimation_status(this->slope[i], sel->slope[i]);
      }
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * selectivity interface with logistic selectivity. It also returns the ID
   * and the parameters. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"module_name\":\"Selectivity\",\n";
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

    std::shared_ptr<fims_popdy::LogisticSelectivity<Type>> selectivity =
        std::make_shared<fims_popdy::LogisticSelectivity<Type>>();
    std::stringstream ss;
    // set relative info
    selectivity->id = this->id;
    selectivity->inflection_point.resize(this->inflection_point.size());
    for (size_t i = 0; i < this->inflection_point.size(); i++) {
      selectivity->inflection_point[i] =
          this->inflection_point[i].initial_value_m;
      ss.str("");
      ss << "Selectivity." << this->id << ".inflection_point."
         << this->inflection_point[i].id_m;
      register_parameter_if_estimable(
          selectivity->inflection_point[i],
          this->inflection_point[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->inflection_point.id_m] =
        &(selectivity)->inflection_point;

    selectivity->slope.resize(this->slope.size());
    for (size_t i = 0; i < this->slope.size(); i++) {
      selectivity->slope[i] = this->slope[i].initial_value_m;
      ss.str("");
      ss << "Selectivity." << this->id << ".slope." << this->slope[i].id_m;
      register_parameter_if_estimable(
          selectivity->slope[i], this->slope[i].estimation_status_m, ss.str());
    }
    info->variable_map[this->slope.id_m] = &(selectivity)->slope;

    // add to Information
    info->selectivity_models[selectivity->id] = selectivity;

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

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <-
 * methods::new(logistic_selectivity)
 */
class DoubleLogisticSelectivityInterface : public SelectivityInterfaceBase {
 public:
  VariableVector inflection_point_asc; /**< the index value at which the
                                     response reaches .5 */
  VariableVector
      slope_asc; /**< the width of the curve at the inflection_point */
  VariableVector inflection_point_desc; /**< the index value at which the
                                      response reaches .5 */
  VariableVector
      slope_desc; /**< the width of the curve at the inflection_point */

  DoubleLogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  /**
   * @brief Interface objects are not copyable.
   */
  DoubleLogisticSelectivityInterface(const DoubleLogisticSelectivityInterface &) = delete;
  DoubleLogisticSelectivityInterface &operator=(const DoubleLogisticSelectivityInterface &) = delete;

  virtual ~DoubleLogisticSelectivityInterface() {}

  /** @brief returns the id for the double logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @copydoc FIMSRcppInterfaceBase::get_parameter
   */
  virtual VariableVector *get_parameter(const std::string &name) {
    if (name == "inflection_point_asc") return &this->inflection_point_asc;
    if (name == "slope_asc") return &this->slope_asc;
    if (name == "inflection_point_desc") return &this->inflection_point_desc;
    if (name == "slope_desc") return &this->slope_desc;
    return nullptr;
  }

  /** @brief evaluate the double logistic selectivity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims_popdy::DoubleLogisticSelectivity<double> DoubleLogisticSel;
    DoubleLogisticSel.inflection_point_asc.resize(1);
    DoubleLogisticSel.inflection_point_asc[0] =
        this->inflection_point_asc[0].initial_value_m;
    DoubleLogisticSel.slope_asc.resize(1);
    DoubleLogisticSel.slope_asc[0] = this->slope_asc[0].initial_value_m;
    DoubleLogisticSel.inflection_point_desc.resize(1);
    DoubleLogisticSel.inflection_point_desc[0] =
        this->inflection_point_desc[0].initial_value_m;
    DoubleLogisticSel.slope_desc.resize(1);
    DoubleLogisticSel.slope_desc[0] = this->slope_desc[0].initial_value_m;
    return DoubleLogisticSel.evaluate(x);
  }
  /**
   * @brief finalize function. Extracts derived quantities back to
   * the Rcpp interface object from the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Double Logistic Selectivity  " +
                       fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::selectivity_models_iterator it;

    // search for maturity in Information
    it = info->selectivity_models.find(this->id);
    // if not found, just return
    if (it == info->selectivity_models.end()) {
      FIMS_WARNING_LOG("Double Logistic Selectivity " +
                       fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<double>> sel =
          std::dynamic_pointer_cast<
              fims_popdy::DoubleLogisticSelectivity<double>>(it->second);

      for (size_t i = 0; i < inflection_point_asc.size(); i++) {
        set_final_value_by_estimation_status(this->inflection_point_asc[i],
                                             sel->inflection_point_asc[i]);
      }

      for (size_t i = 0; i < slope_asc.size(); i++) {
        set_final_value_by_estimation_status(this->slope_asc[i],
                                             sel->slope_asc[i]);
      }

      for (size_t i = 0; i < inflection_point_desc.size(); i++) {
        set_final_value_by_estimation_status(this->inflection_point_desc[i],
                                             sel->inflection_point_desc[i]);
      }

      for (size_t i = 0; i < slope_desc.size(); i++) {
        set_final_value_by_estimation_status(this->slope_desc[i],
                                             sel->slope_desc[i]);
      }
    }
  }

  /**
   * @brief Convert the data to json representation for the output.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"module_name\": \"Selectivity\",\n";
    ss << " \"module_type\": \"DoubleLogistic\",\n";
    ss << " \"module_id\": " << this->id << ",\n";

    ss << " \"parameters\":[\n{\n";
    ss << "   \"name\": \"inflection_point_asc\",\n";
    ss << "   \"id\":" << this->inflection_point_asc.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->inflection_point_asc << "},\n";

    ss << "{\n";
    ss << "   \"name\": \"slope_asc\",\n";
    ss << "   \"id\":" << this->slope_asc.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->slope_asc << "},\n";

    ss << " {\n";
    ss << "   \"name\": \"inflection_point_desc\",\n";
    ss << "   \"id\":" << this->inflection_point_desc.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->inflection_point_desc << "},\n";

    ss << "{\n";
    ss << "   \"name\": \"slope_desc\",\n";
    ss << "   \"id\":" << this->slope_desc.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->slope_desc << "}]\n";

    ss << "}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>> selectivity =
        std::make_shared<fims_popdy::DoubleLogisticSelectivity<Type>>();

    std::stringstream ss;
    // set relative info
    selectivity->id = this->id;
    selectivity->inflection_point_asc.resize(this->inflection_point_asc.size());
    for (size_t i = 0; i < this->inflection_point_asc.size(); i++) {
      selectivity->inflection_point_asc[i] =
          this->inflection_point_asc[i].initial_value_m;
      ss.str("");
      ss << "Selectivity." << this->id << ".inflection_point_asc."
         << this->inflection_point_asc[i].id_m;
      register_parameter_if_estimable(
          selectivity->inflection_point_asc[i],
          this->inflection_point_asc[i].estimation_status_m,
          ss.str());
    }
    info->variable_map[this->inflection_point_asc.id_m] =
        &(selectivity)->inflection_point_asc;

    selectivity->slope_asc.resize(this->slope_asc.size());
    for (size_t i = 0; i < this->slope_asc.size(); i++) {
      selectivity->slope_asc[i] = this->slope_asc[i].initial_value_m;
      ss.str("");
      ss << "Selectivity." << this->id << ".slope_asc."
         << this->slope_asc[i].id_m;
      register_parameter_if_estimable(
          selectivity->slope_asc[i], this->slope_asc[i].estimation_status_m,
          ss.str());
    }
    info->variable_map[this->slope_asc.id_m] = &(selectivity)->slope_asc;

    selectivity->inflection_point_desc.resize(
        this->inflection_point_desc.size());
    for (size_t i = 0; i < this->inflection_point_desc.size(); i++) {
      selectivity->inflection_point_desc[i] =
          this->inflection_point_desc[i].initial_value_m;
      ss.str("");
      ss << "Selectivity." << this->id << ".inflection_point_desc."
         << this->inflection_point_desc[i].id_m;
      register_parameter_if_estimable(
          selectivity->inflection_point_desc[i],
          this->inflection_point_desc[i].estimation_status_m,
          ss.str());
    }
    info->variable_map[this->inflection_point_desc.id_m] =
        &(selectivity)->inflection_point_desc;

    selectivity->slope_desc.resize(this->slope_desc.size());
    for (size_t i = 0; i < this->slope_desc.size(); i++) {
      selectivity->slope_desc[i] = this->slope_desc[i].initial_value_m;
      ss.str("");
      ss << "Selectivity." << this->id << ".slope_desc."
         << this->slope_desc[i].id_m;
      register_parameter_if_estimable(
          selectivity->slope_desc[i], this->slope_desc[i].estimation_status_m,
          ss.str());
    }
    info->variable_map[this->slope_desc.id_m] = &(selectivity)->slope_desc;

    // add to Information
    info->selectivity_models[selectivity->id] = selectivity;

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
