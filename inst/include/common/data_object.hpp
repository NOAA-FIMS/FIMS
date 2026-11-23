/**
 * @file data_object.hpp
 * @brief Sets up a data class to create a generic data object with up to five
 * dimensions. The class contains internal vector for data elements and
 * uncertainty values.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_COMMON_DATA_OBJECT_HPP
#define FIMS_COMMON_DATA_OBJECT_HPP

#include <exception>
#include <vector>
#include <numeric>
#include <functional>

#include "model_object.hpp"
#include "fims_vector.hpp"

namespace fims_data_object {

/**
 * Container to hold user supplied data.
 */
template <typename Type>
struct DataObject : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g;                    /**< id of the Data Object >*/
  fims::Vector<Type> data;                 /**< vector of the data >*/
  fims::Vector<Type> uncertainty;          /**< vector of the data >*/
  fims::Vector<size_t> generic_dimensions; /**< vector indicating NA values >*/
  size_t dimensions;                       /**< dimension of the Data object >*/
  size_t imax;                             /**< 1st dimension of data object >*/
  size_t jmax;                             /**< 2nd dimension of data object>*/
  size_t kmax;                             /**< 3rd dimension of data object>*/
  size_t lmax;                             /**< 4th dimension of data object>*/
  Type na_value = static_cast<Type>(-999); /**< specifying the NA value >*/

  /**
   * Constructs a multi-dimensional data object.
   */
  DataObject(fims::Vector<int> dim) : dimensions(dim.size()) {
    generic_dimensions.resize(dim.size());
    for (size_t i = 0; i < dim.size(); ++i) {
      generic_dimensions.at(i) = dim[i];
    }
    data.resize(
        std::accumulate(dim.begin(), dim.end(), 1, std::multiplies<int>()));
    uncertainty.resize(
        std::accumulate(dim.begin(), dim.end(), 1, std::multiplies<int>()));
    this->id = DataObject<Type>::id_g++;
    this->register_self(this->id);
  }

  /**
   * Constructs a one-dimensional data object.
   */
  DataObject(size_t imax) : dimensions(1), imax(imax) {
    generic_dimensions.resize(1);
    generic_dimensions.at(0) = imax;
    data.resize(imax);
    uncertainty.resize(imax);
    this->id = DataObject<Type>::id_g++;
    this->register_self(this->id);
  }

  /**
   * Constructs a two-dimensional data object.
   */
  DataObject(size_t imax, size_t jmax) : dimensions(2), imax(imax), jmax(jmax) {
    generic_dimensions.resize(2);
    generic_dimensions.at(0) = imax;
    generic_dimensions.at(1) = jmax;
    data.resize(imax * jmax);
    uncertainty.resize(imax * jmax);
    this->id = DataObject<Type>::id_g++;
    this->register_self(this->id);
  }

  /**
   *  Constructs a three-dimensional data object.
   */
  DataObject(size_t imax, size_t jmax, size_t kmax)
      : dimensions(3), imax(imax), jmax(jmax), kmax(kmax) {
    generic_dimensions.resize(3);
    generic_dimensions.at(0) = imax;
    generic_dimensions.at(1) = jmax;
    generic_dimensions.at(2) = kmax;
    data.resize(imax * jmax * kmax);
    uncertainty.resize(imax * jmax * kmax);
    this->id = DataObject<Type>::id_g++;
    this->register_self(this->id);
  }

  /**
   * Constructs a four-dimensional data object.
   */
  DataObject(size_t imax, size_t jmax, size_t kmax, size_t lmax)
      : dimensions(4), imax(imax), jmax(jmax), kmax(kmax), lmax(lmax) {
    generic_dimensions.resize(4);
    generic_dimensions.at(0) = imax;
    generic_dimensions.at(1) = jmax;
    generic_dimensions.at(2) = kmax;
    generic_dimensions.at(3) = lmax;
    data.resize(imax * jmax * kmax * lmax);
    uncertainty.resize(imax * jmax * kmax * lmax);
    this->id = DataObject<Type>::id_g++;
    this->register_self(this->id);
  }

  /**
   * Retrieve element from 1d data set.
   * @param i dimension of 1d data set
   * @return the value of the vector at position i
   */
  inline Type operator()(size_t i) { return data[i]; }

  /**
   * Retrieve element from 1d data set.
   * Throws an exception if index is out of bounds.
   * @param i dimension of 1d data set
   * @return the reference to the value of the vector at position i
   */
  inline Type& at(size_t i) {
    if (i >= this->data.size()) {
      throw std::overflow_error("DataObject error:i index out of bounds");
    }
    return data[i];
  }

  /**
   * Retrieve element from 2d data set.
   * @param i 1st dimension of 2d data set
   * @param j 2nd dimension of 2d data set
   * @return the value of the matrix at position i, j
   */
  inline const Type operator()(size_t i, size_t j) {
    return data[i * jmax + j];
  }

  /**
   * Retrieve element from 2d data set.
   * Throws an exception if index is out of bounds.
   * @param i 1st dimension of 2d data set
   * @param j 2nd dimension of 2d data set
   * @return the reference to the value of the matrix at position i, j
   */
  inline Type& at(size_t i, size_t j) {
    if ((i * jmax + j) >= this->data.size()) {
      throw std::overflow_error("DataObject error: index out of bounds");
    }
    return data[i * jmax + j];
  }

  /**
   * Retrieve element from 3d data set.
   * @param i 1st dimension of 3d data set
   * @param j 2nd dimension of 3d data set
   * @param k 3rd dimension of 3d data set
   * @return the value of the array at position i, j, k
   */
  inline const Type operator()(size_t i, size_t j, size_t k) {
    return data[i * jmax * kmax + j * kmax + k];
  }

  /**
   * Retrieve element from 3d data set.
   * Throws an exception if index is out of bounds.
   * @param i 1st dimension of 3d data set
   * @param j 2nd dimension of 3d data set
   * @param k 3rd dimension of 3d data set
   * @return the reference to the value of the array at position i, j, k
   */
  inline Type& at(size_t i, size_t j, size_t k) {
    if ((i * jmax * kmax + j * kmax + k) >= this->data.size()) {
      throw std::overflow_error("DataObject error: index out of bounds");
    }
    return data[i * jmax * kmax + j * kmax + k];
  }

  /**
   * Retrieve element from 4d data set.
   * @param i 1st dimension of 4d data set
   * @param j 2nd dimension of 4d data set
   * @param k 3rd dimension of 4d data set
   * @param l 4th dimension of 4d data set
   * @return the value of the array at position i, j, k, l
   */
  inline const Type operator()(size_t i, size_t j, size_t k, size_t l) {
    return data[i * jmax * kmax * lmax + j * kmax * lmax + k * lmax + l];
  }

  /**
   * Retrieve element from 3d data set.
   * Throws an exception if index is out of bounds.
   * @param i 1st dimension of 4d data set
   * @param j 2nd dimension of 4d data set
   * @param k 3rd dimension of 4d data set
   * @param l 4th dimension of 4d data set
   * @return the reference to the value of the array at position i, j, k, l
   */
  inline Type& at(size_t i, size_t j, size_t k, size_t l) {
    if ((i * jmax * kmax * lmax + j * kmax * lmax + k * lmax + l) >=
        this->data.size()) {
      throw std::overflow_error("DataObject error: index out of bounds");
    }
    return data[i * jmax * kmax * lmax + j * kmax * lmax + k * lmax + l];
  }

  /**
   * @brief Get the dimensions object
   *
   * @return size_t
   */
  size_t get_dimensions() const { return dimensions; }

  /**
   * @brief Get the generic dimensions object vector
   *
   * @return std::vector<size_t>
   */
  fims::Vector<size_t> get_dimension_vector() const {
    return generic_dimensions;
  }

  /**
   * @brief Get the imax object
   *
   * @return size_t
   */
  size_t get_imax() const { return imax; }

  /**
   * @brief Get the jmax object
   *
   * @return size_t
   */
  size_t get_jmax() const { return jmax; }

  /**
   * @brief Get the kmax object
   *
   * @return size_t
   */
  size_t get_kmax() const { return kmax; }

  /**
   * @brief Get the lmax object
   *
   * @return size_t
   */
  size_t get_lmax() const { return lmax; }
};

template <typename Type>
uint32_t DataObject<Type>::id_g = 0;

}  // namespace fims_data_object

#endif
