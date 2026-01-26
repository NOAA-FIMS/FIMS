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
#include <map>
#include <vector>

// #include "def.hpp"
// #include "fims_vector.hpp"

namespace fims {
template <typename Type> class Vector;
}

namespace fims_model_object {

/**
 * @brief FIMSObject struct that defines member types and returns the unique id
 */
template <typename Type> struct FIMSObject {
  uint32_t id; /**< unique identifier assigned for all fims objects */
  std::vector<Type *> parameters; /**< list of estimable parameters */
  std::vector<Type *>
      random_effects_parameters; /**< list of all random effects parameters */
  std::vector<Type *>
      fixed_effects_parameters; /**< list of fixed effects parameters */

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

  /**
   * @brief Create a map of report vectors for the object.
   * used to populate the report_vectors map in FisheryModelBase.
   */
  virtual void create_report_vectors(
      std::map<std::string, fims::Vector<fims::Vector<Type>>> &report_vectors) {
  }
  /**
   * @brief Get the report vector count object.
   * used to get the length of each report vector for populating the
   * UncertaintyReportInfo struct in FisheryModelBase.
   */
  virtual void
  get_report_vector_count(std::map<std::string, size_t> &report_vector_count) {}
};

} // namespace fims_model_object

#endif /* FIMS_COMMON_MODEL_OBJECT_HPP */
