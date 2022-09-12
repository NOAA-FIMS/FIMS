/*
 * File:   model_object.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */

#ifndef FIMS_COMMON_MODEL_OBJECT_HPP
#define FIMS_COMMON_MODEL_OBJECT_HPP

#include <stdint.h>

#include <vector>

#include "def.hpp"

namespace fims {

/**
 * @brief FIMSObject struct that defines member types and returns the unique id
 */
template <typename T>
struct FIMSObject {
  uint32_t id; /*!< unique identifier assigned for all fims objects */
  std::vector<T*> parameters; /*!< list of estimable parameters */
  std::vector<T*>
      random_effects_parameters; /*!< list of all random effects parameters */
  std::vector<T*>
      fixed_effects_parameters; /*!< list of fixed effects parameters */

    virtual ~FIMSObject(){
        
    }
  /**
   * @brief Getter that returns the unique id for parameters in the model
   */
  uint32_t GetId() const { return id; }
};

}  // namespace fims

#endif /* FIMS_COMMON_MODEL_OBJECT_HPP */
