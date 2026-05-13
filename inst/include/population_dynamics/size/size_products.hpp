/**
 * @file size_products.hpp
 * @brief Defines the SizeProducts container for prepared population-level size
 * outputs.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_PRODUCTS_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_PRODUCTS_HPP

#include "../../common/fims_vector.hpp"

namespace fims_popdy {

/**
 * @brief Prepared population-level size products on a biological size grid.
 *
 * This container stores cached size-related outputs for a pooled population,
 * including mean length-at-age, spread-at-age, and age-to-size probabilities
 * on the internal biological size grid.
 *
 * @tparam Type Numeric type used for cached values.
 */
template <typename Type>
struct SizeProducts {
  std::size_t n_years = 0;      /*!< number of modeled years represented */
  std::size_t n_ages = 0;       /*!< number of modeled ages represented */
  std::size_t n_size_bins = 0;  /*!< number of biological size bins represented */

  fims::Vector<Type> mean_LAA;   /*!< mean length-at-age on the natural scale */
  fims::Vector<Type> sd_LAA;     /*!< spread of length-at-age on the natural scale */
  fims::Vector<Type> prob_size;  /*!< age-to-size probabilities on the biological size grid */

  /**
   * @brief Default constructor.
   */
  SizeProducts() {}

  /**
   * @brief Construct and size the size-product container.
   * @param years Number of modeled years.
   * @param ages Number of modeled ages.
   * @param size_bins Number of biological size bins.
   */
  SizeProducts(std::size_t years, std::size_t ages, std::size_t size_bins) {
    Resize(years, ages, size_bins);
  }

  /**
   * @brief Resize all cached product arrays.
   * @param years Number of modeled years.
   * @param ages Number of modeled ages.
   * @param size_bins Number of biological size bins.
   */
  void Resize(std::size_t years, std::size_t ages, std::size_t size_bins) {
    n_years = years;
    n_ages = ages;
    n_size_bins = size_bins;

    const std::size_t n_age_year = n_years * n_ages;
    const std::size_t n_prob = n_years * n_ages * n_size_bins;

    mean_LAA.resize(n_age_year);
    sd_LAA.resize(n_age_year);
    prob_size.resize(n_prob);
  }

  /**
   * @brief Return the flattened index for year-age products.
   * @param y Year index.
   * @param a Age index.
   * @return Flattened year-age index.
   */
  inline std::size_t AgeYearIndex(std::size_t y, std::size_t a) const {
    return y * n_ages + a;
  }

  /**
   * @brief Return the flattened index for year-age-size-bin probabilities.
   * @param y Year index.
   * @param a Age index.
   * @param l Size-bin index.
   * @return Flattened year-age-size-bin index.
   */
  inline std::size_t AgeYearSizeIndex(std::size_t y,
                                      std::size_t a,
                                      std::size_t l) const {
    return y * (n_ages * n_size_bins) + a * n_size_bins + l;
  }

  /**
   * @brief Access mean length-at-age.
   * @param y Year index.
   * @param a Age index.
   * @return Reference to mean length-at-age.
   */
  inline Type& MeanLAA(std::size_t y, std::size_t a) {
    return mean_LAA[AgeYearIndex(y, a)];
  }

  /**
   * @brief Access spread of length-at-age.
   * @param y Year index.
   * @param a Age index.
   * @return Reference to spread of length-at-age.
   */
  inline Type& SdLAA(std::size_t y, std::size_t a) {
    return sd_LAA[AgeYearIndex(y, a)];
  }

  /**
   * @brief Access age-to-size probability.
   * @param y Year index.
   * @param a Age index.
   * @param l Size-bin index.
   * @return Reference to age-to-size probability.
   */
  inline Type& ProbSize(std::size_t y, std::size_t a, std::size_t l) {
    return prob_size[AgeYearSizeIndex(y, a, l)];
  }

  /**
   * @brief Read mean length-at-age.
   * @param y Year index.
   * @param a Age index.
   * @return Const reference to mean length-at-age.
   */
  inline const Type& MeanLAA(std::size_t y, std::size_t a) const {
    return mean_LAA[AgeYearIndex(y, a)];
  }

  /**
   * @brief Read spread of length-at-age.
   * @param y Year index.
   * @param a Age index.
   * @return Const reference to spread of length-at-age.
   */
  inline const Type& SdLAA(std::size_t y, std::size_t a) const {
    return sd_LAA[AgeYearIndex(y, a)];
  }

  /**
   * @brief Read age-to-size probability.
   * @param y Year index.
   * @param a Age index.
   * @param l Size-bin index.
   * @return Const reference to age-to-size probability.
   */
  inline const Type& ProbSize(std::size_t y,
                              std::size_t a,
                              std::size_t l) const {
    return prob_size[AgeYearSizeIndex(y, a, l)];
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_PRODUCTS_HPP */
