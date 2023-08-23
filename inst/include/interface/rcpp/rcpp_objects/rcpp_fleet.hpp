/*
 * File:   rcpp_fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "../../../population_dynamics/fleet/fleet.hpp"
#include "../../../population_dynamics/fleet/fleet_nll.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../common/def.hpp"

/**
 * @brief Rcpp interface for Fleet as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Fleet)
 *
 */
class FleetInterface : public FIMSRcppInterfaceBase {
    int agecomp_likelihood_id = -999; /*!< id of agecomp likelihood component*/
    int index_likelihood_id = -999; /*!< id of index likelihood component*/
    int observed_agecomp_data_id = -999; /*!< id of observed agecomp data object*/
    int observed_index_data_id = -999; /*!< id of observed index data object*/
    int selectivity_id = -999; /*!< id of selectivity component*/

public:
    bool is_survey = false; /*!< whether this is a survey fleet */
    int nages; /*!< number of ages in the fleet data*/
    int nyears; /*!< number of years in the fleet data */
    double log_q; /*!< log of catchability for the fleet*/
    Rcpp::NumericVector
    log_Fmort; /*!< log of fishing mortality rate for the fleet*/
    bool estimate_F = false; /*!< whether the parameter F should be estimated*/
    bool estimate_q = false; /*!< whether the parameter q should be estimated*/
    bool random_q = false; /*!< whether q should be a random effect*/
    bool random_F = false; /*!< whether F should be a random effect*/
    Parameter log_obs_error; /*!< the log of the observation error */

public:
    static uint32_t id_g; /**< static id of the FleetInterface object */
    uint32_t id; /**< local id of the FleetInterface object */

    FleetInterface() {
        this->id = FleetInterface::id_g++;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    virtual ~FleetInterface() {
    }

    /**
     * @brief Set the unique id for the Age Comp Likelihood object
     *
     * @param agecomp_likelihood_id Unique id for the Age Comp Likelihood object
     */
    void SetAgeCompLikelihood(int agecomp_likelihood_id) {
        this->agecomp_likelihood_id = agecomp_likelihood_id;
    }

    /**
     * @brief Set the unique id for the Index Likelihood object
     *
     * @param index_likelihood_id Unique id for the Index Likelihood object
     */
    void SetIndexLikelihood(int index_likelihood_id) {

        this->index_likelihood_id = index_likelihood_id;
    }

    /**
     * @brief Set the unique id for the Observed Age Comp Data object
     *
     * @param observed_agecomp_data_id Unique id for the Observed Age Comp Data
     * object
     */
    void SetObservedAgeCompData(int observed_agecomp_data_id) {
        this->observed_agecomp_data_id = observed_agecomp_data_id;
    }

    /**
     * @brief Set the unique id for the Observed Index Data object
     *
     * @param observed_index_data_id Unique id for the Observed Index Data object
     */
    void SetObservedIndexData(int observed_index_data_id) {
        this->observed_index_data_id = observed_index_data_id;
    }

    /**
     * @brief Set the unique id for the Selectivity object
     *
     * @param selectivity_id Unique id for the Selectivity object
     */
    void SetSelectivity(int selectivity_id) {
        this->selectivity_id = selectivity_id;
    }

#ifdef TMB_MODEL

    template<typename T>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims::Information < T>> info =
                fims::Information<T>::GetInstance();

        std::shared_ptr<fims::Fleet < T>> f =
                std::make_shared<fims::Fleet < T >> ();

        // set relative info
        f->id = this->id;
        f->is_survey = this->is_survey;
        f->nages = this->nages;
        f->nyears = this-> nyears;
        f->agecomp_likelihood_id = this->agecomp_likelihood_id;
        f->index_likelihood_id = this->index_likelihood_id;
        f->observed_agecomp_data_id = this->observed_agecomp_data_id;
        f->observed_index_data_id = this->observed_index_data_id;
        f->selectivity_id = this->selectivity_id;

        f->log_obs_error = this->log_obs_error.value;
        if (this->log_obs_error.estimated) {
            info->RegisterParameter(f->log_obs_error);
        }
        f->log_q = this->log_q;
        if (this->estimate_q) {
            if (this->random_q) {
                info->RegisterRandomEffect(f->log_q);
            } else {
                info->RegisterParameter(f->log_q);
            }
        }

        f->log_Fmort.resize(this->log_Fmort.size());
        for (int i = 0; i < log_Fmort.size(); i++) {
            f->log_Fmort[i] = this->log_Fmort[i];

            if (this->estimate_F) {
                if (this->random_F) {
                    info->RegisterRandomEffect(f->log_Fmort[i]);
                } else {
                    info->RegisterParameter(f->log_Fmort[i]);
                }
            }
        }
        // add to Information
        info->fleets[f->id] = f;
        return true;
    }

    /** @brief this adds the values to the TMB model object */
    virtual bool add_to_fims_tmb() {
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

        return true;
    }
    
#endif
};

uint32_t FleetInterface::id_g = 1;

#endif
