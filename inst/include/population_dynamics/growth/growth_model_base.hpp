// inst/include/population_dynamics/growth/growth_model_base.hpp
/**
 * @file growth_model_base.hpp
 * @brief Declares the GrowthModelBase interface. This is the growth module
 * boundary that downstream code consumes, not the curve functors.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_MODEL_BASE_HPP
#define POPULATION_DYNAMICS_GROWTH_MODEL_BASE_HPP

#include "growth_products.hpp"

namespace fims_popdy {

/**
 * @brief Growth module boundary (product-oriented).
 *
 * Downstream code calls Prepare() once (per evaluation/update) and then consumes
 * cached products via GetProducts().
 */
template <typename Type>
class GrowthModelBase {
 public:
  virtual ~GrowthModelBase() = default;

  /// Compute and cache growth products (mean LAA, sd LAA, mean WAA).
  virtual void Prepare() = 0;

  /// Read-only access to cached growth products.
  virtual const GrowthProducts<Type>& GetProducts() const = 0;
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_GROWTH_MODEL_BASE_HPP */
