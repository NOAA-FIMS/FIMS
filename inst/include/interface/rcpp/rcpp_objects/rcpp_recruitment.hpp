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

/****************************************************************
 * Recruitment Rcpp interface                                   *
 ***************************************************************/

/**
 * @brief RecruitmentInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Recruitment function
 * */
class RecruitmentInterfaceBase : public FIMSRcppInterfaceBase {
public:
    static uint32_t id_g; /**< static id of the recruitment interface base*/
    uint32_t id; /**< id of the recruitment interface base */
    static std::map<uint32_t, RecruitmentInterfaceBase*> live_objects;
    /**< map associating the ids of RecruitmentInterfaceBase to the objects */

    // static std::vector<double> recruit_deviations; /**< vector of recruitment
    // deviations*/
    /// static bool constrain_deviations; /**< whether or not the rec devs are
    /// constrained*/
    // static std::vector<double> rec_bias_adj; /**< a vector of bias adjustment
    // values*/

    RecruitmentInterfaceBase() {
        this->id = RecruitmentInterfaceBase::id_g++;
        RecruitmentInterfaceBase::live_objects[this->id] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    virtual ~RecruitmentInterfaceBase() {
    }

    /** @brief get the ID of the interface base object
     **/
    virtual uint32_t get_id() = 0;

    /** @brief evaluate method for child recruitment interface objects to inherit
     * **/
    virtual double evaluate(double spawners, double ssbzero) = 0;

    /**
     * @brief evaluate recruitment nll
     * 
     * @return double 
     */
    virtual double evaluate_nll() = 0;
};

uint32_t RecruitmentInterfaceBase::id_g = 1;
std::map<uint32_t, RecruitmentInterfaceBase*>
RecruitmentInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Beverton-Holt as an S4 object. To instantiate
 * from R:
 * beverton_holt <- new(fims$beverton_holt)
 */
class BevertonHoltRecruitmentInterface : public RecruitmentInterfaceBase {
public:
    Parameter logit_steep; /**< steepness or the productivity of the stock*/
    Parameter log_rzero; /**< recruitment at unfished biomass */
    Parameter log_sigma_recruit; /**< the log of the stock recruit deviations */
    Rcpp::NumericVector recruit_bias_adjustment; /**<vector bias adjustment*/
    Rcpp::NumericVector deviations; /**< recruitment deviations*/
    bool estimate_deviations = false; /**< boolean describing whether to estimate */
    bool use_bias_correction = false; /**< boolean describing whether to do bias correction */

    BevertonHoltRecruitmentInterface() : RecruitmentInterfaceBase() {
    }

    virtual ~BevertonHoltRecruitmentInterface() {
    }

    virtual uint32_t get_id() {
        return this->id;
    }

    virtual double evaluate(double spawners, double ssbzero) {
        fims::SRBevertonHolt<double> BevHolt;

        BevHolt.logit_steep = this->logit_steep.value;
        BevHolt.log_rzero = this->log_rzero.value;
        return BevHolt.evaluate(spawners, ssbzero);
    }

    virtual double evaluate_nll() {
        fims::SRBevertonHolt<double> NLL;

        NLL.log_sigma_recruit = this->log_sigma_recruit.value;
        NLL.recruit_deviations.resize(
                deviations.size()); // Vector from TMB
        NLL.recruit_bias_adjustment.resize(
                recruit_bias_adjustment.size()); // Vector from TMB
        for (int i = 0; i < deviations.size(); i++) {
            NLL.recruit_deviations[i] = deviations[i];
            NLL.recruit_bias_adjustment[i] = recruit_bias_adjustment[i];
        }
        Rcout << "Rec devs being passed to C++ are " << deviations
                << std::endl;

        Rcout << "Rec bias adj being passed to C++ are " << recruit_bias_adjustment
                << std::endl;

        NLL.use_recruit_bias_adjustment = this->use_bias_correction;
        NLL.estimate_recruit_deviations = this->estimate_deviations;
        // NLL.PrepareConstrainedDeviations();
        // NLL.PrepareBiasAdjustment();
        return NLL.evaluate_nll();
    }

#ifdef TMB_MODEL

    template<typename T>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims::Information<T> > info =
                fims::Information<T>::GetInstance();

        std::shared_ptr<fims::SRBevertonHolt<T> > recruitment =
                std::make_shared<fims::SRBevertonHolt<T> >();

        // set relative info
        recruitment->id = this->id;
        recruitment->logit_steep = this->logit_steep.value;
        if (this->logit_steep.estimated) {
            if (this->logit_steep.is_random_effect) {
                info->RegisterRandomEffect(recruitment->logit_steep);
            } else {
                info->RegisterParameter(recruitment->logit_steep);
            }
        }
        recruitment->log_rzero = this->log_rzero.value;
        if (this->log_rzero.estimated) {
            if (this->log_rzero.is_random_effect) {
                info->RegisterRandomEffect(recruitment->log_rzero);
            } else {
                info->RegisterParameter(recruitment->log_rzero);
            }
        }
        recruitment->log_sigma_recruit = this->log_sigma_recruit.value;
        if (this->log_sigma_recruit.estimated) {
            if (this->log_sigma_recruit.is_random_effect) {
                info->RegisterRandomEffect(recruitment->log_sigma_recruit);
            } else {
                info->RegisterParameter(recruitment->log_sigma_recruit);
            }
        }

        recruitment->recruit_deviations.resize(this->deviations.size());
        if (this->estimate_deviations) {
            for (size_t i = 0; i < recruitment->recruit_deviations.size(); i++) {
                recruitment->recruit_deviations[i] = this->deviations[i];
                info->RegisterParameter(recruitment->recruit_deviations[i]);
            }
        } else {
            for (size_t i = 0; i < recruitment->recruit_deviations.size(); i++) {
                recruitment->recruit_deviations[i] = this->deviations[i];
            }
        }

        recruitment->use_recruit_bias_adjustment = this->use_bias_correction;
        // add to Information
        info->recruitment_models[recruitment->id] = recruitment;

    }

    /** @brief this adds the parameter values and derivatives to the TMB model
     * object */
    virtual bool add_to_fims_tmb() {

        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

        return true;
    }
    
#endif
};

#endif
