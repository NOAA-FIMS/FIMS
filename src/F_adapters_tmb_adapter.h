#ifndef FIMS_ADAPTERS_TMB_ADAPTER_H
#define FIMS_ADAPTERS_TMB_ADAPTER_H

#ifndef FIMS_ADAPTERS_TMB_ADAPTER_HPP
#define FIMS_ADAPTERS_TMB_ADAPTER_HPP

namespace tmbutils {
template<typename Type>
struct vector;
}
#pragma once
// #include <TMB.hpp>

#include "F_common_fims_vector.h"

namespace fims {
namespace adapter {

// Convert TMB vector -> FIMS vector
template<typename Type>
Vector<Type> from_tmb(const tmbutils::vector<Type>& x);

// Convert FIMS vector -> TMB vector
template<typename Type>
tmbutils::vector<Type> to_tmb(const Vector<Type>& x);
} // namespace adapter
} // namespace fims

#endif  // FIMS_ADAPTERS_TMB_ADAPTER_HPP

#endif
