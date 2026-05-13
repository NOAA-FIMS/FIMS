/**
 * @file growth_derived_alk.hpp
 * @brief Declares the GrowthDerivedALK class, which implements ALKBase
 * using growth-derived length-at-age information and fleet length bins.
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

namespace fims_popdy {

/**
 * @brief Growth-derived ALK implementation using fleet length bins and
 * growth-derived length-at-age information.
 */
template <typename Type>
struct GrowthDerivedALK : public ALKBase<Type> {
  std::weak_ptr<Fleet<Type>> fleet; /**< non-owning link to the fleet */
  std::shared_ptr<GrowthDerivedObservationBase<Type>>
      growth_observation; /**< growth-derived observation capability */

  /**
   * @brief Constructor.
   * @param fleet Shared pointer to the fleet using this ALK.
   * @param growth_observation Shared pointer to the growth-derived
   * observation capability.
   */
  GrowthDerivedALK(
      const std::shared_ptr<Fleet<Type>>& fleet,
      const std::shared_ptr<GrowthDerivedObservationBase<Type>>&
          growth_observation)
      : ALKBase<Type>(),
        fleet(fleet),
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

    return fleet_ptr != nullptr &&
           growth_observation != nullptr &&
           growth_observation->SupportsGrowthDerivedALK() &&
           fleet_ptr->n_ages > 0 &&
           fleet_ptr->n_lengths > 0 &&
           fleet_ptr->lengths.size() == fleet_ptr->n_lengths;
  }

  /**
   * @brief Ensures growth-derived products are available for the current model
   * state.
   *
   * This helper reuses already prepared growth products when available and only
   * triggers preparation when products have not yet been prepared.
   *
   * @return True if prepared products are available and support this ALK path.
   */
  virtual bool PrepareForCurrentState() override {
    if (!this->IsActive()) {
      return false;
    }

    const GrowthProducts<Type>* growth_products = TryGetGrowthProducts();
    if (growth_products == nullptr) {
      growth_observation->PrepareGrowthProducts();
      growth_products = TryGetGrowthProducts();
    }

    return growth_products != nullptr && growth_products->n_sexes == 1;
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
    out_row = BuildNormalizedGrowthDerivedALKRow(year, age);
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();
    return fleet_ptr != nullptr && out_row.size() == fleet_ptr->n_lengths;
  }

 protected:
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

  /**
   * @brief Returns the midpoint between two neighboring fleet length bins.
   * @param fleet_ptr Shared pointer to the fleet.
   * @param left_bin Left bin index.
   * @param right_bin Right bin index.
   * @return Midpoint between the two bin centers.
   */
  Type LengthBinMidpoint(const std::shared_ptr<Fleet<Type>>& fleet_ptr,
                         size_t left_bin,
                         size_t right_bin) const {
    return static_cast<Type>(0.5) *
           (fleet_ptr->lengths[left_bin] + fleet_ptr->lengths[right_bin]);
  }

  /**
   * @brief Computes the probability that a fish of a given age falls in one
   * fleet length bin.
   * @param fleet_ptr Shared pointer to the fleet.
   * @param growth_products Reference to the linked growth products.
   * @param year Year index.
   * @param age Age index.
   * @param length_bin Length-bin index.
   * @return Probability mass in the requested length bin.
   */
  Type GrowthDerivedALKProb(const std::shared_ptr<Fleet<Type>>& fleet_ptr,
                            const GrowthProducts<Type>& growth_products,
                            size_t year,
                            size_t age,
                            size_t length_bin) const {
    if (fleet_ptr->n_lengths == 1) {
      return static_cast<Type>(1.0);
    }

    const std::size_t y =
        (growth_products.n_years == 0)
            ? 0
            : std::min(year, growth_products.n_years - 1);

    const Type mean_laa = growth_products.MeanLAA(y, age, 0);
    const Type sd_laa = fims_math::ad_max(
        growth_products.SdLAA(y, age, 0), static_cast<Type>(1e-8));

    if (length_bin == 0) {
      const Type upper = LengthBinMidpoint(fleet_ptr, 0, 1);
      return fims_math::normalcdf(upper, mean_laa, sd_laa);
    }

    if (length_bin + 1 == fleet_ptr->n_lengths) {
      const Type lower =
          LengthBinMidpoint(fleet_ptr,
                            fleet_ptr->n_lengths - 2,
                            fleet_ptr->n_lengths - 1);
      return static_cast<Type>(1.0) -
             fims_math::normalcdf(lower, mean_laa, sd_laa);
    }

    const Type lower = LengthBinMidpoint(fleet_ptr, length_bin - 1, length_bin);
    const Type upper = LengthBinMidpoint(fleet_ptr, length_bin, length_bin + 1);

    return fims_math::normalcdf(upper, mean_laa, sd_laa) -
           fims_math::normalcdf(lower, mean_laa, sd_laa);
  }

  /**
   * @brief Builds a normalized growth-derived ALK row for one year and age.
   * @param year Year index.
   * @param age Age index.
   * @return Normalized age-to-length probability row.
   */
  fims::Vector<Type> BuildNormalizedGrowthDerivedALKRow(size_t year,
                                                        size_t age) const {
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();
    const GrowthProducts<Type>* growth_products = TryGetGrowthProducts();

    if (fleet_ptr == nullptr ||
        growth_products == nullptr ||
        age >= fleet_ptr->n_ages ||
        growth_products->n_sexes != 1 ||
        fleet_ptr->n_ages == 0 ||
        fleet_ptr->n_lengths == 0 ||
        fleet_ptr->lengths.size() != fleet_ptr->n_lengths) {
      return fims::Vector<Type>();
    }

    fims::Vector<Type> alk_row(fleet_ptr->n_lengths);

    Type row_sum = static_cast<Type>(0.0);
    for (size_t l = 0; l < fleet_ptr->n_lengths; ++l) {
      alk_row[l] = GrowthDerivedALKProb(
        fleet_ptr, *growth_products, year, age, l);
      row_sum += alk_row[l];
    }

    const Type safe_row_sum =
        fims_math::ad_max(row_sum, static_cast<Type>(1e-12));

    for (size_t l = 0; l < fleet_ptr->n_lengths; ++l) {
      alk_row[l] /= safe_row_sum;
    }

    return alk_row;
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_GROWTH_DERIVED_ALK_HPP */
