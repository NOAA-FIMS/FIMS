#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <Eigen/Dense>

#include "../had_quadra.hpp"

namespace quadra {

// Cache-efficient first-order replay tape. The full HAD graph remains the
// source of truth for Hessian and Laplace work, while joint optimization uses
// only the fields required for objective and gradient evaluation.
class CompactFirstOrderTape {
 public:
  void Build(const had::ADGraph& graph,
             const std::vector<had::VertexId>& parameter_vertices,
             had::VertexId objective_vertex) {
    const std::size_t n = graph.vertices.size();
    if (objective_vertex >= n) {
      throw std::out_of_range("Compact tape objective vertex is out of range.");
    }

    parameter_vertices_ = parameter_vertices;
    objective_vertex_ = objective_vertex;
    primal_.resize(n);
    adjoint_.assign(n, 0.0);
    weight_left_.resize(n);
    weight_right_.resize(n);
    constant_.resize(n);
    left_.resize(n);
    right_.resize(n);
    opcode_.resize(n);
    arity_.resize(n);

    for (std::size_t i = 0; i < n; ++i) {
      const auto& vertex = graph.vertices[i];
      if (vertex.op == had::OpCode::Independent &&
          (vertex.e1.to != i || vertex.e2.to != i)) {
        throw std::runtime_error(
            "Compact tape encountered an operation without replay metadata.");
      }
      primal_[i] = vertex.primal;
      weight_left_[i] = vertex.e1.w;
      weight_right_[i] = vertex.e2.w;
      constant_[i] = vertex.constant;
      left_[i] = vertex.left;
      right_[i] = vertex.right;
      opcode_[i] = vertex.op;
      arity_[i] = Arity(vertex.op);
    }

    for (const auto vertex : parameter_vertices_) {
      if (vertex >= n) {
        throw std::out_of_range("Compact tape parameter vertex is out of range.");
      }
    }
  }

  std::size_t VertexCount() const { return primal_.size(); }

  std::size_t Bytes() const {
    return primal_.capacity() * sizeof(double) +
           adjoint_.capacity() * sizeof(double) +
           weight_left_.capacity() * sizeof(double) +
           weight_right_.capacity() * sizeof(double) +
           constant_.capacity() * sizeof(double) +
           left_.capacity() * sizeof(had::VertexId) +
           right_.capacity() * sizeof(had::VertexId) +
           opcode_.capacity() * sizeof(had::OpCode) +
           arity_.capacity() * sizeof(std::uint8_t);
  }

  double Evaluate(const Eigen::VectorXd& values, Eigen::VectorXd& gradient) {
    if (static_cast<std::size_t>(values.size()) != parameter_vertices_.size()) {
      throw std::invalid_argument("Compact tape parameter length mismatch.");
    }
    for (Eigen::Index i = 0; i < values.size(); ++i) {
      primal_[parameter_vertices_[static_cast<std::size_t>(i)]] = values[i];
    }
    Forward();
    Reverse();
    gradient.resize(values.size());
    for (Eigen::Index i = 0; i < values.size(); ++i) {
      gradient[i] = adjoint_[parameter_vertices_[static_cast<std::size_t>(i)]];
    }
    return primal_[objective_vertex_];
  }

 private:
  static std::uint8_t Arity(had::OpCode opcode) {
    switch (opcode) {
      case had::OpCode::Independent:
        return 0;
      case had::OpCode::Add:
      case had::OpCode::Subtract:
      case had::OpCode::Multiply:
      case had::OpCode::Divide:
        return 2;
      default:
        return 1;
    }
  }

  void Forward() {
    for (std::size_t i = 0; i < primal_.size(); ++i) {
      const auto left = left_[i];
      const auto right = right_[i];
      switch (opcode_[i]) {
        case had::OpCode::Independent:
          break;
        case had::OpCode::Add:
          primal_[i] = primal_[left] + primal_[right];
          weight_left_[i] = 1.0;
          weight_right_[i] = 1.0;
          break;
        case had::OpCode::AddConstant:
          primal_[i] = primal_[left] + constant_[i];
          weight_left_[i] = 1.0;
          break;
        case had::OpCode::Subtract:
          primal_[i] = primal_[left] - primal_[right];
          weight_left_[i] = 1.0;
          weight_right_[i] = -1.0;
          break;
        case had::OpCode::SubtractConstant:
          primal_[i] = primal_[left] - constant_[i];
          weight_left_[i] = 1.0;
          break;
        case had::OpCode::ConstantSubtract:
          primal_[i] = constant_[i] - primal_[left];
          weight_left_[i] = -1.0;
          break;
        case had::OpCode::Multiply:
          primal_[i] = primal_[left] * primal_[right];
          weight_left_[i] = primal_[right];
          weight_right_[i] = primal_[left];
          break;
        case had::OpCode::MultiplyConstant:
          primal_[i] = primal_[left] * constant_[i];
          weight_left_[i] = constant_[i];
          break;
        case had::OpCode::Divide:
          primal_[i] = primal_[left] / primal_[right];
          weight_left_[i] = 1.0 / primal_[right];
          weight_right_[i] =
              -primal_[left] / (primal_[right] * primal_[right]);
          break;
        case had::OpCode::DivideConstant:
          primal_[i] = primal_[left] / constant_[i];
          weight_left_[i] = 1.0 / constant_[i];
          break;
        case had::OpCode::ConstantDivide:
          primal_[i] = constant_[i] / primal_[left];
          weight_left_[i] =
              -constant_[i] / (primal_[left] * primal_[left]);
          break;
        case had::OpCode::Exp:
          primal_[i] = std::exp(primal_[left]);
          weight_left_[i] = primal_[i];
          break;
        case had::OpCode::Log:
          primal_[i] = std::log(primal_[left]);
          weight_left_[i] = 1.0 / primal_[left];
          break;
        case had::OpCode::Sqrt:
          primal_[i] = std::sqrt(primal_[left]);
          weight_left_[i] = 0.5 / primal_[i];
          break;
        case had::OpCode::Negate:
          primal_[i] = -primal_[left];
          weight_left_[i] = -1.0;
          break;
      }
    }
  }

  void Reverse() {
    std::fill(adjoint_.begin(), adjoint_.end(), 0.0);
    adjoint_[objective_vertex_] = 1.0;
    for (std::size_t i = primal_.size(); i-- > 0;) {
      const auto arity = arity_[i];
      if (arity == 0) continue;
      const double adjoint = adjoint_[i];
      adjoint_[left_[i]] += adjoint * weight_left_[i];
      if (arity == 2) {
        adjoint_[right_[i]] += adjoint * weight_right_[i];
      }
    }
  }

  std::vector<double> primal_;
  std::vector<double> adjoint_;
  std::vector<double> weight_left_;
  std::vector<double> weight_right_;
  std::vector<double> constant_;
  std::vector<had::VertexId> left_;
  std::vector<had::VertexId> right_;
  std::vector<had::OpCode> opcode_;
  std::vector<std::uint8_t> arity_;
  std::vector<had::VertexId> parameter_vertices_;
  had::VertexId objective_vertex_ = 0;
};

}  // namespace quadra
