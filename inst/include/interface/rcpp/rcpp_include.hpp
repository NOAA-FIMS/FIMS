/**
 * @file rcpp_include.hpp
 * @brief The single place FIMS includes Rcpp. Include this header instead of
 * <Rcpp.h> so that every translation unit sees the same Rcpp configuration.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_INCLUDE_HPP
#define FIMS_INTERFACE_RCPP_RCPP_INCLUDE_HPP

// Only the first inclusion of Rcpp in a translation unit sets its
// configuration, so Rcpp must not be included before this header.
#ifdef Rcpp_hpp
#error "Include interface/rcpp/rcpp_include.hpp instead of <Rcpp.h>."
#endif

// Rcpp calls abort() on a failed type conversion unless NDEBUG is defined.
// R defines NDEBUG for installed builds; defining it here makes debug builds
// (e.g., devtools::load_all()) raise an R error too. TMB.hpp sets NDEBUG
// itself before Eigen and CppAD, so Eigen bounds checks are unaffected.
#undef NDEBUG
#define NDEBUG 1
#include <Rcpp.h>

#endif  // FIMS_INTERFACE_RCPP_RCPP_INCLUDE_HPP
