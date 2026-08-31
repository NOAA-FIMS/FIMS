/**
 * @file rcpp_interface_base.hpp
 * @brief The Rcpp interface to declare objects that are used ubiquitously
 * throughout the Rcpp interface, e.g., Variables and VariableVectors.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_INTERFACE_BASE_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_INTERFACE_BASE_HPP

#ifndef RCPP_NO_SUGAR
#define RCPP_NO_SUGAR
#endif
#include <RcppCommon.h>
#include <Rcpp.h>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

#include "common/enumerations.hpp"
#include "common/information.hpp"
#include "../../interface.hpp"
#include <limits>


// EstimationStatusFromString() and EstimationStatusToString() now live in
// common/enumerations.hpp, next to the enum they convert. Pulled into this
// scope so the call sites below stay unqualified.
using fims_enum::EstimationStatusFromString;
using fims_enum::EstimationStatusToString;

/**
 * @brief An Rcpp interface that defines the Variable class.
 *
 * @details An Rcpp interface class that defines the interface between R and
 * C++ for a variable type.
 */
class Variable {
 public:
  /**
   * @brief The static ID of the Variable object.
   */
  static uint32_t id_g;
  /**
   * @brief The local ID of the Variable object.
   */
  uint32_t id_m;
  /**
   * @brief The initial value of the variable.
   */
  double initial_value_m = 0.0;
  /**
   * @brief The final value of the variable.
   */
  double final_value_m = 0.0;
  /**
   * @brief An enum indicating estimation status.
   */
  fims_enum::EstimationStatus estimation_status_m = fims_enum::EstimationStatus::kAssumedKnown;

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable(double value, std::string estimation_status)
      : id_m(Variable::id_g++),
        initial_value_m(value),
        estimation_status_m(EstimationStatusFromString(estimation_status)) {}

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable(const Variable& other)
      : id_m(other.id_m),
        initial_value_m(other.initial_value_m),
        final_value_m(other.final_value_m),
        estimation_status_m(other.estimation_status_m) {}

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable& operator=(const Variable& right) {
    // Check for self-assignment!
    if (this == &right)  // Same object?
      return *this;      // Yes, so skip assignment, and just return *this.
    this->id_m = right.id_m;
    this->initial_value_m = right.initial_value_m;
    this->estimation_status_m = right.estimation_status_m;
    return *this;
  }

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable(double value) {
    initial_value_m = value;
    id_m = Variable::id_g++;
  }

  /**
   * @brief The constructor for initializing a Variable.
   * @details Set value to 0 when there is no input value.
   */
  Variable() {
    initial_value_m = 0;
    id_m = Variable::id_g++;
  }

  /**
   * @brief Get estimation status as a string.
   */
  std::string get_estimation_status() const {
    return EstimationStatusToString(this->estimation_status_m);
  }

  /**
   * @brief Set estimation status from a string.
   */
  void set_estimation_status(const std::string& status) {
    this->estimation_status_m = EstimationStatusFromString(status);
  }
};

#ifdef FIMS_HEADER_ONLY
uint32_t Variable::id_g = 0;
#endif

/**
 * @brief Sanitize a double value by replacing NaN or Inf with -999.0.
 *
 * @param x The input double value.
 * @return The sanitized double value.
 */
inline double sanitize_val(double x) {
  if (std::isnan(x) || std::isinf(x)) {
    return -999.0;
  }
  return x;
}

/**
 * @brief Output for std::ostream& for a variable.
 *
 * @param out The stream.
 * @param p A variable.
 * @return std::ostream&
 */
inline std::ostream& operator<<(std::ostream& out, const Variable& p) {
  out << "{\"id\": " << p.id_m
      << ",\n\"value\": " << sanitize_val(p.initial_value_m)
      << ",\n\"estimated_value\": " << sanitize_val(p.final_value_m);
  out << ",\n\"estimation_status\": \""
      << EstimationStatusToString(p.estimation_status_m) << "\"\n}";

  return out;
}

RCPP_EXPOSED_CLASS(Variable)

/**
 * @brief An Rcpp interface class that defines the VariableVector class.
 *
 * @details An Rcpp interface class that defines the interface between R and
 * C++ for a variable vector type.
 */
class VariableVector {
 public:
  /**
   * @brief The static ID of the Variable object.
   */
  static uint32_t id_g;
  /**
   * @brief Variable storage.
   */
  std::shared_ptr<std::vector<Variable>> storage_m;
  /**
   * @brief The local ID of the Variable object.
   */
  uint32_t id_m;

  /**
   * @brief The constructor.
   */
  VariableVector() {
    this->id_m = VariableVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Variable>>();
    this->storage_m->resize(1);  // push_back(Rcpp::wrap(p));
  }

  /**
   * @brief The constructor.
   */
  VariableVector(const VariableVector& other)
      : storage_m(other.storage_m), id_m(other.id_m) {}

  /**
   * @brief The constructor.
   */
  VariableVector(size_t size) {
    this->id_m = VariableVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Variable>>();
    this->storage_m->resize(size);
    for (size_t i = 0; i < size; i++) {
      storage_m->at(i) = Variable();
    }
  }

  /**
   * @brief The constructor for initializing a variable vector.
   * @param x A numeric vector.
   * @param size The number of elements to copy over.
   */
  VariableVector(Rcpp::NumericVector x, size_t size) {
    const size_t input_size = static_cast<size_t>(x.size());
    if (input_size != size) {
      throw std::invalid_argument(
          "VariableVector::VariableVector(Rcpp::NumericVector, size_t): `x` "
          "length (" +
          std::to_string(input_size) +
          ") must equal the "
          "requested size (" +
          std::to_string(size) +
          "). Received length: " + std::to_string(input_size) + ".");
    } else {
      this->id_m = VariableVector::id_g++;
      this->storage_m = std::make_shared<std::vector<Variable>>();
      // Use std::min to avoid comparing signed and unsigned types
      size_t n = std::min(input_size, size);
      this->storage_m->resize(n);
      for (size_t i = 0; i < n; i++) {
        storage_m->at(i).initial_value_m = x[i];
      }
    }
  }

  /**
   * @brief The constructor for initializing a variable vector.
   * @param v A vector of doubles.
   */
  VariableVector(const fims::Vector<double>& v) {
    this->id_m = VariableVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Variable>>();
    this->storage_m->resize(v.size());
    for (size_t i = 0; i < v.size(); i++) {
      storage_m->at(i).initial_value_m = v[i];
    }
  }

  /**
   * @brief Destroy the Variable Vector object.
   *
   */
  virtual ~VariableVector() {}

  /**
   * @brief Gets the ID of the VariableVector object.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief The accessor where the first index starts is zero.
   * @param pos The position of the VariableVector that you want returned.
   */
  inline Variable& operator[](size_t pos) { return this->storage_m->at(pos); }

  /**
   * @brief The accessor where the first index starts at one. This function is
   * for calling accessing from R.
   * @param pos The position of the VariableVector that you want returned.
   */
  SEXP at(R_xlen_t pos) {
    if (static_cast<size_t>(pos) == 0 ||
        static_cast<size_t>(pos) > this->storage_m->size()) {
      throw std::invalid_argument("VariableVector: Index out of range");
      FIMS_ERROR_LOG(fims::to_string(pos) + "!<" +
                     fims::to_string(this->size()));
      return NULL;
    }
    return Rcpp::wrap(this->storage_m->at(pos - 1));
  }

  /**
   * @brief An internal accessor for calling a position of a VariableVector
   * from R.
   * @param pos An integer specifying the position of the VariableVector
   * you want returned. The first position is one and the last position is
   * the same as the size of the VariableVector.
   */
  Variable& get(size_t pos) {
    if (pos >= this->storage_m->size()) {
      throw std::invalid_argument("VariableVector: Index out of range");
    }
    return (this->storage_m->at(pos));
  }

  /**
   * @brief An internal setter for setting a position of a VariableVector
   * from R.
   * @param pos An integer specifying the position of the VariableVector
   * you want to set. The first position is one and the last position is the
   * same as the size of the VariableVector.
   * @param p A numeric value specifying the value to set position `pos` to
   * in the VariableVector.
   */
  void set(size_t pos, const Variable& p) { this->storage_m->at(pos) = p; }

  /**
   * @brief Returns the size of a VariableVector.
   */
  size_t size() { return this->storage_m->size(); }

  /**
   * @brief Resizes a VariableVector to the desired length.
   * @param size An integer specifying the desired length for the
   * VariableVector to be resized to.
   */
  void resize(size_t size) { this->storage_m->resize(size); }

  /**
   * @brief Sets the initial values for all Variables within a VariableVector.
   */
  void set_values(Rcpp::NumericVector values) {
    if (values.size() != this->storage_m->size()) {
      const size_t input_size = values.size();
      const size_t vector_size = this->storage_m->size();
      throw std::invalid_argument(
          "VariableVector::set_values(): `values` length (" +
          std::to_string(input_size) +
          ") must equal the VariableVector "
          "size (" +
          std::to_string(vector_size) + "). Received length: " +
          std::to_string(input_size) + ". Pass a numeric vector of length " +
          std::to_string(vector_size) + ".");
    }
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      this->storage_m->at(i).initial_value_m = values[i];
    }
  }

  /**
   * @brief Sets the estimation status for all Variables within a
   * VariableVector.
   */
  void set_estimation_status(Rcpp::CharacterVector estimation_status) {
    const size_t vector_size = this->storage_m->size();
    const size_t input_size = estimation_status.size();

    if (input_size != 1 && input_size != vector_size) {
      throw std::invalid_argument(
          "VariableVector::set_estimation_status(): `estimation_status` length "
          "(" +
          std::to_string(input_size) +
          ") must be 1 (broadcast) or equal to the VariableVector size (" +
          std::to_string(vector_size) +
          ").\n"
          "Received length: " +
          std::to_string(input_size) +
          ". "
          "Pass a single estimation status to apply to all elements, or a "
          "vector of length " +
          std::to_string(vector_size) + ".");
    }

    for (size_t i = 0; i < vector_size; i++) {
      std::string est_status =
          Rcpp::as<std::string>(estimation_status[input_size == 1 ? 0 : i]);
      this->storage_m->at(i).estimation_status_m =
          EstimationStatusFromString(est_status);
    }
  }

  /**
   * @brief Sets the value of all Variables in the VariableVector to the
   * provided value.
   *
   * @param value A double specifying the value to set all Variables to
   * within the VariableVector.
   */
  void fill(double value) {
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i).initial_value_m = value;
    }
  }

  /**
   * @brief The printing methods for a VariableVector.
   *
   */
  void show() {
    Rcpp::Rcout << this->storage_m->data() << "\n";

    for (size_t i = 0; i < this->storage_m->size(); i++) {
      Rcpp::Rcout << storage_m->at(i) << "  ";
    }
  }

  /**
   * @brief Create a deep copy with a new VariableVector ID.
   */
  VariableVector deep_copy() const {
    VariableVector copy;
    copy.storage_m = std::make_shared<std::vector<Variable>>();
    copy.storage_m->reserve(this->storage_m->size());
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      Variable variable_copy;
      const Variable& variable = this->storage_m->at(i);
      variable_copy.initial_value_m = variable.initial_value_m;
      variable_copy.final_value_m = variable.final_value_m;
      variable_copy.estimation_status_m =
          variable.estimation_status_m;
      copy.storage_m->push_back(variable_copy);
    }
    return copy;
  }
};

/**
 * @brief Register a parameter by estimation status.
 */
template <typename Type>
inline void register_parameter_if_estimable(
    Type& parameter, fims_enum::EstimationStatus estimation_status,
    const std::string& parameter_name, bool random_effects_allowed = true) {
  std::shared_ptr<fims_info::Information<Type>> info =
      fims_info::Information<Type>::GetInstance();

  if (!random_effects_allowed &&
      estimation_status == fims_enum::EstimationStatus::kRandomEffects) {
    Rf_error("%s cannot be set to random effects.", parameter_name.c_str());
  }

  switch (estimation_status) {
    case fims_enum::EstimationStatus::kAssumedKnown:
    case fims_enum::EstimationStatus::kDerivedQuantity:
      break;
    case fims_enum::EstimationStatus::kFixedEffects:
      info->RegisterParameterName(parameter_name);
      info->RegisterParameter(parameter);
      break;
    case fims_enum::EstimationStatus::kRandomEffects:
      info->RegisterRandomEffectName(parameter_name);
      info->RegisterRandomEffect(parameter);
      break;
    default:
      Rf_error(
          "Unknown estimation_status code %d. Supported codes are "
          "0 (assumed_known), 1 (fixed_effects), 2 (random_effects), and "
          "3 (derived_quantity).",
          static_cast<int>(estimation_status));
  }
}


/**
 * @brief Set final value from estimated value based on estimation status.
 */
template <typename Type>
inline void set_final_value_by_estimation_status(Variable& variable,
                                                 const Type& estimated_value) {
  switch (variable.estimation_status_m) {
    case fims_enum::EstimationStatus::kAssumedKnown:
    case fims_enum::EstimationStatus::kDerivedQuantity:
      variable.final_value_m = variable.initial_value_m;
      return;
    case fims_enum::EstimationStatus::kFixedEffects:
    case fims_enum::EstimationStatus::kRandomEffects:
      variable.final_value_m = estimated_value;
      return;
    default:
      Rf_error(
          "Unknown estimation_status code %d. Supported codes are "
          "0 (assumed_known), 1 (fixed_effects), 2 (random_effects), and "
          "3 (derived_quantity).",
          static_cast<int>(variable.estimation_status_m));
  }
}

#ifdef FIMS_HEADER_ONLY
uint32_t VariableVector::id_g = 0;
#endif

/**
 * @brief Output for std::ostream& for a VariableVector.
 *
 * @param out The stream.
 * @param v A VariableVector.
 * @return std::ostream&
 */
inline std::ostream& operator<<(std::ostream& out, VariableVector& v) {
  out << "[";
  size_t size = v.size();
  for (size_t i = 0; i < size - 1; i++) {
    out << v[i] << ", ";
  }
  out << v[size - 1] << "]";
  return out;
}

RCPP_EXPOSED_CLASS(VariableVector)

/**
 * @brief Fill a VariableVector from R, setting values and estimation statuses.
 *
 * @details Shared by the standalone XPtr setter functions in the src/*.cpp
 * files. The vector is resized to match `values`. An `estimation_status` of
 * length 1 is recycled across every element; otherwise it must be the same
 * length as `values`.
 *
 * @param target The VariableVector to fill.
 * @param values The initial values.
 * @param estimation_status One status name, or one per element.
 */
inline void set_variable_vector(VariableVector &target,
                                Rcpp::NumericVector values,
                                Rcpp::CharacterVector estimation_status) {
  if (estimation_status.size() != 1 &&
      estimation_status.size() != values.size()) {
    Rcpp::stop(
        "set_variable_vector(): `estimation_status` must be length 1 or the "
        "same length as `values`.");
  }
  target.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    Variable &v = target.storage_m->at(i);
    v.initial_value_m = values[i];
    v.estimation_status_m = EstimationStatusFromString(Rcpp::as<std::string>(
        estimation_status[estimation_status.size() == 1 ? 0 : i]));
  }
}

/**
 * @brief Fill a numeric vector from R, resizing to match.
 *
 * @param target The vector to fill.
 * @param values The values to copy in.
 */
inline void set_real_vector(fims::Vector<double> &target,
                            Rcpp::NumericVector values) {
  target.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    target[i] = values[i];
  }
}

/**
 *@brief Base class for all interface objects.
 */
class FIMSRcppInterfaceBase {
 public:
  /**
   * @brief How many interface modules are alive right now.
   *
   * @details Every module -- population, fleet, selectivity, and the rest --
   * adds one here when it is constructed and takes one away when it is
   * destroyed.
   *
   * clear() reads it to check that R released every module before the ID
   * counters are rewound. The R clear() wrapper releases them all, so this is
   * normally zero by then, and a count above zero means something bypassed the
   * wrapper. That matters because a surviving module keeps an ID that will be
   * handed out again after the rewind, and IDs are how the model finds
   * parameters.
   */
  static inline int live_module_count = 0;

  /**
   * @brief A vector of every ID counter that clear() has to rewind.
   *
   * @details Each family's id_g adds itself here where it is defined, in the
   * corresponding src/rcpp_*.cpp, so clear() can loop over the counters rather
   * than naming them one by one. Adding a module family therefore cannot leave
   * a counter behind.
   *
   * A function-local static rather than a plain static member, so the vector
   * is guaranteed to exist before the first registration runs, whatever order
   * the translation units initialize in.
   *
   * @return The registered counters.
   */
  static std::vector<uint32_t *> &id_counters() {
    static std::vector<uint32_t *> counters;
    return counters;
  }

  /** @brief Count this module as alive. */
  FIMSRcppInterfaceBase() { live_module_count++; }

  /** @brief Stop counting this module as alive. */
  virtual ~FIMSRcppInterfaceBase() { live_module_count--; }

  /**
   * @brief Is the object already finalized? The default is false.
   */
  bool finalized = false;
  /**
   * @brief This module's unique ID.
   *
   * @details Every module carries one, and IDs are how modules refer to each
   * other: a fleet records its selectivity's ID, a population records its
   * growth module's, and fims_info::Information is keyed by them. Declared on
   * the root so one exported getter serves every module.
   *
   * @return The ID.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief Look up one of this module's parameter vectors by name.
   *
   * @details Declared on the root so that one exported setter and one id
   * getter serve every module, reached through an XPtr typed to this class.
   * Modules that carry no named parameters -- the data modules and the
   * fishery models -- inherit this default and report that the name is
   * unknown.
   *
   * Section 2.1 of Parameter_Registry.Rmd replaces these hand-written
   * overrides with a declare_parameter() call per field in each constructor.
   * The signature here is what that design calls find_parameter(), so the
   * exported functions do not change when that lands.
   *
   * @param name The parameter name as used in R.
   * @return A pointer to the vector, or nullptr if this module has no
   * parameter by that name.
   */
  virtual VariableVector *get_parameter(const std::string &name) {
    return nullptr;
  }

  /**
   * @brief Look up one of this module's fixed numeric vectors by name.
   *
   * @details The counterpart to get_parameter(), for vectors that hold plain
   * numbers rather than estimable parameters: observations, uncertainties,
   * ages, empirical weights. These carry no estimation status and no
   * variable_map id, so one setter writes them with values alone.
   *
   * A module author picks between the two by what the field is, not by which
   * family it belongs to: estimable goes in get_parameter(), fixed goes here.
   *
   * @param name The vector name as used in R.
   * @return A pointer to the vector, or nullptr if this module has no vector
   * by that name.
   */
  virtual fims::Vector<double> *get_vector(const std::string &name) {
    return nullptr;
  }

  /**
   * @brief A virtual method to inherit to add objects to the TMB model.
   */
  virtual bool add_to_fims_tmb() {
    Rcpp::Rcout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet "
                   "implemented.\n";
    return false;
  }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {}

  /**
   * @brief Convert the data to json representation for the output.
   */
  virtual std::string to_json() {
    FIMS_WARNING_LOG("Method not yet defined.");
    return "{\"name\": \"not yet implemented\"}";
  }

  /**
   * @brief Report the variable value as a string.
   *
   * @param value
   * @return std::string
   */
  std::string value_to_string(double value) {
    std::stringstream ss;
    if (value == std::numeric_limits<double>::infinity()) {
      ss << "\"Infinity\"";
    } else if (value == -std::numeric_limits<double>::infinity()) {
      ss << "\"-Infinity\"";
    } else if (value != value) {
      ss << "-999";
    } else {
      // Set precision (R default is 16)
      ss << std::fixed << std::setprecision(16) << value;
    }
    return ss.str();
  }
  /**
   * @brief Make a string of dimensions for the model.
   */
  std::string make_dimensions(uint32_t start, uint32_t end, uint32_t rep = 1) {
    std::stringstream ss;

    for (size_t i = 0; i < rep; i++) {
      for (size_t j = start; j < end; j++) {
        ss << j << ", ";
      }
      if (i < (rep - 1)) {
        ss << end << ", ";
      } else {
        ss << end;
      }
    }
    return ss.str();
  }
};

/**
 * @brief A shared pointer that can hold any FIMS interface object.
 *
 * @details Every interface class -- data, fleet, population, and the rest --
 * inherits from FIMSRcppInterfaceBase, so a SharedBase can point at any of
 * them. CreateTMBModel() uses this to hold a mixed list of modules and call
 * add_to_fims_tmb() on each one without needing to know what each module is:
 * C++ runs the right version based on what the object actually is.
 */
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

/**
 * @brief Registers one ID counter with FIMSRcppInterfaceBase::id_counters().
 *
 * @details Declare one of these next to each `id_g` definition in
 * src/rcpp_*.cpp. Its constructor runs at startup and adds the counter to the
 * list clear() rewinds, so the counter cannot be forgotten there.
 */
struct IdCounterRegistration {
  /**
   * @brief Adds the counter to the list.
   * @param counter The id_g to register.
   */
  explicit IdCounterRegistration(uint32_t *counter) {
    FIMSRcppInterfaceBase::id_counters().push_back(counter);
  }
};

#endif
