/**
 * @file rcpp_interface.hpp
 * @brief The Rcpp interface to declare things.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP
#include <chrono>
#include <unordered_set>
#include "../../common/model.hpp"
#include "../../common/model_object.hpp"
#include "../../utilities/fims_json.hpp"
#include "rcpp_objects/rcpp_data.hpp"
#include "rcpp_objects/rcpp_distribution.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_interface_base.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_models.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"

#ifdef QUADRA_MODEL
#include "../Quadra/external/LBFGSpp/include/LBFGS.h"
#include "../Quadra/core/laplace/laplace_lbfgs_optimizer.hpp"
#include "../Quadra/core/laplace/laplace_backend_factory.hpp"
#include "../Quadra/core/optimizer.hpp"
#include "../Quadra/core/autodiff/laplace_graph_plan.hpp"
#include "../Quadra/core/autodiff/compact_first_order_tape.hpp"

namespace fims_quadra
{
  inline had::VertexId &benchmark_probe_vertex()
  {
    static had::VertexId vertex = 0;
    return vertex;
  }
  inline double &benchmark_probe_value()
  {
    static double value = 0.0;
    return value;
  }
  inline had::VertexId &objective_vertex()
  {
    static had::VertexId vertex = 0;
    return vertex;
  }

  inline void propagate_gradient(had::VertexId objective_vertex)
  {
    had::ZeroAdjoints(*had::g_ADGraph);
    had::g_ADGraph->vertices[objective_vertex].w = 1.0;
    for (had::VertexId vertex_id =
             static_cast<had::VertexId>(had::g_ADGraph->vertices.size() - 1);
         vertex_id > 0; --vertex_id)
    {
      had::ADVertex &vertex = had::g_ADGraph->vertices[vertex_id];
      if (vertex.e1.to == vertex_id)
        continue;
      const double adjoint = vertex.w;
      had::g_ADGraph->vertices[vertex.e1.to].w += adjoint * vertex.e1.w;
      if (vertex.e2.to != vertex_id)
      {
        had::g_ADGraph->vertices[vertex.e2.to].w += adjoint * vertex.e2.w;
      }
    }
  }

  class FIMSJointLBFGSObjective
  {
  public:
    quadra::CompactFirstOrderTape tape;
    int evaluations = 0;

    FIMSJointLBFGSObjective(std::vector<QUADRA_FIMS_TYPE *> parameter_pointers,
                            had::VertexId objective)
    {
      std::vector<had::VertexId> parameter_vertices;
      parameter_vertices.reserve(parameter_pointers.size());
      for (const auto *parameter : parameter_pointers)
      {
        parameter_vertices.push_back(parameter->varId);
      }
      tape.Build(*had::g_ADGraph, parameter_vertices, objective);
    }

    double operator()(const Eigen::VectorXd &values,
                      Eigen::VectorXd &gradient)
    {
      ++evaluations;
      return tape.Evaluate(values, gradient);
    }
  };

  class FIMSLaplaceModelAdapter
  {
  public:
    struct ScalarSnapshot
    {
      QUADRA_FIMS_TYPE *scalar_m;
      double value_m;
    };

    std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> info_m;
    std::vector<ScalarSnapshot> scalars_m;

    explicit FIMSLaplaceModelAdapter(
        std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> info)
        : info_m(std::move(info))
    {
      std::unordered_set<QUADRA_FIMS_TYPE *> seen;
      for (auto &entry : info_m->variable_map)
      {
        fims::Vector<QUADRA_FIMS_TYPE> *values = entry.second;
        for (size_t i = 0; i < values->size(); ++i)
        {
          QUADRA_FIMS_TYPE *scalar = &(*values)[i];
          if (seen.insert(scalar).second)
          {
            scalars_m.push_back({scalar, scalar->val});
          }
        }
      }
    }

    void initialize(quadra::ModelReportContext &context) { context.clear(); }

    template <typename Type>
    Type evaluate(const std::vector<Type> &parameters,
                  quadra::ModelReportContext &)
    {
      static_assert(std::is_same_v<Type, QUADRA_FIMS_TYPE>,
                    "FIMS Quadra adapter requires Quadra AD scalars");
      const size_t expected = info_m->fixed_effects_parameters.size() +
                              info_m->random_effects_parameters.size();
      if (parameters.size() != expected)
      {
        throw std::invalid_argument("FIMS Quadra parameter length mismatch");
      }
      for (const auto &snapshot : scalars_m)
      {
        *snapshot.scalar_m = Type(snapshot.value_m);
      }
      size_t index = 0;
      for (auto *parameter : info_m->fixed_effects_parameters)
      {
        *parameter = parameters[index++];
      }
      for (auto *parameter : info_m->random_effects_parameters)
      {
        *parameter = parameters[index++];
      }
      return fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance()->Evaluate();
    }

    template <typename Type>
    Type operator()(const std::vector<Type> &parameters)
    {
      quadra::ModelReportContext context;
      return evaluate<Type>(parameters, context);
    }
  };

  class FIMSFastDenseLaplaceObjective
  {
  public:
    FIMSLaplaceModelAdapter &model_m;
    quadra::ParameterVector &parameters_m;
    std::vector<int> fixed_m;
    std::vector<int> random_m;
    std::vector<double> random_mode_m;
    Eigen::MatrixXd hessian_m;
    double joint_m = 0.0;
    double logdet_m = 0.0;
    double constant_m = 0.0;
    double hessian_jitter_m = 0.0;
    Eigen::VectorXd logdet_gradient_m;
    bool pattern_discovered_m = false;
    quadra::laplace::BackendRecommendation backend_m;
    double pattern_relative_tolerance_m = 1e-10;
    int evaluations_m = 0;
    bool compute_logdet_gradient_m = true;

    struct ReplayWorkspace
    {
      FIMSLaplaceModelAdapter &model_m;
      std::vector<int> fixed_m;
      std::vector<int> random_m;
      had::ADGraph graph_m;
      std::vector<QUADRA_FIMS_TYPE> full_m;
      QUADRA_FIMS_TYPE objective_m;
      quadra::LaplaceGraphPlan plan_m;

      ReplayWorkspace(FIMSLaplaceModelAdapter &model,
                      const std::vector<int> &fixed,
                      const std::vector<int> &random,
                      const Eigen::VectorXd &theta,
                      const Eigen::VectorXd &random_values)
          : model_m(model), fixed_m(fixed), random_m(random)
      {
        had::g_ADGraph = &graph_m;
        full_m.reserve(fixed.size() + random.size());
        for (Eigen::Index i = 0; i < theta.size(); ++i)
          full_m.emplace_back(theta[i]);
        for (Eigen::Index i = 0; i < random_values.size(); ++i)
          full_m.emplace_back(random_values[i]);
        objective_m = model_m(full_m);
        std::vector<had::VertexId> fixed_vertices;
        std::vector<had::VertexId> random_vertices;
        for (size_t i = 0; i < fixed.size(); ++i)
          fixed_vertices.push_back(full_m[i].varId);
        for (size_t i = 0; i < random.size(); ++i)
          random_vertices.push_back(full_m[fixed.size() + i].varId);
        plan_m.Build(graph_m, fixed_vertices, random_vertices,
                     objective_m.varId);
      }

      quadra::FirstOrderJointEvaluation<FIMSLaplaceModelAdapter> evaluate(
          const Eigen::VectorXd &theta,
          const Eigen::VectorXd &random_values)
      {
        had::g_ADGraph = &graph_m;
        for (Eigen::Index i = 0; i < theta.size(); ++i)
          had::SetValue(full_m[static_cast<size_t>(i)], theta[i]);
        for (Eigen::Index i = 0; i < random_values.size(); ++i)
          had::SetValue(full_m[fixed_m.size() + static_cast<size_t>(i)],
                        random_values[i]);
        had::Forward(graph_m);
        quadra::PropagateFirstOrderRestricted(
            graph_m, objective_m.varId, plan_m.laplace_reverse_order());
        quadra::FirstOrderJointEvaluation<FIMSLaplaceModelAdapter> out;
        out.value = graph_m.vertices[objective_m.varId].primal;
        out.fixed_gradient.resize(static_cast<Eigen::Index>(fixed_m.size()));
        out.random_gradient.resize(static_cast<Eigen::Index>(random_m.size()));
        for (size_t i = 0; i < fixed_m.size(); ++i)
          out.fixed_gradient[static_cast<Eigen::Index>(i)] =
              had::GetAdjoint(full_m[i]);
        for (size_t i = 0; i < random_m.size(); ++i)
          out.random_gradient[static_cast<Eigen::Index>(i)] =
              had::GetAdjoint(full_m[fixed_m.size() + i]);
        return out;
      }
    };

    FIMSFastDenseLaplaceObjective(FIMSLaplaceModelAdapter &model,
                                  quadra::ParameterVector &parameters,
                                  const std::vector<double> &random_initial,
                                  bool compute_logdet_gradient = true)
        : model_m(model), parameters_m(parameters),
          fixed_m(quadra::build_fixed_index(parameters)),
          random_m(quadra::build_random_index(parameters)),
          random_mode_m(random_initial),
          compute_logdet_gradient_m(compute_logdet_gradient) {}

    void discover_pattern_once(const Eigen::MatrixXd &hessian)
    {
      if (pattern_discovered_m)
        return;
      const double scale = hessian.cwiseAbs().maxCoeff();
      quadra::laplace::StructureDetectorOptions options;
      options.structure_options.zero_tol =
          pattern_relative_tolerance_m * std::max(1.0, scale);
      options.prefer_dense_for_small_matrices = false;
      options.dense_size_cutoff = 0;
      quadra::laplace::StructureDetector detector(options);
      backend_m = detector.Analyze(hessian);
      pattern_discovered_m = true;
    }

    double cached_backend_logdet(const Eigen::MatrixXd &hessian) const
    {
      if (!pattern_discovered_m)
        throw std::logic_error("Hessian pattern has not been discovered");
      Eigen::SparseMatrix<double> sparse = hessian.sparseView(
          0.0, pattern_relative_tolerance_m *
                   std::max(1.0, hessian.cwiseAbs().maxCoeff()));
      auto backend = quadra::laplace::CreateLaplaceBackend(backend_m);
      backend->analyze_pattern(sparse);
      backend->factorize(sparse);
      if (!backend->is_spd() || !std::isfinite(backend->logdet()))
        throw std::runtime_error("Cached Hessian backend factorization failed");
      return backend->logdet();
    }

    double operator()(const Eigen::VectorXd &theta, Eigen::VectorXd &gradient)
    {
      ++evaluations_m;
      if (!compute_logdet_gradient_m)
      {
        had::ADGraph mode_graph;
        random_mode_m = quadra::solve_random_effects_laplace(
            model_m, parameters_m, theta, fixed_m, random_m, mode_graph,
            &random_mode_m);
        const Eigen::VectorXd random_rebuilt = Eigen::Map<Eigen::VectorXd>(
            random_mode_m.data(),
            static_cast<Eigen::Index>(random_mode_m.size()));
        const auto joint_rebuilt = quadra::evaluate_joint_first_order(
            model_m, parameters_m, theta, random_rebuilt, fixed_m, random_m);
        hessian_m = quadra::dense_random_hessian_from_gradient_fd(
            model_m, parameters_m, theta, random_rebuilt, fixed_m, random_m);
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> spectrum(
            hessian_m, Eigen::EigenvaluesOnly);
        const double minimum_eigenvalue = spectrum.eigenvalues().minCoeff();
        hessian_jitter_m = minimum_eigenvalue > 1e-8
                               ? 0.0
                               : -minimum_eigenvalue + 1e-8;
        if (hessian_jitter_m > 0.0)
          hessian_m.diagonal().array() += hessian_jitter_m;
        Eigen::LDLT<Eigen::MatrixXd> factor(hessian_m);
        discover_pattern_once(hessian_m);
        logdet_m = cached_backend_logdet(hessian_m);
        joint_m = joint_rebuilt.value;
        constant_m = 0.5 * static_cast<double>(random_m.size()) *
                     std::log(2.0 * M_PI);
        logdet_gradient_m = Eigen::VectorXd::Zero(theta.size());
        gradient = joint_rebuilt.fixed_gradient;
        return joint_m + 0.5 * logdet_m - constant_m;
      }
      Eigen::VectorXd random = Eigen::Map<Eigen::VectorXd>(
          random_mode_m.data(), static_cast<Eigen::Index>(random_mode_m.size()));
      ReplayWorkspace workspace(model_m, fixed_m, random_m, theta, random);
      Eigen::VectorXd best_random = random;
      double best_mode_value = std::numeric_limits<double>::infinity();
      auto mode_objective = [&](const Eigen::VectorXd &candidate,
                                Eigen::VectorXd &mode_gradient)
      {
        const auto evaluation = workspace.evaluate(theta, candidate);
        mode_gradient = evaluation.random_gradient;
        if (std::isfinite(evaluation.value) && mode_gradient.allFinite() &&
            evaluation.value < best_mode_value)
        {
          best_mode_value = evaluation.value;
          best_random = candidate;
        }
        return evaluation.value;
      };
      LBFGSpp::LBFGSParam<double> mode_options;
      mode_options.max_iterations = 200;
      mode_options.epsilon = 1e-8;
      mode_options.m = 10;
      mode_options.max_linesearch = 30;
      LBFGSpp::LBFGSSolver<double> mode_solver(mode_options);
      double mode_value = 0.0;
      try
      {
        mode_solver.minimize(mode_objective, random, mode_value);
      }
      catch (const std::exception &)
      {
      }
      if (!random.allFinite() || !std::isfinite(mode_value))
        random = best_random;
      const auto mode_check = workspace.evaluate(theta, random);
      if (!std::isfinite(mode_check.value) ||
          !mode_check.random_gradient.allFinite())
        random = best_random;
      random_mode_m.assign(random.data(), random.data() + random.size());
      const auto joint = workspace.evaluate(theta, random);
      hessian_m.resize(random.size(), random.size());
      for (Eigen::Index column = 0; column < random.size(); ++column)
      {
        const double step = 1e-5 * std::max(1.0, std::abs(random[column]));
        Eigen::VectorXd plus = random;
        Eigen::VectorXd minus = random;
        plus[column] += step;
        minus[column] -= step;
        const Eigen::VectorXd gplus =
            workspace.evaluate(theta, plus).random_gradient;
        const Eigen::VectorXd gminus =
            workspace.evaluate(theta, minus).random_gradient;
        hessian_m.col(column) = (gplus - gminus) / (2.0 * step);
      }
      hessian_m = 0.5 * (hessian_m + hessian_m.transpose());
      gradient = joint.fixed_gradient;
      if (!hessian_m.allFinite() || !std::isfinite(joint.value))
      {
        gradient = 1e3 * theta;
        return 1e100;
      }
      Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> spectrum(hessian_m,
                                                              Eigen::EigenvaluesOnly);
      if (spectrum.info() != Eigen::Success)
      {
        gradient = 1e3 * theta;
        return 1e100;
      }
      const double minimum_eigenvalue = spectrum.eigenvalues().minCoeff();
      hessian_jitter_m = minimum_eigenvalue > 1e-8
                             ? 0.0
                             : -minimum_eigenvalue + 1e-8;
      if (hessian_jitter_m > 0.0)
        hessian_m.diagonal().array() += hessian_jitter_m;
      Eigen::LDLT<Eigen::MatrixXd> factor(hessian_m);
      if (factor.info() != Eigen::Success)
        throw std::runtime_error("Dense random Hessian factorization failed");
      discover_pattern_once(hessian_m);
      logdet_m = cached_backend_logdet(hessian_m);
      joint_m = joint.value;
      constant_m = 0.5 * static_cast<double>(random_m.size()) *
                   std::log(2.0 * M_PI);

      if (!compute_logdet_gradient_m)
      {
        logdet_gradient_m = Eigen::VectorXd::Zero(theta.size());
        gradient = joint.fixed_gradient;
        return joint_m + 0.5 * logdet_m - constant_m;
      }

      // Complete derivative of logdet(H_uu(theta, uhat(theta))). First obtain
      // mode sensitivities from H_uu du/dtheta = -f_{u,theta}, then perturb
      // theta and u together. Curvature values use cheap first-order replays;
      // no global second/third-order edge sweep is required.
      Eigen::MatrixXd cross(random.size(), theta.size());
      Eigen::VectorXd theta_steps(theta.size());
      for (Eigen::Index column = 0; column < theta.size(); ++column)
      {
        const double step = 1e-4 * std::max(1.0, std::abs(theta[column]));
        theta_steps[column] = step;
        Eigen::VectorXd theta_plus = theta;
        theta_plus[column] += step;
        cross.col(column) =
            (workspace.evaluate(theta_plus, random).random_gradient -
             joint.random_gradient) /
            step;
      }
      const Eigen::MatrixXd mode_sensitivity = factor.solve(-cross);
      logdet_gradient_m = Eigen::VectorXd::Zero(theta.size());
      for (Eigen::Index direction = 0; direction < theta.size(); ++direction)
      {
        const double step = theta_steps[direction];
        Eigen::VectorXd theta_plus = theta;
        theta_plus[direction] += step;
        Eigen::VectorXd random_plus =
            random + step * mode_sensitivity.col(direction);
        const auto plus_center = workspace.evaluate(theta_plus, random_plus);
        Eigen::MatrixXd hessian_plus(random.size(), random.size());
        for (Eigen::Index column = 0; column < random.size(); ++column)
        {
          const double random_step =
              1e-5 * std::max(1.0, std::abs(random_plus[column]));
          Eigen::VectorXd shifted = random_plus;
          shifted[column] += random_step;
          hessian_plus.col(column) =
              (workspace.evaluate(theta_plus, shifted).random_gradient -
               plus_center.random_gradient) /
              random_step;
        }
        hessian_plus = 0.5 * (hessian_plus + hessian_plus.transpose());
        if (hessian_jitter_m > 0.0)
          hessian_plus.diagonal().array() += hessian_jitter_m;
        double plus_logdet = std::numeric_limits<double>::quiet_NaN();
        try
        {
          plus_logdet = cached_backend_logdet(hessian_plus);
        }
        catch (const std::exception &)
        {
          gradient = 1e3 * theta;
          return 1e100;
        }
        logdet_gradient_m[direction] = (plus_logdet - logdet_m) / step;
      }
      gradient = joint.fixed_gradient + 0.5 * logdet_gradient_m;
      return joint_m + 0.5 * logdet_m - constant_m;
    }
  };
} // namespace fims_quadra
#endif

/**
 * Initializes the logging system, setting all signal handling.
 */
void init_logging()
{
  std::signal(SIGSEGV, &fims::WriteAtExit);
  std::signal(SIGINT, &fims::WriteAtExit);
  std::signal(SIGABRT, &fims::WriteAtExit);
  std::signal(SIGFPE, &fims::WriteAtExit);
  std::signal(SIGILL, &fims::WriteAtExit);
  std::signal(SIGTERM, &fims::WriteAtExit);
}

/**
 * @brief Initialize and construct the FIMS model using TMB.
 *
 * @details
 * This function sets up the core C++ objects required for building the
 * objective function with TMB before optimizing a FIMS model. The main steps
 * of the function are as follows:
 * - The logging system is initialized and any existing model structures are
 *   cleared, ensuring a clean slate for a new model.
 * - It resets and prepares the main model information objects
 *   (fims_info::Information singletons), ensuring all internal data and
 *   settings are cleared and ready for a new model run. This step is essential
 *   for both initializing the model structure and avoiding conflicts from
 *   previous runs.
 * - It iterates over all registered FIMS interface objects and adds them to
 *   the TMB model context.
 * - After all of the objects are registered, it calls
 *   fims_info::Information::CreateModel() and
 *   fims_info::Information::CheckModel() on the base fims_info::Information
 *   object.
 * - It instantiates the singleton fims_model::Model object which represents
 *   the constructed TMB model.
 *
 * Typically the average user does not interact with this function because it
 * is called within <a href =
 * "https://noaa-fims.github.io/FIMS/reference/initialize_fims.html">`initialize_fims`</a>.
 *
 * @see init_logging()
 * @see fims_info::Information::Clear()
 * @see fims_info::Information::CreateModel()
 * @see fims_info::Information::CheckModel()
 * @see fims_info::Information::GetInstance()
 * @see <a href =
 * "https://noaa-fims.github.io/FIMS/reference/initialize_fims.html"
 * target="_blank">`initialize_fims()`</a>
 * @return A boolean is returned, where true indicates that the model was
 * successfully created.
 */
#ifdef TMB_MODEL
bool CreateTMBModel()
{

  init_logging();

  // clear first
  //  base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  info0->Clear();

  std::shared_ptr<fims_info::Information<TMBAD_FIMS_TYPE>> info =
      fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance();
  info->Clear();

  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
       i++)
  {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  // base model
  info0->CreateModel();
  info0->CheckModel();

  info->CreateModel();

  // instantiate the model? TODO: Ask Matthew what this does
  std::shared_ptr<fims_model::Model<TMB_FIMS_REAL_TYPE>> m0 =
      fims_model::Model<TMB_FIMS_REAL_TYPE>::GetInstance();
  return true;
}
#endif

#ifdef QUADRA_MODEL
/**
 * @brief Initialize and construct the FIMS model using Quadra.
 *
 * @details Rebuilds every registered FIMS interface object with Quadra's AD
 * scalar type. The TMB and double model singletons remain available in the
 * same shared library.
 *
 * @return True when the Quadra model was successfully constructed.
 */

bool CreateQuadraModel()
{
  bool valid = true;
#ifdef QUADRA_MODEL
  init_logging();
  fims_quadra::reset_tape();

  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> info =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  info->Clear();

  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
       ++i)
  {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  valid = info->CreateModel();
  info->CheckModel();
  fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance();
#endif

  return valid;
}

/**
 * @brief Evaluate the joint FIMS objective and gradient using Quadra.
 *
 * @param fixed_values Fixed-effect parameters in FIMS registration order.
 * @param random_values Random-effect parameters in FIMS registration order.
 * @return Joint objective, fixed/random gradients, and combined gradient.
 */
Rcpp::List EvaluateQuadraModel(Rcpp::NumericVector fixed_values,
                               Rcpp::NumericVector random_values)
{
  if (!CreateQuadraModel())
  {
    Rcpp::stop("Unable to construct the Quadra FIMS model.");
  }

  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  if (fixed_values.size() !=
      static_cast<R_xlen_t>(info->fixed_effects_parameters.size()))
  {
    Rcpp::stop("Quadra fixed parameter count does not match the FIMS model.");
  }
  if (random_values.size() !=
      static_cast<R_xlen_t>(info->random_effects_parameters.size()))
  {
    Rcpp::stop("Quadra random parameter count does not match the FIMS model.");
  }

  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
  {
    had::SetValue(*info->fixed_effects_parameters[i], fixed_values[i]);
  }
  for (R_xlen_t i = 0; i < random_values.size(); ++i)
  {
    had::SetValue(*info->random_effects_parameters[i], random_values[i]);
  }
  if (!info->fixed_effects_parameters.empty())
  {
    fims_quadra::benchmark_probe_vertex() =
        info->fixed_effects_parameters.front()->varId;
    fims_quadra::benchmark_probe_value() =
        info->fixed_effects_parameters.front()->val;
  }
  had::Forward();

  auto model = fims_model::Model<QUADRA_FIMS_TYPE>::GetInstance();
  QUADRA_FIMS_TYPE objective = model->Evaluate();
  fims_quadra::objective_vertex() = objective.varId;

  fims_quadra::propagate_gradient(objective.varId);

  Rcpp::NumericVector fixed_gradient(fixed_values.size());
  Rcpp::NumericVector random_gradient(random_values.size());
  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
  {
    fixed_gradient[i] = had::GetAdjoint(*info->fixed_effects_parameters[i]);
  }
  for (R_xlen_t i = 0; i < random_values.size(); ++i)
  {
    random_gradient[i] = had::GetAdjoint(*info->random_effects_parameters[i]);
  }

  // Time only replay of the already-recorded joint objective. Construction
  // and reverse/Hessian work are deliberately outside this hot-path metric.
  constexpr int timing_iterations = 25;
  volatile double replay_value = 0.0;
  const auto timing_start = std::chrono::steady_clock::now();
  for (int i = 0; i < timing_iterations; ++i)
  {
    had::Forward();
    replay_value = had::g_ADGraph->vertices[objective.varId].primal;
  }
  const auto timing_end = std::chrono::steady_clock::now();
  const double evaluation_time_seconds =
      std::chrono::duration<double>(timing_end - timing_start).count() /
      timing_iterations;
  (void)replay_value;

  Rcpp::NumericVector gradient(fixed_values.size() + random_values.size());
  std::copy(fixed_gradient.begin(), fixed_gradient.end(), gradient.begin());
  std::copy(random_gradient.begin(), random_gradient.end(),
            gradient.begin() + fixed_values.size());

  std::vector<had::VertexId> fixed_vertices;
  std::vector<had::VertexId> random_vertices;
  for (auto *parameter : info->fixed_effects_parameters)
    fixed_vertices.push_back(parameter->varId);
  for (auto *parameter : info->random_effects_parameters)
    random_vertices.push_back(parameter->varId);
  quadra::LaplaceGraphPlan graph_plan;
  graph_plan.Build(*had::g_ADGraph, fixed_vertices, random_vertices,
                   objective.varId);
  quadra::PropagateFirstOrderRestricted(
      *had::g_ADGraph, objective.varId, graph_plan.laplace_reverse_order());
  double restricted_gradient_max_difference = 0.0;
  for (size_t i = 0; i < info->fixed_effects_parameters.size(); ++i)
    restricted_gradient_max_difference = std::max(
        restricted_gradient_max_difference,
        std::abs(gradient[i] -
                 had::GetAdjoint(*info->fixed_effects_parameters[i])));
  for (size_t i = 0; i < info->random_effects_parameters.size(); ++i)
    restricted_gradient_max_difference = std::max(
        restricted_gradient_max_difference,
        std::abs(gradient[fixed_values.size() + i] -
                 had::GetAdjoint(*info->random_effects_parameters[i])));

  return Rcpp::List::create(
      Rcpp::Named("objective") = objective.val,
      Rcpp::Named("gradient") = gradient,
      Rcpp::Named("fixed_gradient") = fixed_gradient,
      Rcpp::Named("random_gradient") = random_gradient,
      Rcpp::Named("evaluation_time_seconds") = evaluation_time_seconds,
      Rcpp::Named("timing_iterations") = timing_iterations,
      Rcpp::Named("graph_plan") = Rcpp::List::create(
          Rcpp::Named("total_vertices") = graph_plan.vertex_count(),
          Rcpp::Named("random_active_vertices") =
              graph_plan.random_active_count(),
          Rcpp::Named("laplace_active_vertices") =
              graph_plan.laplace_active_count(),
          Rcpp::Named("restricted_gradient_max_difference") =
              restricted_gradient_max_difference));
}

/** Benchmark repeated forward replay of the most recently evaluated model. */
Rcpp::NumericVector BenchmarkQuadraModel(int iterations)
{
  if (iterations < 1)
  {
    Rcpp::stop("Quadra benchmark iterations must be positive.");
  }

  const double original_value = fims_quadra::benchmark_probe_value();
  QUADRA_FIMS_TYPE probe_parameter(
      original_value, fims_quadra::benchmark_probe_vertex());
  constexpr double perturbation = 1e-8;

  Rcpp::NumericVector timings(iterations);
  for (int i = 0; i < iterations; ++i)
  {
    had::SetValue(probe_parameter,
                  original_value + (i % 2 == 0 ? perturbation : -perturbation));
    const auto start = std::chrono::steady_clock::now();
    had::Forward();
    const auto end = std::chrono::steady_clock::now();
    timings[i] = std::chrono::duration<double>(end - start).count();
  }
  had::SetValue(probe_parameter, original_value);
  had::Forward();
  return timings;
}

/** Compare full and partition-restricted exact random Hessian propagation. */
Rcpp::List BenchmarkQuadraRestrictedHessian(
    Rcpp::NumericVector fixed_values, Rcpp::NumericVector random_values)
{
  EvaluateQuadraModel(fixed_values, random_values);
  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  std::vector<had::VertexId> fixed_vertices;
  std::vector<had::VertexId> random_vertices;
  for (auto *parameter : info->fixed_effects_parameters)
    fixed_vertices.push_back(parameter->varId);
  for (auto *parameter : info->random_effects_parameters)
    random_vertices.push_back(parameter->varId);
  quadra::LaplaceGraphPlan plan;
  plan.Build(*had::g_ADGraph, fixed_vertices, random_vertices,
             fims_quadra::objective_vertex());

  had::ZeroAdjoints(*had::g_ADGraph);
  had::g_ADGraph->vertices[fims_quadra::objective_vertex()].w = 1.0;
  const auto full_start = std::chrono::steady_clock::now();
  had::PropagateAdjoint();
  const double full_seconds = std::chrono::duration<double>(
                                  std::chrono::steady_clock::now() - full_start)
                                  .count();
  const size_t n = info->random_effects_parameters.size();
  Eigen::MatrixXd full(n, n);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < n; ++j)
      full(i, j) = had::GetAdjoint(*info->random_effects_parameters[i],
                                   *info->random_effects_parameters[j]);
  const size_t p = info->fixed_effects_parameters.size();
  Eigen::MatrixXd full_mixed(n, p);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < p; ++j)
      full_mixed(i, j) = had::GetAdjoint(
          *info->random_effects_parameters[i],
          *info->fixed_effects_parameters[j]);

  const auto restricted_start = std::chrono::steady_clock::now();
  quadra::PropagateRandomHessianRestricted(*had::g_ADGraph, plan);
  const double restricted_seconds = std::chrono::duration<double>(
                                        std::chrono::steady_clock::now() - restricted_start)
                                        .count();
  Eigen::MatrixXd restricted(n, n);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < n; ++j)
      restricted(i, j) = had::GetAdjoint(
          *info->random_effects_parameters[i],
          *info->random_effects_parameters[j]);

  const auto mixed_start = std::chrono::steady_clock::now();
  quadra::PropagateLaplaceHessianRestricted(*had::g_ADGraph, plan);
  const double mixed_seconds = std::chrono::duration<double>(
                                   std::chrono::steady_clock::now() - mixed_start)
                                   .count();
  Eigen::MatrixXd restricted_mixed(n, p);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = 0; j < p; ++j)
      restricted_mixed(i, j) = had::GetAdjoint(
          *info->random_effects_parameters[i],
          *info->fixed_effects_parameters[j]);

  return Rcpp::List::create(
      Rcpp::Named("full_seconds") = full_seconds,
      Rcpp::Named("restricted_seconds") = restricted_seconds,
      Rcpp::Named("speedup") = full_seconds / restricted_seconds,
      Rcpp::Named("maximum_absolute_difference") =
          (full - restricted).cwiseAbs().maxCoeff(),
      Rcpp::Named("maximum_absolute_hessian") = full.cwiseAbs().maxCoeff(),
      Rcpp::Named("mixed_restricted_seconds") = mixed_seconds,
      Rcpp::Named("mixed_speedup") = full_seconds / mixed_seconds,
      Rcpp::Named("mixed_maximum_absolute_difference") =
          (full_mixed - restricted_mixed).cwiseAbs().maxCoeff(),
      Rcpp::Named("maximum_absolute_mixed_hessian") =
          full_mixed.cwiseAbs().maxCoeff(),
      Rcpp::Named("total_vertices") = plan.vertex_count(),
      Rcpp::Named("random_active_vertices") = plan.random_active_count(),
      Rcpp::Named("laplace_active_vertices") = plan.laplace_active_count());
}

/** Fit the FIMS joint objective without Laplace profiling. */
Rcpp::List fit_fims_quadra_joint(Rcpp::NumericVector fixed_values,
                                 Rcpp::NumericVector random_values,
                                 int max_iterations,
                                 double gradient_tolerance)
{
  if (max_iterations < 1)
    Rcpp::stop("max_iterations must be positive.");
  if (!(gradient_tolerance > 0.0))
  {
    Rcpp::stop("gradient_tolerance must be positive.");
  }

  Rcpp::List initial = EvaluateQuadraModel(fixed_values, random_values);
  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();

  std::vector<QUADRA_FIMS_TYPE *> parameters;
  parameters.reserve(info->fixed_effects_parameters.size() +
                     info->random_effects_parameters.size());
  parameters.insert(parameters.end(), info->fixed_effects_parameters.begin(),
                    info->fixed_effects_parameters.end());
  parameters.insert(parameters.end(), info->random_effects_parameters.begin(),
                    info->random_effects_parameters.end());

  Eigen::VectorXd values(static_cast<Eigen::Index>(parameters.size()));
  for (Eigen::Index i = 0; i < values.size(); ++i)
  {
    values[i] = parameters[static_cast<size_t>(i)]->val;
  }
  const had::VertexId objective_vertex = fims_quadra::objective_vertex();
  fims_quadra::FIMSJointLBFGSObjective objective(parameters, objective_vertex);

  Eigen::VectorXd compact_initial_gradient;
  const double compact_initial_objective =
      objective(values, compact_initial_gradient);
  const double initial_objective = Rcpp::as<double>(initial["objective"]);
  const Rcpp::NumericVector legacy_initial_gradient = initial["gradient"];
  double validation_gradient_difference = 0.0;
  for (Eigen::Index i = 0; i < compact_initial_gradient.size(); ++i)
  {
    validation_gradient_difference = std::max(
        validation_gradient_difference,
        std::abs(compact_initial_gradient[i] - legacy_initial_gradient[i]));
  }
  const double validation_objective_difference =
      std::abs(compact_initial_objective - initial_objective);
  const double objective_tolerance =
      1e-10 * std::max(1.0, std::abs(initial_objective));
  double gradient_scale = 1.0;
  for (const double value : legacy_initial_gradient)
  {
    gradient_scale = std::max(gradient_scale, std::abs(value));
  }
  const double gradient_validation_tolerance = 1e-10 * gradient_scale;
  if (validation_objective_difference > objective_tolerance ||
      validation_gradient_difference > gradient_validation_tolerance)
  {
    Rcpp::stop(
        "Quadra compact tape validation failed (objective difference %.3g, "
        "gradient difference %.3g).",
        validation_objective_difference, validation_gradient_difference);
  }
  objective.evaluations = 0;

  // Joint L-BFGS now owns everything it needs in the compact tape. Release
  // the Hessian-capable graph before optimization; a later Quadra entry point
  // rebuilds it through CreateQuadraModel().
  fims_quadra::release_tape();

  LBFGSpp::LBFGSParam<double> options;
  options.max_iterations = max_iterations;
  options.epsilon = gradient_tolerance;
  options.m = 20;
  options.max_linesearch = 50;
  LBFGSpp::LBFGSSolver<double> optimizer(options);

  double final_objective = Rcpp::as<double>(initial["objective"]);
  int iterations = 0;
  bool converged = false;
  std::string message = "maximum iterations reached";
  const auto start = std::chrono::steady_clock::now();
  try
  {
    iterations = optimizer.minimize(objective, values, final_objective);
    converged = true;
    message = "L-BFGS converged";
  }
  catch (const std::exception &error)
  {
    message = error.what();
  }
  const double elapsed_seconds = std::chrono::duration<double>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();

  Eigen::VectorXd final_gradient;
  final_objective = objective(values, final_gradient);
  const double gradient_norm = final_gradient.norm();
  converged = converged || gradient_norm <= gradient_tolerance;

  Rcpp::NumericVector fixed_estimates(fixed_values.size());
  Rcpp::NumericVector random_estimates(random_values.size());
  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
  {
    fixed_estimates[i] = values[i];
  }
  for (R_xlen_t i = 0; i < random_values.size(); ++i)
  {
    random_estimates[i] = values[fixed_values.size() + i];
  }

  return Rcpp::List::create(
      Rcpp::Named("par") = fixed_estimates,
      Rcpp::Named("random") = random_estimates,
      Rcpp::Named("objective") = final_objective,
      Rcpp::Named("gradient") = Rcpp::NumericVector(
          final_gradient.data(), final_gradient.data() + final_gradient.size()),
      Rcpp::Named("gradient_norm") = gradient_norm,
      Rcpp::Named("iterations") = iterations,
      Rcpp::Named("evaluations") = objective.evaluations,
      Rcpp::Named("converged") = converged,
      Rcpp::Named("message") = message,
      Rcpp::Named("elapsed_seconds") = elapsed_seconds,
      Rcpp::Named("initial_objective") = initial["objective"],
      Rcpp::Named("compact_tape_vertices") = objective.tape.VertexCount(),
      Rcpp::Named("compact_tape_bytes") = objective.tape.Bytes(),
      Rcpp::Named("full_graph_released") = true,
      Rcpp::Named("compact_validation_objective_difference") =
          validation_objective_difference,
      Rcpp::Named("compact_validation_gradient_max_difference") =
          validation_gradient_difference);
}

/** Fit the Laplace-profiled FIMS objective with Quadra's model-aware path. */
Rcpp::List EvaluateQuadraLaplaceModel(Rcpp::NumericVector fixed_values,
                                      Rcpp::NumericVector random_values)
{
  if (!CreateQuadraModel())
  {
    Rcpp::stop("Unable to construct the Quadra FIMS model.");
  }
  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  std::vector<double> theta(fixed_values.begin(), fixed_values.end());
  std::vector<double> random(random_values.begin(), random_values.end());
  if (theta.size() != info->fixed_effects_parameters.size() ||
      random.size() != info->random_effects_parameters.size())
  {
    Rcpp::stop("Quadra parameter count does not match the FIMS model.");
  }
  quadra::ParameterPartition partition;
  partition.fixed_indices_m.resize(theta.size());
  std::iota(partition.fixed_indices_m.begin(),
            partition.fixed_indices_m.end(), size_t{0});
  partition.random_indices_m.resize(random.size());
  std::iota(partition.random_indices_m.begin(),
            partition.random_indices_m.end(), theta.size());
  fims_quadra::FIMSLaplaceModelAdapter model(info);
  quadra::LaplaceObjectiveOptions options;
  options.newton_m.max_iterations_m = 50;
  options.newton_m.gradient_tolerance_m = 1e-8;
  // Near the random-effect mode the Armijo decrease can fall below double
  // precision before the unscaled gradient reaches its absolute tolerance.
  options.newton_m.step_tolerance_m = 1e-6;
  const auto start = std::chrono::steady_clock::now();
  auto result = quadra::evaluate_laplace_objective(
      model, theta, random, partition, options);
  const double elapsed_seconds = std::chrono::duration<double>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();
  quadra::laplace::StructureDetector detector;
  const auto backend = detector.Analyze(result.hessian_random_m);
  return Rcpp::List::create(
      Rcpp::Named("objective") = result.laplace_objective_m,
      Rcpp::Named("joint_objective") = result.joint_objective_m,
      Rcpp::Named("random") = Rcpp::wrap(result.u_hat_m),
      Rcpp::Named("random_gradient") = Rcpp::wrap(result.gradient_random_m),
      Rcpp::Named("random_gradient_norm") = result.gradient_norm_random_m,
      Rcpp::Named("random_newton_iterations") = result.newton_iterations_m,
      Rcpp::Named("random_step_norm") = result.random_step_norm_m,
      Rcpp::Named("converged") = result.converged_m,
      Rcpp::Named("message") = result.message_m,
      Rcpp::Named("logdet_ok") = result.logdet_ok_m,
      Rcpp::Named("elapsed_seconds") = elapsed_seconds,
      Rcpp::Named("factorization") = Rcpp::List::create(
          Rcpp::Named("backend") = quadra::laplace::ToString(backend.backend),
          Rcpp::Named("structure") =
              quadra::laplace::ToString(backend.structure),
          Rcpp::Named("random_size") = backend.random_size,
          Rcpp::Named("nnz") = backend.nnz,
          Rcpp::Named("bandwidth") = backend.bandwidth,
          Rcpp::Named("fill_ratio") = backend.fill_ratio,
          Rcpp::Named("reason") = backend.reason));
}

/** Diagnose numerical sparsity of the dense random-effect Hessian. */
/** Diagnose numerical sparsity of the dense random-effect Hessian. */
Rcpp::List EvaluateQuadraDenseHessian(Rcpp::NumericVector fixed_values,
                                      Rcpp::NumericVector random_values)
{
  if (!CreateQuadraModel())
    Rcpp::stop("Unable to construct Quadra model.");
  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  fims_quadra::FIMSLaplaceModelAdapter model(info);
  quadra::ParameterVector parameters;
  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
    parameters.add(quadra::Parameter("fixed_" + std::to_string(i + 1),
                                     fixed_values[i]));
  for (R_xlen_t i = 0; i < random_values.size(); ++i)
    parameters.add(quadra::Parameter(
        "random_" + std::to_string(i + 1), random_values[i],
        quadra::ParameterTransform::Identity, true, 0));
  fims_quadra::FIMSFastDenseLaplaceObjective objective(
      model, parameters,
      std::vector<double>(random_values.begin(), random_values.end()), false);
  Eigen::VectorXd theta(fixed_values.size());
  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
    theta[i] = fixed_values[i];
  Eigen::VectorXd gradient;
  const auto start = std::chrono::steady_clock::now();
  const double laplace_objective = objective(theta, gradient);
  const double elapsed = std::chrono::duration<double>(
                             std::chrono::steady_clock::now() - start)
                             .count();
  const Eigen::MatrixXd &hessian = objective.hessian_m;
  const double scale = hessian.cwiseAbs().maxCoeff();
  const std::vector<double> relative_tolerances =
      {1e-14, 1e-12, 1e-10, 1e-8, 1e-6, 1e-4};
  Rcpp::List summaries(relative_tolerances.size());
  for (size_t k = 0; k < relative_tolerances.size(); ++k)
  {
    const double threshold = relative_tolerances[k] * scale;
    int nnz = 0;
    int bandwidth = 0;
    double max_outside_tridiagonal = 0.0;
    for (Eigen::Index i = 0; i < hessian.rows(); ++i)
    {
      for (Eigen::Index j = 0; j < hessian.cols(); ++j)
      {
        const double magnitude = std::abs(hessian(i, j));
        if (magnitude > threshold)
        {
          ++nnz;
          bandwidth = std::max(bandwidth,
                               static_cast<int>(std::abs(i - j)));
        }
        if (std::abs(i - j) > 1)
          max_outside_tridiagonal =
              std::max(max_outside_tridiagonal, magnitude);
      }
    }
    summaries[k] = Rcpp::List::create(
        Rcpp::Named("relative_tolerance") = relative_tolerances[k],
        Rcpp::Named("absolute_threshold") = threshold,
        Rcpp::Named("nnz") = nnz,
        Rcpp::Named("fill_ratio") =
            static_cast<double>(nnz) / static_cast<double>(hessian.size()),
        Rcpp::Named("bandwidth") = bandwidth,
        Rcpp::Named("max_outside_tridiagonal") =
            max_outside_tridiagonal);
  }
  Rcpp::NumericMatrix hessian_r(hessian.rows(), hessian.cols());
  for (Eigen::Index j = 0; j < hessian.cols(); ++j)
    for (Eigen::Index i = 0; i < hessian.rows(); ++i)
      hessian_r(i, j) = hessian(i, j);
  return Rcpp::List::create(
      Rcpp::Named("hessian") = hessian_r,
      Rcpp::Named("random_mode") = Rcpp::wrap(objective.random_mode_m),
      Rcpp::Named("objective") = laplace_objective,
      Rcpp::Named("maximum_absolute_value") = scale,
      Rcpp::Named("summaries") = summaries,
      Rcpp::Named("discovery") = Rcpp::List::create(
          Rcpp::Named("backend") =
              quadra::laplace::ToString(objective.backend_m.backend),
          Rcpp::Named("structure") =
              quadra::laplace::ToString(objective.backend_m.structure),
          Rcpp::Named("nnz") = objective.backend_m.nnz,
          Rcpp::Named("fill_ratio") = objective.backend_m.fill_ratio,
          Rcpp::Named("bandwidth") = objective.backend_m.bandwidth,
          Rcpp::Named("reason") = objective.backend_m.reason,
          Rcpp::Named("relative_tolerance") =
              objective.pattern_relative_tolerance_m),
      Rcpp::Named("elapsed_seconds") = elapsed);
}

/** Fit the Laplace-profiled FIMS objective with Quadra's model-aware path. */
Rcpp::List fit_fims_quadra_finite_difference(
    Rcpp::NumericVector fixed_values, Rcpp::NumericVector random_values,
    int max_iterations, double gradient_tolerance)
{
  if (max_iterations < 1)
    Rcpp::stop("max_iterations must be positive.");
  if (!(gradient_tolerance > 0.0))
  {
    Rcpp::stop("gradient_tolerance must be positive.");
  }
  if (!CreateQuadraModel())
  {
    Rcpp::stop("Unable to construct the Quadra FIMS model.");
  }
  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  if (fixed_values.size() !=
          static_cast<R_xlen_t>(info->fixed_effects_parameters.size()) ||
      random_values.size() !=
          static_cast<R_xlen_t>(info->random_effects_parameters.size()))
  {
    Rcpp::stop("Quadra parameter count does not match the FIMS model.");
  }

  std::vector<double> theta(fixed_values.begin(), fixed_values.end());
  std::vector<double> random(random_values.begin(), random_values.end());
  quadra::ParameterPartition partition;
  partition.fixed_indices_m.resize(theta.size());
  std::iota(partition.fixed_indices_m.begin(),
            partition.fixed_indices_m.end(), size_t{0});
  partition.random_indices_m.resize(random.size());
  std::iota(partition.random_indices_m.begin(),
            partition.random_indices_m.end(), theta.size());

  fims_quadra::FIMSLaplaceModelAdapter model(info);
  quadra::LaplaceLBFGSOptions options;
  options.max_iterations_m = max_iterations;
  options.gradient_tolerance_m = gradient_tolerance;
  options.memory_m = 20;
  options.gradient_m.use_central_difference_m = true;
  options.gradient_m.objective_m.newton_m.max_iterations_m = 50;
  options.gradient_m.objective_m.newton_m.gradient_tolerance_m = 1e-8;

  const auto start = std::chrono::steady_clock::now();
  quadra::LaplaceLBFGSResult result =
      quadra::optimize_laplace_fixed_effects_lbfgs(
          model, theta, random, partition, options);
  const double elapsed_seconds = std::chrono::duration<double>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();

  quadra::laplace::BackendRecommendation backend;
  const auto &hessian =
      result.gradient_result_m.objective_result_m.hessian_random_m;
  if (hessian.rows() > 0)
  {
    quadra::laplace::StructureDetector detector;
    backend = detector.Analyze(hessian);
  }

  return Rcpp::List::create(
      Rcpp::Named("par") = Rcpp::wrap(result.theta_hat_m),
      Rcpp::Named("random") = Rcpp::wrap(result.u_hat_m),
      Rcpp::Named("objective") = result.laplace_objective_m,
      Rcpp::Named("gradient") = Rcpp::wrap(result.gradient_fixed_m),
      Rcpp::Named("gradient_norm") = result.gradient_norm_m,
      Rcpp::Named("iterations") = result.iterations_m,
      Rcpp::Named("converged") = result.converged_m,
      Rcpp::Named("logdet_ok") = result.logdet_ok_m,
      Rcpp::Named("message") = result.message_m,
      Rcpp::Named("elapsed_seconds") = elapsed_seconds,
      Rcpp::Named("random_newton_iterations") =
          result.gradient_result_m.objective_result_m.newton_iterations_m,
      Rcpp::Named("random_gradient_norm") =
          result.gradient_result_m.objective_result_m.gradient_norm_random_m,
      Rcpp::Named("factorization") = Rcpp::List::create(
          Rcpp::Named("backend") =
              quadra::laplace::ToString(backend.backend),
          Rcpp::Named("structure") =
              quadra::laplace::ToString(backend.structure),
          Rcpp::Named("random_size") = backend.random_size,
          Rcpp::Named("nnz") = backend.nnz,
          Rcpp::Named("bandwidth") = backend.bandwidth,
          Rcpp::Named("fill_ratio") = backend.fill_ratio,
          Rcpp::Named("reason") = backend.reason));
}

/** Persistent, exact-gradient, structure-aware Quadra Laplace fit. */
Rcpp::List fit_fims_quadra(Rcpp::NumericVector fixed_values,
                           Rcpp::NumericVector random_values,
                           int max_iterations, double gradient_tolerance)
{
  if (max_iterations < 1)
    Rcpp::stop("max_iterations must be positive.");
  if (!(gradient_tolerance > 0.0))
  {
    Rcpp::stop("gradient_tolerance must be positive.");
  }
  if (!CreateQuadraModel())
  {
    Rcpp::stop("Unable to construct the Quadra FIMS model.");
  }
  auto info = fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  if (fixed_values.size() !=
          static_cast<R_xlen_t>(info->fixed_effects_parameters.size()) ||
      random_values.size() !=
          static_cast<R_xlen_t>(info->random_effects_parameters.size()))
  {
    Rcpp::stop("Quadra parameter count does not match the FIMS model.");
  }
  fims_quadra::FIMSLaplaceModelAdapter model(info);
  quadra::ParameterVector parameters;
  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
  {
    parameters.add(quadra::Parameter("fixed_" + std::to_string(i + 1),
                                     fixed_values[i]));
  }
  for (R_xlen_t i = 0; i < random_values.size(); ++i)
  {
    parameters.add(quadra::Parameter("random_" + std::to_string(i + 1),
                                     random_values[i],
                                     quadra::ParameterTransform::Identity,
                                     true, 0));
  }
  fims_quadra::FIMSFastDenseLaplaceObjective objective(
      model, parameters,
      std::vector<double>(random_values.begin(), random_values.end()));
  Eigen::VectorXd theta(fixed_values.size());
  for (R_xlen_t i = 0; i < fixed_values.size(); ++i)
    theta[i] = fixed_values[i];
  LBFGSpp::LBFGSParam<double> optimizer_options;
  optimizer_options.max_iterations = max_iterations;
  optimizer_options.epsilon = gradient_tolerance;
  optimizer_options.m = 20;
  optimizer_options.max_linesearch = 15;
  optimizer_options.max_step = 0.1;
  LBFGSpp::LBFGSSolver<double> optimizer(optimizer_options);
  const auto start = std::chrono::steady_clock::now();
  double final_objective = std::numeric_limits<double>::quiet_NaN();
  int iterations = 0;
  bool converged = false;
  std::string message = "maximum iterations reached";
  try
  {
    iterations = optimizer.minimize(objective, theta, final_objective);
    message = "dense Laplace L-BFGS stopped";
  }
  catch (const std::exception &error)
  {
    message = error.what();
  }
  Eigen::VectorXd final_gradient;
  final_objective = objective(theta, final_gradient);
  const double gradient_norm = final_gradient.norm();
  converged = gradient_norm <= gradient_tolerance;
  if (converged)
    message = "dense Laplace L-BFGS converged";
  const double elapsed_seconds = std::chrono::duration<double>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();
  return Rcpp::List::create(
      Rcpp::Named("par") = Rcpp::NumericVector(theta.data(),
                                               theta.data() + theta.size()),
      Rcpp::Named("random") = Rcpp::wrap(objective.random_mode_m),
      Rcpp::Named("objective") = final_objective,
      Rcpp::Named("joint_objective") = objective.joint_m,
      Rcpp::Named("laplace_logdet") = objective.logdet_m,
      Rcpp::Named("laplace_constant") = objective.constant_m,
      Rcpp::Named("gradient") = Rcpp::NumericVector(
          final_gradient.data(), final_gradient.data() + final_gradient.size()),
      Rcpp::Named("gradient_norm") = gradient_norm,
      Rcpp::Named("iterations") = iterations,
      Rcpp::Named("evaluations") = objective.evaluations_m,
      Rcpp::Named("converged") = converged,
      Rcpp::Named("message") = message,
      Rcpp::Named("elapsed_seconds") = elapsed_seconds,
      Rcpp::Named("gradient_mode") = "complete_profiled_dense",
      Rcpp::Named("logdet_gradient") = Rcpp::NumericVector(
          objective.logdet_gradient_m.data(),
          objective.logdet_gradient_m.data() +
              objective.logdet_gradient_m.size()),
      Rcpp::Named("hessian_jitter") = objective.hessian_jitter_m,
      Rcpp::Named("factorization") = Rcpp::List::create(
          Rcpp::Named("available") = objective.pattern_discovered_m,
          Rcpp::Named("backend") =
              quadra::laplace::ToString(objective.backend_m.backend),
          Rcpp::Named("structure") =
              quadra::laplace::ToString(objective.backend_m.structure),
          Rcpp::Named("reason") = objective.backend_m.reason,
          Rcpp::Named("random_size") = objective.backend_m.random_size,
          Rcpp::Named("nnz") = objective.backend_m.nnz,
          Rcpp::Named("fill_ratio") = objective.backend_m.fill_ratio,
          Rcpp::Named("bandwidth") = objective.backend_m.bandwidth,
          Rcpp::Named("relative_tolerance") =
              objective.pattern_relative_tolerance_m,
          Rcpp::Named("pattern_reused") = objective.evaluations_m > 1));
}
#endif

/* Dictionary block for shared documentation.
  [details_set_x_parameters]
  Updates the internal parameter values for the model base of type
  TMB_FIMS_REAL_TYPE. It is typically called before finalize() or
  @ref CatchAtAgeInterface::to_json "`get_output()`" to ensure the correct
  values are used because TMB doesn't always keep the updated parameters in
  the "double" version of the tape. So we need to update those first.
  \n\n
  Usage example in R:
  \code{.R}
  set_fixed_parameters(c(1, 2, 3))
  set_random_parameters(c(1, 2, 3))
  catch_at_age$get_output()
  \endcode
  [details_set_x_parameters]
*/
/* Dictionary block for shared documentation.
  [param_par]
  @param par A vector of parameter values.
  [param_par]
 */

/**
 * @brief Update fixed parameters in the tape, so the output is correct.
 * @details @snippet{doc} this details_set_x_parameters
 * @snippet{doc} this param_par
 * @see set_random_parameters()
 */
void set_fixed_parameters(Rcpp::NumericVector par)
{
#ifdef TMB_MODEL
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++)
  {
    *info0->fixed_effects_parameters[i] = par[i];
  }
#endif
#ifdef QUADRA_MODEL
  // base model
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> quadra_info =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  for (size_t i = 0; i < quadra_info->fixed_effects_parameters.size(); i++)
  {
    *quadra_info->fixed_effects_parameters[i] = par[i];
  }
#endif
}

/**
 * @brief Gets the fixed parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_fixed_parameters_vector()
{
#ifdef TMB_MODEL
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++)
  {
    p.push_back(*info0->fixed_effects_parameters[i]);
  }

  return p;
#elif defined(QUADRA_MODEL)
  // base model
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> info0 =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();

  Rcpp::NumericVector p_quadra;

  for (size_t i = 0; i < info0->fixed_effects_parameters.size(); i++)
  {
    p_quadra.push_back(quadra::value_of(*info0->fixed_effects_parameters[i]));
  }

  return p_quadra;
#endif
}

/**
 * @brief Update random effect parameters in the tape, so the output is correct.
 * @details @snippet{doc} this details_set_x_parameters
 * @snippet{doc} this param_par
 * @see set_fixed_parameters()
 */
void set_random_parameters(Rcpp::NumericVector par)
{
#ifdef TMB_MODEL
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> info0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  for (size_t i = 0; i < info0->random_effects_parameters.size(); i++)
  {
    *info0->random_effects_parameters[i] = par[i];
  }
#endif
#ifdef QUADRA_MODEL
  // base model
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> quadra_info =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();

  for (size_t i = 0; i < quadra_info->random_effects_parameters.size(); i++)
  {
    *quadra_info->random_effects_parameters[i] = par[i];
  }
#endif
}

/**
 * @brief Gets the random parameters vector object.
 *
 * @return Rcpp::NumericVector
 */
Rcpp::NumericVector get_random_parameters_vector()
{
#ifdef TMB_MODEL
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->random_effects_parameters.size(); i++)
  {
    p.push_back(*d0->random_effects_parameters[i]);
  }

  return p;
#elif defined(QUADRA_MODEL)
  // base model
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> d0 =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (size_t i = 0; i < d0->random_effects_parameters.size(); i++)
  {
    p.push_back(quadra::value_of(*d0->random_effects_parameters[i]));
  }

  return p;
#endif
}

/**
 * @brief Gets the parameter names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_parameter_names(Rcpp::List pars)
{
#ifdef TMB_MODEL
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->parameter_names;
#elif defined(QUADRA_MODEL)
  // base model
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> d0 =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  pars.attr("names") = d0->parameter_names;
#endif
  return pars;
}

/**
 * @brief Gets the random effects names object.
 *
 * @param pars
 * @return Rcpp::List
 */
Rcpp::List get_random_names(Rcpp::List pars)
{
#ifdef TMB_MODEL
  // base model
  std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  pars.attr("names") = d0->random_effects_names;
#elif defined(QUADRA_MODEL)
  std::shared_ptr<fims_info::Information<QUADRA_FIMS_TYPE>> d0 =
      fims_info::Information<QUADRA_FIMS_TYPE>::GetInstance();
  pars.attr("names") = d0->random_effects_names;
#endif

  return pars;
}

/**
 * @brief Clears the internal objects.
 *
 * @tparam Type
 */
template <typename Type>
void clear_internal()
{
  std::shared_ptr<fims_info::Information<Type>> d0 =
      fims_info::Information<Type>::GetInstance();
  d0->Clear();
}

/**
 * @brief Clears all FIMS pointers and resets model state.
 * @param get_error_msg If true, retains a lingering pointer to trigger
 * dangling pointer diagnostics. Should only be set to true via
 * test_clear_with_leak_check().
 */
void clear_impl(bool get_error_msg)
{
  // rcpp_interface_base.hpp
  FIMSRcppInterfaceBase::fims_interface_objects.clear();

  // Variable and VariableVector
  Variable::id_g = 1;
  VariableVector::id_g = 1;
  // rcpp_data.hpp
  DataInterfaceBase::id_g = 1;
  DataInterfaceBase::live_objects.clear();

  AgeCompDataInterface::id_g = 1;
  AgeCompDataInterface::live_objects.clear();

  LengthCompDataInterface::id_g = 1;
  LengthCompDataInterface::live_objects.clear();

  LandingsDataInterface::id_g = 1;
  LandingsDataInterface::live_objects.clear();

  IndexDataInterface::id_g = 1;
  IndexDataInterface::live_objects.clear();

  // rcpp_fleets.hpp
  FleetInterfaceBase::id_g = 1;
  FleetInterfaceBase::live_objects.clear();

  FleetInterface::id_g = 1;
  FleetInterface::live_objects.clear();

  // rcpp_growth.hpp
  GrowthInterfaceBase::id_g = 1;
  GrowthInterfaceBase::live_objects.clear();

  EWAAGrowthInterface::id_g = 1;
  EWAAGrowthInterface::live_objects.clear();

  // rcpp_maturity.hpp
  MaturityInterfaceBase::id_g = 1;
  MaturityInterfaceBase::live_objects.clear();

  LogisticMaturityInterface::id_g = 1;
  LogisticMaturityInterface::live_objects.clear();

  // rcpp_population.hpp
  PopulationInterfaceBase::id_g = 1;
  PopulationInterfaceBase::live_objects.clear();

  PopulationInterface::id_g = 1;
  PopulationInterface::live_objects.clear();

  // rcpp_recruitment.hpp
  RecruitmentInterfaceBase::id_g = 1;
  RecruitmentInterfaceBase::live_objects.clear();

  BevertonHoltRecruitmentInterface::id_g = 1;
  BevertonHoltRecruitmentInterface::live_objects.clear();

  // rcpp_selectivity.hpp
  SelectivityInterfaceBase::id_g = 1;
  SelectivityInterfaceBase::live_objects.clear();

  LogisticSelectivityInterface::id_g = 1;
  LogisticSelectivityInterface::live_objects.clear();

  DoubleLogisticSelectivityInterface::id_g = 1;
  DoubleLogisticSelectivityInterface::live_objects.clear();

  // rcpp_distribution.hpp
  DistributionsInterfaceBase::id_g = 1;
  DistributionsInterfaceBase::live_objects.clear();

  DnormDistributionsInterface::id_g = 1;
  DnormDistributionsInterface::live_objects.clear();

  DlnormDistributionsInterface::id_g = 1;
  DlnormDistributionsInterface::live_objects.clear();

  DmultinomDistributionsInterface::id_g = 1;
  DmultinomDistributionsInterface::live_objects.clear();

  FisheryModelInterfaceBase::id_g = 1;
  FisheryModelInterfaceBase::live_objects.clear();
#ifdef TMB_MODEL
  clear_internal<TMB_FIMS_REAL_TYPE>();
  clear_internal<TMBAD_FIMS_TYPE>();
#endif
#ifdef QUADRA_MODEL
  clear_internal<QUADRA_FIMS_TYPE>();
#endif

  fims::FIMSLog::fims_log->clear();

  std::unique_ptr<fims_popdy::LogisticSelectivity<double>> test_obj;
  if (get_error_msg)
  {
    test_obj = std::make_unique<fims_popdy::LogisticSelectivity<double>>();
  }

  // --- AUTOMATED DANGLING POINTER DIAGNOSTIC PRINT ---
  if (fims_model_object::FIMSMemoryTracker::total_active_objects > 0)
  {
    std::ostringstream msg;
    msg << "\n⚠️  WARNING: FIMS Dangling Pointer or Module Detected after "
           "clear()!\n";
    msg << "--------------------------------------------------\n";
    msg << "A total of "
        << fims_model_object::FIMSMemoryTracker::total_active_objects
        << " pointer(s) NOT cleared\n";
    msg << "--------------------------------------------------\n";
    msg << "Ensure all pointers are being reset.\n\n";

    Rcpp::warning(msg.str());
  }
}

/**
 * @brief Clears all FIMS pointers and resets model state.
 */
void clear() { clear_impl(false); }

/**
 * @brief Test-only variant of clear() that retains a lingering pointer
 * to validate dangling pointer diagnostics.
 * @note Not exposed to users. Use only in tests.
 */
void test_clear_with_leak_check() { clear_impl(true); }

/**
 * @brief Gets the log entries as a string in JSON format.
 */
std::string get_log() { return fims::FIMSLog::fims_log->get_log(); }

/**
 * @brief Gets the error entries from the log as a string in JSON format.
 */
std::string get_log_errors() { return fims::FIMSLog::fims_log->get_errors(); }

/**
 * @brief Gets the warning entries from the log as a string in JSON format.
 */
std::string get_log_warnings()
{
  return fims::FIMSLog::fims_log->get_warnings();
}

/**
 * @brief Gets the info entries from the log as a string in JSON format.
 */
std::string get_log_info() { return fims::FIMSLog::fims_log->get_info(); }

/**
 * @brief If true, writes the log on exit.
 */
void write_log(bool write) { fims::FIMSLog::fims_log->write_on_exit = write; }

/**
 * @brief Sets the path for the log file to be written to.
 */
void set_log_path(const std::string &path)
{
  fims::FIMSLog::fims_log->set_path(path);
}

/**
 * @brief If true, throws a runtime exception when an error is logged.
 */
void set_log_throw_on_error(bool throw_on_error)
{
  fims::FIMSLog::fims_log->throw_on_error = throw_on_error;
}

/**
 * @brief Adds an info entry to the log from the R environment.
 */
void log_info(std::string log_entry)
{
  fims::FIMSLog::fims_log->info_message(log_entry, -1, "R_env",
                                        "R_script_entry");
}

/**
 * @brief Adds a warning entry to the log from the R environment.
 */
void log_warning(std::string log_entry)
{
  fims::FIMSLog::fims_log->warning_message(log_entry, -1, "R_env",
                                           "R_script_entry");
}

/**
 * @brief Escapes quotations.
 *
 * @param input A string.
 * @return std::string
 */
std::string escapeQuotes(const std::string &input)
{
  std::string result = input;
  std::string search = "\"";
  std::string replace = "\\\"";

  // Find each occurrence of `"` and replace it with `\"`
  size_t pos = result.find(search);
  while (pos != std::string::npos)
  {
    result.replace(pos, search.size(), replace);
    pos = result.find(search,
                      pos + replace.size()); // Move past the replaced position
  }
  return result;
}

/**
 * @brief Adds a error entry to the log from the R environment.
 */
void log_error(std::string log_entry)
{
  std::stringstream ss;
  ss << "capture.output(traceback(4))";
  SEXP expression, result;
  ParseStatus status;

  PROTECT(expression = R_ParseVector(Rf_mkString(ss.str().c_str()), 1, &status,
                                     R_NilValue));
  if (status != PARSE_OK)
  {
    Rcpp::Rcout << "Error parsing expression" << std::endl;
    UNPROTECT(1);
  }
  Rcpp::Rcout << "before call.";
  PROTECT(result = Rf_eval(VECTOR_ELT(expression, 0), R_GlobalEnv));
  Rcpp::Rcout << "after call.";
  UNPROTECT(2);
  std::stringstream ss_ret;
  ss_ret << "traceback: ";
  for (int j = 0; j < LENGTH(result); j++)
  {
    std::string str(CHAR(STRING_ELT(result, j)));
    ss_ret << escapeQuotes(str) << "\\n";
  }

  std::string ret =
      ss_ret.str(); //"find error";//Rcpp::as<std::string>(result);

  fims::FIMSLog::fims_log->error_message(log_entry, -1, "R_env", ret.c_str());
}
#endif // FIMS_INTERFACE_RCPP_INTERFACE_HPP
