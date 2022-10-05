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
  /*TODO: 
  Review, add functions to evaluate, push vectors back to fleet (or point to fleet directly?)
*/
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
        const double proportion_female = 0.5; /*!< Sex proportion fixed at 50/50 for M1*/

        // parameters are estimated; after initialize in create_model, push_back to parameter list - in information.hpp (same for initial F in fleet)
        std::vector<Type> log_naa; /*!< estimated parameter: log numbers at age*/
        std::vector<Type> log_Fmort; /*!< estimated parameter: log Fishing mortality*/
        std::vector<Type> log_M; /*!< estimated parameter: log Natural Mortality*/
        std::vector<Type> log_q; /*!< estimated parameter: log catchability*/
        
        //Transformed values
        std::vector<Type> naa; /*!< transformed parameter: numbers at age*/
        std::vector<Type> Fmort; /*!< transformed parameter: Fishing mortality*/
        std::vector<Type> M; /*!< transformed parameter: Natural Mortality*/
        std::vector<Type> q; /*!< transformed parameter: catchability*/
        
        std::vector<Type> ages; /*!< vector of the ages for referencing*/
        std::vector<Type> mortality_M; /*!< vector of natural mortality by year and age*/
        std::vector<Type> mortality_F; /*!< vector of fishing mortality by year and age*/
        std::vector<Type> mortality_Z; /*!< vector of total mortality by year and age*/

        //derived quantities
        std::vector<Type> weight_at_age; /*!< Derived quantity: expected weight at age */
        // fecundity removed because we don't  need it yet?
        std::vector<Type> numbers_at_age; /*!< Derived quantity: population expected numbers at age in each year*/
        std::vector<Type> unfished_numbers_at_age; /*!< Derived quantity: population expected unfished numbers at age in each year*/
        std::vector<Type> catch_at_age;/*!< Derived quantity: catch at age*/
        std::vector<Type> biomass; /*!< Derived quantity: total population biomass in each year*/
        std::vector<Type> spawning_biomass; /*!< Derived quantity: Spawning_biomass*/
        std::vector<Type> unfished_spawning_biomass; /*!< Derived quanity Spawning biomass assuming unfished*/
        std::vector<Type> proportion_mature_at_age; /*!< Derived quantity: Proportion matura at age */
        std::vector<Type> expected_numbers_at_age; /*!< Expected values: Numbers at age (thousands?? millions??) */
        std::vector<Type> catch_numbers_at_age; /*!< Expected values???: Catch in numbers at age*/
        std::vector<Type> catch_weight_at_age; /*!< Expected values???: Weight at age for catch*/
        std::vector<Type> expected_catch; /*!< Expected values: Catch*/
        std::vector<Type> expected_index; /*!< Expected values: Index (CPUE)*/

        ///recruitment
        int recruitment_id = -999; /*!< id of recruitment model object*/
        std::shared_ptr<fims::RecruitmentBase<Type> > recruitment; /*!< shared pointer to recruitment module */

        //growth
        int growth_id = -999; /*!< id of growth model object*/
        std::shared_ptr<fims::GrowthBase<Type> > growth; /*!< shared pointer to growth module */

        //maturity
        int maturity_id = -999; /*!< id of maturity model object*/
        std::shared_ptr<fims::MaturityBase<Type> > maturity; /*!< shared pointer to maturity module */

        //fleet
        int fleet_id = -999; /*!< id of fleet model object*/
        std::vector<std::shared_ptr<fims::Fleet<Type> > > fleets; /*!< shared pointer to fleet module */

        //this -> means you're referring to a class member (member of self)
        
        //std::vector<std::shared_ptr<fims::Fleet<Type>> > surveys;
        Population() {
            this->id = Population::id_g++;

             
        
        }

        //gets called when Initialize() function called in Information - just once at start of model run
        void Initialize(int nyears, int nseasons, int nages) {
          
            
            //size all the vectors to length of nages
            nfleets = fleets.size();
            ages.resize(nages);
            catch_at_age.resize(nages);
            catch_numbers_at_age.resize((nyears+1)*nages*nfleets);
            mortality_F.resize(nyears*nages);
            mortality_Z.resize(nages);
            proportion_mature_at_age.resize((nyears+1)*nages);
            weight_at_age.resize(nages);
            catch_weight_at_age.resize(nyears*nages*nfleets);
            unfished_numbers_at_age.resize((nyears+1)*nages);
            numbers_at_age.resize((nyears+1)*nages);
            expected_catch.resize(nyears*nfleets);
            expected_index.resize(nyears*nfleets);
            biomass.resize((nyears+1));
            unfished_spawning_biomass.resize((nyears+1));
            spawning_biomass.resize((nyears+1));
            log_naa.resize(nages);
            log_Fmort.resize(nfleets*nyears);
            log_M.resize(nyears*nages);
            log_q.resize(nfleets);
            naa.resize(nages);
            Fmort.resize(nfleets*nyears);
            M.resize(nyears*nages);
            q.resize(nfleets);

            
            
        }

        //gets called at each model iteration (used to zero out derived quantities, values summed with +=, etc.)
        void Prepare() {
          std::fill(unfished_spawning_biomass.begin(), unfished_spawning_biomass.end(), 0);
          std::fill(spawning_biomass.begin(), spawning_biomass.end(), 0);
          std::fill(mortality_F.begin(), mortality_F.end(), 0);
          std::fill(expected_catch.begin(), expected_catch.end(), 0);

          //Transformation Section
          for (int age = 0; age <= this->nages; age++) {
            this -> naa[age] = exp(this -> log_naa[age]);
            for(int year = 0; year < this->nyears; year++) {
              int index_ya = year*nages + age;
              this -> M[index_ya] = exp(this -> log_M[index_ya]);
            }
          }

          for(int fleet_ = 0; fleet_ <= this->nfleets; fleet_++) {
            this -> Fmort[fleet_] = exp(this -> log_Fmort[fleet_]);
            for(int year = 0; year < this->nyears; year++) {
              int index_yf = year * nfleets + fleet_;
              this -> q[index_yf] = exp(this -> log_q[index_yf]);
            }
          }
          // call functions to set up recruitment deviations.
          this -> recruitment -> PrepareConstrainedDeviations();
          this -> recruitment -> PrepareBiasAdjustment();
        }

        /**
         * life history calculations
         */
      
        /**
         * @brief Calculates initial numbers at age for index and age
         * 
         * @param index_ya dimension folded index for year and age
         * @param age age index
         */
        inline void CalculateInitialNumbersAA(int index_ya, int a) { //inline all function unless complicated
          this -> numbers_at_age[index_ya] = this -> naa[a];  
        }
        
        /**
         * @brief Calculates mortality at an index, year, and age
         * 
         * @param index_ya dimension folded index for year and age
         * @param year year index
         * @param age age index
         */
        void CalculateMortality(int index_ya, int year, int age) {
          for (int fleet_=0; fleet_ < this -> nfleets; fleet_++) {
            int index_yf = year*nfleets + fleet_; //index by fleet and years to dimension fold
            this -> mortality_F[index_ya] += Fmort[index_yf]* this -> fleets[fleet_].selectivity->evaluate(age);
          }
          this -> mortality_Z[index_ya] = this -> M[index_ya] + this -> mortality_F[index_ya];
        }

        
        /**
         * @brief Calculates numbers at age at year and age specific indices
         * 
         * @param index_ya dimension folded index for year and age
         * @param index_ya2 dimension folded index for year-1 and age-1
         */
        inline void CalculateNumbersAA(int index_ya, int index_ya2) {
          //using Z from previous age/year - is this correct?
          this -> numbers_at_age[index_ya] = 
            this -> numbers_at_age[index_ya2] *
            (exp(- this -> mortality_Z[index_ya2]));
        }

        
        //used to calculate derived values; not currently used in the model
        
        /**
         * @brief Calculates unfished numbers at age at year and age specific indices
         * 
         * @param index_ya dimension folded index for year and age
         * @param index_ya2 dimension folded index for year-1 and age-1
         */
        inline void CalculateUnfishedNumbersAA(int index_ya, int index_ya2) {
          //using M from previous age/year - is this correct?
          this -> unfished_numbers_at_age[index_ya] = 
            this -> unfished_numbers_at_age[index_ya2] *
            (exp(- this -> M[index_ya2]));
        }
        
        void CalculateSpawningBiomass(int index_ya, int year, int age) {
          this -> spawning_biomass[year] += this -> proportion_female * 
            this -> numbers_at_age[index_ya] * 
            this -> proportion_mature_at_age[age] * 
            this -> weight_at_age[age];
        }

        void CalculateUnfishedSpawningBiomass(int index_ya, int year, int age) {
          this -> unfished_spawning_biomass[year] += this -> proportion_female * 
            this -> numbers_at_age[index_ya] * 
            this -> proportion_mature_at_age[age] * 
            this -> weight_at_age[age];

        }

        void CalculateRecruitment(int index_ya, int year) {
          //This phi_zero is currently tied to time varing M so will vary with M 
          //We could also change to reference year 0 instead of y-1 if we need it fixed
          this -> recruitment.phi_zero = 
            this -> unfished_spawning_biomass[year-1]  / 
            this -> recruitment.rzero;

          this -> numbers_at_ages[index_ya] = 
            this -> recruitment -> evaluate(this -> spawning_biomass[year-1]) * this -> recruitment.recruit_deviations[year];
        }

        //calculate expeted total catch by fleet
        void CalculateCatch(int year, int age) {
          for (int fleet_=0; fleet_ < this -> nfleets; fleet_++) {
            int index_yaf = year*this->nages*this->nfleets + age*this->nfleets + fleet_;
            int index_yf = year*this->nfleets + fleet_; //index by fleet and years to dimension fold
            
            this -> expected_catch[index_yf] += 
              this -> catch_weight_at_age[index_yaf];

            fleets[fleet_] -> expected_catch[index_yf] += 
              this -> catch_weight_at_age[index_yaf];
          }
        }

        //calculate the index by fleet
        void CalculateIndex(int index_ya, int year, int age) {
          for (int fleet_=0; fleet_ < this -> nfleets; fleet_++) {
            int index_yaf = year*nages*nfleets + age*nfleets + fleet_;
             //index by fleet and years to dimension fold
            int index_yf = year*nfleets + fleet_;
            // I = qN (N is total numbers), I is an index in numbers
            Type index_;
            
            index_ = this -> q[fleet_] * 
              this -> fleets[fleet_] -> selectivity -> evaluate(age) * 
              this -> numbers_at_age[index_ya] *
              this -> weight_at_age[age];
              
            this -> expected_index[index_yf] += index_;
            fleets[fleet_] -> expected_index[index_yf] += index_;
          }
        }

        //don't need separate function for survey - both survey and fishery treated as 'fleet'
        void CalculateCatchNumbersAA(int index_ya, int year, int age) {
          for (int fleet_=0; fleet_ < this -> nfleets; fleet_++) {
            int index_yaf = year*nages*nfleets + age*nfleets + fleet_;
            int index_yf = year*nfleets + fleet_; //index by fleet and years to dimension fold
            // make an intermediate value in order to set multiple members (of 
            // current and fleet objects) to that value.
            Type catch_; //catch_ is used to avoid using the c++ keyword catch
            //Baranov Catch Equation
            catch_ = 
              (this -> Fmort[index_yf] * 
              this -> fleets[fleet_] -> selectivity -> evaluate(age) ) /
              this -> mortality_Z[index_ya] * 
              this -> numbers_at_ages[index_ya] * 
              (1 - exp(-(this->mortality_Z[index_ya])));
            this -> catch_numbers_at_age[index_yaf] += catch_;
            fleets[fleet_] -> catch_numbers_at_age[index_yaf] += catch_;
          }
        }

        void CalculateCatchWeightAA(int year, int age) {
          for (int fleet_=0; fleet_ < this -> nfleets; fleet_++) {
            int index_yaf = year*nages*nfleets + age*nfleets + fleet_;
            this->catch_weight_at_age[index_yaf] =
              this->catch_numbers_at_age[index_yaf] *
              this -> weight_at_age[age];
          }
        }

        void CalculateMaturityAA(int index_ya, int age) {
          //this->maturity is pointing to the maturity module, which has
          // an evaluate function. -> can be nested.
          this -> proportion_mature_at_age[index_ya] = 
            this -> maturity -> evaluate(age);
        }

        void Evaluate() { // for loop for everything, call functions above.
        
        /*
          Sets derived vectors to zero
          Performs parameters transformations 
          Sets recruitment deviations to mean 0, and then adds bias adjustment.
        */
         Prepare();      
         /*
          start at year=0, age=0; 
          here year 0 is the estimated initial stock structure and age 0 are recruits
          loops start at zero with if statements inside to specify unique code for 
          initial structure and recruitment 0 loops. Could also have started loops at
          1 with initial structure and recruitment setup outside the loops.
         */
         for (int y = 0; y <= this->nyears; y++) {
            for (int a = 0; a < this->nages; a++) {
              /*
               index naming defines the dimensional folding structure 
               i.e. index_ya is referencing folding over years and ages.
              */
              int index_ya = y * nages + a;
              /*
               Mortality rates are not estimated in the final year which is 
               used to show expected stock structure at the end of the model period.
               This is because biomass in year i represents biomass at the start of 
               the year. 
               Should we add complexity to track more values such as start, 
               mid, and end biomass in all years where, start biomass=end biomass of the 
               previous year? 
              */
              if (y < this->nyears) {
                /*
                 First thing we need is total mortality aggregated across all fleets
                 to inform the subsequent catch and change in numbers at age calculations.
                */
                CalculateMortality(index_ya, y, a);
              }
              CalculateMaturityAA(index_ya, a);
              /* if statements needed because some quantities are only needed 
              for the first year and/or age, so these steps are included here. 
              */ 
              if (y == 0) {
                //Initial numbers at age is a user input or estimated parameter vector.
                CalculateInitialNumbersAA(index_ya, a);
                if(a == 0) {
                  this -> unfished_numbers_at_age[index_ya] = 
                    this -> recruitment.rzero;
                } else {
                  CalculateUnfishedNumbersAA(index_ya, a);
                }
                /*
                 Fished and unfished spawning biomass vectors are summing biomass at age
                 across ages to allow calculation of recruitment in the next year.
                */
                CalculateSpawningBiomass(index_ya, y, a);
                CalculateUnfishedSpawningBiomass(index_ya, y, a);
              } else {
                if (a == 0) {
                  //Set the nrecruits for age a=0 year y (use pointers instead of functional returns)
                  //assuming fecundity = 1 and 50:50 sex ratio
                  CalculateRecruitment(index_ya, y); 
                  this -> unfished_numbers_at_age[index_ya] = this -> recruitment.rzero;
                } else {
                  int index_ya2 = (y-1) * nages + (a-1);
                  CalculateNumbersAA(index_ya, index_ya2);
                  CalculateUnfishedNumbersAA(index_ya, index_ya2);
                }
                CalculateSpawningBiomass(index_ya, y, a);
                CalculateUnfishedSpawningBiomass(index_ya, y, a);
              }
              
              if (y < this->nyears) {
                CalculateCatchNumbersAA(index_ya, y, a);
                CalculateCatchWeightAA(y, a);
                CalculateCatch(y, a); 
                CalculateIndex(index_ya, y, a); 
              }
            }
          }
        }
    };
    template <class Type>
    uint32_t Population<Type>::id_g = 0;

} // namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
