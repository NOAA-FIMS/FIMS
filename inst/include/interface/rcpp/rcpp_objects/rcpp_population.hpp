/*
 * File:   rcpp_population.hpp
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
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP

#include "rcpp_interface_base.hpp"
#include "../../../population_dynamics/population/population.hpp"

/****************************************************************
 * Selectivity Rcpp interface                                   *
 ***************************************************************/
class population_interface_base : public fims_rcpp_interface_base {
public:
    static uint32_t id_g;
    uint32_t id;
    static std::map<uint32_t, population_interface_base*> live_objects;

    population_interface_base() {
        this->id = population_interface_base::id_g++;
        population_interface_base::live_objects[this->id] = this;
        population_interface_base::fims_interface_objects.push_back(this);
    }

    virtual uint32_t get_id() = 0;

};

uint32_t population_interface_base::id_g = 1;
std::map<uint32_t, population_interface_base* > population_interface_base::live_objects;



#endif