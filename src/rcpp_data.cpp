/**
 * \file rcpp_data.cpp
 * \brief Implementation of Rcpp data interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_data.hpp"
// static id of the DataInterfaceBase object
uint32_t DataInterfaceBase::id_g = 1;
namespace {
// Adds this counter to the list clear() rewinds.
IdCounterRegistration data_id_g_registration(
    &DataInterfaceBase::id_g);
}  // namespace

#include <Rcpp.h>

// SharedData is the pointer type used throughout this file and allows
// functions to work on the correct child class based on the input string.
using SharedData = std::shared_ptr<DataInterfaceBase>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a data module of the requested type.
 *
 * @details Allocates the data interface object to heap memory, hands ownership 
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @param type One of "age_comp", "length_comp", "index", or "catch".
 * @param n_years Number of years of observations.
 * @param n_bins Number of age bins for "age_comp", or length bins for
 *   "length_comp". "index" and "catch" are one-dimensional, so passing a
 *   non-zero value for those is reported as an error rather than ignored.
 *
 * @return A pointer to the new data module.
 */
Rcpp::XPtr<SharedData> create_data_interface_(std::string data_type,
                                              int n_years, int n_bins = 0) {
  DataType data_type_ = DataTypeFromString(data_type);

  if (n_bins != 0 &&
      (data_type_ == DataType::index || data_type_ == DataType::catch_data)) {
    Rcpp::stop("Data type '" + data_type +
               "' is one-dimensional; `n_bins` must be 0, got " +
               std::to_string(n_bins) + ".");
  }

  SharedData data_interface;
  switch (data_type_) {
    case DataType::age_comp:
      data_interface = std::make_shared<AgeCompDataInterface>(n_years, n_bins);
      break;
    case DataType::length_comp:
      data_interface =
          std::make_shared<LengthCompDataInterface>(n_years, n_bins);
      break;
    case DataType::index:
      data_interface = std::make_shared<IndexDataInterface>(n_years);
      break;
    case DataType::catch_data:
      data_interface = std::make_shared<CatchDataInterface>(n_years);
      break;
  }
  return Rcpp::XPtr<SharedData>(new SharedData(data_interface), true);
}

// ── Base-class conversion for CreateTMBModel() ───────────────────────────────
// This does NOT create a new object; it returns a second pointer to the SAME
// object typed as the common interface base class, so CreateTMBModel() can
// call the virtual add_to_fims_tmb() through it.
/**
 * @brief Return a second pointer to the same object, typed to the common
 *   interface base class.
 *
 * @details This does not create a new object. CreateTMBModel() holds a mixed
 *   list of modules and calls add_to_fims_tmb() on each, which it can only do
 *   through a pointer typed to the class they all share.
 *
 * @param xp The data module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> data_to_fims_xptr_(Rcpp::XPtr<SharedData> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a pointer to a data module, releasing this pointer's
 * share of the module.
 *
 * @details Called by the R `clear()` wrapper for every module in the registry.
 * The external pointer is set to NULL in place, so the R variable still holding
 * it reports "external pointer is not valid" on the next use instead of quietly
 * operating on a module that is no longer part of any model. Once both this
 * pointer and the module's base pointer are released, nothing owns the module
 * and its memory is returned immediately rather than at the next garbage
 * collection.
 *
 * Releasing an already-released pointer does nothing, so this is safe to call
 * twice.
 *
 * @param xp The module to invalidate.
 */
void release_data_(Rcpp::XPtr<SharedData> xp) { xp.release(); }

/**
 * Function to register data classes with the Rcpp module system.
 */
/**
 * @brief Register the data functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_data(Rcpp::Module& m) {
  // The trailing underscores prevent collision with the R wrapper functions
  // that own the clean names (Section 4b of the design doc).
  Rcpp::function("create_data_interface_", &create_data_interface_);
  Rcpp::function("data_to_fims_xptr_", &data_to_fims_xptr_);
  Rcpp::function("release_data_", &release_data_);
}
