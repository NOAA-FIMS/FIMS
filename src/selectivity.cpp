

#include "../inst/include/population_dynamics/selectivity/selectivity.hpp"

namespace fims_popdy {

template <typename Type>
LogisticSelectivity<Type>::LogisticSelectivity() : SelectivityBase<Type>() {}
template <typename Type> LogisticSelectivity<Type>::~LogisticSelectivity() {}

template <typename Type>
const Type LogisticSelectivity<Type>::evaluate(const Type &x) {
  return fims_math::logistic<Type>(inflection_point[0], slope[0], x);
}

template <typename Type>
const Type LogisticSelectivity<Type>::evaluate(const Type &x, size_t pos) {
  return fims_math::logistic<Type>(inflection_point.get_force_scalar(pos),
                                   slope.get_force_scalar(pos), x);
}

template <typename Type>
void LogisticSelectivity<Type>::create_report_vectors(
    std::map<std::string, fims::Vector<fims::Vector<Type>>> &report_vectors) {
  report_vectors["inflection_point"].emplace_back(inflection_point);
  report_vectors["slope"].emplace_back(slope);
}

template <typename Type>
void LogisticSelectivity<Type>::get_report_vector_count(
    std::map<std::string, size_t> &report_vector_count) {
  report_vector_count["inflection_point"] += 1;
  report_vector_count["slope"] += 1;
}

} // end namespace fims_popdy


