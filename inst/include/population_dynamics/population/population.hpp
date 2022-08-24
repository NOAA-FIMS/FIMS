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
#include "../maturity/maturity.hpp"
#include "../growth/growth.hpp"
#include "../fleet/fleet.hpp"

namespace fims {

    /**
     * @brief Population class. Contains subpopulations
     * that are divided into generic partitions (eg. sex, area).
     */
    template <typename Type>
    struct Population : public FIMSObject<Type> {
        static uint32_t id_g; /*!< reference id for population object*/
        size_t nyears;
        size_t nseasons;
        size_t nages;
        size_t nfleets;

        std::vector<Type> log_naa; // this is estimated; after initialize in create_model, push_back to parameter list - in information.hpp (same for initial F in fleet)
        std::vector<Type> log_F;

        std::vector<Type> ages;
        std::vector<Type> mortality_M;
        std::vector<Type> mortality_F;
        std::vector<Type> mortality_Z;
        std::vector<Type> mature; // Binary mature or not vector

        //derived quantities
        std::vector<Type> initial_numbers;
        std::vector<Type> weight_at_age;
        std::vector<Type> fecundity;
        std::vector<Type> numbers_at_age;
        std::vector<Type> catch_at_age;
        std::vector<Type> biomass;
        std::vector<Type> spawning_biomass;

        ///recruitment
        int recruitment_id = -999; /*!< id of recruitment model object*/
        std::shared_ptr<fims::RecruitmentBase<Type>> recruitment;

        //growth
        int growth_id = -999; /*!< id of growth model object*/
        std::shared_ptr<fims::GrowthBase<Type>> growth;

        //maturity
        int maturity_id = -999; /*!< id of maturity model object*/
        std::shared_ptr<fims::MaturityBase<Type>> maturity;

        //fleet
        int fleet_id = -999; /*!< id of fleet model object*/
        std::vector<std::shared_ptr<fims::Fleet<Type>> > fleets;
        
        //std::vector<std::shared_ptr<fims::Fleet<Type>> > surveys;
        Population() {
            this->id = Population::id_g++;

             
        
        }

        //gets called when Initialize() function called in Information - just once at start of model run
        void Initialize(int nyears, int nseasons, int nages) {
          
            
            //size all the vectors to length of nages
            nfleets = fleets.size();
            ages.resize(nages);
            mortality_M.resize(nages);
            mortality_F.resize(nyears*nages*nfleets);
            mortality_Z.resize(nages);
            mature.resize(nages);
            initial_numbers.resize(nages);
            weight_at_age.resize(nages);
            fecundity.resize(nages);
            numbers_at_age.resize(nyears*nages);
            catch_at_age.resize(nages);
            biomass.resize(nyears);
            spawning_biomass.resize(nyears);
            log_naa.resize(nages);
            log_Fmort.resize(nfleets*nyears);

            
            
        }

        //gets called at each model iteration (used to zero out derived quantities, values summed with +=, etc.)
        void Prepare() {
        }

        /**
         * life history calculations
         */
        //delete std::vector<Type> because already declared; function has access to log_naa
        //exp(this ->log_naa[a]) on line 130 - more explicit that we are reference member
        inline void CalculateInitialNumbersAA(int index, int a) { //inline all function unless complicated
          this -> numbers_at_ages[index] = exp(this -> log_naa[a]);  
        }
        //where should nfleets be defined? initialize or as a member of the class?
        void CalculateMortality(int index, int year) {
          for(int nf; nf < this -> nfleets; nf++){
            // calculate apical F by  
            //How do we reference fleet objects, eg. fleets.Selectivity[index]? 
            this -> F[index] =+ F[year]*fleets.nf.Selectivity[index];
          }
          this -> Z[index] = this -> M[index] + this -> F[index];
        }

        inline void CalculateNumbersAA(int index, int index2) {
          this -> numbers_at_ages[index] = this -> numbers_at_ages[index2]*(1-exp(- this -> Z[index]));
        }

        void CalculateSpawningBiomass() {
        }

        void CalculateRecruitment() {
        }

        void CalculateCatchAA() {
        }

        void CalculateSurveyNumbersAA() {
        }

        void Evaluate() { // for loop for everything, call functions above.
         // In progress, jotting down rough code for the population loop.
         //Set the initial equilibrium numbers at age vector (could be independent or y=0?)
         // This will be set in initial_numbers vector (for year 0, age 0)

        
         //this -> means you're referring to a class member (member of self)
         
         //start at y=0; if y == 0 CalculateInitialNumbersAA else CalculateNAA 
         for (int y = 0; y < this->nyears; y++) {
           
          
           // What is the this pointer doing here???
            CalculateSpawningBiomass();

            //numbers_at_age[(y)*nages] = CalculateRecruitment(y,numbers_at_age,fecundity);
            
            for (int a = 0; a < this->nages; a++) {
              int index = y * nages + a;
              int index2 = (y-1) * nages + a;
              CalculateMortality(index)
              if (y == 0) {
                CalculateInitialNumbersAA(index, a);
              } else {
                if (a == 0) {
                  //Set the nrecruits for age a=0 year y (use pointers instead of functional returns)
                  CalculateRecruitment(y, numbers_at_age -> numbers_at_age[(y-1)*nages], fecundity);
                } else {
                  CalculateNumbersAA(index, index2);
                }
                
              }
                
                
              // mortality_M is fixed, so we may want to set  it in the initialized
              // mortality_F needs to either be a parameter vector or approximated from catch data.
              // where do we tally mortality_F over all fleets?
              CalculateMortality(f -> mortality_Z[index]);
              // fleet loop (move to within the CalculateMortality function body)
              // do we need a separate total F or is total Z fine?
              mortality_Z[index] = mortality_M[index];
              for (int ff = 0; ff < this->nfleets; ff++) {
                sub_index = y*nages + ff*nfleets + ff;
                mortality_Z[index] += fleet_F[sub_index]; 
              }
              
              //calculate numbers at age for the given y and age: CalculateNumbersAA (numbers at age)
              CalculateNumbersAA(numbers_at_age -> numbers_at_age[index], Z -> mortality_Z[a]);
              numbers_at_age[(y)*nages+a] = numbers_at_age[(y-1)*nages+a-1]*(1-exp(-Z[a]));
              
              //TODO: continue calculating. WHAM and SAM could be good refs
            }
         
         }
        }


    };
    template <class Type>
    uint32_t Population<Type>::id_g = 0;

} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
