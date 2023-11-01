/*! \file fleet.hpp
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
#include "../../common/model_object.hpp"
#include "../../distributions/distributions.hpp"
#include "../selectivity/selectivity.hpp"

namespace fims_popdy {

/** @brief Base class for all fleets.
 *
 * @tparam Type The type of the fleet object.
 **/
template <class Type>
struct Fleet : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for fleet object*/
  size_t nyears;        /*!< the number of years in the model*/
  size_t nages;         /*!< the number of ages in the model*/
  using ParameterVector =
      typename fims::ModelTraits<Type>::ParameterVector; /*!< vector of fleet
                                                      parameters */

  // This likelihood index is not currently being used as only one likelihood
  // distribution is available. These are for a future update M2+.
  int fleet_index_likelihood_id_m =
      -999; /*!<id of index likelihood component. The "fleet_" prefix indicates
               it belongs to the Fleet struct, and the "_m" postfix signifies
               that it's a member variable.*/
  std::shared_ptr<fims_distributions::DistributionsBase<Type>>
      index_likelihood; /*!< index likelihood component*/

  // This likelihood index is not currently being used as only one likelihood
  // distribution is available. These are for a future update M2+.
  int fleet_agecomp_likelihood_id_m =
      -999; /*!< id of agecomp likelihood component*/
  std::shared_ptr<fims_distributions::DistributionsBase<Type>>
      agecomp_likelihood; /*!< agecomp likelihood component*/

  // selectivity
  int fleet_selectivity_id_m = -999; /*!< id of selectivity component*/
  std::shared_ptr<SelectivityBase<Type>>
      selectivity; /*!< selectivity component*/

  int fleet_observed_index_data_id_m = -999; /*!< id of index data */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_index_data; /*!< observed index data*/

  int fleet_observed_agecomp_data_id_m = -999; /*!< id of age comp data */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_agecomp_data; /*!< observed agecomp data*/

  // Mortality and catchability
  ParameterVector log_Fmort; /*!< estimated parameter: log Fishing mortality*/
  Type log_q; /*!< estimated parameter: catchability of the fleet */

  Type log_obs_error;    /*!< estimated parameter: observation error associated
                            with index */
  ParameterVector Fmort; /*!< transformed parameter: Fishing mortality*/
  Type q; /*!< transofrmed parameter: the catchability of the fleet */

  // derived quantities
  std::vector<Type> catch_at_age;    /*!<derived quantity catch at age*/
  std::vector<Type> catch_index;     /*!<derived quantity catch index*/
  std::vector<Type> age_composition; /*!<derived quantity age composition*/

  // derived quantities
  std::vector<Type> expected_catch; /*!<model expected total catch*/
  std::vector<Type> expected_index; /*!<model expected index of abundance*/
  std::vector<Type> catch_numbers_at_age; /*!<model expected catch at age*/
  std::vector<Type> catch_weight_at_age;  /*!<model expected weight at age*/
  bool is_survey = false;                 /*!< is this fleet object a survey*/

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
              0); /*!<derived quantity catch at age*/
    std::fill(catch_index.begin(), catch_index.end(),
              0); /*!<derived quantity catch index*/
    std::fill(age_composition.begin(), age_composition.end(), 0);
    std::fill(expected_catch.begin(), expected_catch.end(),
              0); /*!<model expected total catch*/
    std::fill(expected_index.begin(), expected_index.end(),
              0); /*!<model expected index of abundance*/
    std::fill(catch_numbers_at_age.begin(), catch_numbers_at_age.end(),
              0); /*!<model expected catch at age*/
    std::fill(catch_weight_at_age.begin(), catch_weight_at_age.end(),
              0); /*!<model expected weight at age*/
    this->q = fims_math::exp(this->log_q);
    for (size_t year = 0; year < this->nyears; year++) {
      FIMS_LOG << "input F mort " << this->log_Fmort[year] << std::endl;
      FIMS_LOG << "input q " << this->log_q << std::endl;
      this->Fmort[year] = fims_math::exp(this->log_Fmort[year]);
    }
  }

  virtual const Type evaluate_age_comp_nll() {
    Type nll = 0.0; /*!< The negative log likelihood value */
#ifdef TMB_MODEL
    fims_distributions::Dmultinom<Type> dmultinom;
    size_t dims = this->observed_agecomp_data->get_imax() *
                  this->observed_agecomp_data->get_jmax();
    if (dims != this->catch_numbers_at_age.size()) {
      fims::fims_log::get("fleet.log")
          << "Error: observed age comp is of size " << dims
          << " and expected is of size " << this->age_composition.size()
          << std::endl;
    } else {
      for (size_t y = 0; y < this->nyears; y++) {
        // EigenVector declares a vector type from the Eigen library, which is
        // the expected type for TMB's dmultinom
        using Vector = typename fims::ModelTraits<Type>::EigenVector;
        Vector observed_acomp;
        Vector expected_acomp;

        observed_acomp.resize(this->nages);
        expected_acomp.resize(this->nages);
        Type sum = 0.0;
        for (size_t a = 0; a < this->nages; a++) {
          size_t i_age_year = y * this->nages + a;
          sum += this->catch_numbers_at_age[i_age_year];
        }

        for (size_t a = 0; a < this->nages; a++) {
          size_t i_age_year = y * this->nages + a;
          expected_acomp[a] = this->catch_numbers_at_age[i_age_year] /
                              sum;  // probabilities for ages

          observed_acomp[a] = this->observed_agecomp_data->at(y, a);
          fims::fims_log::get("fleet.log")
              << " age " << a << " in year " << y
              << "has expected: " << expected_acomp[a]
              << "  and observed: " << observed_acomp[a] << std::endl;
        }
        dmultinom.x = observed_acomp;
        dmultinom.p = expected_acomp;
        nll -= dmultinom.evaluate(true);
      }
    }

#endif
    return nll;
  }

  virtual const Type evaluate_index_nll() {
    Type nll = 0.0; /*!< The negative log likelihood value */

#ifdef TMB_MODEL
    fims_distributions::Dnorm<Type> dnorm;
    dnorm.sd = fims_math::exp(this->log_obs_error);
    for (size_t i = 0; i < this->expected_index.size(); i++) {
      dnorm.x = fims_math::log(this->observed_index_data->at(i));
      dnorm.mean = fims_math::log(this->expected_index[i]);
      nll -= dnorm.evaluate(true);
      fims::fims_log::get("fleet.log")
          << "observed likelihood component: " << i << " is "
          << this->observed_index_data->at(i)
          << " and expected is: " << this->expected_index[i] << std::endl;
    }
    fims::fims_log::get("fleet.log")
        << " log obs error is: " << this->log_obs_error << std::endl;
    fims::fims_log::get("fleet.log") << " sd is: " << dnorm.sd << std::endl;
#endif
    return nll;
  }
};

// default id of the singleton fleet class
template <class Type>
uint32_t Fleet<Type>::id_g = 0;

}  // end namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
