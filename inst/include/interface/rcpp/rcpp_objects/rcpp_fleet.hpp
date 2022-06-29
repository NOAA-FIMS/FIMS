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

class FleetInterface {
    int observed_index_data_id = -999;
    int observed_agecomp_data_id = -999;
    int index_likelihood_id = -999;
    int agecomp_likelihood_id = -999;
    int selectivity_id = -999;
public:

    void SetAgeCompLikelihood(int agecomp_likelihood_id) {
        this->agecomp_likelihood_id = agecomp_likelihood_id;
    }

    void SetIndexLikelihood(int index_likelihood_id) {
        this->index_likelihood_id = index_likelihood_id;
    }

    void SetObservedAgeCompData(int observed_agecomp_data_id) {
        this->observed_agecomp_data_id = observed_agecomp_data_id;
    }

    void SetObservedIndexData(int observed_index_data_id) {
        this->observed_index_data_id = observed_index_data_id;
    }

    void SetSelectivity(int selectivity_id) {
        this->selectivity_id = selectivity_id;
    }


};

#endif