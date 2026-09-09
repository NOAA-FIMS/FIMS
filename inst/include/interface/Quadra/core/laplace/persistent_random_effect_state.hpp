#pragma once

#include <Eigen/Dense>

#include <stdexcept>

namespace quadra {
namespace laplace {

struct NewtonSolveStatus {
  int iterations = 0;
  double objective = 0.0;
  double grad_norm = 0.0;
  bool converged = false;
};

class PersistentRandomEffectState {
public:
  PersistentRandomEffectState() = default;

  explicit PersistentRandomEffectState(const int size) { resize(size); }

  void resize(const int size) {
    if (size < 0) {
      throw std::invalid_argument(
          "PersistentRandomEffectState size must be non-negative");
    }

    xhat_ = Eigen::VectorXd::Zero(size);
    initialized_ = false;
    status_ = NewtonSolveStatus();
  }

  void initialize(const Eigen::VectorXd &x) {
    xhat_ = x;
    initialized_ = true;
    status_ = NewtonSolveStatus();
  }

  void update_xhat(const Eigen::VectorXd &x, const NewtonSolveStatus &status) {
    if (initialized_ && xhat_.size() != x.size()) {
      throw std::invalid_argument(
          "PersistentRandomEffectState xhat size changed unexpectedly");
    }

    xhat_ = x;
    status_ = status;
    initialized_ = true;
  }

  void clear() {
    xhat_.resize(0);
    initialized_ = false;
    status_ = NewtonSolveStatus();
  }

  bool initialized() const { return initialized_; }

  int size() const { return static_cast<int>(xhat_.size()); }

  const Eigen::VectorXd &xhat() const {
    if (!initialized_) {
      throw std::runtime_error(
          "PersistentRandomEffectState xhat requested before initialization");
    }

    return xhat_;
  }

  Eigen::VectorXd &mutable_xhat() {
    if (!initialized_) {
      throw std::runtime_error("PersistentRandomEffectState mutable xhat "
                               "requested before initialization");
    }

    return xhat_;
  }

  const NewtonSolveStatus &status() const { return status_; }

private:
  Eigen::VectorXd xhat_;
  bool initialized_ = false;
  NewtonSolveStatus status_;
};

} // namespace laplace
} // namespace quadra
