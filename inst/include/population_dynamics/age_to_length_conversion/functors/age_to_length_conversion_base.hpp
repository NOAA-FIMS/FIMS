/**
 * @file age_to_length_conversion_base.hpp
 * @brief Declares the AgeToLengthConversionBase class which is the base class for all
 * age-length key functors.
 * @details Defines guards for the age-to-length conversion module outline to define the
 * age-to-length conversion hpp file if not already defined.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_BASE_HPP
#define POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_BASE_HPP

#include <cstddef>

#include "../../../common/fims_vector.hpp"
#include "../../../common/model_object.hpp"

namespace fims_popdy {

/**
 * @brief Base interface for age-length key implementations.
 * @tparam Type Numeric type used by the model.
 */
template <typename Type>
struct AgeToLengthConversionBase : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g; /**< reference id for age-to-length conversion object */

  AgeToLengthConversionBase() { this->id = AgeToLengthConversionBase<Type>::id_g++; }

  virtual ~AgeToLengthConversionBase() {}

  /**
   * @brief Returns whether this age-to-length conversion is active and valid for use.
   * @return True if the age-to-length conversion can be used to build rows.
   */
  virtual bool IsActive() const = 0;

  /**
   * @brief Prepare this age-to-length conversion for the current model state.
   * @return True if the age-to-length conversion is ready to build rows for the current state.
   */
  virtual bool PrepareForCurrentState() = 0;

  /**
   * @brief Builds the age-to-length conversion row for a given year and age.
   * @param year Year index.
   * @param age Age index.
   * @param out_row Output age-to-length probability row.
   * @return True if the age-to-length conversion row was built successfully.
   */
  virtual bool BuildAgeToLengthConversionRow(size_t year,
                           size_t age,
                           fims::Vector<Type>& out_row) const = 0;
};

template <typename Type>
uint32_t AgeToLengthConversionBase<Type>::id_g = 0; /**< global counter for age-to-length conversion ids */

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_BASE_HPP */
