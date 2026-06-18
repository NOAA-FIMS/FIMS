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
#include <set>
#include <string>
#ifdef __GNUC__
  #include <cxxabi.h>
#endif

#include "fims_vector.hpp"

namespace fims_model_object {

/**
 * @brief FIMS struct that tracks object memory for leak detection
 */
struct FIMSMemoryTracker {
  /** @brief Total number of active FIMSObject instances currently in memory. */
  static inline int total_active_objects = 0;
  /**
   * @brief Set of string keys identifying each live FIMSObject instance.
   * @details Each entry has the format "TypeName [id=X]", where TypeName is
   * the demangled class name and X is the unique FIMS object id. Used by
   * clear() to report which objects were not properly destroyed.
   */

  static inline std::multiset<std::string> active_objects;
  
  /**
 * @brief Returns readable object type name for Linux and Mac (Windows is readable by default)
 */
  static std::string demangle(const char* mangled) {
#ifdef __GNUC__
    int status;
    char* buf = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    std::string result = (status == 0 && buf) ? buf : mangled;
    free(buf);
    return result;
#else
    return mangled;  // MSVC names are already readable
#endif
  }


   /**
   * @brief Registers a FIMSObject instance with the memory tracker.
   * @details Should be called from the constructor of each concrete FIMS class
   * after setting its id, so that the type name and id are both available.
   * Inserts a key of the form "TypeName [id=X]" into active_objects and
   * increments total_active_objects.
   * @param type_name The demangled class name, typically obtained via
   * typeid(*this).name() from the base class constructor.
   * @param id The unique FIMS object id assigned by the derived class.
   */

  void register_self(const char* type_name, uint32_t id) {
    std::string key = demangle(type_name) + " [id=" + std::to_string(id) + "]";
    active_objects.insert(key);
    total_active_objects++;
    tracker_key_ = key;  // store for destructor
  }

  virtual ~FIMSMemoryTracker() {
    if (!tracker_key_.empty()) {
      auto it = active_objects.find(tracker_key_);
      if (it != active_objects.end()) active_objects.erase(it);
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
