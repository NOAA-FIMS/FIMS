/**
 * @file size_distribution_provider_base.hpp
 * @brief Defines the SizeDistributionProviderBase interface for prepared
 * population-level size products.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_DISTRIBUTION_PROVIDER_BASE_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_DISTRIBUTION_PROVIDER_BASE_HPP

#include "size_grid.hpp"


namespace fims_popdy {

/**
 * @brief Base interface for objects that provide population-level size
 * distributions.
 *
 * Implementations of this interface prepare and expose pooled population-level
 * size products on a biological size grid. The interface is intentionally
 * separate from catch-at-age so other FIMS modules can consume size products
 * without depending on CAA-specific logic.
 *
 * @tparam Type Numeric type used for prepared size products.
 */
template <typename Type>
class SizeDistributionProviderBase {
 public:
  SizeDistributionProviderBase() {}
  virtual ~SizeDistributionProviderBase() = default;

  /**
   * @brief Return the biological size grid used by this provider.
   * @return Pointer to the biological size grid, or nullptr if unavailable.
   */
  virtual const SizeGrid* TryGetSizeGrid() const = 0;

  /**
   * @brief Configure the provider to use the canonical population size grid.
   * @param size_grid Canonical population-level biological size grid.
   *
   * Providers should prepare any grid-based size products on this grid rather
   * than creating an independent biological size definition.
   */
  virtual void SetPopulationSizeGrid(const SizeGrid* size_grid) = 0;

  /**
   * @brief Configure the provider dimensions for prepared size products.
   * @param n_years Number of modeled years represented.
   * @param n_ages Number of modeled ages represented.
   *
   * Providers use these dimensions together with the canonical population size
   * grid to size any prepared population-level size products.
   */
  virtual void SetPopulationDimensions(std::size_t n_years,
                                       std::size_t n_ages) = 0;

  /**
   * @brief Prepare size products for the current model state.
   */
  virtual void PrepareSizeProducts() = 0;

  /**
   * @brief Invalidate any prepared size state held by this provider.
   *
   * This is called when upstream biological inputs or the canonical population
   * size grid change and previously prepared size products should no longer be
   * reused.
   */
  virtual void InvalidatePreparedSizeProducts() = 0;

  /**
   * @brief Read prepared mean length-at-age.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Read-only mean length-at-age value.
   */
  virtual const Type& MeanLAA(std::size_t year_index, std::size_t age_index) const = 0;

  /**
   * @brief Read prepared spread of length-at-age.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Read-only spread of length-at-age value.
   */
  virtual const Type& SdLAA(std::size_t year_index, std::size_t age_index) const = 0;

  /**
   * @brief Read prepared age-to-size probability on the population grid.
   * @param year_index Year index.
   * @param age_index Age index.
   * @param size_bin_index Size-bin index.
   * @return Read-only age-to-size probability value.
   */
  virtual const Type& ProbSize(std::size_t year_index,
                               std::size_t age_index,
                               std::size_t size_bin_index) const = 0;
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_DISTRIBUTION_PROVIDER_BASE_HPP */
