/**
 * @file population.hpp
 * @brief Defines the Population class and its fields and methods.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_HPP

#include "../../common/model_object.hpp"
#include "../fleet/fleet.hpp"
#include "../growth/growth.hpp"
#include "../recruitment/recruitment.hpp"
#include "../depletion/depletion.hpp"
#include "../../interface/interface.hpp"
#include "../maturity/maturity.hpp"

namespace fims_popdy {
/*TODO:
 Review, add functions to evaluate, push vectors back to fleet (or point to
 fleet directly?)
 */

/**
 * @brief Population class. Contains subpopulations
 * that are divided into generic partitions (e.g., sex, area).
 */
template <typename Type>
struct Population : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for population object*/
  size_t nyears;        /*!< total number of years in the fishery*/
  size_t nseasons;      /*!< total number of seasons in the fishery*/
  size_t nages;         /*!< total number of ages in the population*/
  size_t nfleets;       /*!< total number of fleets in the fishery*/

  // parameters are estimated; after initialize in create_model, push_back to
  // parameter list - in information.hpp (same for initial F in fleet)
  fims::Vector<Type>
      log_init_naa; /*!< estimated parameter: natural log of numbers at age*/
  fims::Vector<Type>
      logit_init_depletion; /*!< estimated parameter: natural log of depletion*/
  fims::Vector<Type>
      log_M; /*!< estimated parameter: natural log of Natural Mortality*/
  fims::Vector<Type> proportion_female = fims::Vector<Type>(
      1, static_cast<Type>(0.5)); /*!< proportion female by age */

  // Transformed values
  fims::Vector<Type> M; /*!< transformed parameter: natural mortality*/

  fims::Vector<double> ages;      /*!< vector of the ages for referencing*/
  fims::Vector<double> years;     /*!< vector of years for referencing*/
  fims::Vector<Type> mortality_F; /*!< vector of fishing mortality summed across
                             fleet by year and age*/
  fims::Vector<Type>
      mortality_Z; /*!< vector of total mortality by year and age*/

  // derived quantities
  fims::Vector<Type>
      weight_at_age; /*!< Derived quantity: expected weight at age */
  // fecundity removed because we don't need it yet
  fims::Vector<Type> numbers_at_age; /*!< Derived quantity: population expected
                                numbers at age in each year*/
  fims::Vector<Type>
      unfished_numbers_at_age; /*!< Derived quantity: population expected
                              unfished numbers at age in each year*/
  fims::Vector<Type>
      biomass; /*!< Derived quantity: total population biomass in each year*/
  fims::Vector<Type> spawning_biomass; /*!< Derived quantity: Spawning_biomass*/
  fims::Vector<Type> unfished_biomass; /*!< Derived quantity
                                  biomass assuming unfished*/
  fims::Vector<Type> unfished_spawning_biomass; /*!< Derived quantity Spawning
                                           biomass assuming unfished*/
  fims::Vector<Type> proportion_mature_at_age; /*!< Derived quantity: Proportion
                                          mature at age */
  fims::Vector<Type>
      total_landings_weight; /*!< Derived quantity: Total landings in weight*/
  fims::Vector<Type>
      total_landings_numbers; /*!< Derived quantity: Total landings in numbers*/
  fims::Vector<Type> expected_recruitment; /*!< Expected recruitment */
  fims::Vector<Type> sum_selectivity;      /*!< TODO: add documentation */
  fims::Vector<Type> observed_catch;      /*!< TODO: add documentation */
  /// recruitment
  int recruitment_id = -999; /*!< id of recruitment model object*/
  std::shared_ptr<fims_popdy::RecruitmentBase<Type>>
      recruitment; /*!< shared pointer to recruitment module */

  // depletion
  int depletion_id = -999; /*!< id of depletion model object*/
  std::shared_ptr<fims_popdy::DepletionBase<Type>>
      depletion; /*!< shared pointer to recruitment module */

  // growth
  int growth_id = -999; /*!< id of growth model object*/
  std::shared_ptr<fims_popdy::GrowthBase<Type>>
      growth; /*!< shared pointer to growth module */

  // maturity
  int maturity_id = -999; /*!< id of maturity model object*/
  std::shared_ptr<fims_popdy::MaturityBase<Type>>
      maturity; /*!< shared pointer to maturity module */

  // fleet
  std::set<uint32_t> fleet_ids; /*!< id of fleet model object*/
  std::vector<std::shared_ptr<fims_popdy::Fleet<Type>>>
      fleets; /*!< shared pointer to fleet module */

  // Define objective function object to be able to REPORT and ADREPORT

  std::map<std::string, fims::Vector<Type>>
      derived_quantities; /*!< derived quantities for specific model type, i.e.
                             caa, surplus production, etc */
  /**
   * @brief Iterator for the derived quantities.
   *
   */
  typedef typename std::map<std::string, fims::Vector<Type>>::iterator
      derived_quantities_iterator;
  // this -> means you're referring to a class member (member of self)

  Population() { this->id = Population::id_g++; }

 
};
template <class Type>
uint32_t Population<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
