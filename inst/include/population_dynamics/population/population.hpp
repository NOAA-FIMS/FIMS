/**
 * @file population.hpp
 * @brief Defines the Population class and its fields and methods.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_HPP

#include <map>
#include "../../common/model_object.hpp"
#include "../fleet/fleet.hpp"
#include "../growth/growth.hpp"
#include "../recruitment/recruitment.hpp"
#include "../../interface/interface.hpp"
#include "../maturity/maturity.hpp"
#include "../size/size_distribution_provider_base.hpp"
#include "../size/size_grid.hpp"

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

  std::vector<std::string> annual_dates; /*!< January 1, including terminal year. */

  // All fleets and sample types reference this single table of observed dates.
  std::shared_ptr<std::vector<ObservationTime>> observation_times =
      std::make_shared<std::vector<ObservationTime>>();

  // parameters are estimated; after initialize in create_model, push_back to
  // parameter list - in information.hpp (same for initial F in fleet)
  fims::Vector<Type>
      log_init_naa; /*!< estimated parameter: natural log of numbers at age*/
  fims::Vector<Type>
      log_M; /*!< estimated parameter: natural log of Natural Mortality*/
  fims::Vector<Type> proportion_female = fims::Vector<Type>(
      1, static_cast<Type>(0.5));            /*!< proportion female by age */
  fims::Vector<Type> log_f_multiplier;       /*!< estimated parameter: vector of
    annual fishing mortality multipliers to scale total mortality of all fleets*/
  fims::Vector<Type> spawning_biomass_ratio; /*!< estimated parameter: vector of
annual fishing mortality multipliers to scale total mortality of all fleets*/

  // Transformed values
  fims::Vector<Type> M; /*!< transformed parameter: natural mortality*/
  fims::Vector<Type> f_multiplier; /*!< transformed parameter: vector of
annual fishing mortality multipliers to scale total mortality of all fleets*/

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

  // size
  SizeGrid size_grid; /*!< population-level biological size grid */
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<Type>>
      size_distribution_provider; /*!< population-level size distribution
                                     provider */

  // maturity
  int maturity_id = -999; /*!< id of maturity model object*/
  std::shared_ptr<fims_popdy::MaturityBase<Type>>
      maturity; /*!< shared pointer to maturity module */

  // fleet
  std::set<uint32_t> fleet_ids; /*!< id of fleet model object*/
  std::vector<std::shared_ptr<fims_popdy::Fleet<Type>>>
      fleets; /*!< shared pointer to fleet module */

  /** @brief Resolve the annual calendar and shared sample times from data. */
  void InitializeTiming() {
    using Data = std::shared_ptr<fims_data_object::DataObject<Type>>;
    int first_year = 1;
    bool dated = false;
    for (const auto& fleet : fleets) {
      if (fleet->n_years != n_years) {
        throw std::invalid_argument("Fleet and population year dimensions must agree");
      }
      for (const auto& data : {fleet->observed_catch_data, fleet->observed_index_data,
                               fleet->observed_agecomp_data, fleet->observed_lengthcomp_data}) {
        if (!data) continue;
        if (data->imax != n_years ||
            (!data->timing.empty() && data->timing.size() != n_years)) {
          throw std::invalid_argument("Data timing must have one date per annual row");
        }
        for (size_t y = 0; y < data->timing.size(); ++y) {
          const int start = ParseObservationDate(data->timing[y]).year - static_cast<int>(y);
          if (dated && start != first_year) {
            throw std::invalid_argument(
                "Data dates must align by year, with one sample per fleet/type/year");
          }
          first_year = start;
          dated = true;
        }
      }
    }
    annual_dates.clear();
    for (size_t y = 0; y <= n_years; ++y) {
      annual_dates.push_back(JanuaryFirst(first_year + static_cast<int>(y)));
    }
    const auto active = [](const Data& data, size_t year) {
      const size_t bins = data->data.size() / data->imax;
      for (size_t b = 0; b < bins; ++b) {
        const auto& value = data->data[year * bins + b];
        if (value == value && value != data->na_value) return true;
      }
      return false;
    };
    const auto annual = [](const Data& data, size_t year) {
      return data->timing.empty() || IsAnnualTiming(data->timing[year]);
    };
    std::map<std::string, int> time_ids;
    for (const auto& fleet : fleets) {
      for (const auto& data : {fleet->observed_index_data, fleet->observed_agecomp_data,
                               fleet->observed_lengthcomp_data}) {
        if (!data) continue;
        for (size_t y = 0; y < n_years; ++y) {
          if (active(data, y) && !annual(data, y)) time_ids.emplace(data->timing[y], 0);
        }
      }
    }
    observation_times->clear();
    for (auto& entry : time_ids) {
      const auto calendar = ParseObservationDate(entry.first);
      entry.second = static_cast<int>(observation_times->size());
      observation_times->push_back({static_cast<size_t>(calendar.year - first_year),
                                     calendar.fraction, entry.first});
    }
    ValidateObservationTimes(*observation_times, n_years);
    const auto assign = [&](const Data& data, fims::Vector<int>& ids) {
      ids.resize(data ? n_years : 0);
      if (!data) return;
      for (size_t y = 0; y < n_years; ++y) {
        ids[y] = !active(data, y) ? -1 : annual(data, y) ? -2 : time_ids.at(data->timing[y]);
      }
    };
    for (const auto& fleet : fleets) {
      assign(fleet->observed_index_data, fleet->index_time_id);
      assign(fleet->observed_agecomp_data, fleet->age_comp_time_id);
      assign(fleet->observed_lengthcomp_data, fleet->length_comp_time_id);
      fleet->BindObservationTimes(observation_times);
    }
  }

  /**
   * @brief Constructor.
   */
  Population() {
    this->id = Population::id_g++;
    this->register_self(this->id);
  }
};
template <class Type>
uint32_t Population<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_HPP */
