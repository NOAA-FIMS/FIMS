/*
 * File:   rcpp_fleet.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on May 31, 2022 at 12:04 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained by
 * using this  software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express or
 * implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "../../../population_dynamics/fleet/fleet.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface for Fleet as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Fleet)
 * 
 */
class FleetInterface : public FIMSRcppInterfaceBase
{
    int observed_index_data_id = -999; /*!< id of observed index data object*/
    int observed_agecomp_data_id = -999; /*!< id of observed agecomp data object*/
    int index_likelihood_id = -999; /*!< id of index likelihood component*/
    int agecomp_likelihood_id = -999; /*!< id of agecomp likelihood component*/
    int selectivity_id = -999; /*!< id of selectivity component*/

public:
    static uint32_t id_g; /**< static id of the FleetInterface object */
    uint32_t id; /**< local id of the FleetInterface object */

    FleetInterface()
    {
        this->id = FleetInterface::id_g++;
    }

    /**
     * @brief Set the unique id for the Age Comp Likelihood object
     * 
     * @param agecomp_likelihood_id Unique id for the Age Comp Likelihood object
     */
    void SetAgeCompLikelihood(int agecomp_likelihood_id)
    {

        // Check if agecom likelihood has been set already
        if (this->agecomp_likelihood_id != -999)
        {
            warning("Age composition likelihood has been set already.");
            Rcout << "Now you are resetting age composition likelihood with age composition likelihood ID of "
                  << agecomp_likelihood_id << std::endl;
        }

        this->agecomp_likelihood_id = agecomp_likelihood_id;
    }

    /**
     * @brief Set the unique id for the Index Likelihood object
     * 
     * @param index_likelihood_id Unique id for the Index Likelihood object
     */
    void SetIndexLikelihood(int index_likelihood_id)
    {

        // Check if index likelihood has been set already
        if (this->index_likelihood_id != -999)
        {
            warning("Index likelihood has been set already.");
            Rcout << "Now you are resetting index likelihood with index likelihood ID of "
                  << index_likelihood_id << std::endl;
        }

        this->index_likelihood_id = index_likelihood_id;
    }

    /**
     * @brief Set the unique id for the Observed Age Comp Data object
     * 
     * @param observed_agecomp_data_id Unique id for the Observed Age Comp Data object
     */
    void SetObservedAgeCompData(int observed_agecomp_data_id)
    {

        // Check if observed age composition data have been set already
        if (this->observed_agecomp_data_id != -999)
        {
            warning("Observed age composition data have been set already.");
            Rcout << "Now you are resetting observed age composition data with observed age composition ID of "
                  << observed_agecomp_data_id << std::endl;
        }

        this->observed_agecomp_data_id = observed_agecomp_data_id;
    }

    /**
     * @brief Set the unique id for the Observed Index Data object
     * 
     * @param observed_index_data_id Unique id for the Observed Index Data object
     */
    void SetObservedIndexData(int observed_index_data_id)
    {

        // Check if observed index data have been set already
        if (this->observed_index_data_id != -999)
        {
            warning("Observed index data have been set already.");
            Rcout << "Now you are resetting observed index data with observed index data ID of "
                  << observed_index_data_id << std::endl;
        }

        this->observed_index_data_id = observed_index_data_id;
    }

    /**
     * @brief Set the unique id for the Selectivity object
     * 
     * @param selectivity_id Unique id for the Selectivity object
     */
    void SetSelectivity(int selectivity_id)
    {

        // Check if selectivity has been set already
        if (this->selectivity_id != -999)
        {

            warning("Selectivity has been set already.");
            Rcout << "Now you are resetting selectivity with selectivity ID of "
                  << selectivity_id << std::endl;
        }

        this->selectivity_id = selectivity_id;
    }

    /** @brief this adds the values to the TMB model object */
    virtual bool add_to_fims_tmb()
    {

        // base model
        std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
            fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

        std::shared_ptr<fims::Fleet<TMB_FIMS_REAL_TYPE>> f0 =
            std::make_shared<fims::Fleet<TMB_FIMS_REAL_TYPE>>();

        // set relative info
        f0->id = this->id;
        f0->agecomp_likelihood_id = this->agecomp_likelihood_id;
        f0->index_likelihood_id = this->index_likelihood_id;
        f0->observed_agecomp_data_id = this->observed_agecomp_data_id;
        f0->observed_index_data_id = this->observed_index_data_id;
        f0->selectivity_id = this->selectivity_id;

        // add to Information
        d0->fleets[f0->id] = f0;

        // 1st derivative model
        std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
            fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

        std::shared_ptr<fims::Fleet<TMB_FIMS_FIRST_ORDER>> f1 =
            std::make_shared<fims::Fleet<TMB_FIMS_FIRST_ORDER>>();

        f1->id = this->id;
        f1->agecomp_likelihood_id = this->agecomp_likelihood_id;
        f1->index_likelihood_id = this->index_likelihood_id;
        f1->observed_agecomp_data_id = this->observed_agecomp_data_id;
        f1->observed_index_data_id = this->observed_index_data_id;
        f1->selectivity_id = this->selectivity_id;

        // add to Information
        d1->fleets[f1->id] = f1;

        // 2nd derivative model
        std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
            fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

        std::shared_ptr<fims::Fleet<TMB_FIMS_SECOND_ORDER>> f2 =
            std::make_shared<fims::Fleet<TMB_FIMS_SECOND_ORDER>>();

        f2->id = this->id;
        f2->agecomp_likelihood_id = this->agecomp_likelihood_id;
        f2->index_likelihood_id = this->index_likelihood_id;
        f2->observed_agecomp_data_id = this->observed_agecomp_data_id;
        f2->observed_index_data_id = this->observed_index_data_id;
        f2->selectivity_id = this->selectivity_id;

        // add to Information
        d2->fleets[f2->id] = f2;

        // 3rd derivative model
        std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
            fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

        std::shared_ptr<fims::Fleet<TMB_FIMS_THIRD_ORDER>> f3 =
            std::make_shared<fims::Fleet<TMB_FIMS_THIRD_ORDER>>();

        f3->id = this->id;
        f3->agecomp_likelihood_id = this->agecomp_likelihood_id;
        f3->index_likelihood_id = this->index_likelihood_id;
        f3->observed_agecomp_data_id = this->observed_agecomp_data_id;
        f3->observed_index_data_id = this->observed_index_data_id;
        f3->selectivity_id = this->selectivity_id;

        // add to Information
        d3->fleets[f3->id] = f3;
    }
};

uint32_t FleetInterface::id_g = 1;

#endif