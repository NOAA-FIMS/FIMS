/*
 * File:   rcpp_recruitment.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP

#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

/**
 * Recruitment Rcpp interface
 */

/**
 * @brief RecruitmentInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Recruitment function
 */
class RecruitmentInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, RecruitmentInterfaceBase*> live_objects;
  /**< map associating the ids of RecruitmentInterfaceBase to the objects */

  // static std::vector<double> log_recruit_devs; /**< vector of log recruitment
  // deviations*/
  // static bool constrain_deviations; /**< whether or not the rec devs are
  // constrained*/

  RecruitmentInterfaceBase() {
    this->id = RecruitmentInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    RecruitmentInterfaceBase */
    RecruitmentInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~RecruitmentInterfaceBase() {}

  /** @brief get the ID of the interface base object
   */
  virtual uint32_t get_id() = 0;

  /** @brief evaluate method for child recruitment interface objects to inherit
   */
  virtual double evaluate(double spawners, double ssbzero) = 0;

};

uint32_t RecruitmentInterfaceBase::id_g = 1;
std::map<uint32_t, RecruitmentInterfaceBase*>
    RecruitmentInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Beverton-Holt as an S4 object. To instantiate
 * from R:
 * beverton_holt <- new(beverton_holt)
 */
class BevertonHoltRecruitmentInterface : public RecruitmentInterfaceBase {
 public:
  ParameterVector logit_steep; /**< steepness or the productivity of the stock*/
  ParameterVector log_rzero; /**< recruitment at unfished biomass */
  ParameterVector log_devs;   /**< log recruitment deviations*/
  bool estimate_log_devs = false; /**< boolean describing whether to estimate */

    double estimated_logit_steep; /**< estimated steepness or the productivity of the stock*/
    double estimated_log_rzero; /**< estimated recruitment at unfished biomass */
    Rcpp::NumericVector estimated_log_devs; /**< estimated log recruitment deviations*/

  BevertonHoltRecruitmentInterface() : RecruitmentInterfaceBase() {}

  virtual ~BevertonHoltRecruitmentInterface() {}

  virtual uint32_t get_id() { return this->id; }

  virtual double evaluate(double spawners, double ssbzero) {
    fims_popdy::SRBevertonHolt<double> BevHolt;
    BevHolt.logit_steep.resize(1);
    BevHolt.logit_steep[0] = this->logit_steep[0].initial_value_m;
    if (this->logit_steep[0].initial_value_m == 1.0) {
      warning(
          "Steepness is subject to a logit transformation, so its value is "
          "0.7848469. Fixing it at 1.0 is not currently possible.");
    }
    BevHolt.log_rzero.resize(1);
    BevHolt.log_rzero[0] = this->log_rzero[0].initial_value_m;

    return BevHolt.evaluate(spawners, ssbzero);
  }

    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
     */
    virtual void finalize() {

        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("Beverton-Holt Recruitment  " + fims::to_string(this->id) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();


        fims_info::Information<double>::recruitment_models_iterator it;

        it = info->recruitment_models.find(this->id);

        if (it == info->recruitment_models.end()) {
            FIMS_WARNING_LOG("Beverton-Holt Recruitment " + fims::to_string(this->id) + " not found in Information.");
            return;
        } else {

            std::shared_ptr<fims_popdy::SRBevertonHolt<double> > recr =
                    std::dynamic_pointer_cast<fims_popdy::SRBevertonHolt<double> >(it->second);

            for (size_t i = 0; i < this->logit_steep.size(); i++) {
                if (this->logit_steep[i].estimated_m) {
                    this->logit_steep[i].final_value_m = recr->logit_steep[i];
                } else {
                    this->logit_steep[i].final_value_m = this->logit_steep[i].initial_value_m;
                }
            }

            for (size_t i = 0; i < log_rzero.size(); i++) {
                if (log_rzero[i].estimated_m) {
                    this->log_rzero[i].final_value_m = recr->log_rzero[i];
                } else {
                    this->log_rzero[i].final_value_m = this->log_rzero[i].initial_value_m;
                }
            }

            for (size_t i = 0; i < this->estimated_log_devs.size(); i++) {
                if (this->log_devs[i].estimated_m) {
                    this->log_devs[i].final_value_m = recr->log_recruit_devs[i];
                } else {
                    this->log_devs[i].final_value_m = this->log_devs[i].initial_value_m;
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
        ss << " \"name\": \"recruitment\",\n";
        ss << " \"type\": \"Beverton-Holt\",\n";
        ss << " \"id\": " << this->id << ",\n";

        ss << " \"parameter\": {\n";
        ss << "  \"name\": \"logit_steep\",\n";
        ss << "  \"id\":" << this->logit_steep.id_m << ",\n";
        ss << "  \"type\": \"vector\",\n";
        ss << "  \"values\":" << this->logit_steep << ",\n},\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"log_rzero\",\n";
        ss << "   \"id\":" << this->log_rzero.id_m << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":" << this->log_rzero << ",\n },\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"log_devs\",\n";
        ss << "   \"id\":" << this->log_devs.id_m << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":" << this->log_devs << ",\n },\n";


        return ss.str();
    }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::SRBevertonHolt<Type> > recruitment =
        std::make_shared<fims_popdy::SRBevertonHolt<Type> >();

    // set relative info
    recruitment->id = this->id;
    //set logit_steep
        recruitment->logit_steep.resize(this->logit_steep.size());
        for (size_t i = 0; i < this->logit_steep.size(); i++) {

            recruitment->logit_steep[i] = this->logit_steep[i].initial_value_m;

            if (this->logit_steep[i].estimated_m) {
                info->RegisterParameterName("logit_steep");
                if (this->logit_steep[i].is_random_effect_m) {
                    info->RegisterRandomEffect(recruitment->logit_steep[i]);
                } else {
                    info->RegisterParameter(recruitment->logit_steep[i]);
                }
            }

        }

        info->variable_map[this->logit_steep.id_m] = &(recruitment)->logit_steep;


        //set log_rzero
        recruitment->log_rzero.resize(this->log_rzero.size());
        for (size_t i = 0; i < this->log_rzero.size(); i++) {

            recruitment->log_rzero[i] = this->log_rzero[i].initial_value_m;

            if (this->log_rzero[i].estimated_m) {
                info->RegisterParameterName("log_rzero");
                if (this->log_rzero[i].is_random_effect_m) {
                    info->RegisterRandomEffect(recruitment->log_rzero[i]);
                } else {
                    info->RegisterParameter(recruitment->log_rzero[i]);
                }
            }
        }

        info->variable_map[this->log_rzero.id_m] = &(recruitment)->log_rzero;

        //set log_recruit_devs
        recruitment->log_recruit_devs.resize(this->log_devs.size());
        for (size_t i = 0; i < this->log_devs.size(); i++) {
            recruitment->log_recruit_devs[i] = this->log_devs[i].initial_value_m;
            if (this->log_devs[i].estimated_m) {
                info->RegisterParameter(recruitment->log_recruit_devs[i]);
            } else {
                recruitment->estimate_log_recruit_devs = false;
            }

        }
    info->variable_map[this->log_devs.id_m] = &(recruitment)->log_recruit_devs;

    // add to Information
    info->recruitment_models[recruitment->id] = recruitment;

    return true;
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    FIMS_INFO_LOG("adding Recruitment object to TMB");
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif
