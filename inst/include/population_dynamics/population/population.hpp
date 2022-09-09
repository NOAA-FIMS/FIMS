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
        size_t nyears; /*!< total number of years in the fishery*/
        size_t nseasons; /*!< total number of seasons in the fishery*/
        size_t nages; /*!< total number of ages in the population*/
        size_t nfleets; /*!< total number of fleets in the fishery*/
        //constants
        const double proportion_female = 0.5;

// parameters are estimated; after initialize in create_model, push_back to parameter list - in information.hpp (same for initial F in fleet)
        std::vector<Type> log_naa; 
        std::vector<Type> log_Fmort;
        std::vector<Type> log_M;

        std::vector<Type> ages;
        std::vector<Type> mortality_M;
        std::vector<Type> mortality_F;
        std::vector<Type> mortality_Z;

        //derived quantities
        std::vector<Type> initial_numbers;
        std::vector<Type> weight_at_age;
        // fecundity removed because we don't  need it yet?
        std::vector<Type> numbers_at_age;
        std::vector<Type> unfished_numbers_at_age;
        std::vector<Type> catch_at_age;
        std::vector<Type> biomass;
        std::vector<Type> spawning_biomass;
        std::vector<Type> unfished_spawning_biomass;
        std::vector<Type> proportion_mature_at_age;
        std::vector<Type> expected_numbers_at_age;

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
            mortality_F.resize(nyears*nages);
            mortality_Z.resize(nages);
            proportion_mature_at_age.resize((nyears+1)*nages);
            initial_numbers.resize(nages);
            weight_at_age.resize(nages);
            unfished_numbers_at_age.resize((nyears+1)*nages);
            numbers_at_age.resize((nyears+1)*nages);
            expected_numbers_at_age.resize((nyears+1)*nages*nfleets);
            catch_at_age.resize(nages);
            biomass.resize((nyears+1));
            unfished_spawning_biomass.resize((nyears+1));
            spawning_biomass.resize((nyears+1));
            log_naa.resize(nages);
            log_Fmort.resize(nfleets*nyears);
            log_M.resize(nyears*nages);

            
            
        }

        //gets called at each model iteration (used to zero out derived quantities, values summed with +=, etc.)
        void Prepare() {
          std::fill(unfished_spawning_biomass.begin(), unfished_spawning_biomass.end(), 0);
          std::fill(spawning_biomass.begin(), spawning_biomass.end(), 0);
          std::fill(mortality_F.begin(), mortality_F.end(), 0);

        }

        /**
         * life history calculations
         */
        //delete std::vector<Type> because already declared; function has access to log_naa
        //exp(this ->log_naa[a]) on line 130 - more explicit that we are reference member
        inline void CalculateInitialNumbersAA(int index, int a) { //inline all function unless complicated
          this -> numbers_at_ages[index] = exp(this -> log_naa[a]);  
        }
        
        /**
        * Calculates mortality at an index, year, and age
        */
        void CalculateMortality(int index, int year, int age) {
          for (int nf=0; nf < this -> nfleets; nf++) {
            int index2 = year*nfleets + nf //index by fleet and years to dimension fold
            this -> mortality_F[index] += exp(log_Fmort[index2])*fleets[nf].selectivity->evaluate(age);
          }
          this -> mortality_Z[index] = this -> exp(log_M[index]) + this -> mortality_F[index];
        }

        inline void CalculateNumbersAA(int index, int index2) {
          //using Z from previous age/year - is this correct?
          this -> numbers_at_ages[index] = this -> numbers_at_ages[index2]*(exp(- this -> mortality_Z[index2]));
        }

        inline void CalculateUnfishedNumbersAA(int index, int index2, int age) {
          //using M from previous age/year - is this correct?
          this -> unfished_numbers_at_age[age] = this -> unfished_numbers_at_age[age-1]*(exp(- this -> exp(log_M[index2])));
        }
        
        void CalculateSpawningBiomass(int index, int year, int age) {
          this -> spawning_biomass[year] += this -> proportion_female * 
            this -> numbers_at_age[index] * this -> proportion_mature_at_age[age] * 
            this -> weight_at_age[age];

        }

        void CalculateUnfishedSpawningBiomass(int index, int year, int age) {
          this -> unfished_spawning_biomass[year] += this -> proportion_female * 
            this -> numbers_at_age[index] * this -> proportion_mature_at_age[age] * 
            this -> weight_at_age[age];

        }

        void CalculateRecruitment(int index, int year) {
          //This phi_zero is currently tied to time varing M so will vary with M 
          //We could also change to reference year 0 instead of y-1 if we need it fixed
          this -> recruitment.phi_zero = unfished_spawning_biomass[y-1]  / this -> recruitment.rzero;
          this -> numbers_at_ages[index] = this -> recruitment -> evaluate(this -> spawning_biomass[year-1])
        }

        void CalculateCatchAA() {
          
        }

        //don't need separate function for survey - both survey and fishery treated as 'fleet'
        void CalculateExpectedNumbersAA(int index, int year, int age) {
          for (int nf=0; nf < this -> nfleets; nf++) {
            int index3 = year*nages*nfleets + age*nfleets + nf;
            int index2 = year*nfleets + nf //index by fleet and years to dimension fold
            //Baranov Catch Equation
            expected_numbers_at_age[index3] = ( (exp(this -> log_Fmort[index2]) * fleets[nf].selectivity->evaluate(age) )/
              this -> mortality_Z[index] ) * this -> numbers_at_ages[index] * ( 1 - exp(- this -> mortality_Z[index]) );;
          }
        }

        void CalculateMaturityAA(int index, int age) {
          //this->maturity is pointing to the maturity module, which has
          // an evaluate function. -> can be nested.
          proportion_mature_at_age[index] = this -> maturity -> evaluate(age);
        }

        void Evaluate() { // for loop for everything, call functions above.
        
         //this -> means you're referring to a class member (member of self)
         
         //start at year=0, age=0; 
         for (int y = 0; y <= this->nyears; y++) {
            for (int a = 0; a < this->nages; a++) {
              int index = y * nages + a;
              if(y < this->nyears){
                CalculateMortality(index, y, a);
                
              }
              CalculateMaturityAA(index, a);
              // if y == 0 CalculateInitialNumbersAA else CalculateNAA 
              if (y == 0) {
                CalculateInitialNumbersAA(index, a);
                if(a == 0) {
                  this -> unfished_numbers_at_age[index] = this -> recruitment.rzero;
                } else {
                  CalculateUnfishedNumbersAA(index, a);
                }
                CalculateSpawningBiomass(index, y, a)
                CalculateUnfishedSpawningBiomass(index, y, a)
              } else {
                if (a == 0) {
                  //Set the nrecruits for age a=0 year y (use pointers instead of functional returns)
                  //assuming fecundity = 1 and 50:50 sex ratio
                  CalculateRecruitment(index, y); 
                  this -> unfished_numbers_at_age[index] = this -> recruitment.rzero;
                } else {
                  int index2 = (y-1) * nages + (a-1);
                  CalculateNumbersAA(index, index2);
                  CalculateUnfishedNumbersAA(index, a);
                }
                CalculateSpawningBiomass(index, y, a)
                CalculateUnfishedSpawningBiomass(index, y, a)
              }
              if(y < this->nyears){
                CalculateExpectedNumbersAA(index, y, a);
              }
            }
          }
        }
    };
    template <class Type>
    uint32_t Population<Type>::id_g = 0;

} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
