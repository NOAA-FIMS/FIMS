/**
 * @file age_to_length_conversion_derived.hpp
 * @brief Declares the AgeToLengthConversionDerived class, which implements AgeToLengthConversionBase
 * by mapping prepared population age-to-size distributions onto fleet
 * observation bins.
 * @details Defines guards for the growth-derived age-to-length conversion functor.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_DERIVED_HPP
#define POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_DERIVED_HPP

#include <algorithm>
#include <cstddef>
#include <memory>

#include "../../../common/fims_math.hpp"
#include "age_to_length_conversion_base.hpp"
#include "../../fleet/fleet.hpp"
#include "../../growth/growth_model_adapter.hpp"
#include "../../size/size_distribution_provider_base.hpp"
#include "../../size/functors/size_bin_mapping.hpp"
#include "../../size/functors/size_probability_normalization.hpp"

namespace fims_popdy {

/**
 * @brief Growth-derived age-to-length conversion implementation that reads prepared population
 * age-to-size probabilities and maps them into fleet observation bins.
 */
template <typename Type>
struct AgeToLengthConversionDerived : public AgeToLengthConversionBase<Type> {
  std::weak_ptr<Fleet<Type>> fleet_; /**< non-owning link to the fleet */
  std::weak_ptr<SizeDistributionProviderBase<Type>>
      size_provider_; /**< non-owning link to the population size provider */
  std::shared_ptr<GrowthDerivedObservationBase<Type>>
      growth_observation_; /**< growth-derived observation capability */

  /**
   * @brief Constructor.
   * @param fleet_ Shared pointer to the fleet using this age-to-length conversion.
   * @param growth_observation_ Shared pointer to the growth-derived
   * observation capability.
   * @param size_provider_ Shared pointer to the population size provider.
   */
  AgeToLengthConversionDerived(
      const std::shared_ptr<Fleet<Type>>& fleet_,
      const std::shared_ptr<GrowthDerivedObservationBase<Type>>&
          growth_observation_,
      const std::shared_ptr<SizeDistributionProviderBase<Type>>&
          size_provider_)
      : AgeToLengthConversionBase<Type>(),
        fleet_(fleet_),
        size_provider_(size_provider_),
        growth_observation_(growth_observation_) {}

  /**
   * @brief Destructor.
   */
  virtual ~AgeToLengthConversionDerived() {}

  /**
   * @brief Returns whether this growth-derived age-to-length conversion is structurally active.
   * @return True if the linked fleet, population size provider, and growth
   * observation are valid and the fleet has consistent observation-bin geometry.
   */
  virtual bool IsActive() const override {
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet_.lock();
    std::shared_ptr<SizeDistributionProviderBase<Type>> size_provider_ptr =
        size_provider_.lock();

    return fleet_ptr != nullptr &&
           size_provider_ptr != nullptr &&
           growth_observation_ != nullptr &&
           growth_observation_->SupportsAgeToLengthConversionDerived() &&
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
   * prepares the linked population size provider so the age-to-length conversion can read
   * population age-to-size rows for mapping into fleet observation bins.
   *
   * @return True if the required growth products are available for this age-to-length conversion
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
      growth_observation_->PrepareGrowthProducts();
      growth_products = TryGetGrowthProducts();
    }

    if (growth_products == nullptr || growth_products->n_sexes != 1) {
      return false;
    }

    size_provider_ptr->PrepareSizeProducts();
    return true;
  }

  /**
   * @brief Builds the normalized age-to-length conversion row for a given year and age.
   * @param year Year index.
   * @param age Age index.
   * @param out_row Output age-to-length probability row.
   * @return True if the age-to-length conversion row was built successfully using prepared growth
   * products for the current model state.
   */
  virtual bool BuildAgeToLengthConversionRow(size_t year,
                           size_t age,
                           fims::Vector<Type>& out_row) const override {
    out_row = BuildMappedFleetAgeToLengthConversionRow(year, age);
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet_.lock();
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

    if (!TryFinalizePreparedProbabilityRow(
            prob_size_row, population_size_grid->n_bins)) {
      throw std::runtime_error(
          "AgeToLengthConversionDerived received an invalid prepared population size row");
    }

    return prob_size_row;
  }

  /**
   * @brief Clamp tiny negative roundoff and reject materially invalid
   * probability rows.
   * @param row Probability row to sanitize in place.
   * @param expected_size Expected number of bins.
   * @param negative_tolerance Smallest allowed negative value before failing.
   * @return True if the row is usable after sanitization.
   */
  bool TrySanitizeProbabilityRowInPlace(
      fims::Vector<Type>& row,
      std::size_t expected_size,
      const Type& negative_tolerance = static_cast<Type>(1e-12)) const {
    if (row.size() != expected_size) {
      return false;
    }

    for (std::size_t i = 0; i < row.size(); ++i) {
      if (!(row[i] >= -negative_tolerance)) {
        return false;
      }

      if (row[i] < static_cast<Type>(0.0)) {
        row[i] = static_cast<Type>(0.0);
      }
    }

    return true;
  }

  /**
   * @brief Validate a prepared population probability row and renormalize small
   * roundoff.
   * @param row Prepared population probability row.
   * @param expected_size Expected number of bins.
   * @param minimum_mass Minimum acceptable total probability mass.
   * @param mass_tolerance Allowed deviation from unit mass before failing.
   * @return True if the row is a usable prepared probability row.
   */
  bool TryFinalizePreparedProbabilityRow(
      fims::Vector<Type>& row,
      std::size_t expected_size,
      const Type& minimum_mass = static_cast<Type>(1e-12)) const {
    if (!TrySanitizeProbabilityRowInPlace(row, expected_size)) {
      return false;
    }

    Type row_sum = static_cast<Type>(0.0);
    for (std::size_t i = 0; i < row.size(); ++i) {
      row_sum += row[i];
    }

    if (!(row_sum > minimum_mass)) {
      return false;
    }

    for (std::size_t i = 0; i < row.size(); ++i) {
      row[i] /= row_sum;
    }

    return true;
  }

  /**
   * @brief Validate and normalize one mapped fleet probability row.
   * @param row Fleet probability row on observation bins.
   * @param expected_size Expected number of fleet bins.
   * @param minimum_mass Minimum acceptable total probability mass.
   * @return True if the mapped fleet row can be used downstream.
   */
  bool TryFinalizeMappedProbabilityRow(
      fims::Vector<Type>& row,
      std::size_t expected_size,
      const Type& minimum_mass = static_cast<Type>(1e-12),
      const Type& minimum_bin_prob = static_cast<Type>(1e-12)) const {
    if (!TrySanitizeProbabilityRowInPlace(row, expected_size)) {
      return false;
    }

    Type row_sum = static_cast<Type>(0.0);
    for (std::size_t i = 0; i < row.size(); ++i) {
      if (row[i] < minimum_bin_prob) {
        row[i] = minimum_bin_prob;
      }
      row_sum += row[i];
    }

    if (!(row_sum > minimum_mass)) {
      return false;
    }

    for (std::size_t i = 0; i < row.size(); ++i) {
      row[i] /= row_sum;
    }

    return true;
  }

  /**
   * @brief Build one fleet age-to-length conversion row by mapping a population age-to-size row
   * onto fleet observation bins.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Normalized fleet observation-bin probability row.
   */
  fims::Vector<Type> BuildMappedFleetAgeToLengthConversionRow(std::size_t year_index,
                                            std::size_t age_index) const {
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet_.lock();
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

    if (fleet_ptr->n_years == 0 ||
        fleet_ptr->n_lengths == 0 ||
        fleet_ptr->lengths.size() != fleet_ptr->n_lengths ||
        fleet_ptr->length_bin_edges.size() != fleet_ptr->n_lengths + 1 ||
        !SizeBinMapping::HasStrictlyIncreasingEdges(
            fleet_ptr->length_bin_edges) ||
        year_index >= fleet_ptr->n_years ||
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

    if (!TryFinalizeMappedProbabilityRow(
            fleet_row, fleet_ptr->n_lengths)) {
      throw std::runtime_error(
          "AgeToLengthConversionDerived produced an invalid mapped fleet probability row");
    }

    return fleet_row;
  }

  /**
   * @brief Try to get the linked population size provider.
   * @return Shared pointer to the size provider, or nullptr if unavailable.
   */
  std::shared_ptr<SizeDistributionProviderBase<Type>> TryGetSizeProvider() const {
    return size_provider_.lock();
  }

  /**
   * @brief Try to get prepared growth products from the linked growth object.
   * @return Pointer to prepared growth products, or nullptr if unavailable.
   */
  const GrowthProducts<Type>* TryGetGrowthProducts() const {
    if (growth_observation_ == nullptr ||
        !growth_observation_->SupportsAgeToLengthConversionDerived()) {
      return nullptr;
    }

    return growth_observation_->TryGetPreparedGrowthProducts();
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_DERIVED_HPP */
