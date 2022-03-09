/*
 * File:   information.hpp
 *
 * Author: Matthew Supernaw, Andrea Havron
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov, andrea.havron@noaa.gov
 *
 * Created on September 30, 2021, 11:47 PM
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
#ifndef FIMS_COMMON_INFORMATION_HPP
#define FIMS_COMMON_INFORMATION_HPP

#include <map>
#include <memory>
#include <vector>
#include "model_object.hpp"
#include "../interface/interface.hpp"


namespace fims {

/**
 * Contains all objects and data pre-model construction
 */
template<typename T>
class Information {
    public:
    typedef fims::FIMSTraits<T>::VariableT variable_t;
    typedef fims::FIMSTraits<T>::RealT real_t;
    std::vector<variable_t*> parameters; //list of all estimated parameters
    std::vector<variable_t*> random_effects_parameters; //list of all random effects parameters
    std::vector<variable_t*> fixed_effects_parameters; //list of all fixed effects parameters

/**
 * @brief Create a Model object
 * 
 * @return true if sucessful
 * @return false on failure
 */
    bool CreateModel(){

    }

};
}



#endif /* FIMS_COMMON_INFORMATION_HPP */

