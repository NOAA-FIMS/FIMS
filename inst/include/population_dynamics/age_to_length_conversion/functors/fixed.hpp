/**
 * @file fixed.hpp
 * @brief Declares the AgeToLengthConversionFixed class, which implements
 * AgeToLengthConversionBase using a fleet's fixed age-to-length conversion
 * matrix.
 * @details Defines guards for the fixed-matrix age-to-length conversion
 * functor.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_FIXED_HPP
#define POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_FIXED_HPP

// Needed for size_t.
#include <cstddef>

// Needed for shared_ptr and weak_ptr.
#include <memory>

// Base age-to-length conversion interface.
#include "base.hpp"

// Fleet definition, since this age-to-length conversion reads the fleet's fixed
// age-to-length conversion matrix.
#include "population_dynamics/fleet/fleet.hpp"

namespace fims_popdy {

/**
 * @brief Fixed age-to-length conversion implementation using a fleet's stored
 * age-to-length conversion matrix.
 */
template <typename Type>
struct AgeToLengthConversionFixed : public AgeToLengthConversionBase<Type> {
  // Non-owning link back to the fleet. weak_ptr avoids creating an
  // ownership cycle if Fleet later owns an AgeToLengthConversionBase pointer.
  std::weak_ptr<Fleet<Type>> fleet; /**< non-owning link to the fleet */

  /**
   * @brief Constructor.
   * @param fleet Shared pointer to the fleet providing the fixed
   * age-to-length conversion matrix.
   */
  AgeToLengthConversionFixed(const std::shared_ptr<Fleet<Type>>& fleet)
      : AgeToLengthConversionBase<Type>(), fleet(fleet) {}

  /**
   * @brief Destructor.
   */
  virtual ~AgeToLengthConversionFixed() {}

  /**
   * @brief Returns whether this fixed-matrix age-to-length conversion is active
   * and usable.
   * @return True if the fleet has a valid fixed age-to-length matrix.
   */
  virtual bool IsActive() const override {
    // Convert the weak_ptr into a temporary shared_ptr so we can safely
    // access the fleet if it still exists.
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();

    // If the fleet no longer exists, this age-to-length conversion cannot be
    // used.
    if (fleet_ptr == nullptr) {
      return false;
    }

    // The fixed age-to-length conversion is active only if:
    // - the fleet has at least one age
    // - the fleet has at least one length bin
    // - the stored fixed matrix has one age x length table, or one per year
    return fleet_ptr->n_ages > 0 && fleet_ptr->n_lengths > 0 &&
           HasValidSize(*fleet_ptr);
  }

  /**
   * @brief Returns whether the fleet stores one age x length table for all
   * years or one table per year.
   * @param fleet_ref Fleet that holds the fixed age-to-length matrix.
   * @return True if the matrix size matches either layout.
   */
  static bool HasValidSize(const Fleet<Type>& fleet_ref) {
    const size_t table_size = fleet_ref.n_ages * fleet_ref.n_lengths;
    const size_t matrix_size = fleet_ref.age_to_length_conversion.size();
    return matrix_size == table_size ||
           matrix_size == fleet_ref.n_years * table_size;
  }

  /**
   * @brief Prepare the fixed-matrix age-to-length conversion for the current
   * model state.
   * @return True if the fixed matrix is active and usable.
   */
  virtual bool PrepareForCurrentState() override { return this->IsActive(); }

  /**
   * @brief Builds the fixed age-to-length conversion row for a given age.
   * @param year Year index. Used only when the fleet stores one table per year.
   * @param age Age index.
   * @param out_row Output age-to-length probability row.
   * @return True if the age-to-length conversion row was built successfully.
   */
  virtual bool BuildAgeToLengthConversionRow(
      size_t year, size_t age, fims::Vector<Type>& out_row) const override {
    // Safely access the linked fleet.
    std::shared_ptr<Fleet<Type>> fleet_ptr = fleet.lock();

    // Stop if:
    // - the fleet no longer exists
    // - dimensions are invalid
    // - the requested age or year is out of range
    // - the fixed matrix does not have the expected size
    if (fleet_ptr == nullptr || fleet_ptr->n_ages == 0 ||
        fleet_ptr->n_lengths == 0 || age >= fleet_ptr->n_ages ||
        !HasValidSize(*fleet_ptr)) {
      return false;
    }
    const size_t table_size = fleet_ptr->n_ages * fleet_ptr->n_lengths;
    const bool has_table_per_year =
        fleet_ptr->age_to_length_conversion.size() != table_size;
    if (has_table_per_year && year >= fleet_ptr->n_years) {
      return false;
    }

    // Resize the output row so it has one entry per fleet length bin.
    out_row.resize(fleet_ptr->n_lengths);

    // Compute the starting position for this age in the flattened
    // year x age x length matrix. A single table is shared by every year.
    const size_t year_offset = has_table_per_year ? year * table_size : 0;
    const size_t row_offset = year_offset + age * fleet_ptr->n_lengths;

    // Copy the fixed age-to-length probabilities for this age into out_row.
    for (size_t l = 0; l < fleet_ptr->n_lengths; ++l) {
      out_row[l] = fleet_ptr->age_to_length_conversion[row_offset + l];
    }

    // Report success.
    return true;
  }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_FIXED_HPP */
