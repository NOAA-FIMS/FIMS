// inst/include/population_dynamics/growth/growth_products.hpp
/**
 * @file growth_products.hpp
 * @brief Declares the GrowthProducts struct holding growth "products" that
 * downstream population dynamics code consumes (year, age, sex).
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_PRODUCTS_HPP
#define POPULATION_DYNAMICS_GROWTH_PRODUCTS_HPP

#include <cstddef>
#include <vector>
#include <stdexcept>

namespace fims_popdy {

/**
 * @brief Growth "products" in a consistent (year, age, sex) space.
 *
 * Storage is contiguous in row-major order:
 * idx = ((y * n_ages + a) * n_sexes + s)
 */
template <typename Type>
struct GrowthProducts {
  std::size_t n_years = 0; /**< number of years represented */
  std::size_t n_ages = 0; /**< number of ages represented */
  std::size_t n_sexes = 0; /**< number of sexes represented */

  std::vector<Type> mean_LAA; /**< mean length-at-age values */
  std::vector<Type> sd_LAA; /**< standard deviation of length-at-age values */
  std::vector<Type> mean_WAA; /**< mean weight-at-age values */

  /**
   * @brief Construct an empty growth-product container.
   */
  GrowthProducts() = default;

  /**
   * @brief Construct and size the growth-product container.
   * @param years Number of years.
   * @param ages Number of ages.
   * @param sexes Number of sexes.
   */
  GrowthProducts(std::size_t years, std::size_t ages, std::size_t sexes) {
    Resize(years, ages, sexes);
  }

  /**
   * @brief Resize all cached product arrays.
   * @param years Number of years.
   * @param ages Number of ages.
   * @param sexes Number of sexes.
   */
  void Resize(std::size_t years, std::size_t ages, std::size_t sexes) {
    n_years = years;
    n_ages = ages;
    n_sexes = sexes;

    const std::size_t n = n_years * n_ages * n_sexes;

    mean_LAA.assign(n, static_cast<Type>(0));
    sd_LAA.assign(n, static_cast<Type>(0));
    mean_WAA.assign(n, static_cast<Type>(0));
  }

  /**
   * @brief Convert a (year, age, sex) triple into the contiguous storage index.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Flattened storage index.
   */
  inline std::size_t Index(std::size_t y, std::size_t a, std::size_t s) const {
    // Cheap runtime safety. Keep it here, not sprinkled everywhere.
    if (y >= n_years || a >= n_ages || s >= n_sexes) {
      throw std::out_of_range("GrowthProducts index out of range");
    }
    return (y * n_ages + a) * n_sexes + s;
  }

  /**
   * @brief Access mean length-at-age by reference.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Mutable reference to the requested value.
   */
  inline Type& MeanLAA(std::size_t y, std::size_t a, std::size_t s) {
    return mean_LAA[Index(y, a, s)];
  }

  /**
   * @brief Access standard deviation of length-at-age by reference.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Mutable reference to the requested value.
   */
  inline Type& SdLAA(std::size_t y, std::size_t a, std::size_t s) {
    return sd_LAA[Index(y, a, s)];
  }

  /**
   * @brief Access mean weight-at-age by reference.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Mutable reference to the requested value.
   */
  inline Type& MeanWAA(std::size_t y, std::size_t a, std::size_t s) {
    return mean_WAA[Index(y, a, s)];
  }

  /**
   * @brief Access mean length-at-age as a const reference.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Const reference to the requested value.
   */
  inline const Type& MeanLAA(std::size_t y, std::size_t a, std::size_t s) const {
    return mean_LAA[Index(y, a, s)];
  }

  /**
   * @brief Access standard deviation of length-at-age as a const reference.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Const reference to the requested value.
   */
  inline const Type& SdLAA(std::size_t y, std::size_t a, std::size_t s) const {
    return sd_LAA[Index(y, a, s)];
  }

  /**
   * @brief Access mean weight-at-age as a const reference.
   * @param y Year index.
   * @param a Age index.
   * @param s Sex index.
   * @return Const reference to the requested value.
   */
  inline const Type& MeanWAA(std::size_t y, std::size_t a, std::size_t s) const {
    return mean_WAA[Index(y, a, s)];
  }

  /**
   * @brief Return the total number of cached cells.
   * @return Total element count of the cached vectors.
   */
  inline std::size_t Size() const { return mean_LAA.size(); }
};

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_GROWTH_PRODUCTS_HPP */
