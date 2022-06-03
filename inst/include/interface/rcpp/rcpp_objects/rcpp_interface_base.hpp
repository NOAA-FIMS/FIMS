#ifndef RCPP_INTERFACE_BASE
#denine RCPP_INTERFACE_BASE

/*
 * File:   rcpp_interface_base.hpp
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


#include <vector>
#include <map>

#include "interface.hpp"
#include "../../../common/def.hpp"
#include "../../../common/information.hpp"


#define RCPP_NO_SUGAR
#include <Rcpp.h>

/**
 * @brief RcppInterface class that defines 
 * the interface between R and C++ for parameter types.
 */
class parameter {
public:
    double value;
    double min = std::numeric_limits<double>::min();
    double max = std::numeric_limits<double>::max();
    bool is_random_effect = false;
    bool estimated = false;

    parameter(double value, double min,
            double max, bool estimated) :
    value(value), min(min), max(max),
    estimated(estimated) {

    }

    parameter(double value) {
        this->value = value;
    }

    parameter() {
        this->value = 0;
    }
};

/**
 *@brief Base class for all interface objects
 */
class fims_rcpp_interface_base {
public:
    static std::vector<fims_rcpp_interface_base*> fims_interface_objects;

    virtual bool add_to_fims_tmb() {
        std::cout << "fims_rcpp_interface_base::add_to_fims_tmb(): Not yet implemented.\n";
        return false;
    }
};
std::vector<fims_rcpp_interface_base*> fims_rcpp_interface_base::fims_interface_objects;


#endif
