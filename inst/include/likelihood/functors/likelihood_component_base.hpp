/**
 * @file likelihood_component_base.hpp
 * @brief Base class for experimental likelihood components.
 */
#ifndef FIMS_LIKELIHOOD_COMPONENT_BASE_HPP
#define FIMS_LIKELIHOOD_COMPONENT_BASE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>

#include "../../common/def.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"

namespace fims_likelihood {

enum class LikelihoodRole {
  Data,
  Prior,
  RandomEffect,
  Penalty
};

template <typename Type>
struct LikelihoodComponentBase {
  fims::Vector<Type> *input_values = nullptr;
  fims::Vector<Type> *expected_values_input = nullptr;
  fims::Vector<Type> input_storage;
  fims::Vector<Type> expected_storage;
  fims::Vector<Type> observed_values;
  fims::Vector<Type> expected_values;
  fims::Vector<Type> nll_components;
  LikelihoodRole role = LikelihoodRole::Data;
  Type nll = static_cast<Type>(0.0);
  Type na_value = static_cast<Type>(-999.0);
  bool simulate_flag = false;

#ifdef TMB_MODEL
  ::objective_function<Type> *of;
#endif

  virtual ~LikelihoodComponentBase() {}

  virtual Type Evaluate() = 0;

  virtual std::string Name() const = 0;

  void SetInput(fims::Vector<Type> *input, LikelihoodRole input_role) {
    input_values = input;
    role = input_role;
  }

  void SetExpected(fims::Vector<Type> *expected) {
    expected_values_input = expected;
  }

  size_t GetInputSize() const {
    if (input_values != nullptr) {
      return input_values->size();
    }
    return observed_values.size();
  }

  Type &GetInput(size_t i) {
    if (input_values != nullptr) {
      return (*input_values)[i];
    }
    return observed_values[i];
  }

  const Type &GetInput(size_t i) const {
    if (input_values != nullptr) {
      return (*input_values)[i];
    }
    return observed_values[i];
  }

  Type GetExpected(size_t i) {
    if (expected_values_input != nullptr) {
      return expected_values_input->get_force_scalar(i);
    }
    return expected_values.get_force_scalar(i);
  }

  void CheckExpectedSize(size_t n_input, const std::string &name) const {
    size_t expected_size = expected_values.size();
    if (expected_values_input != nullptr) {
      expected_size = expected_values_input->size();
    }
    if (expected_size > 1 && expected_size != n_input) {
      throw std::invalid_argument(
          name +
          "::Evaluate: input and expected_values must have the same size "
          "unless expected_values is scalar. input size is " +
          fims::to_string(n_input) + " and expected_values size is " +
          fims::to_string(expected_size));
    }
  }

  void PrepareComponents(size_t n) {
    nll = static_cast<Type>(0.0);
    nll_components.resize(n);
    for (size_t i = 0; i < n; i++) {
      nll_components[i] = static_cast<Type>(0.0);
    }
  }
};

}  // namespace fims_likelihood

#endif /* FIMS_LIKELIHOOD_COMPONENT_BASE_HPP */
