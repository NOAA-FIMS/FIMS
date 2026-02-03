/**
 * @file tmbutils_vector_shim.hpp
 * @brief Shim to safely include <tmbutils/vector.hpp> which lacks its own
 * include guard in some TMB versions. Use this header instead of including
 * <tmbutils/vector.hpp> directly across the project.
 */
#ifndef FIMS_TMBUTILS_VECTOR_SHIM_HPP
#define FIMS_TMBUTILS_VECTOR_SHIM_HPP
namespace tmbutils {
// Include the official tmbutils wrapper. Some TMB versions implement the
// vector by doing `namespace tmbutils { #include "vector.hpp" }` and the
// inner file may not have a header guard. Wrapping this include with our own
// guard ensures the header is only processed once per translation unit.
#include <tmbutils/vector.hpp>
}
#endif /* FIMS_TMBUTILS_VECTOR_SHIM_HPP */
