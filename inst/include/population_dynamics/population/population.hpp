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
#include "../../interface/interface.hpp"
#include "../maturity/maturity.hpp"

namespace fims_popdy {

/**
 * @brief Population class. Contains subpopulations
 * that are divided into generic partitions (e.g., sex, area).
 */
template <typename Type>
struct Population : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /*!< reference id for population object*/
  size_t n_years;       /*!< total number of years in the fishery*/
  size_t n_ages;        /*!< total number of ages in the population*/
  size_t n_fleets;      /*!< total number of fleets in the fishery*/

  // parameters are estimated; after initialize in create_model, push_back to
  // parameter list - in information.hpp (same for initial F in fleet)
  fims::Vector<Type>
      log_init_naa; /*!< estimated parameter: natural log of numbers at age*/
  fims::Vector<Type>
      log_M; /*!< estimated parameter: natural log of Natural Mortality*/
  fims::Vector<Type> proportion_female = fims::Vector<Type>(
      1, static_cast<Type>(0.5)); /*!< proportion female, which is a fixed value of 0.5 */

  // Transformed values
  fims::Vector<Type> M; /*!< transformed parameter: natural mortality*/

  fims::Vector<double> ages;  /*!< vector of the ages for referencing*/
  fims::Vector<double> years; /*!< vector of years for referencing*/

  /// recruitment
  int recruitment_id = -999; /*!< id of recruitment model object*/
  std::shared_ptr<fims_popdy::RecruitmentBase<Type>>
      recruitment; /*!< shared pointer to recruitment module */

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

  /**
   * @brief Constructor.
   */
  Population() { this->id = Population::id_g++; }

  /**
   * @brief Create a map of report vectors for the object.
   */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>>& report_vectors) {
    report_vectors["log_init_naa"].emplace_back(this->log_init_naa);
    report_vectors["log_M"].emplace_back(this->log_M);
  }

  /**
   * @brief Get the report vector count object.
   */
  virtual void get_report_vector_count(
      std::map<std::string, size_t>& report_vector_count) {
    report_vector_count["log_init_naa"] += 1;
    report_vector_count["log_M"] += 1;
  }
};
template <class Type>
uint32_t Population<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
