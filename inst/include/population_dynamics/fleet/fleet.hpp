/*! \file fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the growth functor class
 * which is the base class for all growth functors.
 */
#ifndef FIMS_POPULATION_DYNAMICS_FLEET_HPP
#define FIMS_POPULATION_DYNAMICS_FLEET_HPP

#include "../../common/data_object.hpp"
#include "../../common/model_object.hpp"
#include "../../distributions/distributions.hpp"
#include "../selectivity/selectivity.hpp"

namespace fims {

/** @brief Base class for all fleets.
 *
 * @tparam Type The type of the fleet object.
 **/
template <class Type>
struct Fleet : public FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for fleet object*/
  size_t nyears;        /*!< the number of years in the model*/
  size_t nages;         /*!< the number of ages in the model*/

  // data objects
  int observed_catch_data_id = -999; /*!< id of observed catch data object*/
  std::shared_ptr<fims::DataObject<Type>>
      observed_catch_data; /*!< observed catch data object*/

  int observed_index_data_id = -999; /*!< id of observed index data object*/
  std::shared_ptr<fims::DataObject<Type>>
      observed_index_data; /*!< observed index data object*/

  int observed_agecomp_data_id = -999; /*!< id of observed agecomp data object*/
  std::shared_ptr<fims::DataObject<Type>>
      observed_agecomp_data; /*!< observed agecomp data object*/

  // likelihood components
  int catch_likelihood_id = -999; /*!< id of catch likelihood component*/
  std::shared_ptr<fims::DistributionsBase<Type>>
      catch_likelihood; /*!< catch likelihood component*/

  int index_likelihood_id = -999; /*!< id of index likelihood component*/
  std::shared_ptr<fims::DistributionsBase<Type>>
      index_likelihood; /*!< index likelihood component*/

  int agecomp_likelihood_id = -999; /*!< id of agecomp likelihood component*/
  std::shared_ptr<fims::DistributionsBase<Type>>
      agecomp_likelihood; /*!< agecomp likelihood component*/

  // selectivity
  int selectivity_id = -999; /*!< id of selectivity component*/
  std::shared_ptr<fims::SelectivityBase<Type>>
      selectivity; /*!< selectivity component*/

  // Mortality and catchability
  std::vector<Type> log_Fmort; /*!< estimated parameter: log Fishing mortality*/
  std::vector<Type>
      log_q; /*!< estimated parameter: catchability of the fleet */

  std::vector<Type> Fmort; /*!< transformed parameter: Fishing mortality*/
  std::vector<Type>
      q; /*!< transofrmed parameter: the catchability of the fleet */

  // derived quantities
  std::vector<Type> catch_at_age;    /*!<derived quantity catch at age*/
  std::vector<Type> catch_index;     /*!<derived quantity catch index*/
  std::vector<Type> age_composition; /*!<derived quantity age composition*/

  // derived quantities
  std::vector<Type> expected_catch; /*!<model expected total catch*/
  std::vector<Type> expected_index; /*!<model expected index of abundance*/
  std::vector<Type> catch_numbers_at_age; /*!<model expected catch at age*/
  std::vector<Type>
        catch_weight_at_age; /*!< Expected values???: Weight at age for catch*/

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
    this->nyears = nyears;
    this->nages = nages;

    catch_at_age.resize(nyears * nages);
    catch_weight_at_age.resize(nyears * nages);
    catch_index.resize(nyears);  // assume index is for all ages.
    age_composition.resize(nyears * nages);
    expected_catch.resize(nyears);
    expected_index.resize(nyears);  // assume index is for all ages.
    catch_numbers_at_age.resize(nyears * nages);

    log_Fmort.resize(nyears);
    Fmort.resize(nyears);
    log_q.resize(nyears);
    q.resize(nyears);
  }

  /**
   * @brief Prepare to run the fleet module. Called at each model itartion, and
   * used to exponentiate the log q and Fmort parameters prior to evaluation.
   *
   */
  void Prepare() {
    // for(size_t fleet_ = 0; fleet_ <= this->nfleets; fleet_++) {
    // this -> Fmort[fleet_] = fims::exp(this -> log_Fmort[fleet_]);
    for (size_t year = 0; year < this->nyears; year++) {
      this->Fmort[year] = fims::exp(this->log_Fmort[year]);
      this->q[year] = fims::exp(this->log_q[year]);
    }
  }

  /**
   * @brief Sum of index and agecomp likelihoods
   * @param do_log Whether to take the log of the likelihood.
   */
  const Type likelihood(bool do_log) {
    return this->index_likelihood->evaluate(do_log) +
           this->agecomp_likelihood->evaluate(do_log);
  }
};

// default id of the singleton fleet class
template <class Type>
uint32_t Fleet<Type>::id_g = 0;

}  // end namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
