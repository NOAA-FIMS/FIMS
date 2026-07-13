/**
 * @file size_products.hpp
 * @brief Defines the SizeProducts container for cached population-level size
 * outputs on the biological size grid.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_PRODUCTS_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_PRODUCTS_HPP

#include <cstddef>
#include <stdexcept>

#include "../../common/fims_vector.hpp"

namespace fims_popdy {

/**
 * @brief Cached population-level size outputs on a biological size grid.
 *
 * This container can be used by a size provider to store prepared size-related
 * outputs for a pooled population, including mean length-at-age,
 * spread-at-age, and age-to-size probabilities on the internal biological
 * size grid.
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
   * @brief Resize only the cached age-to-size probability array.
   * @param years Number of modeled years.
   * @param ages Number of modeled ages.
   * @param size_bins Number of biological size bins.
   *
   * This is used by providers that read MeanLAA and SdLAA from upstream growth
   * products and only own ProbSize locally.
   */
  void ResizeProbSizeOnly(std::size_t years,
                          std::size_t ages,
                          std::size_t size_bins) {
    n_years = years;
    n_ages = ages;
    n_size_bins = size_bins;

    mean_LAA.clear();
    sd_LAA.clear();

    const std::size_t n_prob = n_years * n_ages * n_size_bins;
    prob_size.resize(n_prob);
  }

  /**
   * @brief Return the flattened index for year-age products.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Flattened year-age index.
   */
  inline std::size_t AgeYearIndex(std::size_t year_index,
                                  std::size_t age_index) const {
    if (year_index >= n_years || age_index >= n_ages) {
      throw std::out_of_range("SizeProducts year-age index out of range");
    }

    return year_index * n_ages + age_index;
  }

  /**
   * @brief Return the flattened index for year-age-size-bin probabilities.
   * @param year_index Year index.
   * @param age_index Age index.
   * @param size_bin_index Size-bin index.
   * @return Flattened year-age-size-bin index.
   */
  inline std::size_t AgeYearSizeIndex(std::size_t year_index,
                                      std::size_t age_index,
                                      std::size_t size_bin_index) const {
    if (year_index >= n_years ||
        age_index >= n_ages ||
        size_bin_index >= n_size_bins) {
      throw std::out_of_range("SizeProducts year-age-size index out of range");
    }

    return year_index * (n_ages * n_size_bins) +
           age_index * n_size_bins +
           size_bin_index;
  }

  /**
   * @brief Access mean length-at-age.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Reference to mean length-at-age.
   */
  inline Type& MeanLAA(std::size_t year_index, std::size_t age_index) {
    return mean_LAA[AgeYearIndex(year_index, age_index)];
  }

  /**
   * @brief Access spread of length-at-age.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Reference to spread of length-at-age.
   */
  inline Type& SdLAA(std::size_t year_index, std::size_t age_index) {
    return sd_LAA[AgeYearIndex(year_index, age_index)];
  }

  /**
   * @brief Access age-to-size probability.
   * @param year_index Year index.
   * @param age_index Age index.
   * @param size_bin_index Size-bin index.
   * @return Reference to age-to-size probability.
   */
  inline Type& ProbSize(std::size_t year_index, std::size_t age_index, std::size_t size_bin_index) {
    return prob_size[AgeYearSizeIndex(year_index, age_index, size_bin_index)];
  }

  /**
   * @brief Read mean length-at-age.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Const reference to mean length-at-age.
   */
  inline const Type& MeanLAA(std::size_t year_index, std::size_t age_index) const {
    return mean_LAA[AgeYearIndex(year_index, age_index)];
  }

  /**
   * @brief Read spread of length-at-age.
   * @param year_index Year index.
   * @param age_index Age index.
   * @return Const reference to spread of length-at-age.
   */
  inline const Type& SdLAA(std::size_t year_index, std::size_t age_index) const {
    return sd_LAA[AgeYearIndex(year_index, age_index)];
  }

  /**
   * @brief Read age-to-size probability.
   * @param year_index Year index.
   * @param age_index Age index.
   * @param size_bin_index Size-bin index.
   * @return Const reference to age-to-size probability.
   */
  inline const Type& ProbSize(std::size_t year_index,
                              std::size_t age_index,
                              std::size_t size_bin_index) const {
    return prob_size[AgeYearSizeIndex(year_index, age_index, size_bin_index)];
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_PRODUCTS_HPP */
