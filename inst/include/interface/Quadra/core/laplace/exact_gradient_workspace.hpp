#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../had_graph_workspace.hpp"

namespace quadra {
namespace laplace {

struct ExactGradientEvaluation {
  double objective = 0.0;
  Eigen::VectorXd gradient;
  Eigen::VectorXd trace_terms;
};

struct SparseHdotPatternEntry {
  int row = 0;
  int col = 0;

  SparseHdotPatternEntry() = default;
  SparseHdotPatternEntry(int row_, int col_) : row(row_), col(col_) {}
};

// Production-facing scaffold for exact Laplace gradient Hdot reuse.
//
// Responsibilities:
//   - own/reuse a HAD graph via HadGraphWorkspace
//   - seed batched total derivative directions
//   - run batched directional reverse propagation
//   - extract Hdot values over a sparse pattern
//
// Non-responsibilities in v1:
//   - solving uhat
//   - factorization ownership
//   - logdet/objective assembly
//
// Those stay with higher-level Laplace evaluators.
class ExactGradientWorkspace {
public:
  ExactGradientWorkspace() = default;

  ExactGradientWorkspace(const ExactGradientWorkspace &) = delete;
  ExactGradientWorkspace &operator=(const ExactGradientWorkspace &) = delete;

  ExactGradientWorkspace(ExactGradientWorkspace &&) = delete;
  ExactGradientWorkspace &operator=(ExactGradientWorkspace &&) = delete;


  template <class Builder>
  had::AReal Build(Builder &&builder, std::vector<had::AReal> *fixed_effects,
                   std::vector<had::AReal> *random_effects) {
    if (fixed_effects == nullptr || random_effects == nullptr) {
      throw std::invalid_argument(
          "ExactGradientWorkspace::Build requires non-null variable handles.");
    }

    fixed_effects_ = fixed_effects;
    random_effects_ = random_effects;

    output_ = had_workspace_.Build(std::forward<Builder>(builder));
    built_ = true;

    return output_;
  }

  void PropagateBaseAdjoint() {
    RequireBuilt();
    had_workspace_.PropagateAdjoint(output_.varId);
  }

  void ResizeDirectionalBatch(std::size_t n_directions) {
    RequireBuilt();
    n_directions_ = n_directions;
    had_workspace_.ResizeDirectionalBatch(n_directions);
  }

  template <class DirectionProvider>
  void SeedTotalDirections(std::size_t n_directions,
                           DirectionProvider &&direction_provider) {
    RequireBuilt();
    ResizeDirectionalBatch(n_directions);

    const std::size_t n_fixed = fixed_effects_->size();
    const std::size_t n_random = random_effects_->size();

    had_workspace_.Activate();

    for (std::size_t k = 0; k < n_directions; ++k) {
      Eigen::VectorXd theta_direction;
      Eigen::VectorXd random_direction;

      direction_provider(k, theta_direction, random_direction);

      if (theta_direction.size() != static_cast<int>(n_fixed)) {
        throw std::invalid_argument(
            "ExactGradientWorkspace::SeedTotalDirections theta direction size "
            "mismatch.");
      }
      if (random_direction.size() != static_cast<int>(n_random)) {
        throw std::invalid_argument(
            "ExactGradientWorkspace::SeedTotalDirections random direction size "
            "mismatch.");
      }

      for (std::size_t j = 0; j < n_fixed; ++j) {
        had::SetARealDotBatch((*fixed_effects_)[j], static_cast<int>(k),
                              theta_direction[static_cast<int>(j)]);
      }

      for (std::size_t i = 0; i < n_random; ++i) {
        had::SetARealDotBatch((*random_effects_)[i], static_cast<int>(k),
                              random_direction[static_cast<int>(i)]);
      }
    }
  }

  void PropagateDirectionalBatch() {
    RequireBuilt();
    had_workspace_.PropagateAdjointDirectionalBatch();
  }

  Eigen::MatrixXd
  ExtractHdotDense(std::size_t direction_index,
                   const std::vector<SparseHdotPatternEntry> &pattern) {
    RequireBuilt();

    if (direction_index >= n_directions_) {
      throw std::out_of_range("ExactGradientWorkspace::ExtractHdotDense "
                              "direction_index out of range.");
    }

    const int n_random = static_cast<int>(random_effects_->size());
    Eigen::MatrixXd out = Eigen::MatrixXd::Zero(n_random, n_random);

    had_workspace_.Activate();

    for (const auto &entry : pattern) {
      CheckRandomIndex(entry.row);
      CheckRandomIndex(entry.col);

      const double value = had::GetAdjointDotBatch(
          (*random_effects_)[static_cast<std::size_t>(entry.row)],
          (*random_effects_)[static_cast<std::size_t>(entry.col)],
          static_cast<int>(direction_index));

      out(entry.row, entry.col) = value;
      out(entry.col, entry.row) = value;
    }

    return out;
  }

  std::vector<Eigen::Triplet<double>>
  ExtractHdotTriplets(std::size_t direction_index,
                      const std::vector<SparseHdotPatternEntry> &pattern) {
    RequireBuilt();

    if (direction_index >= n_directions_) {
      throw std::out_of_range("ExactGradientWorkspace::ExtractHdotTriplets "
                              "direction_index out of range.");
    }

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(pattern.size() * 2);

    had_workspace_.Activate();

    for (const auto &entry : pattern) {
      CheckRandomIndex(entry.row);
      CheckRandomIndex(entry.col);

      const double value = had::GetAdjointDotBatch(
          (*random_effects_)[static_cast<std::size_t>(entry.row)],
          (*random_effects_)[static_cast<std::size_t>(entry.col)],
          static_cast<int>(direction_index));

      triplets.emplace_back(entry.row, entry.col, value);

      if (entry.row != entry.col) {
        triplets.emplace_back(entry.col, entry.row, value);
      }
    }

    return triplets;
  }

  Eigen::VectorXd
  TraceTerms(const Eigen::MatrixXd &Hinv,
             const std::vector<SparseHdotPatternEntry> &pattern) {
    RequireBuilt();

    const int n_random = static_cast<int>(random_effects_->size());
    if (Hinv.rows() != n_random || Hinv.cols() != n_random) {
      throw std::invalid_argument(
          "ExactGradientWorkspace::TraceTerms Hinv dimension mismatch.");
    }

    Eigen::VectorXd traces =
        Eigen::VectorXd::Zero(static_cast<int>(n_directions_));

    had_workspace_.Activate();

    for (std::size_t k = 0; k < n_directions_; ++k) {
      double trace = 0.0;

      for (const auto &entry : pattern) {
        CheckRandomIndex(entry.row);
        CheckRandomIndex(entry.col);

        const double hdot = had::GetAdjointDotBatch(
            (*random_effects_)[static_cast<std::size_t>(entry.row)],
            (*random_effects_)[static_cast<std::size_t>(entry.col)],
            static_cast<int>(k));

        trace += Hinv(entry.row, entry.col) * hdot;
      }

      traces[static_cast<int>(k)] = trace;
    }

    return traces;
  }

  template <class SelectedInverseAccessor>
  Eigen::VectorXd TraceTermsSelectedInverse(
      SelectedInverseAccessor &&selected_inverse,
      const std::vector<SparseHdotPatternEntry> &pattern) {
    RequireBuilt();

    Eigen::VectorXd traces =
        Eigen::VectorXd::Zero(static_cast<int>(n_directions_));

    had_workspace_.Activate();

    for (std::size_t k = 0; k < n_directions_; ++k) {
      double trace = 0.0;

      for (const auto &entry : pattern) {
        CheckRandomIndex(entry.row);
        CheckRandomIndex(entry.col);

        const double hdot = had::GetAdjointDotBatch(
            (*random_effects_)[static_cast<std::size_t>(entry.row)],
            (*random_effects_)[static_cast<std::size_t>(entry.col)],
            static_cast<int>(k));

        const double hinv = selected_inverse(entry.row, entry.col);
        trace += hinv * hdot;
      }

      traces[static_cast<int>(k)] = trace;
    }

    return traces;
  }

  template <class SelectedInverseAccessor>
  ExactGradientEvaluation
  AssembleExactGradient(double joint_objective, double logdet_huu,
                        const Eigen::VectorXd &joint_envelope_gradient,
                        SelectedInverseAccessor &&selected_inverse,
                        const std::vector<SparseHdotPatternEntry> &pattern) {
    RequireBuilt();

    if (joint_envelope_gradient.size() != static_cast<int>(n_directions_)) {
      throw std::invalid_argument(
          "ExactGradientWorkspace::AssembleExactGradient gradient dimension "
          "mismatch.");
    }

    ExactGradientEvaluation out;
    out.objective = joint_objective + 0.5 * logdet_huu;
    out.trace_terms = TraceTermsSelectedInverse(
        std::forward<SelectedInverseAccessor>(selected_inverse), pattern);
    out.gradient = joint_envelope_gradient + 0.5 * out.trace_terms;

    return out;
  }

  HadGraphWorkspace &HadWorkspace() { return had_workspace_; }
  const HadGraphWorkspace &HadWorkspace() const { return had_workspace_; }

  std::size_t DirectionCount() const { return n_directions_; }

private:
  void RequireBuilt() const {
    if (!built_) {
      throw std::logic_error("ExactGradientWorkspace used before Build.");
    }
  }

  void CheckRandomIndex(int index) const {
    if (index < 0 || index >= static_cast<int>(random_effects_->size())) {
      throw std::out_of_range(
          "ExactGradientWorkspace random-effect index out of range.");
    }
  }

  HadGraphWorkspace had_workspace_;
  had::AReal output_;
  std::vector<had::AReal> *fixed_effects_ = nullptr;
  std::vector<had::AReal> *random_effects_ = nullptr;
  std::size_t n_directions_ = 0;
  bool built_ = false;
};

inline std::vector<SparseHdotPatternEntry> MakeTridiagonalHdotPattern(int n) {
  if (n < 0) {
    throw std::invalid_argument(
        "MakeTridiagonalHdotPattern requires nonnegative n.");
  }

  std::vector<SparseHdotPatternEntry> pattern;
  pattern.reserve(static_cast<std::size_t>(2 * n));

  for (int i = 0; i < n; ++i) {
    pattern.emplace_back(i, i);
    if (i > 0) {
      pattern.emplace_back(i, i - 1);
    }
  }

  return pattern;
}

} // namespace laplace
} // namespace quadra
