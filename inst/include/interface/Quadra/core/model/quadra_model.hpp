#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "model_context.hpp"

namespace quadra {

// Runtime-polymorphic base retained for simple double-only models.
class QuadraModelBase {
public:
  virtual ~QuadraModelBase() = default;

  virtual void initialize(ModelReportContext &ctx) { ctx.clear(); }

  virtual double evaluate(const std::vector<double> &parameters,
                          ModelReportContext &ctx) = 0;

  virtual std::vector<std::string> parameter_names() const = 0;

  virtual std::vector<size_t> random_effect_indices() const { return {}; }

  bool has_random_effects() const { return !random_effect_indices().empty(); }
};

// CRTP model interface for scalar-generic evaluation.
//
// User models should implement:
//
//   std::vector<std::string> parameter_names_impl() const;
//
//   template <typename Type>
//   Type evaluate_impl(
//       const std::vector<Type>& parameters,
//       quadra::ModelReportContext& ctx
//   ) const;
//
// The same evaluate_impl can then run with double, quadra::AD, or future
// scalar types.
template <class Derived> class QuadraModel {
public:
  void initialize(ModelReportContext &ctx) { ctx.clear(); }

  template <typename Type>
  Type evaluate(const std::vector<Type> &parameters, ModelReportContext &ctx) {
    return static_cast<Derived *>(this)->template evaluate_impl<Type>(
        parameters, ctx);
  }

  template <typename Type>
  Type evaluate(const std::vector<Type> &parameters,
                ModelReportContext &ctx) const {
    return static_cast<const Derived *>(this)->template evaluate_impl<Type>(
        parameters, ctx);
  }

  std::vector<std::string> parameter_names() const {
    return static_cast<const Derived *>(this)->parameter_names_impl();
  }

  std::vector<size_t> random_effect_indices() const { return {}; }

  bool has_random_effects() const { return !random_effect_indices().empty(); }
};

} // namespace quadra
