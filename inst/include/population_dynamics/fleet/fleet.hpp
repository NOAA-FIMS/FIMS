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
   * @param nlengths The number of lengths in the model.
   */
  void Initialize(int nyears, int nages, int nlengths = 0) {
    if (this->log_q.size() == 0) {
      this->log_q.resize(1);
      this->log_q[0] = static_cast<Type>(0.0);
    }
    this->nyears = nyears;
    this->nages = nages;
    this->nlengths = nlengths;
    q.resize(this->log_q.size());
    log_Fmort.resize(nyears);
    Fmort.resize(nyears);

    // landings
    landings_numbers_at_age.resize(nyears * nages);
    landings_weight_at_age.resize(nyears * nages);
    landings_numbers_at_length.resize(nyears * nlengths);
    landings_weight.resize(nyears);
    landings_numbers.resize(nyears);
    landings_expected.resize(nyears);
    log_landings_expected.resize(nyears);

    // index
    index_numbers_at_age.resize(nyears * nages);
    index_weight_at_age.resize(nyears * nages);
    index_numbers_at_length.resize(nyears * nlengths);
    index_weight.resize(nyears);
    index_numbers.resize(nyears);
    index_expected.resize(nyears);
    log_index_expected.resize(nyears);

    // composition
    agecomp_expected.resize(nyears * nages);
    lengthcomp_expected.resize(nyears * nlengths);
    agecomp_proportion.resize(nyears * nages);
    lengthcomp_proportion.resize(nyears * nlengths);
    age_to_length_conversion.resize(nages * nlengths);
  }

  /**
   * @brief Prepare to run the fleet module. Called at each model
   * iteration, and used to exponentiate the natural log of q and Fmort
   * parameters prior to evaluation.
   *
   */
  void Prepare() {
    // for(size_t fleet_ = 0; fleet_ <= this->nfleets; fleet_++) {
    // this -> Fmort[fleet_] = fims_math::exp(this -> log_Fmort[fleet_]);

    for (size_t i = 0; i < this->log_q.size(); i++) {
      this->q[i] = fims_math::exp(this->log_q[i]);
    }

    for (size_t year = 0; year < this->nyears; year++) {
      this->Fmort[year] = fims_math::exp(this->log_Fmort[year]);
    }

    // derived quantities
    // landings
    std::fill(landings_weight.begin(), landings_weight.end(),
              static_cast<Type>(0)); /**<model landings in weight*/
    std::fill(landings_numbers.begin(), landings_numbers.end(),
              static_cast<Type>(0)); /**<model landings in numbers*/
    std::fill(landings_expected.begin(), landings_expected.end(),
              static_cast<Type>(0)); /**<model expected landings*/
    std::fill(log_landings_expected.begin(), log_landings_expected.end(),
              static_cast<Type>(0)); /**<model log of expected landings*/
    std::fill(landings_numbers_at_age.begin(), landings_numbers_at_age.end(),
              static_cast<Type>(0)); /**<model landings numbers at age*/
    std::fill(landings_weight_at_age.begin(), landings_weight_at_age.end(),
              static_cast<Type>(0)); /**<model landings weight at age*/
    std::fill(landings_numbers_at_length.begin(),
              landings_numbers_at_length.end(),
              static_cast<Type>(0)); /**<model landings numbers at length*/

    // index
    std::fill(index_weight.begin(), index_weight.end(),
              static_cast<Type>(0)); /**<model index of abundance in weight*/
    std::fill(index_numbers.begin(), index_numbers.end(),
              static_cast<Type>(0)); /**<model index of abundance in numbers*/
    std::fill(index_expected.begin(), index_expected.end(),
              static_cast<Type>(0)); /**<model expected index of abundance*/
    std::fill(
        log_index_expected.begin(), log_index_expected.end(),
        static_cast<Type>(0)); /**<model log of expected index of abundance*/
    std::fill(index_numbers_at_age.begin(), index_numbers_at_age.end(),
              static_cast<Type>(0)); /**<model index numbers at age*/
    std::fill(index_weight_at_age.begin(), index_weight_at_age.end(),
              static_cast<Type>(0)); /**<model index weight at age*/
    std::fill(index_numbers_at_length.begin(), index_numbers_at_length.end(),
              static_cast<Type>(0)); /**<model index numbers at length*/

    // composition
    std::fill(agecomp_expected.begin(), agecomp_expected.end(),
              static_cast<Type>(0)); /**<model composition numbers at age*/
    std::fill(lengthcomp_expected.begin(), lengthcomp_expected.end(),
              static_cast<Type>(0)); /**<model composition numbers at length*/
    std::fill(
        agecomp_proportion.begin(), agecomp_proportion.end(),
        static_cast<Type>(
            0)); /**<model expected composition proportion numbers at age*/
    std::fill(
        lengthcomp_proportion.begin(), lengthcomp_proportion.end(),
        static_cast<Type>(
            0)); /**<model expected composition proportion numbers at length*/
  }

  /**
   * Evaluate the proportion of landings numbers at age.
   */
  void evaluate_age_comp() {
    for (size_t y = 0; y < this->nyears; y++) {
      Type sum = static_cast<Type>(0.0);
      Type sum_obs = static_cast<Type>(0.0);
      // robust_add is a small value to add to expected compostion
      // proportions at age to stabilize likelihood calculations
      // when the expected proportions are close to zero.
      // Type robust_add = static_cast<Type>(0.0); // zeroed out before testing
      // 0.0001;
      // sum robust is used to calculate the total sum of robust
      // additions to ensure that proportions sum to 1.
      // Type robust_sum = static_cast<Type>(1.0);

      for (size_t a = 0; a < this->nages; a++) {
        size_t i_age_year = y * this->nages + a;
        // Here we have a check to determine if the age comp
        // should be calculated from the retained landings or
        // the total population. These values are slightly different.
        // In the future this will have more impact as we implement
        // timing rather than everything occuring at the start of
        // the year.
        if (this->fleet_observed_landings_data_id_m == -999) {
          this->agecomp_expected[i_age_year] =
              this->index_numbers_at_age[i_age_year];
        } else {
          this->agecomp_expected[i_age_year] =
              this->landings_numbers_at_age[i_age_year];
        }
        sum += this->agecomp_expected[i_age_year];
        // robust_sum -= robust_add;

        // This sums over the observed age composition data so that
        // the expected age composition can be rescaled to match the
        // total number observed. The check for na values should not
        // be needed as individual years should not have missing data.
        // This is need to be re-explored if/when we modify FIMS to
        // allow for composition bins that do not match the population
        // bins.
        if (this->fleet_observed_agecomp_data_id_m != -999) {
          if (this->observed_agecomp_data->at(i_age_year) !=
              this->observed_agecomp_data->na_value) {
            sum_obs += this->observed_agecomp_data->at(i_age_year);
          }
        }
      }
      for (size_t a = 0; a < this->nages; a++) {
        size_t i_age_year = y * this->nages + a;
        this->agecomp_proportion[i_age_year] =
            this->agecomp_expected[i_age_year] / sum;
        // robust_add + robust_sum * this->agecomp_expected[i_age_year] / sum;

        if (fleet_observed_agecomp_data_id_m != -999) {
          this->agecomp_expected[i_age_year] =
              this->agecomp_proportion[i_age_year] * sum_obs;
        }
      }
    }
  }

  /**
   * Evaluate the proportion of landings numbers at length.
   */
  void evaluate_length_comp() {
    if (this->nlengths > 0) {
      for (size_t y = 0; y < this->nyears; y++) {
        Type sum = static_cast<Type>(0.0);
        Type sum_obs = static_cast<Type>(0.0);
        // robust_add is a small value to add to expected compostion
        // proportions at age to stabilize likelihood calculations
        // when the expected proportions are close to zero.
        // Type robust_add = static_cast<Type>(0.0); // 0.0001; zeroed out
        // before testing
        // sum robust is used to calculate the total sum of robust
        // additions to ensure that proportions sum to 1.
        // Type robust_sum = static_cast<Type>(1.0);
        for (size_t l = 0; l < this->nlengths; l++) {
          size_t i_length_year = y * this->nlengths + l;
          for (size_t a = 0; a < this->nages; a++) {
            size_t i_age_year = y * this->nages + a;
            size_t i_length_age = a * this->nlengths + l;
            this->lengthcomp_expected[i_length_year] +=
                this->agecomp_expected[i_age_year] *
                this->age_to_length_conversion[i_length_age];

            this->landings_numbers_at_length[i_length_year] +=
                this->landings_numbers_at_age[i_age_year] *
                this->age_to_length_conversion[i_length_age];

            this->index_numbers_at_length[i_length_year] +=
                this->index_numbers_at_age[i_age_year] *
                this->age_to_length_conversion[i_length_age];
          }

          sum += this->lengthcomp_expected[i_length_year];
          // robust_sum -= robust_add;

          if (this->fleet_observed_lengthcomp_data_id_m != -999) {
            if (this->observed_lengthcomp_data->at(i_length_year) !=
                this->observed_lengthcomp_data->na_value) {
              sum_obs += this->observed_lengthcomp_data->at(i_length_year);
            }
          }
        }
        for (size_t l = 0; l < this->nlengths; l++) {
          size_t i_length_year = y * this->nlengths + l;
          this->lengthcomp_proportion[i_length_year] =
              this->lengthcomp_expected[i_length_year] / sum;
          // robust_add + robust_sum * this->lengthcomp_expected[i_length_year]
          // / sum;
          if (this->fleet_observed_lengthcomp_data_id_m != -999) {
            this->lengthcomp_expected[i_length_year] =
                this->lengthcomp_proportion[i_length_year] * sum_obs;
          }
        }
      }
    }
  }

  /**
   * Evaluate the natural log of the expected index.
   */
  void evaluate_index() {
    for (size_t i = 0; i < this->index_weight.size(); i++) {
      if (this->observed_index_units == "number") {
        index_expected[i] = this->index_numbers[i];
      } else {
        index_expected[i] = this->index_weight[i];
      }
      log_index_expected[i] = log(this->index_expected[i]);
    }
  }

  /**
   * Evaluate the natural log of the expected landings.
   */
  void evaluate_landings() {
    for (size_t i = 0; i < this->landings_weight.size(); i++) {
      if (this->observed_landings_units == "number") {
        landings_expected[i] = this->landings_numbers[i];
      } else {
        landings_expected[i] = this->landings_weight[i];
      }
      log_landings_expected[i] = log(this->landings_expected[i]);
    }
  }
};

// default id of the singleton fleet class
template <class Type>
uint32_t Fleet<Type>::id_g = 0;

}  // end namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
