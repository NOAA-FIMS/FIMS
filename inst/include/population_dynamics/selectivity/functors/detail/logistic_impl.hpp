#ifndef FIMS_POPDY_SELECTIVITY_LOGISTIC_IMPL_HPP
#define FIMS_POPDY_SELECTIVITY_LOGISTIC_IMPL_HPP

#pragma once

#include "../logistic.hpp"
// #include "../../../../common/fims_math.hpp"
// #include "../../../../common/fims_vector.hpp"

namespace fims_popdy {
template<typename Type>
LogisticSelectivity<Type>::LogisticSelectivity() {}

template<typename Type>
LogisticSelectivity<Type>::~LogisticSelectivity() = default;

template<typename Type>
const Type LogisticSelectivity<Type>::evaluate(const Type &x) {
    return Type(1.0) /
           (Type(1.0) + exp(-this->slope[0] * (x - this->inflection_point[0])));
}

template<typename Type>
const Type LogisticSelectivity<Type>::evaluate(const Type &x, size_t pos) {
    return Type(1.0) /
           (Type(1.0) + exp(-this->slope[pos] * (x - this->inflection_point[pos])));
}

template<typename Type>
void LogisticSelectivity<Type>::create_report_vectors(
        std::map<std::string, fims::Vector<fims::Vector<Type>>> &report_vectors) {
    report_vectors["inflection_point"].emplace_back(inflection_point.to_tmb());
    report_vectors["slope"].emplace_back(slope.to_tmb());
}

template<typename Type>
void LogisticSelectivity<Type>::get_report_vector_count(
        std::map<std::string, size_t> &report_vector_count) {
    report_vector_count["inflection_point"] += 1;
    report_vector_count["slope"] += 1;
}

}  // namespace fims_popdy


#endif  // FIMS_POPDY_SELECTIVITY_LOGISTIC_IMPL_HPP