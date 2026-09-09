#pragma once

// Quadra Functional Analysis v1
// =============================
//
// Public diagnostics/reporting API used by assessment examples.
//
// Stable v1 facade:
//   quadra::diagnostics::MarkdownReportConfig
//   quadra::diagnostics::write_markdown_report(config)
//   quadra::diagnostics::evaluate_model_health(...)
//   quadra::diagnostics::uncertainty_structure_label(...)
//   quadra::diagnostics::compression_label(...)
//
// Compatibility:
//   write_functional_analysis_markdown(config) remains available.
//
// Next migration target:
//   Move computation-side functional analysis summaries into core so examples
//   only provide model, parameters, and fit result.

#include "../laplace/laplace_structure_report.hpp"
#include "effective_structure.hpp"
#include "functional_analysis_markdown.hpp"
#include "model_health.hpp"
