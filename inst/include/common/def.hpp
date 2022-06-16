/*! \file def.hpp
*/

/*
 * File:   def.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 */
#ifndef DEF_HPP
#define DEF_HPP
#include <vector>

#ifdef TMB_MODEL
// simplify access to singletons
#define TMB_FIMS_REAL_TYPE double
#define TMB_FIMS_FIRST_ORDER AD<TMB_FIMS_REAL_TYPE>
#define TMB_FIMS_SECOND_ORDER AD<TMB_FIMS_FIRST_ORDER>
#define TMB_FIMS_THIRD_ORDER AD<TMB_FIMS_SECOND_ORDER>
#endif

namespace fims {

#ifdef STD_LIB

/**
 * @brief Default trait. These are "T" specific
 * traits that depend on modeling platform.
 */
template <typename T>
struct FIMSTraits {
  typedef double real_t;                            /**< The real type */
  typedef double variable_t;                        /**< The variable type */
  typedef typename std::vector<double> data_vector; /**< The data vector type */
  typedef typename std::vector<double> variable_vector; /**< The variable vector
   type */
  typedef typename std::vector<std::vector<double> > data_matrix;     /**< The
       data matrix type */
  typedef typename std::vector<std::vector<double> > variable_matrix; /**< The
   variable matrix type */
};

#endif
}  // namespace fims

#endif /* TRAITS_HPP */
