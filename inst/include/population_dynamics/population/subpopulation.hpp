/*
 * File:   subpopulation.hpp
 *
 * Author: Matthew Supernaw, Andrea Havron
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov, andrea.havron@noaa.gov
 *
 * Created on September 30, 2021, 1:07 PM
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
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP


#include "../../common/def.hpp"

namespace fims {
    namespace popdy {

        /**
         * Subpopulation class. This class represents a generic partition
         * of a population (eg., sex, area).
         */
        template <typename Type>
        class Subpopulation {
        public:
            SexType sex = fims::UNDIFFERENTIATED;
            uint32_t id_m; /*!< reference id for population object*/
            size_t nyears; /*!< total number of years in the fishery*/
            size_t nseasons; /*!< total number of seasons in the fishery*/
            size_t nages; /*!< total number of ages in the population*/
            size_t nfleets; /*!< total number of fleets in the fishery*/

            // derived quantities
            fims::Vector<Type>
            weight_at_age; /*!< Derived quantity: expected weight at age */
            // fecundity removed because we don't need it yet
            fims::Vector<Type> numbers_at_age; /*!< Derived quantity: subpopulation expected
                                      numbers at age in each year*/
            fims::Vector<Type>
            unfished_numbers_at_age; /*!< Derived quantity: subpopulation expected
                                unfished numbers at age in each year*/
            fims::Vector<Type>
            biomass; /*!< Derived quantity: total subpopulation biomass in each year*/
            fims::Vector<Type> spawning_biomass; /*!< Derived quantity: Spawning_biomass*/
            fims::Vector<Type> unfished_biomass; /*!< Derived quanity
                                        biomass assuming unfished*/
            fims::Vector<Type> unfished_spawning_biomass; /*!< Derived quanity Spawning
                                                 biomass assuming unfished*/
            fims::Vector<Type> proportion_mature_at_age; /*!< Derived quantity: Proportion
                                                mature at age */
            fims::Vector<Type> expected_numbers_at_age; /*!< Expected values: Numbers at
                                                age (thousands?? millions??) */
            fims::Vector<Type> expected_catch; /*!< Expected values: Catch*/
            fims::Vector<Type> expected_recruitment; /*!< Expected recruitment */

            void EvaluatePhase1() {

            }

            void EvaluatePhase2() {

            }


        };

    } // namespace fims
}// namespace popdy
#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP */
