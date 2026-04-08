/**
 * @file fixed_matrix_alk.hpp
 * @brief Declares the FixedMatrixALK class, which implements ALKBase
 * using a fleet's fixed age-to-length conversion matrix.
 * @details Defines guards for the fixed-matrix ALK functor.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_FIXED_MATRIX_ALK_HPP
#define POPULATION_DYNAMICS_FIXED_MATRIX_ALK_HPP

// Needed for size_t.
#include <cstddef>

// Needed for shared_ptr and weak_ptr.
#include <memory>

// Base ALK interface.
#include "alk_base.hpp"

// Fleet definition, since this ALK reads the fleet's fixed
// age-to-length conversion matrix.
#include "../../fleet/fleet.hpp"

namespace fims_popdy {

/**
 * @brief Fixed ALK implementation using a fleet's stored
 * age-to-length conversion matrix.
 */
template <typename Type>
struct FixedMatrixALK : public ALKBase<Type> {
  // Non-owning link back to the fleet. weak_ptr avoids creating an
  // ownership cycle if Fleet later owns an ALKBase pointer.
  std::weak_ptr<Fleet<Type>> fleet; /**< non-owning link to the fleet */

  /**
   * @brief Constructor.
   * @param fleet Shared pointer to the fleet providing the fixed
   * age-to-length conversion matrix.
   */
  FixedMatrixALK(const std::shared_ptr<Fleet<Type>>& fleet)
      : ALKBase<Type>(), fleet(fleet) {}

  /**
   * @brief Destructor.
   */
  virtual ~FixedMatrixALK() {}

  /**
   * @brief Returns whether this fixed-matrix ALK is active and usable.
   * @return True if the fleet has a valid fixed age-to-length matrix.
   */
  virtual bool IsActive() const override {
    // Convert the weak_ptr into a temporary shared_ptr so we can safely
    // access the fleet if it still exists.
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();

    // If the fleet no longer exists, this ALK cannot be used.
    if (fleet_ptr == nullptr) {
      return false;
    }

    // The fixed ALK is active only if:
    // - the fleet has at least one age
    // - the fleet has at least one length bin
    // - the stored fixed matrix has the expected age x length size
    return fleet_ptr->n_ages > 0 &&
           fleet_ptr->n_lengths > 0 &&
           fleet_ptr->age_to_length_conversion.size() ==
               (fleet_ptr->n_ages * fleet_ptr->n_lengths);
  }

  /**
   * @brief Builds the fixed ALK row for a given age.
   * @param year Year index. Unused for the current fixed-matrix path.
   * @param age Age index.
   * @param out_row Output age-to-length probability row.
   * @return True if the ALK row was built successfully.
   */
  virtual bool BuildALKRow(size_t year,
                           size_t age,
                           fims::Vector<Type>& out_row) const override {
    // The current fixed matrix path does not vary by year, but year stays
    // in the interface so all ALK types share the same method signature.
    (void)year;

    // Safely access the linked fleet.
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();

    // Stop if:
    // - the fleet no longer exists
    // - dimensions are invalid
    // - the requested age is out of range
    // - the fixed matrix does not have the expected size
    if (fleet_ptr == nullptr ||
        fleet_ptr->n_ages == 0 ||
        fleet_ptr->n_lengths == 0 ||
        age >= fleet_ptr->n_ages ||
        fleet_ptr->age_to_length_conversion.size() !=
            (fleet_ptr->n_ages * fleet_ptr->n_lengths)) {
      return false;
    }

    // Resize the output row so it has one entry per fleet length bin.
    out_row.resize(fleet_ptr->n_lengths);

    // Compute the starting position for this age in the flattened
    // age x length matrix.
    const size_t row_offset = age * fleet_ptr->n_lengths;

    // Copy the fixed age-to-length probabilities for this age into out_row.
    for (size_t l = 0; l < fleet_ptr->n_lengths; ++l) {
      out_row[l] = fleet_ptr->age_to_length_conversion[row_offset + l];
    }

    // Report success.
    return true;
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_FIXED_MATRIX_ALK_HPP */
