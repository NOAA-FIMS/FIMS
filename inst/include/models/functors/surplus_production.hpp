#ifndef FIMS_MODELS_SURPLUS_PRODUCTION_HPP
#define FIMS_MODELS_SURPLUS_PRODUCTION_HPP

#include <set>
#include <regex>
#include "fishery_model_base.hpp"

namespace fims_popdy {

    template<typename Type>
    class SurplusProduction : public FisheryModelBase<Type> {

    public:
        size_t nyears = 0; //max of all populations
        size_t nages = 0; //max of all populations
        std::set<uint32_t> population_ids;
        std::vector<std::shared_ptr<fims_popdy::Population<Type> > > populations;

        SurplusProduction() : fims_popdy::FisheryModelBase<Type>() {
        }

        virtual void Initialize() {
            this->nyears = 0;
            this->nages = 0;
            
            for (size_t i = 0; i < this->populations.size(); i++) {
                this->nyears = std::max(this->nyears, this->populations[i]->nyears);
                this->nages = std::max(this->nages, this->populations[i]->nages);

                this->populations[i]->derived_quantities["biomass"] =
                        fims::Vector<Type>((this->populations[i]->nyears + 1));

                this->populations[i]->derived_quantities["expected_depletion"] =
                        fims::Vector<Type>(this->populations[i]->nyears *
                        this->populations[i]->nfleets);
                
                this->populations[i]->derived_quantities["observed_catch"] =
                        fims::Vector<Type>(this->populations[i]->nyears *
                        this->populations[i]->nfleets);
                
            }
        }

        void Prepare() {
            for (size_t p = 0; p < this->populations.size(); p++) {
                populations[p] ->Prepare();
            }
        }

        void AddPopulation(uint32_t id) {
            this->population_ids.insert(id);
        }

        std::set<uint32_t>& GetPopulationIds() {
            return population_ids;
        }

        std::vector<std::shared_ptr<fims_popdy::Population<Type> > >& GetPopulations() {
            return populations;
        }


        void CalculateCatch(size_t year) {
            for (size_t p = 0; p < this->populations.size(); p++) {

                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {
                        size_t index_yf = year * this->populations[p]->nfleets +
                                fleet_; // index by fleet and years to dimension fold

                                //MS: where does catch get calculated? this should be in the derived quantities map
                                //AMH: catch is data in a SP model. After cpue branch is merged in, there will be a catch object in fleet for catch data 
                        // this->populations[p]->derived_quantities["observed_catch"][year] +=
                        //         this->populations[p]->fleets[fleet_]->catch[year];

                }

            }
        }

        void CalculateDepletion(size_t i_year){   
            for (size_t p = 0; p < this->populations.size(); p++) {   

                this->populations[p]->derived_quantities["expected_depletion"][i_year] = 
                    this->populations[p]->depletion->evaluate_mean(this->population[p]->derived_quantities["expected_depletion"][i_year-1], 
                        this->populations[p]->derived_quantities["observed_catch"][i_year-1]);
                this->populations[p]->depletion->log_expected_depletion[i_year] = 
                    log(this->populations[p]->derived_quantities["expected_depletion"][i_year]);

            }
                    
        }  
        
        void CalculateIndex(size_t i_year){
            for (size_t p = 0; p < this->populations.size(); p++) {  
                Type index_;
                for (size_t fleet_ = 0; fleet_ < this->populations[p]->nfleets; fleet_++) {
                   
                    // reference depletion->log_depletion here, where
                    // log_depletion ~ LN(log_expected_depletion, sigma)
                    index_ = fims_math::exp(this->populations[p]->depletion->log_depletion[i_year] + 
                        this->populations[p]->fleets[fleet_]->log_q.get_force_scalar(i_year) );
                
                    this->populations[p]->fleets[fleet_]->expected_index[i_year] += index_;
                }
            }
        }

        void CalculateBiomass(size_t i_year){
            for (size_t p = 0; p < this->populations.size(); p++) {  
                this->populations[p]->derived_quantities["biomass"][i_year] =
                    this->populations[p]->derived_quantities["expected_depletion"][i_year] *
                    this->populations[p]->depletion->K;
            }

        }

        virtual void Evaluate() {
            Prepare();
            for (size_t y = 0; y <= this->nyears; y++) {
                CalculateCatch(y);
            }
            for(size_t y = 1; y <= this->nyears; y++){
                CalculateDepletion(y);
            }
            for (size_t y = 0; y <= this->nyears; y++) {
                CalculateIndex(y);
                CalculateBiomass(y);
            }


            
        }

        
    };


}


#endif
