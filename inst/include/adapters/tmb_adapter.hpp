#ifndef FIMS_ADAPTERS_TMB_ADAPTER_HPP
#define FIMS_ADAPTERS_TMB_ADAPTER_HPP

#pragma once

#include <TMB.hpp>
#include "F_common_fims_vector.h"

namespace fims {
namespace adapter {

// Convert TMB vector -> FIMS vector
template<typename Type>
Vector<Type> from_tmb(const tmbutils::vector<Type>& x) {
    Vector<Type> out(x.size());
    for (int i = 0; i < x.size(); ++i) {
        out[i] = x[i];
    }
    return out;
}

// Convert FIMS vector -> TMB vector
template<typename Type>
tmbutils::vector<Type> to_tmb(const Vector<Type>& x) {
    tmbutils::vector<Type> out(x.size());
    for (int i = 0; i < x.size(); ++i) {
        out[i] = x[i];
    }
    return out;
}

} // namespace adapter
} // namespace fims

#endif  // FIMS_ADAPTERS_TMB_ADAPTER_HPP