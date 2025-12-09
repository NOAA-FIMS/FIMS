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
  size_t n_years;       /*!< the number of years in the model*/
  size_t n_ages;        /*!< the number of ages in the model*/
  size_t n_lengths;     /*!< the number of lengths in the model*/

  // selectivity

  // selectivity
  int fleet_selectivity_id_m = -999; /*!< id of selectivity component*/
  std::shared_ptr<SelectivityBase<Type>>
      selectivity; /*!< selectivity component*/

  std::string
      selectivity_units; /*!< units for fleet selectivity (age or length)*/

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

  fims::Vector<Type> age_to_length_conversion; /*!<derived quantity age to
                                                length conversion matrix*/

  fims::Vector<Type> selectivity_at_age;    /*!< selectivity at age*/
  fims::Vector<Type> selectivity_at_length; /*!< selectivity at length*/

  /**
   * @brief Constructor.
   */
  Fleet() { this->id = Fleet::id_g++; }

  /**
   * @brief Destructor.
   */
  virtual ~Fleet() {}

  /**
   * @brief Prepare to run the fleet module. Called at each model
   * iteration, and used to exponentiate the natural log of q and Fmort
   * parameters prior to evaluation.
   *
   */
  void Prepare() {
    // for(size_t fleet_ = 0; fleet_ <= this->n_fleets; fleet_++) {
    // this -> Fmort[fleet_] = fims_math::exp(this -> log_Fmort[fleet_]);

    for (size_t i = 0; i < this->log_q.size(); i++) {
      this->q[i] = fims_math::exp(this->log_q[i]);
    }

    for (size_t year = 0; year < this->n_years; year++) {
      this->Fmort[year] = fims_math::exp(this->log_Fmort[year]);
    }

    std::fill(selectivity_at_age.begin(), selectivity_at_age.end(),
              static_cast<Type>(
                  0)); /**<initialize selectivity with zeros before filling*/

    std::fill(selectivity_at_length.begin(), selectivity_at_length.end(),
              static_cast<Type>(
                  0)); /**<initialize selectivity with zeros before filling*/

    // fill selectivity at age and length
    if (selectivity_units == "age") {
      for (size_t a = 0; a < this->nages; a++) {
        this->selectivity_at_age[a] = this->selectivity->evaluate(ages[a]);

        if (this->nlengths > 0) {
          for (size_t l = 0; l < this->nlengths; l++) {
            // iterate through all lengths within an age and sum the selectivity
            // to get a selectivity at length
            size_t i_length_age = a * this->nlengths + l;

            this->selectivity_at_length[l] +=
                this->age_to_length_conversion[i_length_age] *
                this->selectivity_at_age[a];
          }
        }
      }
    } else if (selectivity_units == "length") {
      for (size_t a = 0; a < this->nages; a++) {
        for (size_t l = 0; l < this->nlengths; l++) {
          this->selectivity_at_length[l] =
              this->selectivity->evaluate(lengths[l]);
          // iterate through all lengths within an age and sum the selectivity
          // to get a selectivity at age
          size_t i_length_age = a * this->nlengths + l;
          this->selectivity_at_age[a] +=
              this->age_to_length_conversion[i_length_age] *
              this->selectivity_at_length[l];
        }
      }
    } else {
      FIMS_ERROR_LOG(
          "Fleet selectivity units must be either 'age' or 'length', not " +
          selectivity_units);
    }
  }

  /**
   * Create a map of report vectors for the object.
   */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>> &report_vectors) {
    report_vectors["log_Fmort"].emplace_back(this->log_Fmort.to_tmb());
    report_vectors["log_q"].emplace_back(this->log_q.to_tmb());
    report_vectors["age_to_length_conversion"].emplace_back(
        this->age_to_length_conversion.to_tmb());
  }

  /**
   * Get the report vector count object.
   */
  virtual void
  get_report_vector_count(std::map<std::string, size_t> &report_vector_count) {
    report_vector_count["log_Fmort"] += 1;
    report_vector_count["log_q"] += 1;
  }
};

// default id of the singleton fleet class
template <class Type> uint32_t Fleet<Type>::id_g = 0;

} // end namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
