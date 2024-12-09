/**
 * @file rcpp_maturity.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP

#include "../../../population_dynamics/maturity/maturity.hpp"
#include "rcpp_interface_base.hpp"

/**
 * maturity Rcpp interface
 */

/**
 * @brief MaturityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible maturity function
 */
class MaturityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, MaturityInterfaceBase*>
      live_objects; /**< map associating the ids of
                                MaturityInterfaceBase to the objects */

  MaturityInterfaceBase() {
    this->id = MaturityInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    MaturityInterfaceBase */
    MaturityInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~MaturityInterfaceBase() {}

  /** @brief get the ID of the interface base object
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief evaluate the function
   *
   */
  virtual double evaluate(double x) = 0;
};

uint32_t MaturityInterfaceBase::id_g = 1;
std::map<uint32_t, MaturityInterfaceBase*> MaturityInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for logistic maturity as an S4 object. To
 * instantiate from R: logistic_maturity <- new(logistic_maturity)
 */
class LogisticMaturityInterface : public MaturityInterfaceBase {
public:
  ParameterVector
      inflection_point; /**< the index value at which the response reaches .5 */
  ParameterVector slope; /**< the width of the curve at the inflection_point */

    LogisticMaturityInterface() : MaturityInterfaceBase() {
    }

  virtual ~LogisticMaturityInterface() {}

  /** @brief returns the id for the logistic maturity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the logistic maturity function
   *   @param x  The independent variable in the logistic function (e.g., age or
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
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
     */
    virtual void finalize() {

        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("Logistic Maturity  " + fims::to_string(this->id) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already


        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();

        fims_info::Information<double>::maturity_models_iterator it;


        //search for maturity in Information
        it = info->maturity_models.find(this->id);
        //if not found, just return
        if (it == info->maturity_models.end()) {
            FIMS_WARNING_LOG("Logistic Maturity " + fims::to_string(this->id) + " not found in Information.");
            return;
        } else {
            std::shared_ptr<fims_popdy::LogisticMaturity<double> > mat =
                    std::dynamic_pointer_cast<fims_popdy::LogisticMaturity<double> >(it->second);

            for (size_t i = 0; i < inflection_point.size(); i++) {
                if (this->inflection_point[i].estimated_m) {
                    this->inflection_point[i].final_value_m = mat->inflection_point[i];
                } else {
                    this->inflection_point[i].final_value_m = this->inflection_point[i].initial_value_m;
                }

            }

            for (size_t i = 0; i < slope.size(); i++) {
                if (this->slope[i].estimated_m) {
                    this->slope[i].final_value_m = mat->slope[i];
                } else {
                    this->slope[i].final_value_m = this->slope[i].initial_value_m;
                }

            }


        }
    }

    /**
     * @brief Convert the data to json representation for the output.
     */
    virtual std::string to_json() {
        std::stringstream ss;
        ss << "\"module\" : {\n";
        ss << " \"name\": \"maturity\",\n";
        ss << " \"type\": \"logistic\",\n";
        ss << " \"id\": " << this->id << ",\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"inflection_point\",\n";
        ss << "   \"id\":" << this->inflection_point.id_m << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":" << this->inflection_point << ",\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"slope\",\n";
        ss << "   \"id\":" << this->slope.id_m << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":" << this->slope << ",\n";


        ss << "}";

        return ss.str();
    }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::LogisticMaturity<Type> > maturity =
        std::make_shared<fims_popdy::LogisticMaturity<Type> >();

    // set relative info
    maturity->id = this->id;
        std::stringstream ss;
        maturity->inflection_point.resize(this->inflection_point.size());
        for (size_t i = 0; i < this->inflection_point.size(); i++) {
            maturity->inflection_point[i] = this->inflection_point[i].initial_value_m;
            if (this->inflection_point[i].estimated_m) {
                ss.str("");
                ss << "maturity.inflection_point." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->inflection_point[i].is_random_effect_m) {
                    info->RegisterRandomEffect(maturity->inflection_point[i]);
                } else {
                    info->RegisterParameter(maturity->inflection_point[i]);
                }
            }
        }

        maturity->slope.resize(this->slope.size());
        for (size_t i = 0; i < this->slope.size(); i++) {
            maturity->slope[i] = this->slope[i].initial_value_m;
            if (this->slope[i].estimated_m) {
                ss.str("");
                ss << "maturity.slope_" << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->slope[i].is_random_effect_m) {
                    info->RegisterRandomEffect(maturity->slope[i]);
                } else {
                    info->RegisterParameter(maturity->slope[i]);
                }
            }
        }

    // add to Information
    info->maturity_models[maturity->id] = maturity;

    return true;
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    FIMS_INFO_LOG("adding Maturity object to TMB");
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif