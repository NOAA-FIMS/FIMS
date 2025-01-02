/**
 * @file rcpp_interface_base.hpp
 * @brief The Rcpp interface to declare objects that are used ubiquitously
 * throughout the Rcpp interface, e.g., Parameters and ParameterVectors.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_INTERFACE_BASE_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_INTERFACE_BASE_HPP

#include <RcppCommon.h>
#include <map>
#include <vector>

#include "../../../common/def.hpp"
#include "../../../common/information.hpp"
#include "../../interface.hpp"

#define RCPP_NO_SUGAR
#include <Rcpp.h>

/**
 * @brief An Rcpp interface that defines the Parameter class.
 *
 * @details An Rcpp interface class that defines the interface between R and
 * C++ for a parameter type.
 */
class Parameter {
 public:
  /**
   * @brief The static ID of the Parameter object.
   */
  static uint32_t id_g;
  /**
   * @brief The local ID of the Parameter object.
   */
  uint32_t id_m;
  /**
   * @brief The initial value of the parameter.
   */
  double initial_value_m = 0.0;
  /**
   * @brief The final value of the parameter.
   */
  double final_value_m = 0.0;
  /**
   * @brief The minimum possible parameter value, where the default is negative
   * infinity.
   */
  double min_m = -std::numeric_limits<double>::infinity();
  /**
   * @brief The maximum possible parameter value, where the default is positive
   * infinity.
   */
  double max_m = std::numeric_limits<double>::infinity();
  /**
   * @brief Is the parameter a random effect? The default is false.
   */
  bool is_random_effect_m = false;
  /**
   * @brief Should the parameter be estimated? The default is false.
   */
  bool estimated_m = false;

  /**
   * @brief The constructor for initializing a parameter.
   */
  Parameter(double value, double min, double max, bool estimated)
      : id_m(Parameter::id_g++), initial_value_m(value), min_m(min), max_m(max), estimated_m(estimated) {}

  /**
   * @brief The constructor for initializing a parameter.
   */
  Parameter(const Parameter& other) :
    id_m(other.id_m), initial_value_m(other.initial_value_m),
    final_value_m(other.final_value_m),
    min_m(other.min_m), max_m(other.max_m),
    is_random_effect_m(other.is_random_effect_m),
    estimated_m(other.estimated_m) {}

  /**
   * @brief The constructor for initializing a parameter.
   */
  Parameter& operator=(const Parameter& right) {
    // Check for self-assignment!
    if (this == &right) // Same object?
      return *this; // Yes, so skip assignment, and just return *this.
    this->id_m = right.id_m;
    this->initial_value_m = right.initial_value_m;
    this->estimated_m = right.estimated_m;
    this->min_m = right.min_m;
    this->max_m = right.max_m;
    this->is_random_effect_m = right.is_random_effect_m;
    return *this;
  }

   

  /**
   * @brief The constructor for initializing a parameter.
   */
  Parameter(double value) {
    initial_value_m = value;
    id_m = Parameter::id_g++;
  }

  /**
   * @brief The constructor for initializing a parameter.
   * @details Set value to 0 when there is no input value.
   */
  Parameter() {
    initial_value_m = 0;
    id_m = Parameter::id_g++;}
};
  /**
   * @brief The unique ID for the variable map that points to a fims::Vector.
   */
  uint32_t Parameter::id_g = 0;

/**
 * @brief Output for std::ostream& for a parameter.
 *
 * @param out The stream.
 * @param p A parameter.
 * @return std::ostream& 
 */
std::ostream& operator<<(std::ostream& out, const Parameter& p) {
  out << "{id:" << p.id_m << ",\nvalue:" << p.initial_value_m
    << ",\nestimated_value:" << p.final_value_m << ",\nmin:"
    << p.min_m << ",\nmax:" << p.max_m << ",\nestimated:" << p.estimated_m << "\n}";
  return out;
}

/**
 * @brief An Rcpp interface class that defines the ParameterVector class.
 *
 * @details An Rcpp interface class that defines the interface between R and
 * C++ for a parameter vector type.
 */
class ParameterVector{
public:
  /**
   * @brief The static ID of the Parameter object.
   */
  static uint32_t id_g;
  /**
   * @brief Parameter storage.
   */
  std::shared_ptr<std::vector<Parameter> > storage_m;
  /**
   * @brief The local ID of the Parameter object.
   */
  uint32_t id_m;

  /**
   * @brief The constructor.
   */
  ParameterVector(){
    this->id_m = ParameterVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Parameter> >();
    this->storage_m->resize(1); //push_back(Rcpp::wrap(p));
  }

  /**
   * @brief The constructor.
   */
  ParameterVector(const ParameterVector& other) :
    storage_m(other.storage_m), id_m(other.id_m) {}

  /**
   * @brief The constructor.
   */
  ParameterVector(size_t size ){
    this->id_m = ParameterVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Parameter> >();
    this->storage_m->resize(size);
    for (size_t i = 0; i < size; i++) {
      storage_m->at(i) = Parameter(); 
    }
  }

  /**
   * @brief The constructor for initializing a parameter vector.
   * @param x A numeric vector.
   * @param size The number of elements to copy over.
   */
  ParameterVector(Rcpp::NumericVector x, size_t size){
    this->id_m = ParameterVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Parameter> >();
    this->resize(size);
    for (size_t i = 0; i < size; i++) {
      storage_m->at(i).initial_value_m = x[i];
    }
  }

  /**
   * @brief The constructor for initializing a parameter vector.
   * @param v A vector of doubles.
   */
  ParameterVector(const fims::Vector<double>& v) {
    this->id_m = ParameterVector::id_g++;
    this->storage_m = std::make_shared<std::vector<Parameter> >();
    this->storage_m->resize(v.size());
    for (size_t i = 0; i < v.size(); i++) {
      storage_m->at(i).initial_value_m = v[i];
    }
  }

  /**
   * @brief Destroy the Parameter Vector object.
   * 
   */
  virtual ~ParameterVector(){}

  /**
   * @brief Gets the ID of the ParameterVector object.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief The accessor where the first index starts is zero.
   * @param pos The position of the ParameterVector that you want returned.
   */
  inline Parameter& operator[](size_t pos) {
    return this->storage_m->at(pos);
  }

  /**
   * @brief The accessor where the first index starts at one. This function is
   * for calling accessing from R.
   * @param pos The position of the ParameterVector that you want returned.
   */
  SEXP at(R_xlen_t pos){
    if (static_cast<size_t>(pos) == 0 ||
      static_cast<size_t>(pos) > this->storage_m->size()) {
      Rcpp::Rcout << "ParameterVector: Index out of range.\n";
      FIMS_ERROR_LOG(fims::to_string(pos) + "!<" + fims::to_string(this->size()));
      return NULL;
    }
    return Rcpp::wrap(this->storage_m->at(pos - 1));
  }

  /**
   * @brief An internal accessor for calling a position of a ParameterVector
   * from R.
   * @param pos An integer specifying the position of the ParameterVector
   * you want returned. The first position is one and the last position is
   * the same as the size of the ParameterVector.
   */
  Parameter& get(size_t pos) {
    if (pos >= this->storage_m->size()) {
      Rcpp::Rcout << "ParameterVector: Index out of range.\n";
      throw std::invalid_argument("ParameterVector: Index out of range");
    }
    return (this->storage_m->at(pos));
  }

  /**
   * @brief An internal setter for setting a position of a ParameterVector
   * from R.
   * @param pos An integer specifying the position of the ParameterVector
   * you want to set. The first position is one and the last position is the
   * same as the size of the ParameterVector.
   * @param p A numeric value specifying the value to set position `pos` to
   * in the ParameterVector.
   */
  void set(size_t pos, const Parameter& p) {
    this->storage_m->at(pos) = p;
  }

  /**
   * @brief Returns the size of a ParameterVector.
   */
  size_t size() {
    return this->storage_m->size();
  }

  /**
   * @brief Resizes a ParameterVector to the desired length.
   * @param size An integer specifying the desired length for the
   * ParameterVector to be resized to.
   */
  void resize(size_t size) {
    this->storage_m->resize(size);
  }

  /**
   * @brief Sets all Parameters within a ParameterVector as estimable.
   *
   * @param estimable A boolean specifying if all Parameters within the
   * ParameterVector should be estimated within the model. A value of true
   * leads to all Parameters being estimated.
   */
  void set_all_estimable(bool estimable){
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i).estimated_m = estimable;
    }
  }

  /**
   * @brief Sets all Parameters within a ParameterVector as random effects.
   *
   * @param random A boolean specifying if all Parameters within the
   * ParameterVector should be designated as random effects. A value of true
   * leads to all Parameters being random effects.
   */
  void set_all_random(bool random){
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i).is_random_effect_m = random;
    }
  }

  /**
   * @brief Sets the value of all Parameters in the ParameterVector to the
   * provided value.
   *
   * @param value A double specifying the value to set all Parameters to
   * within the ParameterVector.
   */
  void fill(double value){
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i).initial_value_m = value;
    }
  }

  /**
   * @brief Assigns the given values to the minimum value of all elements in
   * the vector.
   *
   * @param value The value to be assigned.
   */
  void fill_min(double value){
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i).min_m = value;
    }
  }

  /**
   * @brief Assigns the given values to the maximum value of all elements in
   * the vector.
   *
   * @param value The value to be assigned.
   */
  void fill_max(double value){
    for (size_t i = 0; i < this->storage_m->size(); i++) {
      storage_m->at(i).max_m = value;
    }
  }

  /**
   * @brief The printing methods for a ParameterVector.
   *
   */
  void show() {
    Rcpp::Rcout << this->storage_m->data() << "\n";

    for (size_t i = 0; i < this->storage_m->size(); i++) {
      Rcpp::Rcout << storage_m->at(i) << "  ";
    }
  }

};
uint32_t ParameterVector::id_g = 0;

/**
 * @brief Output for std::ostream& for a ParameterVector.
 *
 * @param out The stream.
 * @param v A ParameterVector.
 * @return std::ostream& 
 */
std::ostream& operator<<(std::ostream& out, ParameterVector& v) {
  out << "[";
  size_t size = v.size();
  for (size_t i = 0; i < size - 1; i++) {
    out << v[i] << ", ";
  }
  out << v[size - 1] << "]";
  return out;
}

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
  static std::vector<FIMSRcppInterfaceBase *> fims_interface_objects;
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
  virtual void finalize() {
  }

  /**
   * @brief Convert the data to json representation for the output.
   */
  virtual std::string to_json() {
    return "";
  }
};
std::vector<FIMSRcppInterfaceBase *>
  FIMSRcppInterfaceBase::fims_interface_objects;

#endif
