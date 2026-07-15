#pragma once

#include "laplace_structure_report.hpp"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numeric>
#include <ostream>
#include <queue>
#include <string>
#include <vector>

namespace quadra {

struct FunctionalOptimizationSummary {
  double objective_value = std::numeric_limits<double>::quiet_NaN();
  double gradient_norm = std::numeric_limits<double>::quiet_NaN();
  std::string max_gradient_parameter;
  double max_gradient_value = std::numeric_limits<double>::quiet_NaN();
  double max_abs_gradient = std::numeric_limits<double>::quiet_NaN();
  int iterations = 0;
  bool converged = false;
  std::string message;
};

struct FunctionalUncertaintySummary {
  std::size_t covariance_rows = 0;
  std::size_t covariance_cols = 0;
  bool covariance_available = false;
  bool correlation_available = false;

  double min_variance = std::numeric_limits<double>::quiet_NaN();
  double max_variance = std::numeric_limits<double>::quiet_NaN();
  std::size_t min_variance_index = 0;
  std::size_t max_variance_index = 0;

  double max_abs_correlation = std::numeric_limits<double>::quiet_NaN();
  std::size_t max_abs_correlation_i = 0;
  std::size_t max_abs_correlation_j = 0;

  std::size_t corr_abs_gt_0_5 = 0;
  std::size_t corr_abs_gt_0_8 = 0;
  std::size_t corr_abs_gt_0_9 = 0;
};

struct FunctionalLatentStateSummary {
  std::size_t count = 0;
  double mean = std::numeric_limits<double>::quiet_NaN();
  double sd = std::numeric_limits<double>::quiet_NaN();
  double min_value = std::numeric_limits<double>::quiet_NaN();
  double max_value = std::numeric_limits<double>::quiet_NaN();
  std::size_t min_index = 0;
  std::size_t max_index = 0;
  double l2_norm = std::numeric_limits<double>::quiet_NaN();
};

struct FunctionalParameterInfluenceRow {
  std::size_t index = 0;
  std::string name;
  double variance = std::numeric_limits<double>::quiet_NaN();
  double sd = std::numeric_limits<double>::quiet_NaN();
  double variance_share = std::numeric_limits<double>::quiet_NaN();

  // Sum of absolute correlations to all other parameters. This is a simple
  // uncertainty-network centrality score.
  double correlation_centrality = std::numeric_limits<double>::quiet_NaN();
  double correlation_centrality_share =
      std::numeric_limits<double>::quiet_NaN();

  // Precision-side local curvature from the supplied Hessian, if available.
  double curvature_diagonal = std::numeric_limits<double>::quiet_NaN();
  double curvature_column_norm = std::numeric_limits<double>::quiet_NaN();

  // Composite scale-sensitive importance score:
  //
  //   sd * curvature_column_norm * (1 + correlation_centrality)
  //
  // If curvature is unavailable, falls back to:
  //
  //   sd * (1 + correlation_centrality)
  //
  // This is intended for ranking, not as a formal statistical estimator.
  double importance_score = std::numeric_limits<double>::quiet_NaN();
  double importance_share = std::numeric_limits<double>::quiet_NaN();
};

struct FunctionalCorrelationInfluenceRow {
  std::size_t i = 0;
  std::size_t j = 0;
  std::string name_i;
  std::string name_j;
  double correlation = std::numeric_limits<double>::quiet_NaN();
  double abs_correlation = std::numeric_limits<double>::quiet_NaN();
};

struct FunctionalParameterInfluenceSummary {
  bool available = false;
  std::vector<FunctionalParameterInfluenceRow> variance_rows;
  std::vector<FunctionalCorrelationInfluenceRow> top_correlation_rows;
};

struct FunctionalCorrelationGraphSummary {
  bool available = false;
  double abs_correlation_threshold = 0.5;
  std::size_t node_count = 0;
  std::size_t edge_count = 0;
  double average_degree = std::numeric_limits<double>::quiet_NaN();
  std::size_t maximum_degree = 0;
  std::size_t maximum_degree_index = 0;
  std::string maximum_degree_name;
  std::size_t connected_components = 0;
  std::size_t largest_component_size = 0;
  int graph_diameter = -1;
};

struct FunctionalGradientVolatilitySummary {
  bool available = false;
  double perturbation_scale = 0.0;
  std::size_t samples = 0;
  double baseline_gradient_norm = std::numeric_limits<double>::quiet_NaN();
  double mean_gradient_norm = std::numeric_limits<double>::quiet_NaN();
  double sd_gradient_norm = std::numeric_limits<double>::quiet_NaN();
  double max_gradient_norm = std::numeric_limits<double>::quiet_NaN();
  double gradient_norm_cv = std::numeric_limits<double>::quiet_NaN();
  std::string most_volatile_parameter;
  std::size_t most_volatile_parameter_index = 0;
  double most_volatile_parameter_sd = std::numeric_limits<double>::quiet_NaN();
  std::string most_sign_flips_parameter;
  std::size_t most_sign_flips_parameter_index = 0;
  std::size_t most_sign_flips = 0;
};

struct FunctionalParameterGeometryRow {
  std::size_t index = 0;
  std::string name;
  double gradient = std::numeric_limits<double>::quiet_NaN();
  double abs_gradient = std::numeric_limits<double>::quiet_NaN();
  double curvature_column_norm = std::numeric_limits<double>::quiet_NaN();
  double curvature_diagonal = std::numeric_limits<double>::quiet_NaN();
  double curvature_share = std::numeric_limits<double>::quiet_NaN();
};

struct FunctionalParameterGeometrySummary {
  bool available = false;
  std::vector<FunctionalParameterGeometryRow> rows;
  std::string dominant_parameter;
  std::size_t dominant_parameter_index = 0;
  double dominant_curvature_column_norm =
      std::numeric_limits<double>::quiet_NaN();
};

struct FunctionalSpectralStructureSummary {
  bool available = false;
  std::size_t eigen_count = 0;
  double eigen_sum = std::numeric_limits<double>::quiet_NaN();
  double largest_eigen_share = std::numeric_limits<double>::quiet_NaN();
  double effective_rank_entropy = std::numeric_limits<double>::quiet_NaN();

  std::size_t eigen_count_for_50 = 0;
  std::size_t eigen_count_for_90 = 0;
  std::size_t eigen_count_for_95 = 0;
  std::size_t eigen_count_for_99 = 0;

  std::vector<double> eigenvalues_desc;
  std::vector<double> cumulative_share;
};

struct FunctionalAnalysisReport {
  FunctionalOptimizationSummary optimization;
  LaplaceStructureReport laplace_structure;
  FunctionalUncertaintySummary uncertainty;
  FunctionalLatentStateSummary latent_states;
  FunctionalParameterInfluenceSummary parameter_influence;
  FunctionalCorrelationGraphSummary correlation_graph;
  FunctionalGradientVolatilitySummary gradient_volatility;
  FunctionalParameterGeometrySummary parameter_geometry;
  FunctionalSpectralStructureSummary spectral_structure;
};

inline FunctionalLatentStateSummary
summarize_latent_states(const std::vector<double> &u) {
  FunctionalLatentStateSummary out;
  out.count = u.size();

  if (u.empty()) {
    return out;
  }

  double sum = 0.0;
  double sumsq = 0.0;
  out.min_value = u[0];
  out.max_value = u[0];
  out.min_index = 0;
  out.max_index = 0;

  for (std::size_t i = 0; i < u.size(); ++i) {
    const double x = u[i];
    sum += x;
    sumsq += x * x;

    if (x < out.min_value) {
      out.min_value = x;
      out.min_index = i;
    }
    if (x > out.max_value) {
      out.max_value = x;
      out.max_index = i;
    }
  }

  out.mean = sum / static_cast<double>(u.size());
  const double var =
      sumsq / static_cast<double>(u.size()) - out.mean * out.mean;
  out.sd = std::sqrt(std::max(0.0, var));
  out.l2_norm = std::sqrt(sumsq);
  return out;
}

inline FunctionalUncertaintySummary
summarize_covariance_correlation(const Eigen::MatrixXd &cov) {
  FunctionalUncertaintySummary out;
  out.covariance_rows = static_cast<std::size_t>(cov.rows());
  out.covariance_cols = static_cast<std::size_t>(cov.cols());

  if (cov.rows() == 0 || cov.cols() == 0 || cov.rows() != cov.cols()) {
    return out;
  }

  out.covariance_available = true;

  out.min_variance = cov(0, 0);
  out.max_variance = cov(0, 0);
  out.min_variance_index = 0;
  out.max_variance_index = 0;

  for (Eigen::Index i = 0; i < cov.rows(); ++i) {
    const double v = cov(i, i);
    if (v < out.min_variance) {
      out.min_variance = v;
      out.min_variance_index = static_cast<std::size_t>(i);
    }
    if (v > out.max_variance) {
      out.max_variance = v;
      out.max_variance_index = static_cast<std::size_t>(i);
    }
  }

  out.correlation_available = true;
  out.max_abs_correlation = 0.0;

  for (Eigen::Index i = 0; i < cov.rows(); ++i) {
    for (Eigen::Index j = i + 1; j < cov.cols(); ++j) {
      const double denom = std::sqrt(std::abs(cov(i, i) * cov(j, j)));
      if (denom <= 0.0 || !std::isfinite(denom)) {
        out.correlation_available = false;
        continue;
      }

      const double corr = cov(i, j) / denom;
      const double ac = std::abs(corr);

      if (ac > out.max_abs_correlation) {
        out.max_abs_correlation = ac;
        out.max_abs_correlation_i = static_cast<std::size_t>(i);
        out.max_abs_correlation_j = static_cast<std::size_t>(j);
      }

      if (ac > 0.5)
        ++out.corr_abs_gt_0_5;
      if (ac > 0.8)
        ++out.corr_abs_gt_0_8;
      if (ac > 0.9)
        ++out.corr_abs_gt_0_9;
    }
  }

  return out;
}

inline FunctionalParameterInfluenceSummary summarize_parameter_influence(
    const Eigen::MatrixXd &cov, const std::vector<std::string> &names = {},
    std::size_t top_correlation_count = 10,
    const Eigen::MatrixXd *precision_hessian = nullptr) {
  FunctionalParameterInfluenceSummary out;

  if (cov.rows() == 0 || cov.cols() == 0 || cov.rows() != cov.cols()) {
    return out;
  }

  out.available = true;

  const std::size_t n = static_cast<std::size_t>(cov.rows());

  double variance_sum = 0.0;
  for (Eigen::Index i = 0; i < cov.rows(); ++i) {
    variance_sum += std::max(0.0, cov(i, i));
  }

  std::vector<double> centrality(n, 0.0);
  std::vector<FunctionalCorrelationInfluenceRow> corr_rows;

  for (Eigen::Index i = 0; i < cov.rows(); ++i) {
    for (Eigen::Index j = i + 1; j < cov.cols(); ++j) {
      const double denom = std::sqrt(std::abs(cov(i, i) * cov(j, j)));
      if (denom <= 0.0 || !std::isfinite(denom)) {
        continue;
      }

      FunctionalCorrelationInfluenceRow row;
      row.i = static_cast<std::size_t>(i);
      row.j = static_cast<std::size_t>(j);
      row.name_i = row.i < names.size() ? names[row.i]
                                        : ("param_" + std::to_string(row.i));
      row.name_j = row.j < names.size() ? names[row.j]
                                        : ("param_" + std::to_string(row.j));
      row.correlation = cov(i, j) / denom;
      row.abs_correlation = std::abs(row.correlation);
      corr_rows.push_back(row);

      centrality[row.i] += row.abs_correlation;
      centrality[row.j] += row.abs_correlation;
    }
  }

  const double centrality_sum =
      std::accumulate(centrality.begin(), centrality.end(), 0.0);

  std::vector<double> raw_importance(n, 0.0);
  double importance_sum = 0.0;

  out.variance_rows.reserve(n);
  for (Eigen::Index i = 0; i < cov.rows(); ++i) {
    const std::size_t ii = static_cast<std::size_t>(i);

    FunctionalParameterInfluenceRow row;
    row.index = ii;
    row.name = ii < names.size() ? names[ii] : ("param_" + std::to_string(ii));
    row.variance = cov(i, i);
    row.sd = row.variance >= 0.0 ? std::sqrt(row.variance)
                                 : std::numeric_limits<double>::quiet_NaN();
    row.variance_share =
        variance_sum > 0.0 ? std::max(0.0, row.variance) / variance_sum : 0.0;

    row.correlation_centrality = centrality[ii];
    row.correlation_centrality_share =
        centrality_sum > 0.0 ? centrality[ii] / centrality_sum : 0.0;

    if (precision_hessian != nullptr &&
        precision_hessian->rows() == cov.rows() &&
        precision_hessian->cols() == cov.cols()) {
      row.curvature_diagonal = (*precision_hessian)(i, i);
      row.curvature_column_norm = precision_hessian->col(i).norm();
    }

    const double curvature_factor = std::isfinite(row.curvature_column_norm)
                                        ? row.curvature_column_norm
                                        : 1.0;
    const double sd_factor = std::isfinite(row.sd) ? row.sd : 0.0;
    const double centrality_factor = std::isfinite(row.correlation_centrality)
                                         ? (1.0 + row.correlation_centrality)
                                         : 1.0;

    row.importance_score = sd_factor * curvature_factor * centrality_factor;
    raw_importance[ii] = row.importance_score;
    importance_sum += row.importance_score;

    out.variance_rows.push_back(row);
  }

  for (auto &row : out.variance_rows) {
    row.importance_share =
        importance_sum > 0.0 ? row.importance_score / importance_sum : 0.0;
  }

  std::sort(out.variance_rows.begin(), out.variance_rows.end(),
            [](const FunctionalParameterInfluenceRow &a,
               const FunctionalParameterInfluenceRow &b) {
              return a.importance_score > b.importance_score;
            });

  std::sort(corr_rows.begin(), corr_rows.end(),
            [](const FunctionalCorrelationInfluenceRow &a,
               const FunctionalCorrelationInfluenceRow &b) {
              return a.abs_correlation > b.abs_correlation;
            });

  if (corr_rows.size() > top_correlation_count) {
    corr_rows.resize(top_correlation_count);
  }

  out.top_correlation_rows = std::move(corr_rows);
  return out;
}

inline FunctionalCorrelationGraphSummary
summarize_correlation_graph(const Eigen::MatrixXd &cov,
                            const std::vector<std::string> &names = {},
                            double abs_corr_threshold = 0.5) {
  FunctionalCorrelationGraphSummary out;
  out.abs_correlation_threshold = abs_corr_threshold;
  out.node_count = static_cast<std::size_t>(cov.rows());

  if (cov.rows() == 0 || cov.cols() == 0 || cov.rows() != cov.cols()) {
    return out;
  }

  out.available = true;
  std::vector<std::vector<std::size_t>> adj(out.node_count);

  for (Eigen::Index i = 0; i < cov.rows(); ++i) {
    for (Eigen::Index j = i + 1; j < cov.cols(); ++j) {
      const double denom = std::sqrt(std::abs(cov(i, i) * cov(j, j)));
      if (denom <= 0.0 || !std::isfinite(denom)) {
        continue;
      }

      const double ac = std::abs(cov(i, j) / denom);
      if (ac >= abs_corr_threshold) {
        const auto ii = static_cast<std::size_t>(i);
        const auto jj = static_cast<std::size_t>(j);
        adj[ii].push_back(jj);
        adj[jj].push_back(ii);
        ++out.edge_count;
      }
    }
  }

  out.average_degree = out.node_count > 0
                           ? (2.0 * static_cast<double>(out.edge_count)) /
                                 static_cast<double>(out.node_count)
                           : 0.0;

  for (std::size_t i = 0; i < adj.size(); ++i) {
    if (adj[i].size() > out.maximum_degree) {
      out.maximum_degree = adj[i].size();
      out.maximum_degree_index = i;
    }
  }
  out.maximum_degree_name =
      out.maximum_degree_index < names.size()
          ? names[out.maximum_degree_index]
          : ("param_" + std::to_string(out.maximum_degree_index));

  std::vector<int> component(out.node_count, -1);
  std::size_t component_id = 0;

  for (std::size_t start = 0; start < out.node_count; ++start) {
    if (component[start] != -1) {
      continue;
    }

    std::queue<std::size_t> q;
    q.push(start);
    component[start] = static_cast<int>(component_id);
    std::size_t component_size = 0;

    while (!q.empty()) {
      const std::size_t v = q.front();
      q.pop();
      ++component_size;

      for (const std::size_t nb : adj[v]) {
        if (component[nb] == -1) {
          component[nb] = static_cast<int>(component_id);
          q.push(nb);
        }
      }
    }

    out.largest_component_size =
        std::max(out.largest_component_size, component_size);
    ++component_id;
  }

  out.connected_components = component_id;

  auto bfs_max_distance = [&](std::size_t source) -> int {
    std::vector<int> dist(out.node_count, -1);
    std::queue<std::size_t> q;
    dist[source] = 0;
    q.push(source);
    int max_dist = 0;

    while (!q.empty()) {
      const std::size_t v = q.front();
      q.pop();
      max_dist = std::max(max_dist, dist[v]);

      for (const std::size_t nb : adj[v]) {
        if (dist[nb] == -1) {
          dist[nb] = dist[v] + 1;
          q.push(nb);
        }
      }
    }

    return max_dist;
  };

  out.graph_diameter = 0;
  for (std::size_t i = 0; i < out.node_count; ++i) {
    if (!adj[i].empty()) {
      out.graph_diameter = std::max(out.graph_diameter, bfs_max_distance(i));
    }
  }

  return out;
}

inline FunctionalParameterGeometrySummary
summarize_parameter_geometry(const Eigen::MatrixXd &H,
                             const std::vector<double> &gradient = {},
                             const std::vector<std::string> &names = {}) {
  FunctionalParameterGeometrySummary out;

  if (H.rows() == 0 || H.cols() == 0 || H.rows() != H.cols()) {
    return out;
  }

  out.available = true;

  std::vector<double> column_norms(static_cast<std::size_t>(H.cols()), 0.0);
  double total_column_norm = 0.0;

  for (Eigen::Index j = 0; j < H.cols(); ++j) {
    const double norm = H.col(j).norm();
    column_norms[static_cast<std::size_t>(j)] = norm;
    total_column_norm += norm;
  }

  out.rows.reserve(static_cast<std::size_t>(H.cols()));
  for (Eigen::Index j = 0; j < H.cols(); ++j) {
    const std::size_t jj = static_cast<std::size_t>(j);

    FunctionalParameterGeometryRow row;
    row.index = jj;
    row.name = jj < names.size() ? names[jj] : ("param_" + std::to_string(jj));
    if (jj < gradient.size()) {
      row.gradient = gradient[jj];
      row.abs_gradient = std::abs(gradient[jj]);
    }
    row.curvature_column_norm = column_norms[jj];
    row.curvature_diagonal = H(j, j);
    row.curvature_share = total_column_norm > 0.0
                              ? row.curvature_column_norm / total_column_norm
                              : 0.0;
    out.rows.push_back(row);
  }

  std::sort(out.rows.begin(), out.rows.end(),
            [](const FunctionalParameterGeometryRow &a,
               const FunctionalParameterGeometryRow &b) {
              return a.curvature_column_norm > b.curvature_column_norm;
            });

  if (!out.rows.empty()) {
    out.dominant_parameter = out.rows.front().name;
    out.dominant_parameter_index = out.rows.front().index;
    out.dominant_curvature_column_norm = out.rows.front().curvature_column_norm;
  }

  return out;
}

inline FunctionalGradientVolatilitySummary summarize_gradient_volatility(
    const std::vector<std::vector<double>> &gradient_samples,
    const std::vector<double> &baseline_gradient,
    const std::vector<std::string> &names = {},
    double perturbation_scale = 0.0) {
  FunctionalGradientVolatilitySummary out;
  out.perturbation_scale = perturbation_scale;
  out.samples = gradient_samples.size();

  if (baseline_gradient.empty() || gradient_samples.empty()) {
    return out;
  }

  out.available = true;

  double baseline_sumsq = 0.0;
  for (const double g : baseline_gradient) {
    baseline_sumsq += g * g;
  }
  out.baseline_gradient_norm = std::sqrt(baseline_sumsq);

  std::vector<double> norms;
  norms.reserve(gradient_samples.size());

  for (const auto &g : gradient_samples) {
    double ss = 0.0;
    for (const double v : g) {
      ss += v * v;
    }
    norms.push_back(std::sqrt(ss));
  }

  const double norm_sum = std::accumulate(norms.begin(), norms.end(), 0.0);
  out.mean_gradient_norm = norm_sum / static_cast<double>(norms.size());

  double norm_var = 0.0;
  out.max_gradient_norm = norms[0];
  for (const double x : norms) {
    norm_var += (x - out.mean_gradient_norm) * (x - out.mean_gradient_norm);
    out.max_gradient_norm = std::max(out.max_gradient_norm, x);
  }
  out.sd_gradient_norm =
      std::sqrt(norm_var / static_cast<double>(norms.size()));
  out.gradient_norm_cv =
      std::abs(out.mean_gradient_norm) > 0.0
          ? out.sd_gradient_norm / std::abs(out.mean_gradient_norm)
          : std::numeric_limits<double>::quiet_NaN();

  const std::size_t p = baseline_gradient.size();
  std::vector<double> component_sd(p, 0.0);
  std::vector<std::size_t> sign_flips(p, 0);

  for (std::size_t j = 0; j < p; ++j) {
    double mean = 0.0;
    std::size_t count = 0;
    for (const auto &g : gradient_samples) {
      if (j < g.size()) {
        mean += g[j];
        ++count;
      }
    }
    if (count == 0)
      continue;
    mean /= static_cast<double>(count);

    double var = 0.0;
    for (const auto &g : gradient_samples) {
      if (j < g.size()) {
        var += (g[j] - mean) * (g[j] - mean);

        if ((baseline_gradient[j] > 0.0 && g[j] < 0.0) ||
            (baseline_gradient[j] < 0.0 && g[j] > 0.0)) {
          ++sign_flips[j];
        }
      }
    }
    component_sd[j] = std::sqrt(var / static_cast<double>(count));
  }

  for (std::size_t j = 0; j < p; ++j) {
    if (j == 0 || component_sd[j] > out.most_volatile_parameter_sd) {
      out.most_volatile_parameter_sd = component_sd[j];
      out.most_volatile_parameter_index = j;
    }
    if (sign_flips[j] > out.most_sign_flips) {
      out.most_sign_flips = sign_flips[j];
      out.most_sign_flips_parameter_index = j;
    }
  }

  out.most_volatile_parameter =
      out.most_volatile_parameter_index < names.size()
          ? names[out.most_volatile_parameter_index]
          : ("param_" + std::to_string(out.most_volatile_parameter_index));
  out.most_sign_flips_parameter =
      out.most_sign_flips_parameter_index < names.size()
          ? names[out.most_sign_flips_parameter_index]
          : ("param_" + std::to_string(out.most_sign_flips_parameter_index));

  return out;
}

inline FunctionalSpectralStructureSummary
summarize_spectral_structure(const Eigen::MatrixXd &H) {
  FunctionalSpectralStructureSummary out;

  if (H.rows() == 0 || H.cols() == 0 || H.rows() != H.cols()) {
    return out;
  }

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(H);
  if (solver.info() != Eigen::Success) {
    return out;
  }

  std::vector<double> vals;
  vals.reserve(static_cast<std::size_t>(H.rows()));
  for (Eigen::Index i = 0; i < solver.eigenvalues().size(); ++i) {
    vals.push_back(std::max(0.0, solver.eigenvalues()(i)));
  }

  std::sort(vals.begin(), vals.end(), std::greater<double>());

  const double total = std::accumulate(vals.begin(), vals.end(), 0.0);
  if (total <= 0.0) {
    return out;
  }

  out.available = true;
  out.eigen_count = vals.size();
  out.eigen_sum = total;
  out.largest_eigen_share = vals.empty() ? 0.0 : vals.front() / total;
  out.eigenvalues_desc = vals;

  double entropy = 0.0;
  double cumulative = 0.0;
  out.cumulative_share.reserve(vals.size());

  auto needed_for = [&](double target) {
    std::size_t k = 0;
    double cum = 0.0;
    for (double v : vals) {
      ++k;
      cum += v / total;
      if (cum >= target) {
        return k;
      }
    }
    return vals.size();
  };

  for (double v : vals) {
    const double p = v / total;
    if (p > 0.0) {
      entropy -= p * std::log(p);
    }
    cumulative += p;
    out.cumulative_share.push_back(cumulative);
  }

  out.effective_rank_entropy = std::exp(entropy);
  out.eigen_count_for_50 = needed_for(0.50);
  out.eigen_count_for_90 = needed_for(0.90);
  out.eigen_count_for_95 = needed_for(0.95);
  out.eigen_count_for_99 = needed_for(0.99);

  return out;
}

inline Eigen::MatrixXd
covariance_from_positive_definite_hessian(const Eigen::MatrixXd &H) {
  if (H.rows() == 0 || H.cols() == 0 || H.rows() != H.cols()) {
    return Eigen::MatrixXd();
  }

  Eigen::LLT<Eigen::MatrixXd> llt(H);
  if (llt.info() != Eigen::Success) {
    return Eigen::MatrixXd();
  }

  return llt.solve(Eigen::MatrixXd::Identity(H.rows(), H.cols()));
}

inline FunctionalAnalysisReport make_functional_analysis_report(
    const FunctionalOptimizationSummary &optimization,
    const Eigen::MatrixXd &Huu, const std::vector<double> &latent_states,
    double nonzero_tol = 1.0e-8,
    const std::vector<std::string> &random_effect_names = {},
    std::size_t top_correlation_count = 10) {
  FunctionalAnalysisReport report;
  report.optimization = optimization;
  report.laplace_structure =
      summarize_laplace_hessian_structure(Huu, nonzero_tol);

  const Eigen::MatrixXd cov = covariance_from_positive_definite_hessian(Huu);
  report.uncertainty = summarize_covariance_correlation(cov);
  report.latent_states = summarize_latent_states(latent_states);
  report.parameter_influence = summarize_parameter_influence(
      cov, random_effect_names, top_correlation_count, &Huu);
  report.correlation_graph =
      summarize_correlation_graph(cov, random_effect_names, 0.5);
  report.spectral_structure = summarize_spectral_structure(Huu);

  return report;
}

inline void
write_functional_analysis_report_text(const FunctionalAnalysisReport &report,
                                      std::ostream &out) {
  out << std::setprecision(15);
  out << "Functional Analysis Report\n";
  out << "==========================\n\n";

  out << "Optimization\n";
  out << "------------\n";
  out << "objective_value:            " << report.optimization.objective_value
      << "\n";
  out << "gradient_norm:              " << report.optimization.gradient_norm
      << "\n";
  out << "max_gradient_parameter:     "
      << report.optimization.max_gradient_parameter << "\n";
  out << "max_gradient_value:         "
      << report.optimization.max_gradient_value << "\n";
  out << "max_abs_gradient:           " << report.optimization.max_abs_gradient
      << "\n";
  out << "iterations:                 " << report.optimization.iterations
      << "\n";
  out << "converged:                  "
      << (report.optimization.converged ? "yes" : "no") << "\n";
  out << "message:                    " << report.optimization.message
      << "\n\n";

  out << "Curvature\n";
  out << "---------\n";
  out << "positive_definite:          "
      << (report.laplace_structure.positive_definite ? "yes" : "no") << "\n";
  out << "min_eigenvalue:             "
      << report.laplace_structure.min_eigenvalue << "\n";
  out << "max_eigenvalue:             "
      << report.laplace_structure.max_eigenvalue << "\n";
  out << "condition_number_abs:       "
      << report.laplace_structure.condition_number_abs << "\n\n";

  out << "Spectral Structure\n";
  out << "------------------\n";
  out << "available:                  "
      << (report.spectral_structure.available ? "yes" : "no") << "\n";
  out << "eigen_count:                " << report.spectral_structure.eigen_count
      << "\n";
  out << "largest_eigen_share:        "
      << report.spectral_structure.largest_eigen_share << "\n";
  out << "effective_rank_entropy:     "
      << report.spectral_structure.effective_rank_entropy << "\n";
  out << "eigen_count_for_50%:        "
      << report.spectral_structure.eigen_count_for_50 << "\n";
  out << "eigen_count_for_90%:        "
      << report.spectral_structure.eigen_count_for_90 << "\n";
  out << "eigen_count_for_95%:        "
      << report.spectral_structure.eigen_count_for_95 << "\n";
  out << "eigen_count_for_99%:        "
      << report.spectral_structure.eigen_count_for_99 << "\n\n";

  out << "Huu Structure\n";
  out << "-------------\n";
  out << "random_effects:             "
      << report.laplace_structure.random_effects << "\n";
  out << "total_entries:              "
      << report.laplace_structure.total_entries << "\n";
  out << "structural_nonzeros:        "
      << report.laplace_structure.structural_nonzeros << "\n";
  out << "structural_density:         "
      << report.laplace_structure.structural_density << "\n\n";

  out << "Effective Sparsity\n";
  out << "------------------\n";
  out << "curvature_retained,entries_required,entry_share,compression_vs_"
         "structural\n";
  for (const auto &row : report.laplace_structure.effective_sparsity) {
    out << row.label << "," << row.entries_required << "," << row.entry_share
        << "," << row.compression_vs_structural << "\n";
  }
  out << "\n";

  out << "Effective Bandwidth\n";
  out << "-------------------\n";
  out << "curvature_retained,bandwidth,entry_count_if_banded,entry_share_if_"
         "banded\n";
  for (const auto &row : report.laplace_structure.effective_bandwidth) {
    out << row.label << "," << row.bandwidth << "," << row.entry_count_if_banded
        << "," << row.entry_share_if_banded << "\n";
  }
  out << "\n";

  out << "Uncertainty\n";
  out << "-----------\n";
  out << "covariance_available:       "
      << (report.uncertainty.covariance_available ? "yes" : "no") << "\n";
  out << "correlation_available:      "
      << (report.uncertainty.correlation_available ? "yes" : "no") << "\n";
  out << "covariance_size:            " << report.uncertainty.covariance_rows
      << " x " << report.uncertainty.covariance_cols << "\n";
  out << "min_variance:               " << report.uncertainty.min_variance
      << "\n";
  out << "max_variance:               " << report.uncertainty.max_variance
      << "\n";
  out << "min_variance_index:         " << report.uncertainty.min_variance_index
      << "\n";
  out << "max_variance_index:         " << report.uncertainty.max_variance_index
      << "\n";
  out << "max_abs_correlation:        "
      << report.uncertainty.max_abs_correlation << "\n";
  out << "max_abs_correlation_pair:   "
      << report.uncertainty.max_abs_correlation_i << ","
      << report.uncertainty.max_abs_correlation_j << "\n";
  out << "count_abs_corr_gt_0_5:      " << report.uncertainty.corr_abs_gt_0_5
      << "\n";
  out << "count_abs_corr_gt_0_8:      " << report.uncertainty.corr_abs_gt_0_8
      << "\n";
  out << "count_abs_corr_gt_0_9:      " << report.uncertainty.corr_abs_gt_0_9
      << "\n\n";

  out << "Parameter Influence\n";
  out << "-------------------\n";
  out << "available:                  "
      << (report.parameter_influence.available ? "yes" : "no") << "\n";
  out << "Top parameter importance\n";
  out << "index,name,variance,sd,variance_share,correlation_centrality,"
         "correlation_centrality_share,curvature_column_norm,"
         "curvature_diagonal,importance_score,importance_share\n";
  for (const auto &row : report.parameter_influence.variance_rows) {
    out << row.index << "," << row.name << "," << row.variance << "," << row.sd
        << "," << row.variance_share << "," << row.correlation_centrality << ","
        << row.correlation_centrality_share << "," << row.curvature_column_norm
        << "," << row.curvature_diagonal << "," << row.importance_score << ","
        << row.importance_share << "\n";
  }
  out << "\nTop correlation pairs\n";
  out << "i,j,name_i,name_j,correlation,abs_correlation\n";
  for (const auto &row : report.parameter_influence.top_correlation_rows) {
    out << row.i << "," << row.j << "," << row.name_i << "," << row.name_j
        << "," << row.correlation << "," << row.abs_correlation << "\n";
  }
  out << "\n";

  out << "Correlation Graph\n";
  out << "-----------------\n";
  out << "available:                  "
      << (report.correlation_graph.available ? "yes" : "no") << "\n";
  out << "abs_correlation_threshold:  "
      << report.correlation_graph.abs_correlation_threshold << "\n";
  out << "node_count:                 " << report.correlation_graph.node_count
      << "\n";
  out << "edge_count:                 " << report.correlation_graph.edge_count
      << "\n";
  out << "average_degree:             "
      << report.correlation_graph.average_degree << "\n";
  out << "maximum_degree:             "
      << report.correlation_graph.maximum_degree << "\n";
  out << "maximum_degree_parameter:   "
      << report.correlation_graph.maximum_degree_name << "\n";
  out << "connected_components:       "
      << report.correlation_graph.connected_components << "\n";
  out << "largest_component_size:     "
      << report.correlation_graph.largest_component_size << "\n";
  out << "graph_diameter:             "
      << report.correlation_graph.graph_diameter << "\n\n";

  out << "Parameter Geometry\n";
  out << "------------------\n";
  out << "available:                  "
      << (report.parameter_geometry.available ? "yes" : "no") << "\n";
  out << "dominant_parameter:         "
      << report.parameter_geometry.dominant_parameter << "\n";
  out << "dominant_parameter_index:   "
      << report.parameter_geometry.dominant_parameter_index << "\n";
  out << "dominant_curvature_norm:    "
      << report.parameter_geometry.dominant_curvature_column_norm << "\n";
  out << "index,name,gradient,abs_gradient,curvature_column_norm,"
         "curvature_diagonal,curvature_share\n";
  for (const auto &row : report.parameter_geometry.rows) {
    out << row.index << "," << row.name << "," << row.gradient << ","
        << row.abs_gradient << "," << row.curvature_column_norm << ","
        << row.curvature_diagonal << "," << row.curvature_share << "\n";
  }
  out << "\n";

  out << "Gradient Volatility\n";
  out << "-------------------\n";
  out << "available:                  "
      << (report.gradient_volatility.available ? "yes" : "no") << "\n";
  out << "perturbation_scale:         "
      << report.gradient_volatility.perturbation_scale << "\n";
  out << "samples:                    " << report.gradient_volatility.samples
      << "\n";
  out << "baseline_gradient_norm:     "
      << report.gradient_volatility.baseline_gradient_norm << "\n";
  out << "mean_gradient_norm:         "
      << report.gradient_volatility.mean_gradient_norm << "\n";
  out << "sd_gradient_norm:           "
      << report.gradient_volatility.sd_gradient_norm << "\n";
  out << "max_gradient_norm:          "
      << report.gradient_volatility.max_gradient_norm << "\n";
  out << "gradient_norm_cv:           "
      << report.gradient_volatility.gradient_norm_cv << "\n";
  out << "most_volatile_parameter:    "
      << report.gradient_volatility.most_volatile_parameter << "\n";
  out << "most_volatile_parameter_sd: "
      << report.gradient_volatility.most_volatile_parameter_sd << "\n";
  out << "most_sign_flips_parameter:  "
      << report.gradient_volatility.most_sign_flips_parameter << "\n";
  out << "most_sign_flips:            "
      << report.gradient_volatility.most_sign_flips << "\n\n";

  out << "Latent States\n";
  out << "-------------\n";
  out << "count:                      " << report.latent_states.count << "\n";
  out << "mean:                       " << report.latent_states.mean << "\n";
  out << "sd:                         " << report.latent_states.sd << "\n";
  out << "min_value:                  " << report.latent_states.min_value
      << "\n";
  out << "max_value:                  " << report.latent_states.max_value
      << "\n";
  out << "min_index:                  " << report.latent_states.min_index
      << "\n";
  out << "max_index:                  " << report.latent_states.max_index
      << "\n";
  out << "l2_norm:                    " << report.latent_states.l2_norm << "\n";
}

inline void
write_functional_analysis_report_text(const FunctionalAnalysisReport &report,
                                      const std::string &path) {
  std::ofstream out(path);
  write_functional_analysis_report_text(report, out);
}

inline void
write_functional_analysis_report_csv(const FunctionalAnalysisReport &report,
                                     std::ostream &out) {
  out << std::setprecision(15);
  out << "section,metric,target,value,extra\n";

  out << "optimization,objective_value,," << report.optimization.objective_value
      << ",\n";
  out << "optimization,gradient_norm,," << report.optimization.gradient_norm
      << ",\n";
  out << "optimization,max_gradient_parameter,,"
      << report.optimization.max_gradient_parameter << ",\n";
  out << "optimization,max_gradient_value,,"
      << report.optimization.max_gradient_value << ",\n";
  out << "optimization,max_abs_gradient,,"
      << report.optimization.max_abs_gradient << ",\n";
  out << "optimization,iterations,," << report.optimization.iterations << ",\n";
  out << "optimization,converged,,"
      << (report.optimization.converged ? "yes" : "no") << ",\n";
  out << "optimization,message,," << report.optimization.message << ",\n";

  out << "curvature,positive_definite,,"
      << (report.laplace_structure.positive_definite ? "yes" : "no") << ",\n";
  out << "curvature,min_eigenvalue,," << report.laplace_structure.min_eigenvalue
      << ",\n";
  out << "curvature,max_eigenvalue,," << report.laplace_structure.max_eigenvalue
      << ",\n";
  out << "curvature,condition_number_abs,,"
      << report.laplace_structure.condition_number_abs << ",\n";

  out << "spectral_structure,available,,"
      << (report.spectral_structure.available ? "yes" : "no") << ",\n";
  out << "spectral_structure,largest_eigen_share,,"
      << report.spectral_structure.largest_eigen_share << ",\n";
  out << "spectral_structure,effective_rank_entropy,,"
      << report.spectral_structure.effective_rank_entropy << ",\n";
  out << "spectral_structure,eigen_count_for_50%,,"
      << report.spectral_structure.eigen_count_for_50 << ",\n";
  out << "spectral_structure,eigen_count_for_90%,,"
      << report.spectral_structure.eigen_count_for_90 << ",\n";
  out << "spectral_structure,eigen_count_for_95%,,"
      << report.spectral_structure.eigen_count_for_95 << ",\n";
  out << "spectral_structure,eigen_count_for_99%,,"
      << report.spectral_structure.eigen_count_for_99 << ",\n";

  out << "huu_structure,random_effects,,"
      << report.laplace_structure.random_effects << ",\n";
  out << "huu_structure,total_entries,,"
      << report.laplace_structure.total_entries << ",\n";
  out << "huu_structure,structural_nonzeros,,"
      << report.laplace_structure.structural_nonzeros << ",\n";
  out << "huu_structure,structural_density,,"
      << report.laplace_structure.structural_density << ",\n";

  for (const auto &row : report.laplace_structure.effective_sparsity) {
    out << "effective_sparsity,entries_required," << row.label << ","
        << row.entries_required
        << ",compression_vs_structural=" << row.compression_vs_structural
        << "\n";
  }

  for (const auto &row : report.laplace_structure.effective_bandwidth) {
    out << "effective_bandwidth,bandwidth," << row.label << "," << row.bandwidth
        << ",entry_count_if_banded=" << row.entry_count_if_banded << "\n";
  }

  out << "uncertainty,covariance_available,,"
      << (report.uncertainty.covariance_available ? "yes" : "no") << ",\n";
  out << "uncertainty,correlation_available,,"
      << (report.uncertainty.correlation_available ? "yes" : "no") << ",\n";
  out << "uncertainty,min_variance,," << report.uncertainty.min_variance
      << ",index=" << report.uncertainty.min_variance_index << "\n";
  out << "uncertainty,max_variance,," << report.uncertainty.max_variance
      << ",index=" << report.uncertainty.max_variance_index << "\n";
  out << "uncertainty,max_abs_correlation,,"
      << report.uncertainty.max_abs_correlation
      << ",pair=" << report.uncertainty.max_abs_correlation_i << ";"
      << report.uncertainty.max_abs_correlation_j << "\n";
  out << "uncertainty,count_abs_corr_gt_0_5,,"
      << report.uncertainty.corr_abs_gt_0_5 << ",\n";
  out << "uncertainty,count_abs_corr_gt_0_8,,"
      << report.uncertainty.corr_abs_gt_0_8 << ",\n";
  out << "uncertainty,count_abs_corr_gt_0_9,,"
      << report.uncertainty.corr_abs_gt_0_9 << ",\n";

  for (const auto &row : report.parameter_influence.variance_rows) {
    out << "parameter_influence,importance," << row.name << ","
        << row.importance_score << ",index=" << row.index << ";sd=" << row.sd
        << ";variance=" << row.variance
        << ";variance_share=" << row.variance_share
        << ";correlation_centrality=" << row.correlation_centrality
        << ";curvature_column_norm=" << row.curvature_column_norm
        << ";importance_share=" << row.importance_share << "\n";
  }

  for (const auto &row : report.parameter_influence.top_correlation_rows) {
    out << "parameter_influence,correlation_pair," << row.name_i << "__"
        << row.name_j << "," << row.correlation << ",i=" << row.i
        << ";j=" << row.j << ";abs_correlation=" << row.abs_correlation << "\n";
  }

  out << "correlation_graph,abs_correlation_threshold,,"
      << report.correlation_graph.abs_correlation_threshold << ",\n";
  out << "correlation_graph,node_count,," << report.correlation_graph.node_count
      << ",\n";
  out << "correlation_graph,edge_count,," << report.correlation_graph.edge_count
      << ",\n";
  out << "correlation_graph,average_degree,,"
      << report.correlation_graph.average_degree << ",\n";
  out << "correlation_graph,maximum_degree,,"
      << report.correlation_graph.maximum_degree
      << ",parameter=" << report.correlation_graph.maximum_degree_name << "\n";
  out << "correlation_graph,connected_components,,"
      << report.correlation_graph.connected_components << ",\n";
  out << "correlation_graph,largest_component_size,,"
      << report.correlation_graph.largest_component_size << ",\n";
  out << "correlation_graph,graph_diameter,,"
      << report.correlation_graph.graph_diameter << ",\n";

  out << "parameter_geometry,available,,"
      << (report.parameter_geometry.available ? "yes" : "no") << ",\n";
  out << "parameter_geometry,dominant_parameter,,"
      << report.parameter_geometry.dominant_parameter
      << ",index=" << report.parameter_geometry.dominant_parameter_index
      << ";curvature_column_norm="
      << report.parameter_geometry.dominant_curvature_column_norm << "\n";

  for (const auto &row : report.parameter_geometry.rows) {
    out << "parameter_geometry,curvature_column_norm," << row.name << ","
        << row.curvature_column_norm << ",index=" << row.index
        << ";gradient=" << row.gradient << ";abs_gradient=" << row.abs_gradient
        << ";curvature_diagonal=" << row.curvature_diagonal
        << ";curvature_share=" << row.curvature_share << "\n";
  }

  out << "gradient_volatility,available,,"
      << (report.gradient_volatility.available ? "yes" : "no") << ",\n";
  out << "gradient_volatility,perturbation_scale,,"
      << report.gradient_volatility.perturbation_scale << ",\n";
  out << "gradient_volatility,samples,," << report.gradient_volatility.samples
      << ",\n";
  out << "gradient_volatility,baseline_gradient_norm,,"
      << report.gradient_volatility.baseline_gradient_norm << ",\n";
  out << "gradient_volatility,mean_gradient_norm,,"
      << report.gradient_volatility.mean_gradient_norm << ",\n";
  out << "gradient_volatility,sd_gradient_norm,,"
      << report.gradient_volatility.sd_gradient_norm << ",\n";
  out << "gradient_volatility,max_gradient_norm,,"
      << report.gradient_volatility.max_gradient_norm << ",\n";
  out << "gradient_volatility,gradient_norm_cv,,"
      << report.gradient_volatility.gradient_norm_cv << ",\n";
  out << "gradient_volatility,most_volatile_parameter,,"
      << report.gradient_volatility.most_volatile_parameter
      << ",sd=" << report.gradient_volatility.most_volatile_parameter_sd
      << "\n";
  out << "gradient_volatility,most_sign_flips_parameter,,"
      << report.gradient_volatility.most_sign_flips_parameter
      << ",sign_flips=" << report.gradient_volatility.most_sign_flips << "\n";

  out << "latent_states,count,," << report.latent_states.count << ",\n";
  out << "latent_states,mean,," << report.latent_states.mean << ",\n";
  out << "latent_states,sd,," << report.latent_states.sd << ",\n";
  out << "latent_states,min_value,," << report.latent_states.min_value
      << ",index=" << report.latent_states.min_index << "\n";
  out << "latent_states,max_value,," << report.latent_states.max_value
      << ",index=" << report.latent_states.max_index << "\n";
  out << "latent_states,l2_norm,," << report.latent_states.l2_norm << ",\n";
}

inline void
write_functional_analysis_report_csv(const FunctionalAnalysisReport &report,
                                     const std::string &path) {
  std::ofstream out(path);
  write_functional_analysis_report_csv(report, out);
}

} // namespace quadra
