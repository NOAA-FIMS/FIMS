/**
 * @file growth_derived_alk.hpp
 * @brief Declares the GrowthDerivedALK class, which implements ALKBase
 * by mapping prepared population age-to-size distributions onto fleet
 * observation bins.
 * @details Defines guards for the growth-derived ALK functor.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_DERIVED_ALK_HPP
#define POPULATION_DYNAMICS_GROWTH_DERIVED_ALK_HPP

#include <algorithm>
#include <cstddef>
#include <memory>

#include "../../../common/fims_math.hpp"
#include "alk_base.hpp"
#include "../../fleet/fleet.hpp"
#include "../../growth/growth_model_adapter.hpp"
#include "../../size/size_distribution_provider_base.hpp"
#include "../../size/functors/size_bin_mapping.hpp"
#include "../../size/functors/size_probability_normalization.hpp"

namespace fims_popdy {

/**
 * @brief Growth-derived ALK implementation that reads prepared population
 * age-to-size probabilities and maps them into fleet observation bins.
 */
template <typename Type>
struct GrowthDerivedALK : public ALKBase<Type> {
  std::weak_ptr<Fleet<Type>> fleet; /**< non-owning link to the fleet */
  std::weak_ptr<SizeDistributionProviderBase<Type>>
      size_provider; /**< non-owning link to the population size provider */
  std::shared_ptr<GrowthDerivedObservationBase<Type>>
      growth_observation; /**< growth-derived observation capability */

  /**
   * @brief Constructor.
   * @param fleet Shared pointer to the fleet using this ALK.
   * @param growth_observation Shared pointer to the growth-derived
   * observation capability.
   * @param size_provider Shared pointer to the population size provider.
   */
  GrowthDerivedALK(
      const std::shared_ptr<Fleet<Type>>& fleet,
      const std::shared_ptr<GrowthDerivedObservationBase<Type>>&
          growth_observation,
      const std::shared_ptr<SizeDistributionProviderBase<Type>>&
          size_provider)
      : ALKBase<Type>(),
        fleet(fleet),
        size_provider(size_provider),
        growth_observation(growth_observation) {}

  /**
   * @brief Destructor.
   */
  virtual ~GrowthDerivedALK() {}

  /**
   * @brief Returns whether this growth-derived ALK is structurally active.
   * @return True if the linked fleet and growth object are valid and the fleet
   * has a consistent explicit bin definition.
   */
  virtual bool IsActive() const override {
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();
    std::shared_ptr<SizeDistributionProviderBase<Type>> size_provider_ptr =
        size_provider.lock();

    return fleet_ptr != nullptr &&
           size_provider_ptr != nullptr &&
           growth_observation != nullptr &&
           growth_observation->SupportsGrowthDerivedALK() &&
           fleet_ptr->n_ages > 0 &&
           fleet_ptr->n_lengths > 0 &&
           fleet_ptr->lengths.size() == fleet_ptr->n_lengths &&
           fleet_ptr->length_bin_edges.size() == fleet_ptr->n_lengths + 1 &&
           SizeBinMapping::HasStrictlyIncreasingEdges(
               fleet_ptr->length_bin_edges);
  }

  /**
   * @brief Ensures the linked growth products and population size-provider
   * products are prepared for the current model state.
   *
   * This helper reuses already prepared growth products when available and only
   * triggers preparation when products have not yet been prepared. It then
   * prepares the linked population size provider so the ALK can read
   * population age-to-size rows for mapping into fleet observation bins.
   *
   * @return True if the required growth products are available for this ALK
   * path and the linked population size provider can be prepared.
   */
  virtual bool PrepareForCurrentState() override {
    if (!this->IsActive()) {
      return false;
    }

    std::shared_ptr<SizeDistributionProviderBase<Type>> size_provider_ptr =
        TryGetSizeProvider();
    if (size_provider_ptr == nullptr) {
      return false;
    }

    const GrowthProducts<Type>* growth_products = TryGetGrowthProducts();
    if (growth_products == nullptr) {
      growth_observation->PrepareGrowthProducts();
      growth_products = TryGetGrowthProducts();
    }

    if (growth_products == nullptr || growth_products->n_sexes != 1) {
      return false;
    }

    size_provider_ptr->PrepareSizeProducts();
    return true;
  }

  /**
   * @brief Builds the normalized ALK row for a given year and age.
   * @param year Year index.
   * @param age Age index.
   * @param out_row Output age-to-length probability row.
   * @return True if the ALK row was built successfully using prepared growth
   * products for the current model state.
   */
  virtual bool BuildALKRow(size_t year,
                           size_t age,
                           fims::Vector<Type>& out_row) const override {
    out_row = BuildMappedFleetALKRow(year, age);
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();
    return fleet_ptr != nullptr && out_row.size() == fleet_ptr->n_lengths;
  }

 protected:
  /**
   * @brief Read one prepared age-to-size row from the linked population size provider.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Population-grid probability row for the requested year and age.
   */
  fims::Vector<Type> BuildPopulationProbSizeRow(std::size_t year_index,
                                                std::size_t age_index) const {
    std::shared_ptr<SizeDistributionProviderBase<Type>> size_provider_ptr =
        TryGetSizeProvider();

    if (size_provider_ptr == nullptr) {
      return fims::Vector<Type>();
    }

    const SizeGrid* population_size_grid = size_provider_ptr->TryGetSizeGrid();
    if (population_size_grid == nullptr ||
        !population_size_grid->IsConsistent() ||
        population_size_grid->n_bins == 0) {
      return fims::Vector<Type>();
    }

    fims::Vector<Type> prob_size_row(population_size_grid->n_bins);

    try {
      for (std::size_t size_bin_index = 0;
           size_bin_index < population_size_grid->n_bins;
           ++size_bin_index) {
        prob_size_row[size_bin_index] =
            size_provider_ptr->ProbSize(
                year_index, age_index, size_bin_index);
      }
    } catch (const std::runtime_error&) {
      return fims::Vector<Type>();
    }

    return prob_size_row;
  }

  /**
   * @brief Build one fleet ALK row by mapping a population age-to-size row
   * onto fleet observation bins.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Normalized fleet observation-bin probability row.
   */
  fims::Vector<Type> BuildMappedFleetALKRow(std::size_t year_index,
                                            std::size_t age_index) const {
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();
    std::shared_ptr<SizeDistributionProviderBase<Type>> size_provider_ptr =
        TryGetSizeProvider();

    if (fleet_ptr == nullptr || size_provider_ptr == nullptr) {
      return fims::Vector<Type>();
    }

    const SizeGrid* population_size_grid = size_provider_ptr->TryGetSizeGrid();
    if (population_size_grid == nullptr ||
        !population_size_grid->IsConsistent() ||
        population_size_grid->n_bins == 0) {
      return fims::Vector<Type>();
    }

    if (fleet_ptr->n_lengths == 0 ||
        fleet_ptr->lengths.size() != fleet_ptr->n_lengths ||
        fleet_ptr->length_bin_edges.size() != fleet_ptr->n_lengths + 1 ||
        !SizeBinMapping::HasStrictlyIncreasingEdges(
            fleet_ptr->length_bin_edges) ||
        age_index >= fleet_ptr->n_ages) {
      return fims::Vector<Type>();
    }

    const fims::Vector<Type> population_prob_size =
        BuildPopulationProbSizeRow(year_index, age_index);
    if (population_prob_size.size() != population_size_grid->n_bins) {
      return fims::Vector<Type>();
    }

    const fims::Vector<double> mapping_fleet_edges =
        SizeBinMapping::ExpandDestinationEdgesToCoverSourceRange(
            population_size_grid->edges,
            fleet_ptr->length_bin_edges);

    const fims::Vector<fims::Vector<double>> rebin_weights =
        SizeBinMapping::BuildRebinWeights(
            population_size_grid->edges,
            mapping_fleet_edges);

    fims::Vector<Type> fleet_row =
        SizeBinMapping::ApplyRebinWeights(rebin_weights, population_prob_size);

    Type row_sum = static_cast<Type>(0.0);
    for (std::size_t length_bin_index = 0;
         length_bin_index < fleet_row.size();
         ++length_bin_index) {
      row_sum += fleet_row[length_bin_index];
    }

    const Type safe_row_sum =
        SizeProbabilityNormalization::SafeDenominator(row_sum);

    for (std::size_t length_bin_index = 0;
         length_bin_index < fleet_row.size();
         ++length_bin_index) {
      fleet_row[length_bin_index] /= safe_row_sum;
    }

    return fleet_row;
  }

  /**
   * @brief Try to get the linked population size provider.
   * @return Shared pointer to the size provider, or nullptr if unavailable.
   */
  std::shared_ptr<SizeDistributionProviderBase<Type>> TryGetSizeProvider() const {
    return size_provider.lock();
  }

  /**
   * @brief Try to get prepared growth products from the linked growth object.
   * @return Pointer to prepared growth products, or nullptr if unavailable.
   */
  const GrowthProducts<Type>* TryGetGrowthProducts() const {
    if (growth_observation == nullptr ||
        !growth_observation->SupportsGrowthDerivedALK()) {
      return nullptr;
    }

    return growth_observation->TryGetPreparedGrowthProducts();
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_GROWTH_DERIVED_ALK_HPP */
