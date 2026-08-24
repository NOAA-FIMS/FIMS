/**
 * \file quadra.cpp
 * \brief Quadra backend linkage for the FIMS shared library.
 */

#ifdef QUADRA_MODEL

#ifndef R_NO_REMAP
#define R_NO_REMAP
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <R.h>
#include <Rinternals.h>

#include <quadra/quadra.hpp>
#include <quadra/stats/laplace.hpp>
#include "../inst/include/interface/Quadra/core/autodiff/compact_first_order_tape.hpp"

#include "../inst/include/common/model.hpp"

// Optimizer implementations are compiled into this translation unit.
// Quadra's header-only AD engine requires exactly one graph definition in the
// final binary. Keeping it here lets Quadra and TMB live in the same FIMS DLL.
DECLARE_ADGRAPH()

namespace fims_quadra {

namespace {
// Shared interface registration can construct Quadra scalar mirrors even when
// TMB is the first backend requested, so an empty graph must exist as soon as
// the combined FIMS library is loaded.
std::unique_ptr<quadra::TapeContext> tape =
    std::make_unique<quadra::TapeContext>();
}

void reset_tape() {
  if (tape) {
    tape->reset();
  } else {
    tape = std::make_unique<quadra::TapeContext>();
  }
}

void release_tape() {
  // Quadra scalar mirrors can be constructed while CreateTMBModel() registers
  // shared interface objects in a combined build. Free the large graph, but
  // immediately leave a valid empty graph active for those constructors.
  tape.reset();
  tape = std::make_unique<quadra::TapeContext>();
}

class NativeModelAdapter {
 public:
  struct ScalarSnapshot {
    QUADRA_FIMS_TYPE* scalar;
    double value;
  };

  explicit NativeModelAdapter(
      std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> info)
      : info_(std::move(info)) {
    bound_graph_ = had::g_ADGraph;
    refresh_scalars();
  }

  void refresh_scalars() {
    scalars_.clear();
    std::unordered_set<QUADRA_FIMS_TYPE*> seen;
    for (auto& entry : info_->variable_map) {
      fims::Vector<QUADRA_FIMS_TYPE>* values = entry.second;
      for (size_t i = 0; i < values->size(); ++i) {
        QUADRA_FIMS_TYPE* scalar = &(*values)[i];
        if (seen.insert(scalar).second) {
          scalars_.push_back({scalar, scalar->val});
        }
      }
    }
  }

  void initialize(quadra::ModelReportContext& context) { context.clear(); }

  template <typename Type>
  Type evaluate(const std::vector<Type>& parameters,
                quadra::ModelReportContext&) {
    if (bound_graph_ != had::g_ADGraph) {
      for (const auto& snapshot : scalars_) {
        *snapshot.scalar = QUADRA_FIMS_TYPE(snapshot.value);
      }
      if (!info_->CreateModel()) {
        throw std::runtime_error(
            "Unable to rebuild the native FIMS model on Quadra's active graph");
      }
      bound_graph_ = had::g_ADGraph;
      refresh_scalars();
    }
    const size_t expected = info_->fixed_effects_parameters.size() +
                            info_->random_effects_parameters.size();
    if (parameters.size() != expected) {
      throw std::invalid_argument("FIMS Quadra parameter length mismatch");
    }
    auto assign_parameters = [&]() {
      for (const auto& snapshot : scalars_) {
        *snapshot.scalar = QUADRA_FIMS_TYPE(snapshot.value);
      }
      size_t index = 0;
      for (auto* parameter : info_->fixed_effects_parameters) {
        *parameter = QUADRA_FIMS_TYPE(parameters[index++]);
      }
      for (auto* parameter : info_->random_effects_parameters) {
        *parameter = QUADRA_FIMS_TYPE(parameters[index++]);
      }
    };
    if constexpr (std::is_same_v<Type, double>) {
      had::ADGraph primal_graph;
      assign_parameters();
      return quadra::value_of(
          fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance()->Evaluate());
    } else {
      assign_parameters();
      return fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance()->Evaluate();
    }
  }

  template <typename Type>
  Type operator()(const std::vector<Type>& parameters) {
    quadra::ModelReportContext context;
    return evaluate(parameters, context);
  }

 private:
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> info_;
  std::vector<ScalarSnapshot> scalars_;
  had::ADGraph* bound_graph_ = nullptr;
};

SEXP numeric_vector(const std::vector<double>& values) {
  SEXP result = PROTECT(
      Rf_allocVector(REALSXP, static_cast<R_xlen_t>(values.size())));
  std::copy(values.begin(), values.end(), REAL(result));
  UNPROTECT(1);
  return result;
}

SEXP numeric_matrix(const Eigen::MatrixXd& values) {
  SEXP result = PROTECT(Rf_allocMatrix(
      REALSXP, static_cast<int>(values.rows()),
      static_cast<int>(values.cols())));
  for (Eigen::Index column = 0; column < values.cols(); ++column) {
    for (Eigen::Index row = 0; row < values.rows(); ++row) {
      REAL(result)[row + values.rows() * column] = values(row, column);
    }
  }
  UNPROTECT(1);
  return result;
}

void set_list_element(SEXP list, SEXP names, int index, const char* name,
                      SEXP value) {
  SET_VECTOR_ELT(list, index, value);
  SET_STRING_ELT(names, index, Rf_mkChar(name));
}

}  // namespace fims_quadra

extern "C" SEXP fims_call_quadra_fit(SEXP fixed_sexp, SEXP random_sexp,
                                      SEXP method_sexp,
                                      SEXP max_iterations_sexp,
                                      SEXP tolerance_sexp) {
  try {
    if (TYPEOF(fixed_sexp) != REALSXP || TYPEOF(random_sexp) != REALSXP) {
      Rf_error("Quadra parameters must be numeric vectors.");
    }
    const std::string method = CHAR(Rf_asChar(method_sexp));
    const int max_iterations = Rf_asInteger(max_iterations_sexp);
    const double tolerance = Rf_asReal(tolerance_sexp);
    if (max_iterations < 1 || !(tolerance > 0.0)) {
      Rf_error("Quadra optimizer controls must be positive.");
    }
    std::vector<double> fixed(REAL(fixed_sexp),
                              REAL(fixed_sexp) + XLENGTH(fixed_sexp));
    std::vector<double> random(REAL(random_sexp),
                               REAL(random_sexp) + XLENGTH(random_sexp));
    auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
    if (fixed.size() != info->fixed_effects_parameters.size() ||
        random.size() != info->random_effects_parameters.size()) {
      Rf_error("Quadra parameter count does not match the FIMS model.");
    }

    fims_quadra::NativeModelAdapter model(info);
    std::vector<double> fixed_result;
    std::vector<double> random_result;
    std::vector<double> gradient;
    double objective = NA_REAL;
    int iterations = 0;
    bool converged = false;
    std::string message;
    const auto started = std::chrono::steady_clock::now();

    if (method == "laplace" && !random.empty()) {
      quadra::ParameterPartition partition;
      partition.fixed_indices_m.resize(fixed.size());
      std::iota(partition.fixed_indices_m.begin(),
                partition.fixed_indices_m.end(), size_t{0});
      partition.random_indices_m.resize(random.size());
      std::iota(partition.random_indices_m.begin(),
                partition.random_indices_m.end(), fixed.size());
      quadra::stats::LaplaceOptimizerOptions options;
      options.max_iterations = max_iterations;
      options.gradient_tolerance = tolerance;
      auto result = quadra::stats::optimize_laplace(
          model, fixed, random, partition, options);
      fixed_result = result.fixed;
      random_result = result.random_mode;
      gradient = result.gradient;
      objective = result.objective;
      iterations = result.iterations;
      converged = result.converged;
      message = result.message;
    } else {
      std::vector<double> combined = fixed;
      combined.insert(combined.end(), random.begin(), random.end());
      size_t parameter_index = 0;
      for (auto* parameter : info->fixed_effects_parameters) {
        had::SetValue(*parameter, combined[parameter_index++]);
      }
      for (auto* parameter : info->random_effects_parameters) {
        had::SetValue(*parameter, combined[parameter_index++]);
      }
      had::Forward();
      QUADRA_FIMS_TYPE loss =
          fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance()->Evaluate();
      std::vector<had::VertexId> vertices;
      for (const auto* parameter : info->fixed_effects_parameters) {
        vertices.push_back(parameter->varId);
      }
      for (const auto* parameter : info->random_effects_parameters) {
        vertices.push_back(parameter->varId);
      }
      quadra::CompactFirstOrderTape compact;
      compact.Build(*had::g_ADGraph, vertices, loss.varId);
      auto joint_objective = [&compact](const Eigen::VectorXd& values,
                                        Eigen::VectorXd& grad) {
        return compact.Evaluate(values, grad);
      };
      Eigen::VectorXd values = Eigen::Map<Eigen::VectorXd>(
          combined.data(), static_cast<Eigen::Index>(combined.size()));
      LBFGSpp::LBFGSParam<double> options;
      options.max_iterations = max_iterations;
      options.epsilon = tolerance;
      options.epsilon_rel = 0.0;
      LBFGSpp::LBFGSSolver<double> optimizer(options);
      try {
        iterations = optimizer.minimize(joint_objective, values, objective);
      } catch (const std::exception& exception) {
        message = exception.what();
      }
      Eigen::VectorXd final_gradient;
      objective = joint_objective(values, final_gradient);
      gradient.assign(final_gradient.data(),
                      final_gradient.data() + final_gradient.size());
      combined.assign(values.data(), values.data() + values.size());
      fixed_result.assign(combined.begin(), combined.begin() + fixed.size());
      random_result.assign(combined.begin() + fixed.size(), combined.end());
      const double gradient_norm = final_gradient.norm();
      converged = std::isfinite(gradient_norm) && gradient_norm <= tolerance;
      if (message.empty()) message = converged ? "Quadra L-BFGS converged" :
                                                "Quadra L-BFGS stopped";
    }

    const double elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started).count();
    SEXP out = PROTECT(Rf_allocVector(VECSXP, 9));
    SEXP names = PROTECT(Rf_allocVector(STRSXP, 9));
    fims_quadra::set_list_element(out, names, 0, "par",
                                  fims_quadra::numeric_vector(fixed_result));
    fims_quadra::set_list_element(out, names, 1, "random",
                                  fims_quadra::numeric_vector(random_result));
    fims_quadra::set_list_element(out, names, 2, "objective",
                                  Rf_ScalarReal(objective));
    fims_quadra::set_list_element(out, names, 3, "gradient",
                                  fims_quadra::numeric_vector(gradient));
    fims_quadra::set_list_element(out, names, 4, "iterations",
                                  Rf_ScalarInteger(iterations));
    fims_quadra::set_list_element(out, names, 5, "converged",
                                  Rf_ScalarLogical(converged));
    fims_quadra::set_list_element(out, names, 6, "message",
                                  Rf_mkString(message.c_str()));
    fims_quadra::set_list_element(out, names, 7, "elapsed_seconds",
                                  Rf_ScalarReal(elapsed));
    fims_quadra::set_list_element(out, names, 8, "backend",
                                  Rf_mkString(method.c_str()));
    Rf_setAttrib(out, R_NamesSymbol, names);
    UNPROTECT(2);
    return out;
  } catch (const std::exception& exception) {
    Rf_error("Quadra fit failed: %s", exception.what());
  }
  return R_NilValue;
}

extern "C" SEXP fims_call_quadra_sdreport(SEXP fixed_sexp,
                                           SEXP random_sexp) {
  try {
    if (TYPEOF(fixed_sexp) != REALSXP || TYPEOF(random_sexp) != REALSXP) {
      Rf_error("Quadra parameters must be numeric vectors.");
    }
    std::vector<double> fixed(REAL(fixed_sexp),
                              REAL(fixed_sexp) + XLENGTH(fixed_sexp));
    std::vector<double> random(REAL(random_sexp),
                               REAL(random_sexp) + XLENGTH(random_sexp));
    auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
    if (fixed.size() != info->fixed_effects_parameters.size() ||
        random.size() != info->random_effects_parameters.size()) {
      Rf_error("Quadra parameter count does not match the FIMS model.");
    }

    std::vector<QUADRA_FIMS_TYPE*> parameters;
    parameters.reserve(fixed.size() + random.size());
    size_t index = 0;
    for (auto* parameter : info->fixed_effects_parameters) {
      had::SetValue(*parameter, fixed[index++]);
      parameters.push_back(parameter);
    }
    index = 0;
    for (auto* parameter : info->random_effects_parameters) {
      had::SetValue(*parameter, random[index++]);
      parameters.push_back(parameter);
    }
    had::Forward();
    QUADRA_FIMS_TYPE objective =
        fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance()->Evaluate();
    had::ZeroAdjoints(*had::g_ADGraph);
    had::g_ADGraph->vertices[objective.varId].w = 1.0;
    had::PropagateAdjoint();

    const Eigen::Index n = static_cast<Eigen::Index>(parameters.size());
    Eigen::VectorXd gradient(n);
    Eigen::MatrixXd hessian(n, n);
    for (Eigen::Index row = 0; row < n; ++row) {
      gradient[row] = had::GetAdjoint(*parameters[row]);
      for (Eigen::Index column = 0; column < n; ++column) {
        hessian(row, column) =
            had::GetAdjoint(*parameters[row], *parameters[column]);
      }
    }
    hessian = 0.5 * (hessian + hessian.transpose()).eval();

    Eigen::LLT<Eigen::MatrixXd> factorization(hessian);
    const bool positive_definite =
        factorization.info() == Eigen::Success;
    Eigen::MatrixXd covariance = Eigen::MatrixXd::Constant(
        n, n, std::numeric_limits<double>::quiet_NaN());
    if (positive_definite) {
      covariance = factorization.solve(Eigen::MatrixXd::Identity(n, n));
      covariance = 0.5 * (covariance + covariance.transpose()).eval();
    }
    std::vector<double> standard_error(static_cast<size_t>(n), NA_REAL);
    for (Eigen::Index i = 0; i < n; ++i) {
      if (positive_definite && covariance(i, i) >= 0.0 &&
          std::isfinite(covariance(i, i))) {
        standard_error[static_cast<size_t>(i)] = std::sqrt(covariance(i, i));
      }
    }
    std::vector<double> gradient_values(
        gradient.data(), gradient.data() + gradient.size());

    SEXP out = PROTECT(Rf_allocVector(VECSXP, 10));
    SEXP names = PROTECT(Rf_allocVector(STRSXP, 10));
    fims_quadra::set_list_element(out, names, 0, "par.fixed",
                                  fims_quadra::numeric_vector(fixed));
    fims_quadra::set_list_element(out, names, 1, "par.random",
                                  fims_quadra::numeric_vector(random));
    fims_quadra::set_list_element(out, names, 2, "cov.all",
                                  fims_quadra::numeric_matrix(covariance));
    fims_quadra::set_list_element(
        out, names, 3, "cov.fixed",
        fims_quadra::numeric_matrix(covariance.topLeftCorner(
            static_cast<Eigen::Index>(fixed.size()),
            static_cast<Eigen::Index>(fixed.size()))));
    fims_quadra::set_list_element(out, names, 4, "hessian",
                                  fims_quadra::numeric_matrix(hessian));
    fims_quadra::set_list_element(out, names, 5, "gradient",
                                  fims_quadra::numeric_vector(gradient_values));
    fims_quadra::set_list_element(out, names, 6, "std.error",
                                  fims_quadra::numeric_vector(standard_error));
    fims_quadra::set_list_element(out, names, 7, "pdHess",
                                  Rf_ScalarLogical(positive_definite));
    fims_quadra::set_list_element(out, names, 8, "objective",
                                  Rf_ScalarReal(objective.val));
    fims_quadra::set_list_element(out, names, 9, "backend",
                                  Rf_mkString("quadra"));
    Rf_setAttrib(out, R_NamesSymbol, names);
    UNPROTECT(2);
    return out;
  } catch (const std::exception& exception) {
    Rf_error("Quadra sdreport failed: %s", exception.what());
  }
  return R_NilValue;
}

#endif  // QUADRA_MODEL
