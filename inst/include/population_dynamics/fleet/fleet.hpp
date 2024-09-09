/** \file fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the fleet functor class
 * which is the base class for all fleet functors.
 */
#ifndef FIMS_POPULATION_DYNAMICS_FLEET_HPP
#define FIMS_POPULATION_DYNAMICS_FLEET_HPP

#include "../../common/data_object.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/model_object.hpp"
#include "../../distributions/distributions.hpp"
#include "../selectivity/selectivity.hpp"

namespace fims_popdy {

/** @brief Base class for all fleets.
 *
 * @tparam Type The type of the fleet object.
 */
template <class Type>
struct Fleet : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for fleet object*/
  size_t nyears;        /*!< the number of years in the model*/
  size_t nages;         /*!< the number of ages in the model*/

  // selectivity
  int fleet_selectivity_id_m = -999; /*!< id of selectivity component*/
  std::shared_ptr<SelectivityBase<Type>>
      selectivity; /*!< selectivity component*/

  // Mortality and catchability
  fims::Vector<Type>
      log_Fmort; /*!< estimated parameter: log Fishing mortality*/
  fims::Vector<Type> log_q; /*!< estimated parameter: catchability of the fleet */

  fims::Vector<Type> Fmort; /*!< transformed parameter: Fishing mortality*/
  fims::Vector<Type> q; /*!< transofrmed parameter: the catchability of the fleet */

  // derived quantities
  fims::Vector<Type> catch_at_age;    /*!<derived quantity catch at age*/
  fims::Vector<Type> catch_index;     /*!<derived quantity catch index*/
  fims::Vector<Type> age_composition; /*!<derived quantity age composition*/

  // derived quantities
  fims::Vector<Type> observed_catch_lpdf; /*!<observed total catch linked
    to log probability density function*/
  fims::Vector<Type> observed_index_lpdf; /*!<observed index of abundance linked
    to log probability density function*/
  fims::Vector<Type> expected_catch; /*!<model expected total catch*/
  fims::Vector<Type> expected_index; /*!<model expected index of abundance*/
  fims::Vector<Type> log_expected_index; /*!<model expected index of abundance*/
  fims::Vector<Type> expected_catch_lpdf; /*!<model expected total catch linked
    to log probability density function*/
  fims::Vector<Type> expected_index_lpdf; /*!<model expected index of abundance linked
    to log probability density function*/
  fims::Vector<Type> catch_numbers_at_age; /*!<model expected catch at age*/
  fims::Vector<Type> proportion_catch_numbers_at_age; /*!<model expected catch at age*/
  fims::Vector<Type> catch_weight_at_age;  /*!<model expected weight at age*/
  bool is_survey = false;                  /*!< is this fleet object a survey*/

#ifdef TMB_MODEL
  ::objective_function<Type> *of;
#endif

  /**
   * @brief Constructor.
   */
  Fleet() { this->id = Fleet::id_g++; }

  /**
   * @brief Destructor.
   */
  virtual ~Fleet() {}

  /**
   * @brief Intialize Fleet Class
   * @param nyears The number of years in the model.
   * @param nages The number of ages in the model.
   */
  void Initialize(int nyears, int nages) {
    if(this->log_q.size()==0){
        this->log_q.resize(1);
        this->log_q[0] = 0.0;
    }
    this->nyears = nyears;
    this->nages = nages;

    catch_at_age.resize(nyears * nages);
    catch_numbers_at_age.resize(nyears * nages);
    catch_weight_at_age.resize(nyears * nages);
    catch_index.resize(nyears);  // assume index is for all ages.
    expected_catch.resize(nyears);
    expected_index.resize(nyears);
    log_expected_index.resize(nyears);
    age_composition.resize(nyears * nages);
    q.resize(this->log_q.size());
    log_Fmort.resize(nyears);
    Fmort.resize(nyears);
  }

  /**
   * @brief Prepare to run the fleet module. Called at each model itartion, and
   * used to exponentiate the log q and Fmort parameters prior to evaluation.
   *
   */
  void Prepare() {
    // for(size_t fleet_ = 0; fleet_ <= this->nfleets; fleet_++) {
    // this -> Fmort[fleet_] = fims_math::exp(this -> log_Fmort[fleet_]);

    // derived quantities
    std::fill(catch_at_age.begin(), catch_at_age.end(),
              0); /**<derived quantity catch at age*/
    std::fill(catch_index.begin(), catch_index.end(),
              0); /**<derived quantity catch index*/
    std::fill(age_composition.begin(), age_composition.end(), 0);
    std::fill(expected_catch.begin(), expected_catch.end(),
              0); /**<model expected total catch*/
    std::fill(expected_index.begin(), expected_index.end(),
              0); /**<model expected index of abundance*/
    std::fill(log_expected_index.begin(), log_expected_index.end(),
              0); /**<model expected index of abundance*/
    std::fill(catch_numbers_at_age.begin(), catch_numbers_at_age.end(),
              0); /**<model expected catch at age*/
    std::fill(proportion_catch_numbers_at_age.begin(), proportion_catch_numbers_at_age.end(),
              0); /**<model expected catch at age*/
    std::fill(catch_weight_at_age.begin(), catch_weight_at_age.end(),
              0); /**<model expected weight at age*/
  
    for (size_t i = 0; i < this->log_q.size(); i++) {
        this->q[i] = fims_math::exp(this->log_q[i]);
    }

    for (size_t year = 0; year < this->nyears; year++) {
      FLEET_LOG << "input F mort " << this->log_Fmort[year] << std::endl;
      FLEET_LOG << "input q " << this->log_q << std::endl;
      this->Fmort[year] = fims_math::exp(this->log_Fmort[year]);
    }
  }

  /**
   * Evaluate the proportion of catch numbers at age.
   */
  void evaluate_age_comp() {
    for (size_t y = 0; y < this->nyears; y++) {
      Type sum = 0.0;
      for (size_t a = 0; a < this->nages; a++) {
        size_t i_age_year = y * this->nages + a;
        sum += this->catch_numbers_at_age[i_age_year];
      }
      for (size_t a = 0; a < this->nages; a++) {
        size_t i_age_year = y * this->nages + a;
        this->proportion_catch_numbers_at_age[i_age_year] = this->catch_numbers_at_age[i_age_year] / sum;

      }
    }
  }

  /**
   * Evaluate the log of the expected index.
   */
  void evaluate_index() {
    for(size_t i=0; i<this->expected_index.size(); i++){
      log_expected_index[i] = log(this->expected_index[i]);
    }
  }
};

// default id of the singleton fleet class
template <class Type>
uint32_t Fleet<Type>::id_g = 0;

}  // end namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
