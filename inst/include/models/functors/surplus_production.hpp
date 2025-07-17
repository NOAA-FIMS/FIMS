#ifndef FIMS_MODELS_SURPLUS_PRODUCTION_HPP
#define FIMS_MODELS_SURPLUS_PRODUCTION_HPP

#include <set>
#include <regex>
#include "fishery_model_base.hpp"

namespace fims_popdy {

template<typename Type>
class SurplusProduction : public FisheryModelBase<Type> {

    public:
     /**
     * @brief The name of the model.
     *
     */
    std::string name_m;

    /**
     * @brief Iterate the derived quantities.
     *
     */
    typedef typename std::map<std::string, fims::Vector<Type>>::iterator
        derived_quantities_iterator;

    /**
     * @brief The derived quantities for all fleets, indexed by fleet id.
     *
     */
    std::map<uint32_t, std::map<std::string, fims::Vector<Type>>>
        fleet_derived_quantities;
    /**
     * @brief The derived quantities for all populations, indexed by
     * population id.
     *
     */
    std::map<uint32_t, std::map<std::string, fims::Vector<Type>>>
        population_derived_quantities;
    /**
     * @brief Iterate through fleet-based derived quantities.
     *
     */
    typedef typename std::map<uint32_t,
                                std::map<std::string, fims::Vector<Type>>>::iterator
        fleet_derived_quantities_iterator;
    /**
     * @brief Iterate through population-based derived quantities.
     *
     */
    typedef typename std::map<uint32_t,
                                std::map<std::string, fims::Vector<Type>>>::iterator
        population_derived_quantities_iterator;
    /**
     * @brief Iterate through fleets.
     *
     */
    typedef typename std::map<uint32_t,
                                std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
        fleet_iterator;
    /**
     * @brief Iterate through derived quantities.
     *
     */
    typedef
      typename std::map<std::string, fims::Vector<Type>>::iterator dq_iterator;

    size_t nyears = 0; //max of all populations
    size_t nages = 0; //max of all populations
    std::set<uint32_t> population_ids;
    std::vector<std::shared_ptr<fims_popdy::Population<Type> > > populations;

    SurplusProduction() : fims_popdy::FisheryModelBase<Type>() {
        this->model_type_m = "sp";
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

        auto derived_quantities =
            this->population_derived_quantities[this->populations[p]->GetId()];

        typename fims_popdy::Population<Type>::derived_quantities_iterator it;
        for (it = derived_quantities.begin(); it != derived_quantities.end();
            it++) {
            fims::Vector<Type> &dq = (*it).second;
            this->ResetVector(dq);
        }
        }

        for (size_t p = 0; p < this->populations.size(); p++) {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
            this->populations[p];
        std::map<std::string, fims::Vector<Type>> &derived_quantities =
            this->population_derived_quantities[this->populations[p]->GetId()];
        }

        for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
            std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
            std::map<std::string, fims::Vector<Type>> &derived_quantities =
                this->fleet_derived_quantities[fleet->GetId()];
            typename fims_popdy::Population<Type>::derived_quantities_iterator it;
            for (it = derived_quantities.begin(); it != derived_quantities.end();
                it++) {
                fims::Vector<Type> &dq = (*it).second;
                this->ResetVector(dq);
            }
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


    void CalculateCatch(std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t year) {
        for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
            size_t index_yf = year * population->nfleets +
            fleet_; // index by fleet and years to dimension fold
            this->population_derived_quantities[population->GetId()]["observed_catch"][year] +=
                population->fleets[fleet_]->observed_landings_data->at(year);

        }

    }
    

    void CalculateDepletion(std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_year){   
        if(i_year == 0){
            population->depletion->log_expected_depletion[0] = 
                population->log_init_depletion[0];
        } else {
            population->depletion->log_expected_depletion[i_year] = 
                population->depletion->evaluate_mean(fims_math::exp(population->depletion->log_expected_depletion[i_year-1]), 
                this->population_derived_quantities[population->GetId()]["observed_catch"][i_year-1]);  
        }     
        this->population_derived_quantities[population->GetId()]["expected_depletion"][i_year] = 
            fims_math::exp(population->depletion->log_depletion[i_year]);
    }  
    
    void CalculateIndex(std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_year){
        Type index_;
        for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
            
            // reference depletion->log_depletion here, where
            // log_depletion ~ LN(log_expected_depletion, sigma)
            index_ = fims_math::exp(population->depletion->log_depletion[i_year] + 
                population->fleets[fleet_]->log_q.get_force_scalar(i_year) );
        
            population->fleets[fleet_]->index_expected[i_year] += index_;
        }
    }
    

    void CalculateBiomass(std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_year){
        this->population_derived_quantities[population->GetId()]["biomass"][i_year] =
            this->population_derived_quantities[population->GetId()]["expected_depletion"][i_year] *
            fims_math::exp(population->depletion->log_K[0]);
    }

    virtual void Evaluate() {
        Prepare();
        for (size_t p = 0; p < this->populations.size(); p++) {
          std::shared_ptr<fims_popdy::Population<Type>> &population =
            this->populations[p];

            for (size_t y = 0; y <= this->nyears; y++) {
                CalculateCatch(population, y);
                CalculateDepletion(population, y);
                CalculateIndex(population, y);
                CalculateBiomass(population, y);
            }

        }
        
    }

    virtual void Report(){

        int n_pops = this->populations.size();
        #ifdef TMB_MODEL
            // Create vector lists to store output for reporting
            // vector< vector<Type> > creates a nested vector structure where
            // each vector can be a different dimension. Does not work with ADREPORT
            
            vector<vector<Type>> biomass(n_pops);
            vector<vector<Type>> expected_depletion(n_pops);
            vector<vector<Type>> observed_catch(n_pops);
            // initiate population index for structuring report out objects
            int pop_idx = 0;
            for (size_t p = 0; p < this->populations.size(); p++) {
                std::map<std::string, fims::Vector<Type>> &derived_quantities =
                    this->population_derived_quantities[this->populations[p]->GetId()];
                biomass(pop_idx) = vector<Type>(derived_quantities["biomass"]);
                expected_depletion(pop_idx) = vector<Type>(derived_quantities["expected_depletion"]);
                observed_catch(pop_idx) = vector<Type>(derived_quantities["observed_catch"]);

            pop_idx += 1;
            }
            FIMS_REPORT_F(biomass, this->of);
            FIMS_REPORT_F(expected_depletion, this->of);
            FIMS_REPORT_F(observed_catch, this->of);

            /*ADREPORT using ADREPORTvector defined in
            * inst/include/interface/interface.hpp:
            * function collapses the nested vector into a single vector
            */
            vector<Type> Biomass = ADREPORTvector(biomass);
            vector<Type> DepletionExpected = ADREPORTvector(expected_depletion);
            vector<Type> ObservedCatch = ADREPORTvector(observed_catch);
        
            ADREPORT_F(Biomass, this->of);
            ADREPORT_F(DepletionExpected, this->of);
            ADREPORT_F(ObservedCatch, this->of);
            
        #endif
    }

    
};


}


#endif