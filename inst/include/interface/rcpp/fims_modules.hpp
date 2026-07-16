#ifndef FIMS_MODULES_HPP
#define FIMS_MODULES_HPP

#include <Rcpp.h>

/**
 * \brief Register the population module.
 * \param m The Rcpp module to register functions in.
 */
void register_population(Rcpp::Module &m);
/**
 * \brief Register the selectivity module.
 * \param m The Rcpp module to register functions in.
 */
void register_selectivity(Rcpp::Module &m);
/**
 * \brief Register the growth module.
 * \param m The Rcpp module to register functions in.
 */
void register_growth(Rcpp::Module &m);
/**
 * \brief Register the distributions module.
 * \param m The Rcpp module to register functions in.
 */
void register_distributions(Rcpp::Module &m);
/**
 * \brief Register the data module.
 * \param m The Rcpp module to register functions in.
 */
void register_data(Rcpp::Module &m);
/**
 * \brief Register the recruitment module.
 * \param m The Rcpp module to register functions in.
 */
void register_recruitment(Rcpp::Module &m);
/**
 * \brief Register the fleet module.
 * \param m The Rcpp module to register functions in.
 */
void register_fleet(Rcpp::Module &m);
/**
 * \brief Register the variable module.
 * \param m The Rcpp module to register functions in.
 */
void register_variable(Rcpp::Module &m);
/**
 * \brief Register the vectors module.
 * \param m The Rcpp module to register functions in.
 */
void register_vectors(Rcpp::Module &m);
/**
 * \brief Register the shared module.
 * \param m The Rcpp module to register functions in.
 */
void register_shared(Rcpp::Module &m);
/**
 * \brief Register the functions module.
 * \param m The Rcpp module to register functions in.
 */
void register_functions(Rcpp::Module &m);

#ifdef QUADRA_MODEL
/**
 * \brief Register the Quadra inference backend functions.
 * \param m The Rcpp module to register functions in.
 */
void register_quadra(Rcpp::Module &m);
#endif
/**
 * \brief Register the fishery models module.
 * \param m The Rcpp module to register functions in.
 */
void register_fishery_models(Rcpp::Module &m);
/**
 * \brief Register the maturity module.
 * \param m The Rcpp module to register functions in.
 */
void register_maturity(Rcpp::Module &m);

/**
 * Rcpp module definition for the FIMS framework. This module registers all the
 * classes and functions that are exposed to R through Rcpp.
 */
// RCPP_MODULE expands to multiple declarations. Export its generated boot
// function explicitly so hidden visibility and LTO cannot discard it. R's
// attribute_visible macro can be empty on macOS, so spell out the compiler
// attribute when it is supported.
#if defined(__GNUC__) || defined(__clang__)
extern "C" __attribute__((visibility("default"))) SEXP
RCPP_MODULE_BOOT(fims)();
#else
RcppExport SEXP RCPP_MODULE_BOOT(fims)();
#endif
RCPP_MODULE(fims) {
  Rcpp::Module m("fims");
  register_functions(m);
#ifdef QUADRA_MODEL
  register_quadra(m);
#endif
  register_variable(m);
  register_vectors(m);
  register_shared(m);
  register_recruitment(m);
  register_maturity(m);
  register_fleet(m);
  register_population(m);
  register_selectivity(m);
  register_growth(m);
  register_distributions(m);
  register_data(m);
  register_fishery_models(m);
}

#endif  // FIMS_MODULES_HPP
