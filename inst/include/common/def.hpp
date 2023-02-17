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
#include <fstream>
#include <vector>

std::ofstream FIMS_LOG("fims.log"); /**< Log file */

#ifdef TMB_MODEL
// simplify access to singletons
#define TMB_FIMS_REAL_TYPE double
#define TMB_FIMS_FIRST_ORDER AD<TMB_FIMS_REAL_TYPE>
#define TMB_FIMS_SECOND_ORDER AD<TMB_FIMS_FIRST_ORDER>
#define TMB_FIMS_THIRD_ORDER AD<TMB_FIMS_SECOND_ORDER>
#endif

namespace fims {

    //#ifdef STD_LIB

    /**
     * @brief Default trait. These are "T" specific
     * traits that depend on modeling platform.
     */
    template <typename T>
    struct FIMSTraits {
        typedef T real_t; /**< The real type */
        typedef T variable_t; /**< The variable type */
        typedef typename std::vector<T> data_vector; /**< The data vector type */
        typedef typename std::vector<T> variable_vector; /**< The variable vector
   type */
        typedef typename std::vector<std::vector<T> > data_matrix; /**< The
       data matrix type */
        typedef typename std::vector<std::vector<T> > variable_matrix; /**< The
   variable matrix type */
    };


#ifdef TMB_MODEL

    //template <>
    //struct FIMSTraits<TMB_FIMS_REAL_TYPE> {
    //  typedef double real_t;                            /**< The real type */
    //  typedef TMB_FIMS_REAL_TYPE variable_t;                        /**< The variable type */
    //  typedef typename std::vector<double> data_vector; /**< The data vector type */
    //  typedef typename std::vector<double> variable_vector; /**< The variable vector
    //   type */
    //  typedef typename std::vector<std::vector<TMB_FIMS_REAL_TYPE> > data_matrix;     /**< The
    //       data matrix type */
    //  typedef typename std::vector<std::vector<TMB_FIMS_REAL_TYPE> > variable_matrix; /**< The
    //   variable matrix type */
    //};

    template <>
    struct FIMSTraits<TMB_FIMS_FIRST_ORDER> {
        typedef double real_t; /**< The real type */
        typedef TMB_FIMS_FIRST_ORDER variable_t; /**< The variable type */
        typedef typename std::vector<double> data_vector; /**< The data vector type */
        typedef typename std::vector<double> variable_vector; /**< The variable vector
   type */
        typedef typename std::vector<std::vector<TMB_FIMS_FIRST_ORDER> > data_matrix; /**< The
       data matrix type */
        typedef typename std::vector<std::vector<TMB_FIMS_FIRST_ORDER> > variable_matrix; /**< The
   variable matrix type */
    };

    template <>
    struct FIMSTraits<TMB_FIMS_SECOND_ORDER> {
        typedef double real_t; /**< The real type */
        typedef TMB_FIMS_SECOND_ORDER variable_t; /**< The variable type */
        typedef typename std::vector<double> data_vector; /**< The data vector type */
        typedef typename std::vector<double> variable_vector; /**< The variable vector
   type */
        typedef typename std::vector<std::vector<TMB_FIMS_SECOND_ORDER> > data_matrix; /**< The
       data matrix type */
        typedef typename std::vector<std::vector<TMB_FIMS_SECOND_ORDER> > variable_matrix; /**< The
   variable matrix type */
    };

    template <>
    struct FIMSTraits<TMB_FIMS_THIRD_ORDER> {
        typedef double real_t; /**< The real type */
        typedef TMB_FIMS_THIRD_ORDER variable_t; /**< The variable type */
        typedef typename std::vector<double> data_vector; /**< The data vector type */
        typedef typename std::vector<double> variable_vector; /**< The variable vector
   type */
        typedef typename std::vector<std::vector<TMB_FIMS_THIRD_ORDER> > data_matrix; /**< The
       data matrix type */
        typedef typename std::vector<std::vector<TMB_FIMS_THIRD_ORDER> > variable_matrix; /**< The
   variable matrix type */
    };



#endif
} // namespace fims

#endif /* TRAITS_HPP */
