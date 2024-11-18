/*
 * File:   rcpp_interface_base.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 *
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
 * @brief RcppInterface class that defines
 * the interface between R and C++ for parameter types.
 */
class Parameter {
 public:
 static uint32_t id_g; /**< global id of the parameter */
    uint32_t id_m; /**< id of the parameter */
  double initial_value_m = 0.0; /**< initial value of the parameter */
  double final_value_m = 0.0; /**< final value of the parameter */
  double min_m =
      -std::numeric_limits<double>::infinity(); /**< min value of the parameter; default is negative infinity*/
  double max_m =
      std::numeric_limits<double>::infinity(); /**< max value of the parameter; default is positive infinity*/
  bool is_random_effect_m = false;        /**< Is the parameter a random effect
                                           parameter? Default value is false.*/
  bool estimated_m =
      false; /**< Is the parameter estimated? Default value is false.*/

  bool random_m =
    false; /**< is the parameter random? Default value is false.*/

  /**
   * @brief Constructor for initializing Parameter.
   * @details Inputs include value, min, max, estimated.
   */
  Parameter(double value, double min, double max, bool estimated)
      : id_m(Parameter::id_g++), initial_value_m(value), min_m(min), max_m(max), estimated_m(estimated) {}

  /**
   * @brief Constructor for initializing Parameter.
   */
    Parameter(const Parameter& other) :
    id_m(other.id_m), initial_value_m(other.initial_value_m),
    final_value_m(other.final_value_m),
    min_m(other.min_m), max_m(other.max_m),
    is_random_effect_m(other.is_random_effect_m),
    estimated_m(other.estimated_m),
    random_m(other.random_m) {
    }

  /**
   * @brief Constructor for initializing Parameter.
   */
    Parameter& operator=(const Parameter& right) {
        // Check for self-assignment!
        if (this == &right) // Same object?
            return *this; // Yes, so skip assignment, and just return *this.
        this->id_m = right.id_m;
        this->initial_value_m = right.initial_value_m;
        this->estimated_m = right.estimated_m;
        this->random_m = right.random_m;
        this->min_m = right.min_m;
        this->max_m = right.max_m;
        this->is_random_effect_m = right.is_random_effect_m;
        return *this;
    }

   

    /**
     * @brief Constructor for initializing Parameter.
     * @details Inputs include value.
     */
    Parameter(double value) {
        initial_value_m = value;
        id_m = Parameter::id_g++;
    }

  /**
   * @brief Constructor for initializing Parameter.
   * @details Set value to 0 when there is no input value.
   */
  Parameter() {
    initial_value_m = 0;
    id_m = Parameter::id_g++;}
};

uint32_t Parameter::id_g = 0;

std::ostream& operator<<(std::ostream& out, const Parameter& p) {
    out << "Parameter:{" << "id:" << p.id_m << ",\nvalue:" << p.initial_value_m
            << ",\nestimated_value:" << p.final_value_m << ",\nmin:"
            << p.min_m << ",\nmax:" << p.max_m << ",\nestimated:" << p.estimated_m << "\n}";
    return out;
}

/**
 * @brief Rcpp representation of a Parameter vector
 * interface between R and cpp.
 */
class ParameterVector{
public:
    static uint32_t id_g; /**< global identifier*/
    std::shared_ptr<std::vector<Parameter> > storage_m; /**< parameter storage*/
    uint32_t id_m; /**< unique identifier*/

    /**
     *  @brief default constructor
     */
    ParameterVector(){
        this->id_m = ParameterVector::id_g++;
        this->storage_m = std::make_shared<std::vector<Parameter> >();
        //        Parameter p;
        this->storage_m->resize(1); //push_back(Rcpp::wrap(p));
    }


    /**
     *  @brief vector constructor
     */
    ParameterVector(const ParameterVector& other) :
    storage_m(other.storage_m), id_m(other.id_m) {
    }

    /**
     *  @brief vector constructor
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
     *  @brief vector constructor
     *  @param x numeric vector
     *  @param size number of elements to copy over
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
     *  @brief vector constructor
     *  @param v A vector of doubles.
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
     * @brief get the ID of the interface base object
     */
    virtual uint32_t get_id() { return this->id_m; }

    /**
     *  @brief Accessor. First index starts is zero.
     *  @param pos return a Parameter at position "pos".
     */
    inline Parameter& operator[](size_t pos) {
        return this->storage_m->at(pos); }

    /**
     *  @brief Accessor. First index is one. For calling from R.
     *  @param pos return a Parameter at position "pos".
     */
    SEXP at(R_xlen_t pos){
        if (static_cast<size_t>(pos) == 0 ||
                static_cast<size_t>(pos)  > this->storage_m->size()) {
            Rcpp::Rcout << "ParameterVector: Index out of range.\n";
            FIMS_ERROR_LOG(fims::to_string(pos) + "!<" + fims::to_string(this->size()));
            return NULL;
        }
        return Rcpp::wrap(this->storage_m->at(pos - 1));
    }

    /**
     *  @brief Internal accessor. First index is one. For calling from R.
     *  @param pos return a Parameter at position "pos".
     */
    Parameter& get(size_t pos) {
        if (pos >= this->storage_m->size()) {
            std::cout << "ParameterVector: Index out of range.\n";
            throw std::invalid_argument("ParameterVector: Index out of range");
        }
        return (this->storage_m->at(pos));
    }

    /**
     *  @brief Internal setter.
     *  @param pos The position
     *  @param p The parameter
     */
    void set(size_t pos, const Parameter& p) {
        this->storage_m->at(pos) = p;
    }

    /**
     *  @brief returns vector length
     */
    size_t size() {
        return this->storage_m->size();
    }

    /**
     *  @brief resize to length "size"
     *  @param size new length of vector to be resized
     */
    void resize(size_t size) {
        this->storage_m->resize(size);
    }

    /**
     * @brief Sets all parameters within a vector as estimable
     *
     * @param estimable Boolean; if true, all parameters are set to be estimated in the model
     */
    void set_all_estimable(bool estimable){
        Rcpp::Rcout << this->storage_m->data() << "\n";
        for (size_t i = 0; i < this->storage_m->size(); i++) {
            storage_m->at(i).estimated_m = estimable;
        }
    }

    /**
     * @brief Sets all parameters within a vector as random
     *
     * @param random Boolean; if true, all parameters are set to be random effects in the model
     */
    void set_all_random(bool random){
        for (size_t i = 0; i < this->storage_m->size(); i++) {
            storage_m->at(i).random_m = random;
        }
    }

    /**
     * @brief Assigns the given values to all elements in the vector
     *
     * @param value The value to be assigned
     */
    void fill(double value){
        for (size_t i = 0; i < this->storage_m->size(); i++) {
            storage_m->at(i).initial_value_m = value;
        }
    }

    /**
     * @brief Assigns the given values to the minimum value of all elements in the vector
     *
     * @param value The value to be assigned
     */
    void fill_min(double value){
        for (size_t i = 0; i < this->storage_m->size(); i++) {
            storage_m->at(i).min_m = value;
        }
    }

    /**
     * @brief Assigns the given values to the maximum value of all elements in the vector
     *
     * @param value The value to be assigned
     */
    void fill_max(double value){
        for (size_t i = 0; i < this->storage_m->size(); i++) {
            storage_m->at(i).max_m = value;
        }
    }

    /**
     * @brief Printing methods for ParameterVector
     *
     */
    void show() {
        std::cout << this->storage_m->data() << "\n";

        for (size_t i = 0; i < this->storage_m->size(); i++) {
            std::cout << storage_m->at(i) << "  ";
        }
    }

};
uint32_t ParameterVector::id_g = 0;

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
 *@brief Base class for all interface objects
 */
class FIMSRcppInterfaceBase {
public:

    bool finalized = false; /**< If already finalized */

    static std::vector<FIMSRcppInterfaceBase *>
    fims_interface_objects; /**< FIMS interface object vectors */

  /** @brief virtual method to inherit to add objects to the TMB model */
  virtual bool add_to_fims_tmb() {
    std::cout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet "
                 "implemented.\n";
    return false;
  }
    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
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
