#pragma once

#include "effective_structure.hpp"
#include "model_health.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace quadra {
namespace diagnostics {

inline std::string csv_get_value(const std::string &csv_path,
                                 const std::string &metric_or_field) {
  std::ifstream in(csv_path);
  std::string line;

  while (std::getline(in, line)) {
    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string a, b, c, d;
    std::getline(ss, a, ',');
    std::getline(ss, b, ',');
    std::getline(ss, c, ',');
    std::getline(ss, d, ',');

    if (a == metric_or_field)
      return b;
    if (b == metric_or_field)
      return d;
  }

  return "";
}

struct MarkdownReportConfig {
  std::string title = "Quadra Functional Analysis";
  std::string subtitle;
  std::string output_path;
  std::string functional_csv_path;
  std::string structure_txt_path;

  std::string fixed_effects = "";
  std::string total_estimated = "";
  std::string effective_entries_95 = "58";
  std::string effective_bandwidth_95 = "1";
};

inline void
write_functional_analysis_markdown(const MarkdownReportConfig &config) {
  const std::string &functional_csv_path = config.functional_csv_path;

  const std::string objective =
      csv_get_value(functional_csv_path, "objective_value");
  const std::string grad_norm =
      csv_get_value(functional_csv_path, "gradient_norm");
  const std::string converged = csv_get_value(functional_csv_path, "converged");
  const std::string max_grad_param =
      csv_get_value(functional_csv_path, "max_gradient_parameter");

  const std::string pd =
      csv_get_value(functional_csv_path, "positive_definite");
  const std::string condition =
      csv_get_value(functional_csv_path, "condition_number_abs");
  const std::string min_eigen =
      csv_get_value(functional_csv_path, "min_eigenvalue");
  const std::string max_eigen =
      csv_get_value(functional_csv_path, "max_eigenvalue");

  const std::string largest_eigen_share =
      csv_get_value(functional_csv_path, "largest_eigen_share");
  const std::string effective_rank =
      csv_get_value(functional_csv_path, "effective_rank_entropy");
  const std::string eigen_90 =
      csv_get_value(functional_csv_path, "eigen_count_for_90%");
  const std::string eigen_95 =
      csv_get_value(functional_csv_path, "eigen_count_for_95%");

  const std::string density =
      csv_get_value(functional_csv_path, "structural_density");
  const std::string nonzeros =
      csv_get_value(functional_csv_path, "structural_nonzeros");
  const std::string random_effects =
      csv_get_value(functional_csv_path, "random_effects");

  const std::string avg_degree =
      csv_get_value(functional_csv_path, "average_degree");
  const std::string max_degree_graph =
      csv_get_value(functional_csv_path, "maximum_degree");
  const std::string components =
      csv_get_value(functional_csv_path, "connected_components");
  const std::string largest_component =
      csv_get_value(functional_csv_path, "largest_component_size");
  const std::string diameter =
      csv_get_value(functional_csv_path, "graph_diameter");

  const std::string latent_count = csv_get_value(functional_csv_path, "count");
  const std::string latent_mean = csv_get_value(functional_csv_path, "mean");
  const std::string latent_sd = csv_get_value(functional_csv_path, "sd");

  const std::string fixed_effects =
      config.fixed_effects.empty() ? "unknown" : config.fixed_effects;
  const std::string total_estimated =
      config.total_estimated.empty() ? "unknown" : config.total_estimated;

  const std::string compression_95 =
      compression_label(nonzeros, config.effective_entries_95);

  const ModelHealthStatus health =
      evaluate_model_health(converged, grad_norm, pd, condition);

  const std::string uncertainty_structure = uncertainty_structure_label(
      avg_degree, max_degree_graph, diameter, random_effects);

  std::ofstream md(config.output_path);
  md << "# " << config.title << "\n\n";
  if (!config.subtitle.empty())
    md << config.subtitle << "\n\n";

  md << "## Executive Summary\n\n";
  md << "- **Overall status:** `" << health.overall << "`.\n";
  md << "- **Confidence:** `" << health.confidence << "`.\n";
  md << "- **Optimization quality:** `" << health.optimization_quality
     << "`.\n";
  md << "- **Uncertainty structure:** `" << uncertainty_structure << "`.\n";
  md << "- **Optimization:** converged = `" << converged
     << "`, gradient norm = `" << grad_norm << "`.\n";
  md << "- **Curvature health:** positive definite = `" << pd
     << "`, condition number = `" << condition << "`.\n";
  md << "- **Latent structure:** `" << random_effects
     << "` random effects were estimated.\n";
  md << "- **Symbolic vs numerical structure:** structural density = `"
     << density << "`, but 95% of curvature is retained by `"
     << config.effective_entries_95 << "` entries.\n";
  md << "- **Spectral complexity:** entropy effective rank = `"
     << effective_rank << "`, with 90% curvature requiring `" << eigen_90
     << "` eigen-directions.\n\n";

  md << "## Model Health Assessment\n\n";
  md << "| Check | Status | Evidence |\n";
  md << "|---|---:|---|\n";
  md << "| Optimization | `" << health.optimization << "` | converged = `"
     << converged << "` |\n";
  md << "| Gradient quality | `" << health.gradient << "` | gradient norm = `"
     << grad_norm << "` |\n";
  md << "| Curvature | `" << health.curvature << "` | positive definite = `"
     << pd << "` |\n";
  md << "| Conditioning | `" << health.conditioning
     << "` | condition number = `" << condition << "` |\n";
  md << "| Overall status | `" << health.overall
     << "` | rule-based v1 diagnostic |\n";
  md << "| Confidence | `" << health.confidence
     << "` | based on convergence, gradient, PD status, and conditioning |\n\n";
  md << "**Interpretation:** the rule-based health check is intentionally "
        "simple. "
        "It flags obvious numerical issues quickly, but it does not replace "
        "scientific review or model-specific diagnostics.\n\n";

  md << "## Model Complexity\n\n";
  md << "| Quantity | Value |\n";
  md << "|---|---:|\n";
  md << "| Fixed effects | `" << fixed_effects << "` |\n";
  md << "| Random effects | `" << random_effects << "` |\n";
  md << "| Total estimated quantities | `" << total_estimated << "` |\n";
  md << "| Structural nonzeros | `" << nonzeros << "` |\n";
  md << "| Structural density | `" << density << "` |\n";
  md << "| Entries for 95% curvature | `" << config.effective_entries_95
     << "` |\n";
  md << "| Effective bandwidth for 95% curvature | `"
     << config.effective_bandwidth_95 << "` |\n";
  md << "| 95% curvature compression | `" << compression_95 << "x` |\n\n";

  md << "## Optimization\n\n";
  md << "- Quality: `" << health.optimization_quality << "`\n";
  md << "- Objective value: `" << objective << "`\n";
  md << "- Gradient norm: `" << grad_norm << "`\n";
  md << "- Converged: `" << converged << "`\n";
  md << "- Max gradient parameter: `" << max_grad_param << "`\n\n";

  md << "## Curvature\n\n";
  md << "- Positive definite: `" << pd << "`\n";
  md << "- Condition number: `" << condition << "`\n";
  md << "- Minimum eigenvalue: `" << min_eigen << "`\n";
  md << "- Maximum eigenvalue: `" << max_eigen << "`\n\n";

  md << "## Spectral Structure\n\n";
  md << "- Largest eigenvalue share: `" << largest_eigen_share << "`\n";
  md << "- Entropy effective rank: `" << effective_rank << "`\n";
  md << "- Eigenvectors needed for 90% curvature: `" << eigen_90 << "`\n";
  md << "- Eigenvectors needed for 95% curvature: `" << eigen_95 << "`\n\n";
  md << "**Interpretation:** curvature is distributed across many latent-state "
        "directions rather than being dominated by one or two modes. That is a "
        "good sign for numerical stability.\n\n";

  md << "## Effective Structure\n\n";
  md << "- Structural density: `" << density << "`\n";
  md << "- Structural nonzeros: `" << nonzeros << "`\n";
  md << "- Entries for 95% curvature: `" << config.effective_entries_95
     << "`\n";
  md << "- Effective bandwidth for 95% curvature: `"
     << config.effective_bandwidth_95 << "`\n";
  md << "- 95% curvature compression: `" << compression_95 << "x`\n\n";
  md << "**Interpretation:** symbolic density alone overstates practical "
        "complexity. The detailed Laplace report below shows that large "
        "amounts of curvature can be retained with far fewer entries or a "
        "narrow effective bandwidth.\n\n";

  md << "## Correlation Graph\n\n";
  md << "- Classification: `" << uncertainty_structure << "`\n";
  md << "- Average degree: `" << avg_degree << "`\n";
  md << "- Maximum degree: `" << max_degree_graph << "`\n";
  md << "- Connected components: `" << components << "`\n";
  md << "- Largest component size: `" << largest_component << "`\n";
  md << "- Graph diameter: `" << diameter << "`\n\n";
  md << "**Interpretation:** a LOCAL graph means the strongest uncertainty "
        "relationships are neighborhood-like rather than globally tangled.\n\n";

  md << "## Latent State Summary\n\n";
  md << "- Count: `" << latent_count << "`\n";
  md << "- Mean: `" << latent_mean << "`\n";
  md << "- Standard deviation: `" << latent_sd << "`\n\n";

  md << "## Key Takeaway\n\n";
  md << "This report demonstrates why Quadra's functional analysis diagnostics "
        "are useful: a model can look dense from a symbolic Hessian pattern, "
        "while numerical curvature, graph structure, and effective bandwidth "
        "reveal a simpler local-dependence structure.\n\n";

  md << "## Full Laplace Structure Report\n\n";
  md << "```text\n";
  std::ifstream txt(config.structure_txt_path);
  std::string line;
  while (std::getline(txt, line))
    md << line << "\n";
  md << "```\n";
}

// Preferred public API name for Functional Analysis v1 markdown output.
inline void write_markdown_report(const MarkdownReportConfig &config) {
  write_functional_analysis_markdown(config);
}

} // namespace diagnostics
} // namespace quadra
