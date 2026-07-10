/**
 * @file model_object.hpp
 * @brief Definition of the FIMSObject structure.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_COMMON_MODEL_OBJECT_HPP
#define FIMS_COMMON_MODEL_OBJECT_HPP

#include <stdint.h>
#include <vector>

#include "fims_vector.hpp"

namespace fims_model_object {

/**
 * @brief FIMS struct that tracks object memory for leak detection
 */
struct FIMSMemoryTracker {
  /** @brief Total number of active FIMSObject instances currently in memory. */
  static inline int total_active_objects = 0;

  /**
   * @brief Registers a FIMSObject instance with the memory tracker.
   * @details Should be called from the constructor of each concrete FIMS class
   * after setting its id, so that the type name and id are both available.
   * Inserts a key of the form "TypeName [id=X]" into active_objects and
   * increments total_active_objects.
   * @param id The unique FIMS object id assigned by the derived class.
   */

  void register_self(const uint32_t id) {
    std::string key = " [id=" + std::to_string(id) + "]";
    total_active_objects++;
    tracker_key_ = key;  // store for destructor
  }

  virtual ~FIMSMemoryTracker() {
    if (!tracker_key_.empty()) {
      total_active_objects--;
    }
  }

 private:
  std::string tracker_key_;
};

/**
 * @brief FIMSObject struct that defines member types and returns the unique id
 */
template <typename Type>
struct FIMSObject : public FIMSMemoryTracker {
  uint32_t id; /**< unique identifier assigned for all fims objects */
  
  //TODO: look into if these are needed here, they don't seem to be used
  //currently as everything points to the shared information objects.
  std::vector<Type*> parameters; /**< list of estimable parameters */
  std::vector<Type*>
      random_effects_parameters; /**< list of all random effects parameters */
  std::vector<Type*>
      fixed_effects_parameters; /**< list of fixed effects parameters */

  FIMSObject() {}

  virtual ~FIMSObject() {}

  /**
   * @brief Getter that returns the unique id for parameters in the model
   */
  uint32_t GetId() const { return id; }

  /**
   * @brief Check the dimensions of an object
   *
   * @param actual The actual dimensions.
   * @param expected The expected dimensions.
   * @return true
   * @return false
   */
  inline bool CheckDimensions(size_t actual, size_t expected) {
    if (actual != expected) {
      return false;
    }

    return true;
  }
};

}  // namespace fims_model_object

#endif /* FIMS_COMMON_MODEL_OBJECT_HPP */
