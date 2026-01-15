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
   * @brief The map associating the IDs of SelectivityInterfaceBase to the
   * objects. This is a live object, which is an object that has been created
   * and lives in memory.
   */
  static std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>
      live_objects;

  /**
   * @brief The constructor.
   */
  SelectivityInterfaceBase() {
    this->id = SelectivityInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    SelectivityInterfaceBase */
    // SelectivityInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Selectivity Interface Base object
   *
   * @param other
   */
  SelectivityInterfaceBase(const SelectivityInterfaceBase &other)
      : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~SelectivityInterfaceBase() {}

  /**
   * @brief Get the ID for the child selectivity interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief A method for each child selectivity interface object to inherit so
   * each selectivity option can have an evaluate() function.
   */
  virtual double evaluate(double x) = 0;
};
// static id of the SelectivityInterfaceBase object
uint32_t SelectivityInterfaceBase::id_g = 1;
// local id of the SelectivityInterfaceBase object map relating the ID of the
// SelectivityInterfaceBase to the SelectivityInterfaceBase objects
std::map<uint32_t, std::shared_ptr<SelectivityInterfaceBase>>
    SelectivityInterfaceBase::live_objects;



/**
 * @brief Rcpp interface for descending logistic selectivity to instantiate the object
 * from R:
 * descending_logistic_selectivity <- methods::new(descending_logistic_selectivity).
 */
class DescendingLogisticSelectivityInterface : public SelectivityInterfaceBase {
public:
  /**
   * @brief The index value at which the response reaches 0.5.
   */
  ParameterVector inflection_point_desc;
  /**
   * @brief The width of the curve at the inflection point.
   */
  ParameterVector slope_desc;

  /**
   * @brief The constructor.
   */
  DescendingLogisticSelectivityInterface() : SelectivityInterfaceBase() {
    SelectivityInterfaceBase::live_objects[this->id] =
      std::make_shared<DescendingLogisticSelectivityInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
      SelectivityInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Descending Logistic Selectivity Interface object
   *
   * @param other
   */
  DescendingLogisticSelectivityInterface(const DescendingLogisticSelectivityInterface &other)
    : SelectivityInterfaceBase(other),
      inflection_point_desc(other.inflection_point_desc),
      slope_desc(other.slope_desc) {}

  /**
   * @brief The destructor.
   */
  virtual ~DescendingLogisticSelectivityInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Evaluate selectivity using the descending logistic function.
   * @param x The independent variable in the descending logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims_popdy::DescendingLogisticSelectivity<double> DescendingLogisticSel;
    DescendingLogisticSel.inflection_point_desc.resize(1);
    DescendingLogisticSel.inflection_point_desc[0] = this->inflection_point_desc[0].initial_value_m;
    DescendingLogisticSel.slope_desc.resize(1);
    DescendingLogisticSel.slope_desc[0] = this->slope_desc[0].initial_value_m;
    return DescendingLogisticSel.evaluate(x);
  }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Descending Logistic Selectivity  " + fims::to_string(this->id) +
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
      FIMS_WARNING_LOG("Descending Logistic Selectivity " + fims::to_string(this->id) +
        " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::DescendingLogisticSelectivity<double>> sel =
        std::dynamic_pointer_cast<fims_popdy::DescendingLogisticSelectivity<double>>(
          it->second);

      for (size_t i = 0; i < inflection_point_desc.size(); i++) {
        if (this->inflection_point_desc[i].estimation_type_m.get() == "constant") {
          this->inflection_point_desc[i].final_value_m =
            this->inflection_point_desc[i].initial_value_m;
        } else {
          this->inflection_point_desc[i].final_value_m = sel->inflection_point_desc[i];
        }
      }

      for (size_t i = 0; i < slope_desc.size(); i++) {
        if (this->slope_desc[i].estimation_type_m.get() == "constant") {
          this->slope_desc[i].final_value_m = this->slope_desc[i].initial_value_m;
        } else {
          this->slope_desc[i].final_value_m = sel->slope_desc[i];
        }
      }
    }
  }
  /**
   * @brief Set uncertainty values for selectivity parameters.
   *
   * @details Sets the standard error values for the inflection point and slope_desc
   * parameters using the provided map.
   * @param se_values A map from parameter names to vectors of standard error
   * values.
   */
  virtual void set_uncertainty(
      std::map<std::string, std::vector<double>> &se_values) {
    fims::Vector<double> inflection_point_desc_uncertainty(
        this->inflection_point_desc.size(), -999);
    this->get_se_values("inflection_point_desc", se_values,
                        inflection_point_desc_uncertainty);
    fims::Vector<double> slope_uncertainty(this->slope_desc.size(), -999);
    this->get_se_values("slope_desc", se_values, slope_uncertainty);
    for (size_t i = 0; i < this->inflection_point_desc.size(); i++) {
      this->inflection_point_desc[i].uncertainty_m = inflection_point_desc_uncertainty[i];
    }
    for (size_t i = 0; i < this->slope_desc.size(); i++) {
      this->slope_desc[i].uncertainty_m = slope_uncertainty[i];
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * selectivity interface with descending logistic selectivity. It also returns the ID
   * and the parameters. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"module_name\":\"Selectivity\",\n";
    ss << " \"module_type\": \"DescendingLogistic\",\n";
    ss << " \"module_id\": " << this->id << ",\n";

    ss << " \"parameters\": [\n{\n";
    ss << "   \"name\": \"inflection_point_desc\",\n";
    ss << "   \"id\":" << this->inflection_point_desc.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [null],\n";
    ss << "  \"dimensions\": [1]\n},\n";
    ss << "   \"values\":" << this->inflection_point_desc << "},\n ";

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

    std::shared_ptr<fims_popdy::DescendingLogisticSelectivity<Type>> selectivity =
      std::make_shared<fims_popdy::DescendingLogisticSelectivity<Type>>();
    std::stringstream ss;
    // set relative info
    selectivity->id = this->id;
    selectivity->inflection_point_desc.resize(this->inflection_point_desc.size());
    for (size_t i = 0; i < this->inflection_point_desc.size(); i++) {
      selectivity->inflection_point_desc[i] =
        this->inflection_point_desc[i].initial_value_m;
      if (this->inflection_point_desc[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point_desc."
           << this->inflection_point_desc[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->inflection_point_desc[i]);
      }
      if (this->inflection_point_desc[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point_desc."
           << this->inflection_point_desc[i].id_m;
        info->RegisterRandomEffect(selectivity->inflection_point_desc[i]);
        info->RegisterRandomEffectName(ss.str());
      }
    }
    info->variable_map[this->inflection_point_desc.id_m] =
      &(selectivity)->inflection_point_desc;

      selectivity->slope_desc.resize(this->slope_desc.size());
      for (size_t i = 0; i < this->slope_desc.size(); i++) {
        selectivity->slope_desc[i] = this->slope_desc[i].initial_value_m;
        if (this->slope_desc[i].estimation_type_m.get() == "fixed_effects") {
          ss.str("");
          ss << "Selectivity." << this->id << ".slope_desc." << this->slope_desc[i].id_m;
          info->RegisterParameterName(ss.str());
          info->RegisterParameter(selectivity->slope_desc[i]);
        }
        if (this->slope_desc[i].estimation_type_m.get() == "random_effects") {
          ss.str("");
          ss << "Selectivity." << this->id << ".slope_desc." << this->slope_desc[i].id_m;
          info->RegisterRandomEffectName(ss.str());
          info->RegisterRandomEffect(selectivity->slope_desc[i]);
        }
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
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
#endif

    return true;
  }

#endif
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
  ParameterVector inflection_point;
  /**
   * @brief The width of the curve at the inflection point.
   */
  ParameterVector slope;

  /**
   * @brief The constructor.
   */
  LogisticSelectivityInterface() : SelectivityInterfaceBase() {
    SelectivityInterfaceBase::live_objects[this->id] =
        std::make_shared<LogisticSelectivityInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        SelectivityInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Logistic Selectivity Interface object
   *
   * @param other
   */
  LogisticSelectivityInterface(const LogisticSelectivityInterface &other)
      : SelectivityInterfaceBase(other),
        inflection_point(other.inflection_point),
        slope(other.slope) {}

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
        if (this->inflection_point[i].estimation_type_m.get() == "constant") {
          this->inflection_point[i].final_value_m =
              this->inflection_point[i].initial_value_m;
        } else {
          this->inflection_point[i].final_value_m = sel->inflection_point[i];
        }
      }

      for (size_t i = 0; i < slope.size(); i++) {
        if (this->slope[i].estimation_type_m.get() == "constant") {
          this->slope[i].final_value_m = this->slope[i].initial_value_m;
        } else {
          this->slope[i].final_value_m = sel->slope[i];
        }
      }
    }
  }
  /**
   * @brief Set uncertainty values for selectivity parameters.
   *
   * @details Sets the standard error values for the inflection point and slope
   * parameters using the provided map.
   * @param se_values A map from parameter names to vectors of standard error
   * values.
   */
  virtual void set_uncertainty(
      std::map<std::string, std::vector<double>> &se_values) {
    fims::Vector<double> inflection_point_uncertainty(
        this->inflection_point.size(), -999);
    this->get_se_values("inflection_point", se_values,
                        inflection_point_uncertainty);
    fims::Vector<double> slope_uncertainty(this->slope.size(), -999);
    this->get_se_values("slope", se_values, slope_uncertainty);
    for (size_t i = 0; i < this->inflection_point.size(); i++) {
      this->inflection_point[i].uncertainty_m = inflection_point_uncertainty[i];
    }
    for (size_t i = 0; i < this->slope.size(); i++) {
      this->slope[i].uncertainty_m = slope_uncertainty[i];
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
      if (this->inflection_point[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point."
           << this->inflection_point[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->inflection_point[i]);
      }
      if (this->inflection_point[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point."
           << this->inflection_point[i].id_m;
        info->RegisterRandomEffect(selectivity->inflection_point[i]);
        info->RegisterRandomEffectName(ss.str());
      }
    }
    info->variable_map[this->inflection_point.id_m] =
        &(selectivity)->inflection_point;

    selectivity->slope.resize(this->slope.size());
    for (size_t i = 0; i < this->slope.size(); i++) {
      selectivity->slope[i] = this->slope[i].initial_value_m;
      if (this->slope[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".slope." << this->slope[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->slope[i]);
      }
      if (this->slope[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".slope." << this->slope[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(selectivity->slope[i]);
      }
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
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
#endif

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
  ParameterVector inflection_point_asc; /**< the index value at which the
                                     response reaches .5 */
  ParameterVector
      slope_asc; /**< the width of the curve at the inflection_point */
  ParameterVector inflection_point_desc; /**< the index value at which the
                                      response reaches .5 */
  ParameterVector
      slope_desc; /**< the width of the curve at the inflection_point */

  DoubleLogisticSelectivityInterface() : SelectivityInterfaceBase() {
    SelectivityInterfaceBase::live_objects[this->id] =
        std::make_shared<DoubleLogisticSelectivityInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        SelectivityInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Double Logistic Selectivity Interface object
   *
   * @param other
   */
  DoubleLogisticSelectivityInterface(
      const DoubleLogisticSelectivityInterface &other)
      : SelectivityInterfaceBase(other),
        inflection_point_asc(other.inflection_point_asc),
        slope_asc(other.slope_asc),
        inflection_point_desc(other.inflection_point_desc),
        slope_desc(other.slope_desc) {}

  virtual ~DoubleLogisticSelectivityInterface() {}

  /** @brief returns the id for the double logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

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
        if (this->inflection_point_asc[i].estimation_type_m.get() ==
            "constant") {
          this->inflection_point_asc[i].final_value_m =
              this->inflection_point_asc[i].initial_value_m;
        } else {
          this->inflection_point_asc[i].final_value_m =
              sel->inflection_point_asc[i];
        }
      }

      for (size_t i = 0; i < slope_asc.size(); i++) {
        if (this->slope_asc[i].estimation_type_m.get() == "constant") {
          this->slope_asc[i].final_value_m = this->slope_asc[i].initial_value_m;
        } else {
          this->slope_asc[i].final_value_m = sel->slope_asc[i];
        }
      }

      for (size_t i = 0; i < inflection_point_desc.size(); i++) {
        if (this->inflection_point_desc[i].estimation_type_m.get() ==
            "constant") {
          this->inflection_point_desc[i].final_value_m =
              this->inflection_point_desc[i].initial_value_m;
        } else {
          this->inflection_point_desc[i].final_value_m =
              sel->inflection_point_desc[i];
        }
      }

      for (size_t i = 0; i < slope_desc.size(); i++) {
        if (this->slope_desc[i].estimation_type_m.get() == "constant") {
          this->slope_desc[i].final_value_m =
              this->slope_desc[i].initial_value_m;
        } else {
          this->slope_desc[i].final_value_m = sel->slope_desc[i];
        }
      }
    }
  }

  /**
   * @brief Set uncertainty values for double logistic selectivity parameters.
   *
   * @details Sets the standard error values for the ascending and descending
   * inflection points and slopes using the provided map.
   * @param se_values A map from parameter names to vectors of standard error
   * values.
   */
  virtual void set_uncertainty(
      std::map<std::string, std::vector<double>> &se_values) {
    fims::Vector<double> inflection_point_asc_uncertainty(
        this->inflection_point_asc.size(), -999);
    this->get_se_values("inflection_point_asc", se_values,
                        inflection_point_asc_uncertainty);
    fims::Vector<double> slope_asc_uncertainty(this->slope_asc.size(), -999);
    this->get_se_values("slope_asc", se_values, slope_asc_uncertainty);
    fims::Vector<double> inflection_point_desc_uncertainty(
        this->inflection_point_desc.size(), -999);
    this->get_se_values("inflection_point_desc", se_values,
                        inflection_point_desc_uncertainty);
    fims::Vector<double> slope_desc_uncertainty(this->slope_desc.size(), -999);
    this->get_se_values("slope_desc", se_values, slope_desc_uncertainty);
    for (size_t i = 0; i < this->inflection_point_asc.size(); i++) {
      this->inflection_point_asc[i].uncertainty_m =
          inflection_point_asc_uncertainty[i];
    }
    for (size_t i = 0; i < this->slope_asc.size(); i++) {
      this->slope_asc[i].uncertainty_m = slope_asc_uncertainty[i];
    }
    for (size_t i = 0; i < this->inflection_point_desc.size(); i++) {
      this->inflection_point_desc[i].uncertainty_m =
          inflection_point_desc_uncertainty[i];
    }
    for (size_t i = 0; i < this->slope_desc.size(); i++) {
      this->slope_desc[i].uncertainty_m = slope_desc_uncertainty[i];
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
      if (this->inflection_point_asc[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point_asc."
           << this->inflection_point_asc[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->inflection_point_asc[i]);
      }
      if (this->inflection_point_asc[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point_asc."
           << this->inflection_point_asc[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(selectivity->inflection_point_asc[i]);
      }
    }
    info->variable_map[this->inflection_point_asc.id_m] =
        &(selectivity)->inflection_point_asc;

    selectivity->slope_asc.resize(this->slope_asc.size());
    for (size_t i = 0; i < this->slope_asc.size(); i++) {
      selectivity->slope_asc[i] = this->slope_asc[i].initial_value_m;

      if (this->slope_asc[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".slope_asc."
           << this->slope_asc[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->slope_asc[i]);
      }
      if (this->slope_asc[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".slope_asc."
           << this->slope_asc[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(selectivity->slope_asc[i]);
      }
    }
    info->variable_map[this->slope_asc.id_m] = &(selectivity)->slope_asc;

    selectivity->inflection_point_desc.resize(
        this->inflection_point_desc.size());
    for (size_t i = 0; i < this->inflection_point_desc.size(); i++) {
      selectivity->inflection_point_desc[i] =
          this->inflection_point_desc[i].initial_value_m;

      if (this->inflection_point_desc[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point_desc."
           << this->inflection_point_desc[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->inflection_point_desc[i]);
      }
      if (this->inflection_point_desc[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".inflection_point_desc."
           << this->inflection_point_desc[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(selectivity->inflection_point_desc[i]);
      }
    }
    info->variable_map[this->inflection_point_desc.id_m] =
        &(selectivity)->inflection_point_desc;

    selectivity->slope_desc.resize(this->slope_desc.size());
    for (size_t i = 0; i < this->slope_desc.size(); i++) {
      selectivity->slope_desc[i] = this->slope_desc[i].initial_value_m;

      if (this->slope_desc[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".slope_desc."
           << this->slope_desc[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(selectivity->slope_desc[i]);
      }
      if (this->slope_desc[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Selectivity." << this->id << ".slope_desc."
           << this->slope_desc[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(selectivity->slope_desc[i]);
      }
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
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
#endif

    return true;
  }

#endif
};

#endif
