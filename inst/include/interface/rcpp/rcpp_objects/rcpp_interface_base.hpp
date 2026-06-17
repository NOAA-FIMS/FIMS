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
#include <vector>

#include "common/information.hpp"
#include "../../interface.hpp"
#include "rcpp_shared_primitive.hpp"
#include <limits>

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
   * @brief A string indicating the estimation type. Options are: constant,
   * fixed_effects, or random_effects, where the default is constant.
   */
  SharedString estimation_type_m = SharedString("constant");

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable(double value, std::string estimation_type)
      : id_m(Variable::id_g++),
        initial_value_m(value),
        estimation_type_m(estimation_type) {}

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable(const Variable& other)
      : id_m(other.id_m),
        initial_value_m(other.initial_value_m),
        final_value_m(other.final_value_m),
        estimation_type_m(other.estimation_type_m) {}

  /**
   * @brief The constructor for initializing a variable.
   */
  Variable& operator=(const Variable& right) {
    // Check for self-assignment!
    if (this == &right)  // Same object?
      return *this;      // Yes, so skip assignment, and just return *this.
    this->id_m = right.id_m;
    this->initial_value_m = right.initial_value_m;
    this->estimation_type_m = right.estimation_type_m;
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
   * @brief Create a deep copy with a new variable ID.
   */
  std::shared_ptr<Variable> deep_copy() const {
    std::shared_ptr<Variable> copy = std::make_shared<Variable>();
    copy->initial_value_m = this->initial_value_m;
    copy->final_value_m = this->final_value_m;
    copy->estimation_type_m = SharedString(this->estimation_type_m.get());
    return copy;
  }

  /**
   * @brief Rcpp-facing deep copy wrapper.
   */
  Variable* deep_copy_rcpp() const { return new Variable(*this->deep_copy()); }
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
  out << ",\n\"estimation_type\": \"" << p.estimation_type_m << "\"\n}";

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
   * @brief Sets the estimation type for all Variables within a
   * VariableVector.
   */
  void set_estimation_types(Rcpp::CharacterVector estimation_types) {
    const size_t vector_size = this->storage_m->size();
    const size_t input_size = estimation_types.size();

    if (input_size != 1 && input_size != vector_size) {
      throw std::invalid_argument(
          "VariableVector::set_estimation_types(): `estimation_types` length "
          "(" +
          std::to_string(input_size) +
          ") must be 1 (broadcast) or equal to the VariableVector size (" +
          std::to_string(vector_size) +
          ").\n"
          "Received length: " +
          std::to_string(input_size) +
          ". "
          "Pass a single estimation type to apply to all elements, or a "
          "vector of length " +
          std::to_string(vector_size) + ".");
    }

    auto validate_estimation_type = [&](const std::string& est_type) {
      if (est_type != "constant" && est_type != "fixed_effects" &&
          est_type != "random_effects") {
        throw std::invalid_argument(
            "Invalid estimation_type: " + est_type +
            ". Valid options are: constant, fixed_effects, or random_effects.");
      }
    };

    for (size_t i = 0; i < vector_size; i++) {
      std::string est_type =
          Rcpp::as<std::string>(estimation_types[input_size == 1 ? 0 : i]);
      validate_estimation_type(est_type);
      this->storage_m->at(i).estimation_type_m.set(est_type);
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
  std::shared_ptr<VariableVector> deep_copy() const {
    std::shared_ptr<VariableVector> copy = std::make_shared<VariableVector>();
    copy->storage_m = std::make_shared<std::vector<Variable>>();
    copy->storage_m->reserve(this->storage_m->size());
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      Variable variable_copy;
      const Variable& variable = this->storage_m->at(i);
      variable_copy.initial_value_m = variable.initial_value_m;
      variable_copy.final_value_m = variable.final_value_m;
      variable_copy.estimation_type_m =
          SharedString(variable.estimation_type_m.get());
      copy->storage_m->push_back(variable_copy);
    }
    return copy;
  }

  /**
   * @brief Rcpp-facing deep copy wrapper.
   */
  VariableVector* deep_copy_rcpp() const {
    return new VariableVector(*this->deep_copy());
  }
};

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

/**
 * @brief An Rcpp interface class that defines the RealVector class.
 *
 * @details An Rcpp interface class that defines the interface between R and
 * C++ for a real vector type. Underlying values are held in a shared pointer
 * and are carried over to any copies of this vector.
 */
class RealVector {
 public:
  /**
   * @brief The static ID of the RealVector object.
   */
  static uint32_t id_g;
  /**
   * @brief real storage.
   */
  std::shared_ptr<std::vector<double>> storage_m;
  /**
   * @brief The local ID of the RealVector object.
   */
  uint32_t id_m;

  /**
   * @brief The constructor.
   */
  RealVector() {
    this->id_m = RealVector::id_g++;
    this->storage_m = std::make_shared<std::vector<double>>();
    this->storage_m->resize(1);
  }

  /**
   * @brief The constructor.
   */
  RealVector(const RealVector& other)
      : storage_m(other.storage_m), id_m(other.id_m) {}

  /**
   * @brief The constructor.
   */
  RealVector(size_t size) {
    this->id_m = RealVector::id_g++;
    this->storage_m = std::make_shared<std::vector<double>>();
    this->storage_m->resize(size);
  }

  /**
   * @brief The constructor for initializing a real vector.
   * @param x A numeric vector.
   * @param size The number of elements to copy over.
   */
  RealVector(Rcpp::NumericVector x, size_t size) {
    this->id_m = RealVector::id_g++;
    this->storage_m = std::make_shared<std::vector<double>>();
    const size_t input_size = static_cast<size_t>(x.size());
    if (input_size != size) {
      throw std::invalid_argument(
          "RealVector::RealVector(Rcpp::NumericVector, size_t): `x` length (" +
          std::to_string(input_size) +
          ") must equal the requested "
          "size (" +
          std::to_string(size) +
          "). Received length: " + std::to_string(input_size) + ".");
    }
    this->storage_m->assign(x.begin(), x.end());
  }

  /**
   * @brief The constructor for initializing a real vector.
   * @param v A vector of doubles.
   */
  RealVector(const fims::Vector<double>& v) {
    this->id_m = RealVector::id_g++;
    this->storage_m = std::make_shared<std::vector<double>>();
    this->storage_m->resize(v.size());
    for (size_t i = 0; i < v.size(); i++) {
      storage_m->at(i) = v[i];
    }
  }

  /**
   * @brief Destroy the real Vector object.
   *
   */
  virtual ~RealVector() {}

  /**
   * @brief
   *
   * @param v
   * @return RealVector&
   */
  RealVector& operator=(const Rcpp::NumericVector& v) {
    this->storage_m->assign(v.begin(), v.end());
    return *this;
  }

  /**
   * @brief Gets the ID of the RealVector object.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief
   *
   * @param orig
   */
  void set_values(const Rcpp::NumericVector& orig) {
    this->storage_m->resize(orig.size());
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      this->storage_m->at(i) = orig[i];
    }
  }

  /**
   * @brief
   *
   * @return Rcpp::NumericVector
   */
  Rcpp::NumericVector get_values() {
    Rcpp::NumericVector ret(this->storage_m->size());
    for (size_t i = 0; i < this->size(); i++) {
      ret[i] = this->storage_m->at(i);
    }

    return ret;
  }

  /**
   * @brief The accessor where the first index starts is zero.
   * @param pos The position of the RealVector that you want returned.
   */
  inline double& operator[](size_t pos) { return this->storage_m->at(pos); }

  /**
   * @brief The accessor where the first index starts at one. This function is
   * for calling accessing from R.
   * @param pos The position of the VariableVector that you want returned.
   */
  SEXP at(R_xlen_t pos) {
    if (static_cast<size_t>(pos) == 0 ||
        static_cast<size_t>(pos) > this->storage_m->size()) {
      throw std::invalid_argument("RealVector: Index out of range");
      FIMS_ERROR_LOG(fims::to_string(pos) + "!<" +
                     fims::to_string(this->size()));
      return NULL;
    }
    return Rcpp::wrap(this->storage_m->at(pos - 1));
  }

  /**
   * @brief An internal accessor for calling a position of a RealVector
   * from R.
   * @param pos An integer specifying the position of the RealVector
   * you want returned. The first position is one and the last position is
   * the same as the size of the RealVector.
   */
  double& get(size_t pos) {
    if (pos >= this->storage_m->size()) {
      throw std::invalid_argument("RealVector: Index out of range");
    }
    return (this->storage_m->at(pos));
  }

  /**
   * @brief An internal setter for setting a position of a RealVector
   * from R.
   * @param pos An integer specifying the position of the RealVector
   * you want to set. The first position is one and the last position is the
   * same as the size of the RealVector.
   * @param p A numeric value specifying the value to set position `pos` to
   * in the RealVector.
   */
  void set(size_t pos, const double& p) { this->storage_m->at(pos) = p; }

  /**
   * @brief Returns the size of a RealVector.
   */
  size_t size() { return this->storage_m->size(); }

  /**
   * @brief Resizes a RealVector to the desired length.
   * @param size An integer specifying the desired length for the
   * RealVector to be resized to.
   */
  void resize(size_t size) { this->storage_m->resize(size); }

  /**
   * @brief Sets the value of all elements in the RealVector to the
   * provided value.
   *
   * @param value A double specifying the value to set all elements to
   * within the RealVector.
   */
  void fill(double value) {
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i) = value;
    }
  }

  /**
   * @brief The printing methods for a RealVector.
   *
   */
  void show() {
    Rcpp::Rcout << this->storage_m->data() << "\n";

    for (size_t i = 0; i < this->storage_m->size(); i++) {
      Rcpp::Rcout << storage_m->at(i) << "  ";
    }
  }

  /**
   * @brief Create a deep copy with a new RealVector ID.
   */
  std::shared_ptr<RealVector> deep_copy() const {
    std::shared_ptr<RealVector> copy = std::make_shared<RealVector>();
    copy->storage_m = std::make_shared<std::vector<double>>(*this->storage_m);
    return copy;
  }

  /**
   * @brief Rcpp-facing deep copy wrapper.
   */
  RealVector* deep_copy_rcpp() const { return new RealVector(*this->deep_copy()); }
};
#ifdef FIMS_HEADER_ONLY
uint32_t RealVector::id_g = 0;
#endif

/**
 * @brief Create a Variable copy with independent shared members.
 */
inline Variable DeepCopyVariable(const Variable& other) {
  Variable copy;
  copy.initial_value_m = other.initial_value_m;
  copy.final_value_m = other.final_value_m;
  copy.estimation_type_m = SharedString(other.estimation_type_m.get());
  return copy;
}

/**
 * @brief Create a VariableVector copy with independent storage.
 */
inline VariableVector DeepCopyVariableVector(const VariableVector& other) {
  VariableVector copy;
  copy.storage_m = std::make_shared<std::vector<Variable>>();
  copy.storage_m->reserve(other.storage_m->size());
  for (size_t i = 0; i < other.storage_m->size(); i++) {
    copy.storage_m->push_back(DeepCopyVariable(other.storage_m->at(i)));
  }
  return copy;
}

/**
 * @brief Create a RealVector copy with independent storage.
 */
inline RealVector DeepCopyRealVector(const RealVector& other) {
  RealVector copy;
  copy.storage_m = std::make_shared<std::vector<double>>(*other.storage_m);
  return copy;
}

RCPP_EXPOSED_CLASS(VariableVector)
RCPP_EXPOSED_CLASS(RealVector)

/**
 *@brief Base class for all interface objects.
 */
class FIMSRcppInterfaceBase {
 public:
  /**
   * @brief Is the object already finalized? The default is false.
   */
  bool finalized = false;
  /**
   * @brief FIMS interface object vectors.
   */
  static std::vector<std::shared_ptr<FIMSRcppInterfaceBase>>
      fims_interface_objects;

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

#endif
