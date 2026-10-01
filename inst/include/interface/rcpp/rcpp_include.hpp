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

// Rcpp is included with NDEBUG defined so a failed type conversion raises an
// R error instead of calling abort(). R defines NDEBUG for installed builds;
// defining it here makes debug builds (e.g., devtools::load_all()) behave the
// same. push_macro and pop_macro restore the build's NDEBUG afterward, and
// <cassert> is included again so assert() in FIMS code follows that setting.
#pragma push_macro("NDEBUG")
#undef NDEBUG
/**
 * @brief Makes Rcpp raise an R error instead of calling abort() on a failed
 * type conversion. Defined only while Rcpp is included.
 */
#define NDEBUG 1
#include <Rcpp.h>
#pragma pop_macro("NDEBUG")
#include <cassert>

#endif  // FIMS_INTERFACE_RCPP_RCPP_INCLUDE_HPP
