 /*
 * File:   rcpp_nll.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_NLL_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_NLL_HPP


#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

 class NLLInterfaceBase : public FIMSRcppInterfaceBase {
    public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  static std::map<uint32_t, NLLInterfaceBase*> live_objects;
  /**< map associating the ids of NLLInterfaceBase to the objects */

  NLLInterfaceBase() {
    this->id = NLLInterfaceBase::id_g++;
    NLLInterfaceBase::live_objects[this->id] = this;
    NLLInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~NLLInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() = 0;

   /** @brief evaluate method for child nll interface objects to inherit **/
  virtual double evaluate_nll() = 0;
};

uint32_t NLLInterfaceBase::id_g = 1;
std::map<uint32_t, NLLInterfaceBase*>
    NLLInterfaceBase::live_objects;

 class RecruitmentNLLInterface : public NLLInterfaceBase {
 public:
  Parameter log_sigma_recruit;   /**< log of the sd recruitment*/
  Rcpp::NumericVector recruit_deviations; /**< vector of recruitment devs*/
  Rcpp::NumericVector recruit_bias_adjustment; /**<vector bias adjustment*/
  bool use_recruit_bias_adjustment;   /**< should the lognormal be bias corrected */

  RecruitmentNLLInterface() : NLLInterfaceBase() {}

  virtual ~RecruitmentNLLInterface() {}

  virtual uint32_t get_id() { return this->id; }

  virtual double evaluate_nll(){
    fims::RecruitmentNLL<double> NLL;

    NLL.log_sigma_recruit = this->log_sigma_recruit.value;
    typedef typename ModelTraits<TMB_FIMS_REAL_TYPE>::EigenVector TMBVector;
    NLL.recruit_deviations = TMBVector(recruit_deviations.size());  // Vector from TMB
    NLL.recruit_bias_adjustment = TMBVector(recruit_bias_adjustment.size());  // Vector from TMB
    for (int i = 0; i < recruit_deviations.size(); i++) {
      NLL.recruit_deviations[i] = recruit_deviations[i];
      NLL.recruit_bias_adjustment[i] = recruit_bias_adjustment[i];
    }
    
    NLL.use_recruit_bias_adjustment = this->use_recruit_bias_adjustment;
    NLL.PrepareConstrainedDeviations();
    NLL.PrepareBiasAdjustment();
    return NLL.evaluate_nll();
  }

   /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    return true;
  }
};
#endif