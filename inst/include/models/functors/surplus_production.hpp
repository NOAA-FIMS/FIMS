#ifndef FIMS_MODELS_SURPLUS_PRODUCTION_HPP
#define FIMS_MODELS_SURPLUS_PRODUCTION_HPP

#include <set>
#include <regex>
#include "fishery_model_base.hpp"

namespace fims_popdy {

template <typename Type>
/**
 * @brief SurplusProduction is a surplus production fishery model in FIMS.
 *
 */
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
   * @brief Used to iterate through fleet-based derived quantities.
   *
   */
  typedef typename std::map<uint32_t,
                            std::map<std::string, fims::Vector<Type>>>::iterator
      fleet_derived_quantities_iterator;

  /**
   * @brief Used to iterate through fleet-based derived quantities dimensions.
   */
  typedef
      typename std::map<uint32_t,
                        std::map<std::string, fims::Vector<size_t>>>::iterator
          fleet_derived_quantities_dims_iterator;
  /**
   * @brief Used to iterate through population-based derived quantities.
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

  size_t nyears = 0;  /*!< max years of all populations */
  size_t nages = 0;   /*!< max ages of all populations */

  SurplusProduction() : fims_popdy::FisheryModelBase<Type>() {
    this->model_type_m = "sp";
  }

    /**
   * @brief Destroy the Surplus Production object.
   *
   */
  virtual ~SurplusProduction() {}

  virtual void Initialize() {
    this->nyears = 0;
    this->nages = 0;

    for (size_t p = 0; p < this->populations.size(); p++) {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];
      this->nyears = std::max(this->nyears, population->nyears);
      this->nages = std::max(this->nages, population->nages);
      //TODO: remove setting population nfleets through the interface and add 
      //change intialization in caa
      population->nfleets = population->fleets.size();

      std::map<std::string, fims::Vector<Type>> &derived_quantities =
            this->GetPopulationDerivedQuantities(this->populations[p]->GetId());

      derived_quantities["biomass"] =
          fims::Vector<Type>((this->nyears + 1));
      
      derived_quantities["observed_catch"] =
          fims::Vector<Type>(this->nyears);

      //TODO: does this derived quantity need to be added to population?
      derived_quantities["harvest_rate"] =
          fims::Vector<Type>(this->nyears + 1);
      
      //TODO: does this derived quantity need to be added to population?
      derived_quantities["fmsy"] = fims::Vector<Type>(1);

      //TODO: does this derived quantity need to be added to population?
      derived_quantities["bmsy"] = fims::Vector<Type>(1);

      //TODO: does this derived quantity need to be added to population?
      derived_quantities["msy"] = fims::Vector<Type>(1);
    }

    for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
 
      if (fleet->log_q.size() == 0) {
        fleet->log_q.resize(1);
        fleet->log_q[0] = static_cast<Type>(0.0);
      }
      fleet->q.resize(fleet->log_q.size());

    }
  }

  void Prepare() {
    for (size_t p = 0; p < this->populations.size(); p++) {
      std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];
      auto &derived_quantities =
          this->GetPopulationDerivedQuantities(population->GetId());

      typename fims_popdy::Population<Type>::derived_quantities_iterator it;
      for (it = derived_quantities.begin(); it != derived_quantities.end();
           it++) {
        fims::Vector<Type> &dq = (*it).second;
        this->ResetVector(dq);
      }
      population->depletion->K[0] = fims_math::exp(population->depletion->log_K[0]);
      population->depletion->r[0] = fims_math::exp(population->depletion->log_r[0]);
      population->depletion->m[0] = fims_math::exp(population->depletion->log_m[0]);
    }

    for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      auto &derived_quantities =
          this->GetFleetDerivedQuantities(fleet->GetId());
      typename fims_popdy::Population<Type>::derived_quantities_iterator it;

      for (it = derived_quantities.begin(); it != derived_quantities.end();
           it++) {
        fims::Vector<Type> &dq = (*it).second;
        this->ResetVector(dq);
      }

      // Transformation Section
      for (size_t i = 0; i < fleet->log_q.size(); i++) {
        fleet->q[i] = fims_math::exp(fleet->log_q[i]);
      }
    }
  }

  /**
   * This function is used to add a population id to the set of population ids.
   */
  void AddPopulation(uint32_t id) { this->population_ids.insert(id); }

    /**
   * @brief Get the population ids of the model.
   */
  std::set<uint32_t> &GetPopulationIds() { return this->population_ids; }

  /**
   * This function is used to get the populations of the model. It returns a
   * vector of shared pointers to the populations.
   * @return std::vector<std::shared_ptr<fims_popdy::Population<Type>>>&
   */
  std::vector<std::shared_ptr<fims_popdy::Population<Type>>> &GetPopulations() {
    return this->populations;
  }

/**
 * @brief Sum over the observed catch for a given population and year.
 * @param population A shared pointer to the population object.
 * @param year The year for which to calculate the catch.
 */
  void CalculateCatch(std::shared_ptr<fims_popdy::Population<Type>> &population,
                      size_t year) {
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
      //if fleet_observed landings data exists for the fleet:
      if (population->fleets[fleet_]->fleet_observed_landings_data_id_m != -999) {
        //if the observed landings data is not -999:
        if(population->fleets[fleet_]->observed_landings_data->at(year) !=
          population->fleets[fleet_]->observed_landings_data->na_value){
            pdq_["observed_catch"][year] +=
                population->fleets[fleet_]->observed_landings_data->at(year);
          }
      }
    }
  }

  /**
   * @brief Evaluate the log expected depletion for a given population and year.
   * @copydoc CalculateCatch()
   */
  void CalculateDepletion(
    //depletion ~ LN(log_expected_depletion, sigma)
    std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t year) {    
    
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());
    
        // in first year, set depletion to initial depletion 
    if (year == 0) {
      population->depletion->log_expected_depletion[0] =
      fims_math::log(fims_math::inv_logit(
          static_cast<Type>(0.0), static_cast<Type>(1.0),
          population->logit_init_depletion[0])
      );

    // for rest of time series, evaluation log_expected_depltion based on 
    // user-defined function
    // depletion[year+1] is the inv-logit of logit_depletion where depletion[year=0]
    // is estimated based off of log_expected_depletion[0] = init_depletion
    } else {
      population->depletion->log_expected_depletion[year] =
          fims_math::log(
            //centered parameterization to avoid issues with negative depletion (Best and Punt, 2020)
            fims_math::ad_max(population->depletion->evaluate_mean( 
              population->depletion->depletion[year-1], 
              pdq_["observed_catch"][year - 1]), 
            static_cast<Type>(0.001))
          );
        
    }
    //first year depletion is initialized in rcpp interface at 0.5
    //subsequent years are calculated based on inv_logit of logit_depletion
    //logit_depletion is nyears-1 as first year depletion is informed by init_logit_depletion
    population->depletion->depletion[year] =
      fims_math::squeeze(
        fims_math::inv_logit(static_cast<Type>(0.0), static_cast<Type>(1.0),
        population->depletion->logit_depletion[year]));
      
  
    
  }

  /**
   * @brief Evaluate the population index for a given population and year.
   * @copydoc CalculateCatch()
   */
  void CalculateIndex(std::shared_ptr<fims_popdy::Population<Type>> &population,
                      size_t year) {
    fleet_iterator fit;
    for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      
      std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(fleet->GetId());

      // reference depletion->depletion here, where
      // depletion ~ LN(log_expected_depletion, sigma)
      fdq_["log_index_expected"][year] = 
          fims_math::log(population->depletion->depletion[year]) +
          fleet->log_q.get_force_scalar(year) + population->depletion->log_K[0];

      fdq_["index_expected"][year] =
          fims_math::exp(fdq_["log_index_expected"][year]);
    }
    
  }

  /**
   * @brief Evaluate the population biomass for a given population and year.
   * @copydoc CalculateCatch()
   */
  void CalculateBiomass(
      std::shared_ptr<fims_popdy::Population<Type>> &population,
      size_t year) {
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());
    pdq_["biomass"][year] =
        population->depletion->depletion[year] *
        fims_math::exp(population->depletion->log_K[0]);
  }

  void CalculateIndexToDepletionKRatio(std::shared_ptr<fims_popdy::Population<Type>> &population,
                      size_t year) {
    //This does not work for many populations to one fleet relationships
    for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
    std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
        //TODO: currently hard coded to -999 to indicate missing data, but need to update
      if (population->fleets[fleet_]->fleet_observed_index_data_id_m != -999) {

        if(population->fleets[fleet_]->observed_index_data->at(year) != 
          population->fleets[fleet_]->observed_index_data->na_value){
          fdq_["log_index_depletionK_ratio"][year] =
            fims_math::log(population->fleets[fleet_]->observed_index_data->at(year)) -
            fims_math::log(population->depletion->depletion[year]) - 
            population->depletion->log_K[0];
        } else {
          fdq_["log_index_depletionK_ratio"][year] = 
            population->fleets[fleet_]->observed_index_data->na_value;
        }
      }
    }
  } 

  void CalculateMeanLogQ(std::shared_ptr<fims_popdy::Population<Type>> &population) {
    for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
      std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
      Type sum_log_q = 0.0;
      int ny = 0;
      for (size_t year = 0; year < population->nyears; year++) {
        //TODO: currently hard coded to -999 to indicate missing data, but need to update
        if(fdq_["log_index_depletionK_ratio"][year] != -999){
          sum_log_q += fdq_["log_index_depletionK_ratio"][year];
          ny++;
        }
      }
      fdq_["mean_log_q"][0] = sum_log_q / ny;
    }

  } 

  void CalculateReferencePoints(std::shared_ptr<fims_popdy::Population<Type>> &population) {
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());
    
    Type log_fmsy = population->depletion->log_r[0] - 
      fims_math::log(fims_math::exp(population->depletion->log_m[0]) - 1) +
      fims_math::log(1.0 - 1 / fims_math::exp(population->depletion->log_m[0]));
    pdq_["fmsy"][0] = fims_math::exp(log_fmsy);
    Type log_bmsy = population->depletion->log_K[0] - 
      1 / (fims_math::exp(population->depletion->log_m[0]) - 1) * 
      population->depletion->log_m[0];
    pdq_["bmsy"][0] = fims_math::exp(log_bmsy);
    pdq_["msy"][0] = pdq_["fmsy"][0] * pdq_["bmsy"][0];
  }

  void CalculateHarvestRate(std::shared_ptr<fims_popdy::Population<Type>> &population,
                      size_t year) {
    std::map<std::string, fims::Vector<Type>> &pdq_ =
        this->GetPopulationDerivedQuantities(population->GetId());

    pdq_["harvest_rate"][year] =
        pdq_["observed_catch"][year] / pdq_["biomass"][year];
  }

  void CalculateBiomassPenalties(std::shared_ptr<fims_popdy::Population<Type>> &population,
                      size_t year) {
    population->depletion->biomass_expected_penalty[year] = fims_math::smooth_piecemeal(
        population->depletion->depletion[year],
        fims_math::exp(population->depletion->log_K[0]),
        //bounds are hard coded for now; should be user-defined later
        static_cast<Type>(0.02),
        static_cast<Type>(0.9));
    }

  void CalculateParameterPenalties(std::shared_ptr<fims_popdy::Population<Type>> &population) {
    
    population->depletion->K_expected_penalty[0] = fims_math::smooth_piecemeal(
        fims_math::exp(population->depletion->log_K[0]),
        static_cast<Type>(1.0),
        //bounds are hard coded for now; should be user-defined later
        static_cast<Type>(1e-02),
        static_cast<Type>(2000.0));
  }



  

  /**
  * @brief This method is used to evaluate the surplus production model.
  */
  virtual void Evaluate() {
    Prepare();
    for (size_t p = 0; p < this->populations.size(); p++) {
      std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];

      for (size_t y = 0; y <= this->nyears; y++) {
        if(y < nyears){
          CalculateCatch(population, y);
        }
        CalculateDepletion(population, y);
        if(y < nyears){
          CalculateIndex(population, y);
          CalculateIndexToDepletionKRatio(population, y);
        }
        CalculateBiomass(population, y);
        if(y < nyears){
          CalculateHarvestRate(population, y);
          CalculateBiomassPenalties(population, y);
        }
      }
      CalculateMeanLogQ(population);
      CalculateReferencePoints(population);
      CalculateParameterPenalties(population);
    }
  }

  /**
   * @brief This method is used to report the results of the surplus production model.
   */
  virtual void Report() {
    int n_pops = this->populations.size();
    int n_fleets = this->fleets.size();
#ifdef TMB_MODEL
    // Create vector lists to store output for reporting
    // vector< vector<Type> > creates a nested vector structure where
    // each vector can be a different dimension. Does not work with ADREPORT

    vector<vector<Type>> log_index_expected(n_fleets);
    vector<vector<Type>> biomass(n_pops);
    vector<vector<Type>> observed_catch(n_pops);
    vector<vector<Type>> pop_depletion(n_pops);
    vector<vector<Type>> log_depletion_expected(n_pops);
    vector<vector<Type>> fmsy(n_pops);
    vector<vector<Type>> bmsy(n_pops);
    vector<vector<Type>> msy(n_pops);
    vector<vector<Type>> harvest_rate(n_pops);
    vector<vector<Type>> mean_q(n_fleets);
    // initiate population index for structuring report out objects
    int pop_idx = 0;
    for (size_t p = 0; p < this->populations.size(); p++) {
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          this->GetPopulationDerivedQuantities(this->populations[p]->GetId());
      biomass(pop_idx) = vector<Type>(derived_quantities["biomass"]);
      pop_depletion(pop_idx) =
          vector<Type>(this->populations[p]->depletion->depletion);
      log_depletion_expected(pop_idx) =
          vector<Type>(this->populations[p]->depletion->log_expected_depletion);
      observed_catch(pop_idx) =
          vector<Type>(derived_quantities["observed_catch"]);
      harvest_rate(pop_idx) =
          vector<Type>(derived_quantities["harvest_rate"]);
      fmsy(pop_idx) = vector<Type>(derived_quantities["fmsy"]);
      bmsy(pop_idx) = vector<Type>(derived_quantities["bmsy"]);
      msy(pop_idx) = vector<Type>(derived_quantities["msy"]);

      pop_idx += 1;
    }
    int fleet_idx = 0;
    fleet_iterator fit;
    for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
            this->GetFleetDerivedQuantities(fleet->GetId());

      log_index_expected(fleet_idx) =
          vector<Type>(derived_quantities["log_index_expected"]);
      mean_q(fleet_idx) =
          fims_math::exp(vector<Type>(derived_quantities["mean_log_q"]));
      fleet_idx += 1;
    }
    FIMS_REPORT_F(biomass, this->of);
    FIMS_REPORT_F(pop_depletion, this->of);
    FIMS_REPORT_F(log_depletion_expected, this->of);
    FIMS_REPORT_F(observed_catch, this->of);
    FIMS_REPORT_F(harvest_rate, this->of);
    FIMS_REPORT_F(fmsy, this->of);
    FIMS_REPORT_F(bmsy, this->of);
    FIMS_REPORT_F(msy, this->of);
    FIMS_REPORT_F(log_index_expected, this->of);
    FIMS_REPORT_F(mean_q, this->of);

    /*ADREPORT using ADREPORTvector defined in
     * inst/include/interface/interface.hpp:
     * function collapses the nested vector into a single vector
     */
    vector<Type> Biomass = ADREPORTvector(biomass);
    vector<Type> Depletion = ADREPORTvector(pop_depletion);
    vector<Type> ObservedCatch = ADREPORTvector(observed_catch);
    vector<Type> HarvestRate = ADREPORTvector(harvest_rate);
    vector<Type> Fmsy = ADREPORTvector(fmsy);
    vector<Type> Bmsy = ADREPORTvector(bmsy);
    vector<Type> Msy = ADREPORTvector(msy);
    vector<Type> LogIndexExpected = ADREPORTvector(log_index_expected);
    vector<Type> MeanQ = ADREPORTvector(mean_q);

    ADREPORT_F(Biomass, this->of);
    ADREPORT_F(Depletion, this->of);
    ADREPORT_F(ObservedCatch, this->of);
    ADREPORT_F(HarvestRate, this->of);
    ADREPORT_F(Fmsy, this->of);
    ADREPORT_F(Bmsy, this->of);
    ADREPORT_F(Msy, this->of);
    ADREPORT_F(LogIndexExpected, this->of);
    ADREPORT_F(MeanQ, this->of);

#endif
  }
};

}  // namespace fims_popdy

#endif