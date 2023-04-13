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
  using ParameterVector =
      typename ModelTraits<Type>::ParameterVector; /*!< vector of fleet
                                                      parameters */

  int index_likelihood_id = -999; /*!<id of index likelihood component*/
  std::shared_ptr<fims::DistributionsBase<Type>>
      index_likelihood; /*!< index likelihood component*/

  int agecomp_likelihood_id = -999; /*!< id of agecomp likelihood component*/
  std::shared_ptr<fims::DistributionsBase<Type>>
      agecomp_likelihood; /*!< agecomp likelihood component*/

  // selectivity
  int selectivity_id = -999; /*!< id of selectivity component*/
  std::shared_ptr<fims::SelectivityBase<Type>>
      selectivity; /*!< selectivity component*/

  int observed_index_data_id = -999; /*!< id of index data */
  std::shared_ptr<fims::DataObject<Type>>
      observed_index_data; /*!< observed index data*/

  int observed_agecomp_data_id = -999; /*!< id of age comp data */
  std::shared_ptr<fims::DataObject<Type>>
      observed_agecomp_data; /*!< observed agecomp data*/

  // Mortality and catchability
  ParameterVector log_Fmort; /*!< estimated parameter: log Fishing mortality*/
  ParameterVector log_q; /*!< estimated parameter: catchability of the fleet */

  Type log_obs_error; /*!< estimated parameter: observation error associated
                         with index */
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
  std::vector<Type> catch_weight_at_age;  /*!<model expected weight at age*/

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

  // derived quantities
  std::fill(catch_at_age.begin(), catch_at_age.end(), 0);    /*!<derived quantity catch at age*/
  std::fill(catch_index.begin(), catch_index.end(), 0);     /*!<derived quantity catch index*/
  std::fill(age_composition.begin(), age_composition.end(), 0);
  std::fill(expected_catch.begin(), expected_catch.end(), 0); /*!<model expected total catch*/
  std::fill(expected_index.begin(), expected_index.end(), 0); /*!<model expected index of abundance*/
  std::fill(catch_numbers_at_age.begin(), catch_numbers_at_age.end(), 0); /*!<model expected catch at age*/
  std::fill(catch_weight_at_age.begin(), catch_weight_at_age.end(), 0);  /*!<model expected weight at age*/

    for (size_t year = 0; year < this->nyears; year++) {
      FIMS_LOG << "input F mort " << this->log_Fmort[year] << std::endl;
      FIMS_LOG << "input q " << this->log_q[year] << std::endl;
      this->Fmort[year] = fims::exp(this->log_Fmort[year]);
      this->q[year] = fims::exp(this->log_q[year]);
    }
  }
  #ifdef TMB_MODEL
  void ReportFleet(){
  typename ModelTraits<Type>::EigenVector fleet_index =
      expected_index;
      REPORT_F(fleet_index, of);
}


  virtual const Type evaluate_age_comp_ll() {
    Type nll = 0.0; /*!< The negative log likelihood value */
    #ifdef TMB_MODEL
      fims::Dmultinom<Type> dmultinom;
      size_t dims = this->observed_agecomp_data->get_imax() *
        this->observed_agecomp_data->get_jmax();
      if (dims != this->catch_numbers_at_age.size()) {
        FIMS_LOG << "Error: observed age comp is of size " << dims
                 << " and expected is of size " << this->age_composition.size()
                 << std::endl;
      } else {
        for (size_t y = 0; y < this->nyears; y++) {
          using Vector = typename ModelTraits<Type>::EigenVector;
          Vector observed_acomp;
          Vector expected_acomp;

          observed_acomp.resize(this->nages);
          expected_acomp.resize(this->nages);
          Type sum = 0.0;
          for (size_t a = 0; a < this->nages; a++) {
            size_t index_ya = y * this->nages + a;
            sum += this->catch_numbers_at_age[index_ya];
          }

          FIMS_LOG << "observed and expected age comp is: " << std::endl;
          for (size_t a = 0; a < this->nages; a++) {
            size_t index_ya = y * this->nages + a;
            expected_acomp[a] = this->catch_numbers_at_age[index_ya] /
              sum;  // probabilities for ages
            observed_acomp[a] = this->observed_agecomp_data->at(y, a);
          }
          dmultinom.x = observed_acomp;
          dmultinom.p = expected_acomp;
          nll -= dmultinom.evaluate(true);
        }
      }
    #endif
    return nll;
  }

  virtual const Type evaluate_index_ll() {
    Type nll = 0.0; /*!< The negative log likelihood value */
    #ifdef TMB_MODEL
    fims::Dnorm<Type> dnorm;
    dnorm.sd = fims::exp(this->log_obs_error);
    for (size_t i = 0; i < this->expected_index.size(); i++) {
      dnorm.x = fims::log(this->observed_index_data->at(i));
      dnorm.mean = fims::log(this->expected_index[i]);
      nll -= dnorm.evaluate(true);
    }
    #endif
    return nll;
  }

};



// default id of the singleton fleet class
template <class Type>
uint32_t Fleet<Type>::id_g = 0;

}  // end namespace fims

#endif /* FIMS_POPULATION_DYNAMICS_FLEET_HPP */
