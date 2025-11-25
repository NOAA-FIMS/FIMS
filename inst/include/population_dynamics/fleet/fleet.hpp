/**
 * @file fleet.hpp
 * @brief Declare the fleet functor class which is the base class for all fleet
 * functors.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
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
  size_t nlengths;      /*!< the number of lengths in the model*/

  // selectivity
  int fleet_selectivity_id_m = -999; /*!< id of selectivity component*/
  std::shared_ptr<SelectivityBase<Type>>
      selectivity; /*!< selectivity component*/

  // landings data
  int fleet_observed_landings_data_id_m = -999; /*!< id of landings data */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_landings_data; /*!< observed landings data*/

  std::string observed_landings_units; /*!< is this fleet landings in weight*/

  // index data
  int fleet_observed_index_data_id_m = -999; /*!< id of index data */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_index_data; /*!< observed index data*/

  std::string observed_index_units; /*!< is this fleet index in weight*/

  // age comp data
  int fleet_observed_agecomp_data_id_m = -999; /*!< id of age comp data */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_agecomp_data; /*!< observed agecomp data*/

  // length comp data
  int fleet_observed_lengthcomp_data_id_m = -999; /*!< id of length comp data */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_lengthcomp_data; /*!< observed lengthcomp data*/

  // Mortality and catchability
  fims::Vector<Type>
      log_Fmort; /*!< estimated parameter: log Fishing mortality*/
  fims::Vector<Type>
      log_q; /*!< estimated parameter: catchability of the fleet */

  fims::Vector<Type> Fmort; /*!< transformed parameter: Fishing mortality*/
  fims::Vector<Type>
      q; /*!< transformed parameter: the catchability of the fleet */

  // derived quantities
  // landings
  fims::Vector<Type> landings_weight;       /*!<model landings in weight*/
  fims::Vector<Type> landings_numbers;      /*!<model landings in numbers*/
  fims::Vector<Type> landings_expected;     /*!<model expected landings*/
  fims::Vector<Type> log_landings_expected; /*!<model log expected landings*/
  fims::Vector<Type>
      landings_numbers_at_age;               /*!<model landings numbers at age*/
  fims::Vector<Type> landings_weight_at_age; /*!<model landings weight at age*/
  fims::Vector<Type>
      landings_numbers_at_length; /*!<model landings numbers at length*/

  // index
  fims::Vector<Type> index_weight;   /*!<model index of abundance in weight*/
  fims::Vector<Type> index_numbers;  /*!<model index of abundance in numbers*/
  fims::Vector<Type> index_expected; /*!<model expected index of abundance*/
  fims::Vector<Type>
      log_index_expected; /*!<model expected log index of abundance*/
  fims::Vector<Type>
      index_numbers_at_age; /*!<model index sampled numbers at age*/
  fims::Vector<Type> index_weight_at_age; /*!<model index weight at age*/
  fims::Vector<Type>
      index_numbers_at_length; /*!<model index sampled numbers at length*/

  // composition
  fims::Vector<Type> age_to_length_conversion; /*!<derived quantity age to
                                                  length conversion matrix*/
  fims::Vector<Type>
      agecomp_expected; /*!<model expected composition numbers at age*/
  fims::Vector<Type>
      lengthcomp_expected; /*!<model expected composition numbers at length*/
  fims::Vector<Type> agecomp_proportion;    /*!<model expected composition
                                               proportion numbers at age*/
  fims::Vector<Type> lengthcomp_proportion; /*!<model expected composition
                                               proportion numbers at length*/

  std::map<std::string, fims::Vector<Type>>
      derived_quantities; /*!< derived quantities for specific model type, i.e.
                             caa, surplus production, etc */
  /**
   * @brief Derived quantities iterator.
   *
   */
  typedef typename std::map<std::string, fims::Vector<Type>>::iterator
      derived_quantities_iterator;

  /**
   * @brief Constructor.
   */
  Fleet() { this->id = Fleet::id_g++; }

  /**
   * @brief Destructor.
   */
  virtual ~Fleet() {}

 
};

// default id of the singleton fleet class
template <class Type>
uint32_t Fleet<Type>::id_g = 0;

}  // end namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
