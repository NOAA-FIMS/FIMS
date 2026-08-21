#pragma once

#include "laplace_backend_factory.hpp"
#include "persistent_laplace_cache.hpp"

#include <memory>

namespace quadra {
namespace laplace {

struct RuntimeBackendState {
  bool backend_initialized = false;
  BackendRecommendation recommendation;
  std::unique_ptr<LaplaceBackend> backend;
};

template <class ModelAdapter> class PersistentLaplaceRuntime {
public:
  explicit PersistentLaplaceRuntime(ModelAdapter adapter)
      : cache_(std::move(adapter)) {}

  LaplaceEvaluation evaluate() {
    auto result = cache_.evaluate();

    if (!runtime_.backend_initialized) {
      Eigen::SparseMatrix<double> H = cache_.adapter().prototype_hessian();

      runtime_.backend =
          CreateLaplaceBackendForHessian(H, &runtime_.recommendation);

      runtime_.backend_initialized = true;
    }

    return result;
  }

  const RuntimeBackendState &runtime() const { return runtime_; }

  PersistentLaplaceCache<ModelAdapter> &cache() { return cache_; }

private:
  PersistentLaplaceCache<ModelAdapter> cache_;
  RuntimeBackendState runtime_;
};

} // namespace laplace
} // namespace quadra
