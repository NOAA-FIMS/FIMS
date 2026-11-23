/** @file constant.hpp @brief Fixed maturity or spawning-contribution schedule.
 */
#ifndef FIMS_MATURITY_CONSTANT_HPP
#define FIMS_MATURITY_CONSTANT_HPP
#include "maturity_base.hpp"
namespace fims_popdy {
/** @brief An age-independent fixed proportion, including exact zero. */
template <typename Type> struct ConstantMaturity : public MaturityBase<Type> {
  double proportion = 0.0;
  const Type evaluate(const Type &) override { return Type(proportion); }
  const Type evaluate(const Type &, size_t) override {
    return Type(proportion);
  }
};
} // namespace fims_popdy
#endif
