/**
 * @file variable_object.hpp
 * @brief Sets up a data class to create a generic variable object with multiple
 * dimensions. The class contains internal vector for elements and
 * uncertainty values.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_COMMON_VARIABLE_OBJECT_HPP
#define FIMS_COMMON_VARIABLE_OBJECT_HPP

#include <exception>
#include <vector>

#include "model_object.hpp"
#include "fims_vector.hpp"

namespace fims_variable_object {

enum class VariableKind {
    DETERMINISTIC,
    FIXED,
    RANDOM,
    DERIVED
};

/**
 * @brief Structure to hold information for derived quantities.
 */
struct VariableInfo {
  
};


/**
 * Container to hold user supplied data.
 */
template <typename Type>
struct VariableObject : public fims_model_object::FIMSObject<Type> {
  static uint32_t id_g;                    /**< id of the Variable Object >*/
  std::string name;                    /*!< name of the variable */
  VariableKind kind;                    /*!< the kind of variable*/
  uint32_t ndims;                           /*!< number of dimensions */
  uint32_t nvals;                             /**< number of values >*/
  std::vector<size_t> dims;              /*!< vector of dimensions */
  std::vector<std::string> dim_names; /*!< vector of dimension names */
  fims::Vector<Type> values;           /*!< vector of values */
  fims::Vector<Type> uncertainty;    /*!< vector of uncertainty values */
  Type na_value = static_cast<Type>(-999); /**< specifying the NA value >*/

  //NOTE: Does it make sense to add options to include vectors to store
  //transformed values inside the variable object? For example, log, exp, etc. 
  //This would be useful for parameters that are estimated on a transformed scale,
  //but are used inside FIMS as natural values. It could also be useful if 
  //reporting is done on a transformed scale. This would require adding tracking
  //of the transformations needed but could be cleaner than tracking individual
  //parameters for each of the transformations such as log_M and M separately.

  /**
   * @brief Default constructor for variable.
   */
  VariableObject() : ndims(0) {}

  /**
   * @brief Constructor with parameters.
   * @param name The name of the variable.
   * @param kind The kind of variable, which can be deterministic, fixed, random, or derived.`
   * @param dims A vector of integers representing the dimensions.
   * @param dim_names A vector of strings representing the names of the
   * dimensions.
   */
  VariableObject(const std::string &name, const VariableKind &kind, const std::vector<size_t> &dims,
                  const std::vector<std::string> &dim_names)
      : name(name), kind(kind), ndims(dims.size()), dims(dims), dim_names(dim_names) {
        this->nvals = 1;
        for (size_t i = 0; i < dims.size(); i++) {
          this->nvals *= dims[i];
        }
        this->values.resize(this->nvals);
        this->uncertainty.resize(this->nvals);
        this->id = VariableObject<Type>::id_g++;
      }

  /**
   * Copy constructor
   */
  VariableObject(const VariableObject &other)
      : name(other.name),
        kind(other.kind),
        ndims(other.ndims),
        dims(other.dims),
        dim_names(other.dim_names) {}

  /**
   * @brief Assignment operator for VariableObject.
   */
  VariableObject &operator=(const VariableObject &other) {
    if (this != &other) {
      name = other.name;
      kind = other.kind;
      ndims = other.ndims;
      dims = other.dims;
      dim_names = other.dim_names;
      values = other.values;
      uncertainty = other.uncertainty;
      na_value = other.na_value;
    }
    return *this;
  }

  /**
   * Retrieve element from variable vector.
   * @param location scalar location of value to retrieve.
   * @return the value of the variable at location
   */
  inline const Type operator()(uint32_t location) { 
    if(location >= this->values.size()) {
      throw std::invalid_argument("VariableObject error: location index out of bounds");
    }
    return values[location];
  }

  /**
   * Retrieve dimension folded element from variable vector.
   * @param location location index of value to retrieve. Vector of length equal to the number of dimensions, where each element is the index along that dimension.
   * @return the value of the variable at location
   */
  inline const Type operator()(std::vector<size_t> location) {
    if(location.size() != this->ndims) {
      throw std::invalid_argument("VariableObject error: location vector length does not match number of dimensions");
    }
    uint32_t location_index;
    for(int i = 0; i < location.size(); i++) {
      if(location[i] >= this->dims[i]) {
        throw std::invalid_argument("VariableObject error: location index out of bounds for dimension " + std::to_string(i));
      }
      if(i == 0) {
        location_index = location[i];
      } else {
        location_index = location_index * this->dims[i] + location[i];
      }
    }
    return values[location_index]; 
  }

  /**
   * Retrieve element from variable vector.
   * Throws an exception if index is out of bounds.
   * @param location scalar location of value to retrieve.
   * @return the reference to the value of the vector at position location
   */
  inline Type& at(uint32_t location) {
    if (location >= this->values.size()) {
      throw std::overflow_error("VariableObject error: location index out of bounds");
    }
    return values[location];
  }

  /**
   * Retrieve dimension folded element from variable vector.
   * @param location location index of value to retrieve. Vector of length equal to the number of dimensions, where each element is the index along that dimension.
   * @return the value of the variable at location
   */
  inline Type& at(std::vector<size_t> location) {
    if(location.size() != this->ndims) {
      throw std::invalid_argument("VariableObject error: location vector length does not match number of dimensions");
    }
    uint32_t location_index;
    for(int i = 0; i < location.size(); i++) {
      if(location[i] >= this->dims[i]) {
        throw std::invalid_argument("VariableObject error: location index out of bounds for dimension " + std::to_string(i));
      }
      if(i == 0) {
        location_index = location[i];
      } else {
        location_index = location_index * this->dims[i] + location[i];
      }
    }
    return values[location_index]; 
  }

  /**
   * @brief Get the number of dimensions of the variable object.
   *
   * @return size_t
   */
  size_t get_n_dimensions() const { return ndims; }

  /**
   * @brief Get the dimensions of the variable object.
   *
   * @return vector of dimension sizes
   */
  std::vector<size_t> get_dimensions() const { return dims; }
};

template <typename Type>
uint32_t VariableObject<Type>::id_g = 0;

}  // namespace fims_variable_object

#endif
