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
#include "size_products.hpp"

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
   * @brief Prepare size products for the current model state.
   */
  virtual void PrepareSizeProducts() = 0;

  /**
   * @brief Return prepared size products without triggering preparation.
   * @return Pointer to prepared size products, or nullptr if unavailable.
   */
  virtual const SizeProducts<Type>* TryGetPreparedSizeProducts() const = 0;
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_DISTRIBUTION_PROVIDER_BASE_HPP */
