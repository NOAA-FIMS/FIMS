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
   * @brief Used to iterate through population-based derived quantities
   * dimensions.
   */
  typedef
      typename std::map<uint32_t,
                        std::map<std::string, fims::Vector<size_t>>>::iterator
          population_derived_quantities_dims_iterator;


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
  /**
   * @brief A map of report vectors for the object.
   * used to populate the report_vectors map in for submodule
   * parameters.
   */
  std::map<std::string, fims::Vector<fims::Vector<Type>>> report_vectors;

  SurplusProduction() : fims_popdy::FisheryModelBase<Type>() {
    std::stringstream ss;
    ss << "sp_" << this->GetId() << "_";
    this->name_m = ss.str();
    this->model_type_m = "sp";
  }

  /**
   * @brief Copy constructor for the CatchAtAge class.
   *
   * @param other The other CatchAtAge object to copy from.
   */
  SurplusProduction(const SurplusProduction &other)
      : FisheryModelBase<Type>(other), name_m(other.name_m) {
    this->model_type_m = "sp";
  }

    /**
   * @brief Destroy the Surplus Production object.
   *
   */
  virtual ~SurplusProduction() {}

  virtual void Initialize() {
    for (size_t p = 0; p < this->populations.size(); p++) {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];
      //TODO: remove setting population n_fleets through the interface and add 
      //change intialization in caa
      population->n_fleets = population->fleets.size();
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

      // Reset the derived quantities for the population
      for (auto &kv : derived_quantities) {
        this->ResetVector(kv.second);
      }
      // Transformation Section - apply bidirectional transformations base don user input
      population->depletion->ApplyLogTransformations();
    }

    for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
         ++fit) {
     std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      auto &derived_quantities =
          this->GetFleetDerivedQuantities(fleet->GetId());

      for (auto &kv : derived_quantities) {
        this->ResetVector(kv.second);
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

    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
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
      population->depletion->log_init_depletion[0];

    // for rest of time series, evaluation log_expected_depltion based on 
    // user-defined function
    // depletion[year+1] is the inv-log of log_depletion where depletion[year=0]
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
    //subsequent years are calculated based on inverse log of log_depletion
    //log_depletion is n_years-1 as first year depletion is informed by init_log_depletion
    population->depletion->depletion[year] = fims_math::exp(population->depletion->log_depletion[year]);
      
  
    
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
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
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
    for (size_t fleet_ = 0; fleet_ < population->n_fleets; fleet_++) {
      std::map<std::string, fims::Vector<Type>> &fdq_ =
        this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
      Type sum_log_q = 0.0;
      int ny = 0;
      for (size_t year = 0; year < population->n_years; year++) {
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

 

  /**
  * @brief This method is used to evaluate the surplus production model.
  */
  virtual void Evaluate() {
    Prepare();
    for (size_t p = 0; p < this->populations.size(); p++) {
      std::shared_ptr<fims_popdy::Population<Type>> &population =
          this->populations[p];

      for (size_t y = 0; y <= population->n_years; y++) {
        if(y < population->n_years){
          CalculateCatch(population, y);
        }
        CalculateDepletion(population, y);
        if(y < population->n_years){
          CalculateIndex(population, y);
          CalculateIndexToDepletionKRatio(population, y);
        }
        CalculateBiomass(population, y);
        if(y < population->n_years){
          CalculateHarvestRate(population, y);
        }
      }
      CalculateMeanLogQ(population);
      CalculateReferencePoints(population);
    }
  }

  /**
   * @brief This method is used to report the results of the surplus production model.
   */
  virtual void Report() {
    int n_pops = this->populations.size();
    int n_fleets = this->fleets.size();
#ifdef TMB_MODEL
    if (this->do_reporting == true) {
      report_vectors.clear();
    // Create vector lists to store output for reporting
    // vector< vector<Type> > creates a nested vector structure where
    // each vector can be a different dimension. Does not work with ADREPORT

    vector<vector<Type>> log_index_expected_f(n_fleets);
    vector<vector<Type>> biomass_p(n_pops);
    vector<vector<Type>> observed_catch_p(n_pops);
    vector<vector<Type>> pop_depletion_p(n_pops);
    vector<vector<Type>> log_depletion_expected_p(n_pops);
    vector<vector<Type>> fmsy_p(n_pops);
    vector<vector<Type>> bmsy_p(n_pops);
    vector<vector<Type>> msy_p(n_pops);
    vector<vector<Type>> harvest_rate_p(n_pops);
    vector<vector<Type>> mean_q_f(n_fleets);
    // initiate population index for structuring report out objects
    int pop_idx = 0;
    for (size_t p = 0; p < this->populations.size(); p++) {
      this->populations[p]->create_report_vectors(report_vectors);
      // std::shared_ptr<fims_popdy::Population<Type>> &population =
      //     this->populations[p];
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          this->GetPopulationDerivedQuantities(this->populations[p]->GetId());
      this->populations[p]->depletion->create_report_vectors(report_vectors);
      biomass_p(pop_idx) = derived_quantities["biomass"].to_tmb();
      pop_depletion_p(pop_idx) = 
          (this->populations[p]->depletion->depletion).to_tmb();
      log_depletion_expected_p(pop_idx) =
          (this->populations[p]->depletion->log_expected_depletion).to_tmb();
      observed_catch_p(pop_idx) =
          derived_quantities["observed_catch"].to_tmb();
      harvest_rate_p(pop_idx) =
          derived_quantities["harvest_rate"].to_tmb();
      fmsy_p(pop_idx) = derived_quantities["fmsy"].to_tmb();
      bmsy_p(pop_idx) = derived_quantities["bmsy"].to_tmb();
      msy_p(pop_idx) = derived_quantities["msy"].to_tmb();

      pop_idx += 1;
    }
    int fleet_idx = 0;
    fleet_iterator fit;
    for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      fleet->create_report_vectors(report_vectors);
      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          this->GetFleetDerivedQuantities(fleet->GetId());

      log_index_expected_f(fleet_idx) =
          derived_quantities["log_index_expected"].to_tmb();
      mean_q_f(fleet_idx) =
          fims_math::exp(derived_quantities["mean_log_q"].to_tmb());
      fleet_idx += 1;
    }
    FIMS_REPORT_F_("biomass", biomass_p, this->of);
    FIMS_REPORT_F_("depletion", pop_depletion_p, this->of);
    FIMS_REPORT_F_("log_depletion_expected", log_depletion_expected_p, this->of);
    FIMS_REPORT_F_("observed_catch", observed_catch_p, this->of);
    FIMS_REPORT_F_("harvest_rate", harvest_rate_p, this->of);
    FIMS_REPORT_F_("fmsy", fmsy_p, this->of);
    FIMS_REPORT_F_("bmsy", bmsy_p, this->of);
    FIMS_REPORT_F_("msy", msy_p, this->of);
    FIMS_REPORT_F_("log_index_expected", log_index_expected_f, this->of);
    FIMS_REPORT_F_("mean_q", mean_q_f, this->of);
    /*ADREPORT using ADREPORTvector defined in
     * inst/include/interface/interface.hpp:
     * function collapses the nested vector into a single vector
     */
    vector<Type> biomass = ADREPORTvector(biomass_p);
    vector<Type> depletion = ADREPORTvector(pop_depletion_p);
    vector<Type> observed_catch = ADREPORTvector(observed_catch_p);
    vector<Type> harvest_rate = ADREPORTvector(harvest_rate_p);
    vector<Type> fmsy = ADREPORTvector(fmsy_p);
    vector<Type> bmsy = ADREPORTvector(bmsy_p);
    vector<Type> msy = ADREPORTvector(msy_p);
    vector<Type> log_index_expected = ADREPORTvector(log_index_expected_f);
    vector<Type> mean_q = ADREPORTvector(mean_q_f);

    ADREPORT_F(biomass, this->of);
    ADREPORT_F(depletion, this->of);
    ADREPORT_F(observed_catch, this->of);
    ADREPORT_F(harvest_rate, this->of);
    ADREPORT_F(fmsy, this->of);
    ADREPORT_F(bmsy, this->of);
    ADREPORT_F(msy, this->of);
    ADREPORT_F(log_index_expected, this->of);
    ADREPORT_F(mean_q, this->of);
     std::stringstream var_name;
      typename std::map<std::string, fims::Vector<fims::Vector<Type>>>::iterator
          rvit;
      for (rvit = report_vectors.begin(); rvit != report_vectors.end();
           ++rvit) {
        auto &x = rvit->second;

        int outer_dim = x.size();
        int dim = 0;
        for (int i = 0; i < outer_dim; i++) {
          dim += x[i].size();
        }
        vector<Type> res(dim);
        int idx = 0;
        for (int i = 0; i < outer_dim; i++) {
          int inner_dim = x[i].size();
          for (int j = 0; j < inner_dim; j++) {
            res(idx) = x[i][j];
            idx += 1;
          }
        }
        this->of->reportvector.push(res, rvit->first.c_str());
      }
  }
#endif
  }
};

}  // namespace fims_popdy

#endif