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
  double value_m; /**< initial value of the parameter*/
  double min_m =
      std::numeric_limits<double>::min(); /**< min value of the parameter*/
  double max_m =
      std::numeric_limits<double>::max(); /**< max value of the parameter*/
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
      : value_m(value), min_m(min), max_m(max), estimated_m(estimated) {}

  /**
   * @brief Constructor for initializing Parameter.
   * @details Inputs include value.
   */
  Parameter(double value) { value_m = value; }

  /**
   * @brief Constructor for initializing Parameter.
   * @details Set value to 0 when there is no input value.
   */
  Parameter() { value_m = 0; }
};

/**
 * @brief Rcpp representation of a Parameter vector
 * interface between R and cpp.
 */
class ParameterVector{
    static uint32_t id_g;
   
public:
    Rcpp::List storage_m; //use a list because it's easier
    uint32_t id;
    
    
    /**
     * default constructor
     */
    ParameterVector(){
        this->id = ParameterVector::id_g++;
        Parameter p;
        this->storage_m.push_back(Rcpp::wrap(p));
    }
    /**
     * constructor
     */
    ParameterVector(size_t size ){
        this->id = ParameterVector::id_g++;
        for(size_t i =0; i < size; i++){
            Parameter p;
            this->storage_m.push_back(Rcpp::wrap(p));
        }
    }
    /**
     * vector constructor
     */
    ParameterVector(Rcpp::NumericVector x, size_t size){
        this->id = ParameterVector::id_g++;
        for(size_t i =0; i < size; i++){
            Parameter p = x[i];
            this->storage_m.push_back(Rcpp::wrap(p));
        }
    }
    
    /**
     * Accessor. First index starts is zero.
     */
    inline Parameter operator[](size_t pos) {
        return this->storage_m[pos]; }
    
    /**
     * Accessor. First index is one. For calling from R.
     */
    SEXP at(size_t pos){
        if(pos == 0 || pos > this->storage_m.size()){
            Rcpp::Rcout <<"Index out of range.\n";
            return NULL;
        }
        return this->storage_m[pos-1];
    }
    
    /**
     * returns vector length
     */
    size_t size(){
        return this->storage_m.size();
    }
    
    /**
     * resize to length "size"
     */
    void resize(size_t size){
        size_t n = this->storage_m.size();
        
        if(size > n){
            size_t m = size - n;
            
            for(size_t i = 0; i < m; i++){
                Parameter p;
                this->storage_m.push_back(Rcpp::wrap(p));
            }
        }else if(n > size){
            size_t m = size;
            Rcpp::List l(m);
            for(size_t i = 0; i < m; i++){
                l[i] = this->storage_m[i];
            }
            this->storage_m = l;
        }
        
    }
    
    void set_all_estimable(bool estimable){
        for(size_t i = 0; i < this->storage_m.size(); i++){
            Parameter p = Rcpp::as<Parameter>(this->storage_m[i]);
            p.estimated_m = estimable;
            this->storage_m[i] = Rcpp::wrap(p);
        }
    }

    void set_all_random(bool random){
        for(size_t i = 0; i < this->storage_m.size(); i++){
            Parameter p = Rcpp::as<Parameter>(this->storage_m[i]);
            p.random_m = random;
            this->storage_m[i] = Rcpp::wrap(p);
        }
    }
    
    void fill(double value){
        for(size_t i = 0; i < this->storage_m.size(); i++){
            Parameter p = Rcpp::as<Parameter>(this->storage_m[i]);
            p.value_m = value;
            this->storage_m[i] = Rcpp::wrap(p);
        }
    }
    
};
uint32_t ParameterVector::id_g = 0;



ParameterVector CreateVector(size_t size){
    return ParameterVector(size);
}


/**
 *@brief Base class for all interface objects
 */
class FIMSRcppInterfaceBase {
 public:
  /**< FIMS interface object vectors */
  static std::vector<FIMSRcppInterfaceBase *> fims_interface_objects;

  /** @brief virtual method to inherit to add objects to the TMB model */
  virtual bool add_to_fims_tmb() {
    std::cout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet "
                 "implemented.\n";
    return false;
  }
};
std::vector<FIMSRcppInterfaceBase *>
    FIMSRcppInterfaceBase::fims_interface_objects;

#endif
