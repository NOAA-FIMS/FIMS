/*
 * File:   population.hpp
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
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_HPP

#include "../../common/model_object.hpp"
#include "subpopulation.hpp"
#include "../recruitment/recruitment.hpp"
#include "../growth/growth.hpp"
#include "../fleet/fleet.hpp"

namespace fims {

    /**
     * Population class. Contains subpopulations
     * that are divided into generic partitions (eg. sex, area).
     */
    template <typename T>
    struct Population : public FIMSObject<T> {
        size_t nyears;
        size_t nseasons;
        size_t nages;
        std::vector<T> ages;

        std::vector<T> mortality;
        std::vector<T> maturity;


        std::shared_ptr<fims::RecruitmentBase> recruitment_model;
        std::shared_ptr<fims::GrowthBase> growth_model;

        std::vector<std::shared_ptr<fims::Fleet> > fleets;
        std::vector<std::shared_ptr<fims::Fleet> > surveys;

        //derived quantities
        std::vector<T> weight_at_age;
        std::vector<T> fecundity;
        std::vector<T> numbers_at_age;
        std::vector<T> catch_at_age;
        std::vector<T> biomass;

        void Initialize() {
        }

        void Prepare() {
        }

        /**
         * life history calculations
         */

        void CalculateMortality() {
        }

        void CalculateNumbersAtAge() {
        }

        void CalculateSpawningBiomass() {
        }

        void CalculateRecruitment() {
        }

        void CalculateCatchAtAge() {
        }

        void CalculateSurveyNumbersAtAge() {
        }

        void Evaluate() {
        }


    };
} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
