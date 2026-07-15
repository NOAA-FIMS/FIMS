/**
    HAD is a single header C++ reverse-mode automatic differentiation library
using operator overloading, with focus on second-order derivatives (Hessian).

    Quadra extension note:
    This header is an extension of the original had.h code by Tzu-Mao Li. It
preserves the original MIT-licensed reverse-mode and edge-pushing Hessian
implementation, and has been extended and modified to improve the Quadra
framework by adding experimental third-order derivative support. The current
third-order API focuses on exact directional third derivatives through a
lightweight third-order forward scalar, while retaining the original
    reverse-mode Hessian machinery for value, gradient, and Hessian
calculations. It implements the edge_pushing algorithm (see "Hessian Matrices
via Automatic Differentiation", Gower and Mello 2010) to efficiently compute the
second derivatives.

    See https://github.com/BachiLi/had for more details.

    Author: Tzu-Mao Li


    The MIT License (MIT)

    Copyright (c) 2015 Tzu-Mao Li

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
**/

#ifndef HAD_QUADRA_H__
#define HAD_QUADRA_H__

#include "had/batch_directional_flat_accumulator.hpp"
#include "had/intermediate_edge_slot_registry.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#ifdef WIN32
#define threadDefine thread_local
#endif
#ifdef __APPLE__
#define USE_AATREE
#define threadDefine __thread
#endif

#ifdef __unix
// #define USE_AATREE
#define threadDefine __thread
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef M_PI
#define M_PI std::acos(-1)
#endif

namespace had {

// Change the following line if you want to use single precision floats
typedef double Real;
typedef unsigned int VertexId;

enum class OpCode {
  Independent,
  Add,
  AddConstant,
  Subtract,
  SubtractConstant,
  ConstantSubtract,
  Multiply,
  MultiplyConstant,
  Divide,
  DivideConstant,
  ConstantDivide,
  Exp,
  Log,
  Sqrt,
  Negate
};

struct ADGraph;
struct AReal;

extern threadDefine ADGraph *g_ADGraph;
// Declare this in your .cpp source
#define DECLARE_ADGRAPH()                                                      \
  namespace had {                                                              \
  threadDefine ADGraph *g_ADGraph = 0;                                         \
  }

AReal NewAReal(const Real val);

struct AReal {
  AReal() {}

  AReal(const Real val) { *this = NewAReal(val); }

  AReal(const Real val, const VertexId varId)
      : val(val), dot(Real(0.0)), varId(varId) {}

  Real val;
  // First-order directional tangent used by directional edge-pushing.
  // Important: this must be propagated on the AReal object itself,
  // not only in ADGraph::vertices, because later operator overloads
  // read operands' .dot values directly.
  Real dot = Real(0.0);

  // Batched first-order directional tangents for future multi-direction
  // Hdot propagation. The scalar dot remains the active production path.
  std::vector<Real> dotBatch;
  VertexId varId;
};

struct ADEdge {
  ADEdge() {}
  ADEdge(const VertexId to, const Real w = Real(0.0), const Real dw = Real(0.0))
      : to(to), w(w), dw(dw) {}

  VertexId to;
  Real w;
  Real dw; // directional derivative of edge weight

  // Batched directional derivative of edge weight.
  std::vector<Real> dwBatch;
};

// We assume there is at most 2 outgoing edges from this vertex
struct ADVertex {
  ADVertex(const VertexId newId) {
    e1 = e2 = ADEdge(newId);
    w = wDot = soW = soWDot = toW = dot = Real(0.0);
  }

  // If ei.to == the id of this vertex, then the edge does not exist
  ADEdge e1, e2;
  // first-order adjoint weight
  Real w;
  // directional derivative of first-order adjoint weight
  Real wDot;
  // second-order weights
  // for vertex with single outgoing edge,
  // soW represents the second-order weight of the conntecting vertex
  // (d^2f/dx^2) for vertex with two outgoing edges, soW represents the
  // second-order weight between the conntecting vertices (d^2f/dxdy) the system
  // assumes d^2f/dx^2 & d^2f/dy^2 are both zero in the two outgoing edges case
  // to save memory
  Real soW;
  // directional derivative of soW along seeded primal tangent
  Real soWDot;
  // third-order local derivative weight. For unary vertices this is d^3 child /
  // d parent^3. For binary vertices this is reserved for future full
  // third-order edge-pushing support.
  Real toW;
  // Optional directional tangent associated with this vertex.
  Real dot;

  // Batched directional tangent associated with this vertex.
  std::vector<Real> dotBatch;

  // Batched directional derivative of first-order adjoint weight.
  std::vector<Real> wDotBatch;

  // Batched directional derivative of soW along seeded primal tangents.
  std::vector<Real> soWDotBatch;

  // Cached active-direction mask for batched directional reverse.
  // Bit k is set when direction k has local signal at this vertex.
  // Used for fast paths when nBatchDirections <= 64.
  std::uint64_t batchActiveDirectionMask = 0;

  // Replay primal value and operation metadata.
  Real primal = Real(0.0);
  OpCode op = OpCode::Independent;
  VertexId left = 0;
  VertexId right = 0;
  Real constant = Real(0.0);
};

struct BTNode {
  BTNode() {}
  BTNode(const VertexId key, const Real val) : key(key), val(val) {
    left = right = -1;
#ifdef USE_AATREE
    level = 1;
#endif
  }

  VertexId key;
  Real val;
  int left;
  int right;
#ifdef USE_AATREE
  int level;
#endif
};

struct BTree {
  BTree() {
    nodes.reserve(32);
    root = 0;
  }
#ifdef USE_AATREE
  inline void Skew() {
    if (nodes.size() == 0)
      return;

    while (nodes[root].left != -1 &&
           nodes[nodes[root].left].level == nodes[root].level) {
      int l = nodes[root].left;
      nodes[root].left = nodes[l].right;
      nodes[l].right = root;
      root = l;
    }
  }

  inline void Split() {
    if (nodes.size() == 0)
      return;

    while (nodes[root].right != -1 && nodes[nodes[root].right].right != -1 &&
           nodes[root].level == nodes[nodes[nodes[root].right].right].level) {
      int r = nodes[root].right;
      nodes[root].right = nodes[r].left;
      nodes[r].left = root;
      nodes[r].level++;
      root = r;
    }
  }
#endif
  inline void Insert(const VertexId key, const Real val) {
    int index = root;
    if (nodes.size() > 0) {
      int *lastEdge;
      do {
        if (key == nodes[index].key) {
          nodes[index].val += val;
          return;
        }
        lastEdge = &(nodes[index].left) + (key > nodes[index].key);
        index = *lastEdge;
      } while (index >= 0);

      *lastEdge = nodes.size();
    }
    nodes.push_back(BTNode(key, val));
#ifdef USE_AATREE
    Skew();
    Split();
#endif
  }

  inline Real Query(const VertexId key) {
    int index = root;
    while (index >= 0 && index < (int)nodes.size()) {
      if (key == nodes[index].key) {
        return nodes[index].val;
      } else if (key < nodes[index].key) {
        index = nodes[index].left;
      } else {
        index = nodes[index].right;
      }
    }
    return Real(0.0);
  }

  inline std::size_t CountMappedKeys(BTree &slot_map_tree) const {
    std::size_t mapped = 0;
    for (const auto &node : nodes) {
      if (slot_map_tree.Query(node.key) != Real(0.0)) {
        ++mapped;
      }
    }
    return mapped;
  }

  inline void Clear() {
    nodes.clear();
    root = 0;
  }

  inline void Reserve(const size_t n) {
    if (nodes.capacity() < n) {
      nodes.reserve(n);
    }
  }

  inline size_t Capacity() const { return nodes.capacity(); }

  inline size_t Size() const { return nodes.size(); }

  std::vector<BTNode> nodes;
  int root;
};

struct ADGraph {
  ADGraph() { g_ADGraph = this; }

  inline void Clear() {
    vertices.clear();
    soEdges.clear();
    selfSoEdges.clear();
    soEdgesDot.clear();
    intermediateEdgeSlotRegistry.Clear();
    intermediateEdgeSlotRegistryBuilt = false;
    selfSoEdgesDot.clear();
  }

  // Reset accumulated reverse-mode adjoints and Hessian accumulator
  // state without deleting the graph structure or local derivative
  // metadata stored on vertices.

  // Replay primal values and local derivative weights using stored
  // operation metadata.
  inline void Forward() {
    for (size_t i = 0; i < vertices.size(); ++i) {
      ADVertex &v = vertices[i];

      switch (v.op) {
      case OpCode::Independent:
        break;

      case OpCode::Add:
        v.primal = vertices[v.left].primal + vertices[v.right].primal;
        v.e1.w = Real(1.0);
        v.e2.w = Real(1.0);
        v.soW = Real(0.0);
        break;

      case OpCode::AddConstant:
        v.primal = vertices[v.left].primal + v.constant;
        v.e1.w = Real(1.0);
        v.soW = Real(0.0);
        break;

      case OpCode::Subtract:
        v.primal = vertices[v.left].primal - vertices[v.right].primal;
        v.e1.w = Real(1.0);
        v.e2.w = -Real(1.0);
        v.soW = Real(0.0);
        break;

      case OpCode::SubtractConstant:
        v.primal = vertices[v.left].primal - v.constant;
        v.e1.w = Real(1.0);
        v.soW = Real(0.0);
        break;

      case OpCode::ConstantSubtract:
        v.primal = v.constant - vertices[v.left].primal;
        v.e1.w = -Real(1.0);
        v.soW = Real(0.0);
        break;

      case OpCode::Multiply:
        v.primal = vertices[v.left].primal * vertices[v.right].primal;
        v.e1.w = vertices[v.right].primal;
        v.e2.w = vertices[v.left].primal;
        v.soW = Real(1.0);
        break;

      case OpCode::MultiplyConstant:
        v.primal = vertices[v.left].primal * v.constant;
        v.e1.w = v.constant;
        v.soW = Real(0.0);
        break;

      case OpCode::Divide:
        v.primal = vertices[v.left].primal / vertices[v.right].primal;
        v.e1.w = Real(1.0) / vertices[v.right].primal;
        v.e2.w = -vertices[v.left].primal /
                 (vertices[v.right].primal * vertices[v.right].primal);
        v.soW =
            -Real(1.0) / (vertices[v.right].primal * vertices[v.right].primal);
        break;

      case OpCode::DivideConstant:
        v.primal = vertices[v.left].primal / v.constant;
        v.e1.w = Real(1.0) / v.constant;
        v.soW = Real(0.0);
        break;

      case OpCode::ConstantDivide:
        v.primal = v.constant / vertices[v.left].primal;
        v.e1.w =
            -v.constant / (vertices[v.left].primal * vertices[v.left].primal);
        v.soW = Real(2.0) * v.constant /
                (vertices[v.left].primal * vertices[v.left].primal *
                 vertices[v.left].primal);
        break;

      case OpCode::Exp:
        v.primal = std::exp(vertices[v.left].primal);
        v.e1.w = v.primal;
        v.soW = v.primal;
        break;

      case OpCode::Log:
        v.primal = std::log(vertices[v.left].primal);
        v.e1.w = Real(1.0) / vertices[v.left].primal;
        v.soW =
            -Real(1.0) / (vertices[v.left].primal * vertices[v.left].primal);
        break;

      case OpCode::Sqrt:
        v.primal = std::sqrt(vertices[v.left].primal);
        v.e1.w = Real(0.5) / v.primal;
        v.soW = -Real(0.25) / (vertices[v.left].primal * v.primal);
        break;

      case OpCode::Negate:
        v.primal = -vertices[v.left].primal;
        v.e1.w = -Real(1.0);
        v.soW = Real(0.0);
        break;
      }
    }
  }

  inline void ZeroAdjoints() {
    for (ADVertex &v : vertices) {
      v.w = Real(0.0);
      v.wDot = Real(0.0);
    }

    soEdges.clear();
    selfSoEdges.clear();
    soEdgesDot.clear();
    selfSoEdgesDot.clear();
  }

  std::vector<ADVertex> vertices;
  std::vector<BTree> soEdges;
  std::vector<Real> selfSoEdges;
  std::vector<BTree> soEdgesDot;
  // Cached slot registry for intermediate second-order edges.
  // Future flat reverse backend will use this to replace BTree query/insert
  // with direct slot-indexed directional accumulation.
  had::IntermediateEdgeSlotRegistry intermediateEdgeSlotRegistry;
  bool intermediateEdgeSlotRegistryBuilt = false;
  bool useFlatIntermediateDirectionalBackend = false;
  had::BatchDirectionalFlatAccumulator flatIntermediateDirectionalValues;

  std::vector<Real> selfSoEdgesDot;

  // Number of active batched directional tangents.
  int nBatchDirections = 0;

  // Batched directional Hessian edge storage.
  // Indexed as soEdgesDotBatch[k][vertex] and selfSoEdgesDotBatch[k][vertex].
  std::vector<std::vector<BTree>> soEdgesDotBatch;
  std::vector<std::vector<Real>> selfSoEdgesDotBatch;

  // Optional write-side directional slot workspace.
  // batchSelfSlot[vertex] gives the direct slot for diagonal
  // Hdot(vertex,vertex). batchSlotOuterInnerToSlot[outer].Query(inner) gives
  // off-diagonal slot, or 0 if no slot is mapped. Slot ids are stored as slot+1
  // so 0 can mean absent.
  bool useBatchDirectionalSlotWorkspace = false;
  std::vector<int> batchSelfSlot;
  std::vector<BTree> batchSlotOuterInnerToSlot;
  std::vector<std::vector<Real>> batchDirectionalSlotValues;
};

inline AReal NewAReal(const Real val) {
  std::vector<ADVertex> &vertices = g_ADGraph->vertices;
  VertexId newId = vertices.size();
  vertices.push_back(ADVertex(newId));
  vertices[newId].primal = val;
  vertices[newId].op = OpCode::Independent;
  return AReal(val, newId);
}

inline void AddEdge(AReal &c, const AReal &p, const Real w, const Real soW,
                    const Real toW = Real(0.0)) {
  ADVertex &v = g_ADGraph->vertices[c.varId];
  const Real dw = soW * p.dot;
  v.e1 = ADEdge(p.varId, w, dw);
  v.soW = soW;
  v.soWDot = toW * p.dot;
  v.toW = toW;
  v.dot = w * p.dot;
  c.dot = v.dot;
}
inline void AddEdge(AReal &c, const AReal &p1, const AReal &p2, const Real w1,
                    const Real w2, const Real soW, const Real toW = Real(0.0)) {
  ADVertex &v = g_ADGraph->vertices[c.varId];

  Real dw1 = Real(0.0);
  Real dw2 = Real(0.0);
  if (soW != Real(0.0)) {
    dw1 = soW * p2.dot;
    dw2 = soW * p1.dot;
  }

  v.e1 = ADEdge(p1.varId, w1, dw1);
  v.e2 = ADEdge(p2.varId, w2, dw2);
  v.soW = soW;
  v.soWDot = toW * (p1.dot + p2.dot);
  v.toW = toW;
  v.dot = w1 * p1.dot + w2 * p2.dot;
  c.dot = v.dot;
}

inline void SetReplayBinary(AReal &c, const OpCode op, const AReal &left,
                            const AReal &right) {
  ADVertex &v = g_ADGraph->vertices[c.varId];
  v.op = op;
  v.left = left.varId;
  v.right = right.varId;
}

inline void SetReplayUnary(AReal &c, const OpCode op, const AReal &left) {
  ADVertex &v = g_ADGraph->vertices[c.varId];
  v.op = op;
  v.left = left.varId;
}

inline void SetReplayConstant(AReal &c, const OpCode op, const AReal &left,
                              const Real constant) {
  ADVertex &v = g_ADGraph->vertices[c.varId];
  v.op = op;
  v.left = left.varId;
  v.constant = constant;
}

inline void SetValue(AReal &x, const Real value) {
  x.val = value;
  g_ADGraph->vertices[x.varId].primal = value;
}

////////////////////// Addition ///////////////////////////
inline AReal operator+(const AReal &l, const AReal &r) {
  AReal ret = NewAReal(l.val + r.val);
  AddEdge(ret, l, r, Real(1.0), Real(1.0), Real(0.0));
  SetReplayBinary(ret, OpCode::Add, l, r);
  return ret;
}
inline AReal operator+(const AReal &l, const Real r) {
  AReal ret = NewAReal(l.val + r);
  AddEdge(ret, l, Real(1.0), Real(0.0));
  SetReplayConstant(ret, OpCode::AddConstant, l, r);
  return ret;
}
inline AReal operator+(const Real l, const AReal &r) { return r + l; }
inline AReal &operator+=(AReal &l, const AReal &r) { return (l = l + r); }
inline AReal &operator+=(AReal &l, const Real r) { return (l = l + r); }
///////////////////////////////////////////////////////////

////////////////// Subtraction ////////////////////////////
inline AReal operator-(const AReal &l, const AReal &r) {
  AReal ret = NewAReal(l.val - r.val);
  AddEdge(ret, l, r, Real(1.0), -Real(1.0), Real(0.0));
  SetReplayBinary(ret, OpCode::Subtract, l, r);
  return ret;
}
inline AReal operator-(const AReal &l, const Real r) {
  AReal ret = NewAReal(l.val - r);
  AddEdge(ret, l, Real(1.0), Real(0.0));
  SetReplayConstant(ret, OpCode::SubtractConstant, l, r);
  return ret;
}
inline AReal operator-(const Real l, const AReal &r) {
  AReal ret = NewAReal(l - r.val);
  AddEdge(ret, r, Real(-1.0), Real(0.0));
  SetReplayConstant(ret, OpCode::ConstantSubtract, r, l);
  return ret;
}
inline AReal &operator-=(AReal &l, const AReal &r) { return (l = l - r); }
inline AReal &operator-=(AReal &l, const Real r) { return (l = l - r); }
inline AReal operator-(const AReal &x) {
  AReal ret = NewAReal(-x.val);
  AddEdge(ret, x, Real(-1.0), Real(0.0));
  SetReplayUnary(ret, OpCode::Negate, x);
  return ret;
}
///////////////////////////////////////////////////////////

////////////////// Multiplication /////////////////////////
inline AReal operator*(const AReal &l, const AReal &r) {
  AReal ret = NewAReal(l.val * r.val);
  AddEdge(ret, l, r, r.val, l.val, Real(1.0));
  SetReplayBinary(ret, OpCode::Multiply, l, r);
  return ret;
}
inline AReal operator*(const AReal &l, const Real r) {
  AReal ret = NewAReal(l.val * r);
  AddEdge(ret, l, r, Real(0.0));
  SetReplayConstant(ret, OpCode::MultiplyConstant, l, r);
  return ret;
}
inline AReal operator*(const Real l, const AReal &r) { return r * l; }
inline AReal &operator*=(AReal &l, const AReal &r) { return (l = l * r); }
inline AReal &operator*=(AReal &l, const Real r) { return (l = l * r); }
///////////////////////////////////////////////////////////

////////////////// Inversion //////////////////////////////
inline AReal Inv(const AReal &x) {
  Real invX = Real(1.0) / x.val;
  Real invXSq = invX * invX;
  Real invXCu = invXSq * invX;
  AReal ret = NewAReal(invX);
  AddEdge(ret, x, -invXSq, Real(2.0) * invXCu, -Real(6.0) * invXCu * invX);
  SetReplayConstant(ret, OpCode::ConstantDivide, x, Real(1.0));
  return ret;
}
inline Real Inv(const Real x) { return Real(1.0) / x; }
///////////////////////////////////////////////////////////

////////////////// Division ///////////////////////////////
inline AReal operator/(const AReal &l, const AReal &r) { return l * Inv(r); }
inline AReal operator/(const AReal &l, const Real r) { return l * Inv(r); }
inline AReal operator/(const Real l, const AReal &r) { return l * Inv(r); }
inline AReal &operator/=(AReal &l, const AReal &r) { return (l = l / r); }
inline AReal &operator/=(AReal &l, const Real r) { return (l = l / r); }
///////////////////////////////////////////////////////////

////////////////// Comparisons ////////////////////////////
inline bool operator<(const AReal &l, const AReal &r) { return l.val < r.val; }
inline bool operator<=(const AReal &l, const AReal &r) {
  return l.val <= r.val;
}
inline bool operator>(const AReal &l, const AReal &r) { return l.val > r.val; }
inline bool operator>=(const AReal &l, const AReal &r) {
  return l.val >= r.val;
}
inline bool operator==(const AReal &l, const AReal &r) {
  return l.val == r.val;
}
///////////////////////////////////////////////////////////

//////////////// Misc functions ///////////////////////////
inline Real square(const Real x) { return x * x; }
inline AReal square(const AReal &x) {
  Real sqX = x.val * x.val;
  AReal ret = NewAReal(sqX);
  AddEdge(ret, x, Real(2.0) * x.val, Real(2.0), Real(0.0));
  return ret;
}
inline AReal sqrt(const AReal &x) {
  Real sqrtX = std::sqrt(x.val);
  Real invSqrtX = Real(1.0) / sqrtX;
  AReal ret = NewAReal(sqrtX);
  AddEdge(ret, x, Real(0.5) * invSqrtX, -Real(0.25) * invSqrtX / x.val,
          Real(0.375) * invSqrtX / (x.val * x.val));
  SetReplayUnary(ret, OpCode::Sqrt, x);
  return ret;
}
inline AReal pow(const AReal &x, const Real a) {
  Real powX = std::pow(x.val, a);
  AReal ret = NewAReal(powX);
  AddEdge(ret, x, a * std::pow(x.val, a - Real(1.0)),
          a * (a - Real(1.0)) * std::pow(x.val, a - Real(2.0)),
          a * (a - Real(1.0)) * (a - Real(2.0)) *
              std::pow(x.val, a - Real(3.0)));
  return ret;
}
inline AReal exp(const AReal &x) {
  Real expX = std::exp(x.val);
  AReal ret = NewAReal(expX);
  AddEdge(ret, x, expX, expX, expX);
  SetReplayUnary(ret, OpCode::Exp, x);
  return ret;
}
inline AReal log(const AReal &x) {
  Real logX = std::log(x.val);
  AReal ret = NewAReal(logX);
  Real invX = Real(1.0) / x.val;
  AddEdge(ret, x, invX, -invX * invX, Real(2.0) * invX * invX * invX);
  SetReplayUnary(ret, OpCode::Log, x);
  return ret;
}
inline AReal sin(const AReal &x) {
  Real sinX = std::sin(x.val);
  AReal ret = NewAReal(sinX);
  AddEdge(ret, x, std::cos(x.val), -sinX, -std::cos(x.val));
  return ret;
}
inline AReal cos(const AReal &x) {
  Real cosX = std::cos(x.val);
  AReal ret = NewAReal(cosX);
  AddEdge(ret, x, -std::sin(x.val), -cosX, std::sin(x.val));
  return ret;
}
inline AReal tan(const AReal &x) {
  Real tanX = std::tan(x.val);
  Real secX = Real(1.0) / std::cos(x.val);
  Real sec2X = secX * secX;
  AReal ret = NewAReal(tanX);
  AddEdge(ret, x, sec2X, Real(2.0) * tanX * sec2X,
          Real(2.0) * sec2X * sec2X + Real(4.0) * tanX * tanX * sec2X);
  return ret;
}
inline AReal asin(const AReal &x) {
  Real asinX = std::asin(x.val);
  AReal ret = NewAReal(asinX);
  Real tmp = Real(1.0) / (Real(1.0) - x.val * x.val);
  Real sqrtTmp = std::sqrt(tmp);
  AddEdge(ret, x, sqrtTmp, x.val * sqrtTmp * tmp,
          (Real(1.0) + Real(2.0) * x.val * x.val) * sqrtTmp * tmp * tmp);
  return ret;
}
inline AReal acos(const AReal &x) {
  Real acosX = std::acos(x.val);
  AReal ret = NewAReal(acosX);
  Real tmp = Real(1.0) / (Real(1.0) - x.val * x.val);
  Real negSqrtTmp = -std::sqrt(tmp);
  AddEdge(ret, x, negSqrtTmp, x.val * negSqrtTmp * tmp,
          -(Real(1.0) + Real(2.0) * x.val * x.val) * std::sqrt(tmp) * tmp *
              tmp);
  return ret;
}
///////////////////////////////////////////////////////////

inline void Forward(ADGraph &graph) { graph.Forward(); }

inline void Forward() {
  if (g_ADGraph) {
    g_ADGraph->Forward();
  }
}

inline void ZeroAdjoints(ADGraph &graph) { graph.ZeroAdjoints(); }

inline void ZeroAdjoints() {
  if (g_ADGraph) {
    g_ADGraph->ZeroAdjoints();
  }
}

inline void SetAdjoint(const AReal &v, const Real adj) {
  g_ADGraph->vertices[v.varId].w = adj;
}

inline Real GetAdjoint(const AReal &v) {
  return g_ADGraph->vertices[v.varId].w;
}

inline Real GetAdjoint(const AReal &i, const AReal &j) {
  if (i.varId == j.varId) {
    return g_ADGraph->selfSoEdges[i.varId];
  } else {
    return g_ADGraph->soEdges[std::max(i.varId, j.varId)].Query(
        std::min(i.varId, j.varId));
  }
}

inline VertexId SingleEdgePropagate(VertexId x, Real &a) {
  bool cont =
      g_ADGraph->vertices[x].e1.to != x && g_ADGraph->vertices[x].e2.to == x;
  while (cont) {
    a *= g_ADGraph->vertices[x].e1.w;
    x = g_ADGraph->vertices[x].e1.to;
    cont =
        g_ADGraph->vertices[x].e1.to != x && g_ADGraph->vertices[x].e2.to == x;
  }
  return x;
}

inline void PushEdge(const ADEdge &foEdge, const ADEdge &soEdge) {
  if (foEdge.to == soEdge.to) {
    g_ADGraph->selfSoEdges[foEdge.to] += Real(2.0) * foEdge.w * soEdge.w;
  } else {
    g_ADGraph->soEdges[std::max(foEdge.to, soEdge.to)].Insert(
        std::min(foEdge.to, soEdge.to), foEdge.w * soEdge.w);
  }
}

inline void PushEdgeDot(const ADEdge &foEdge, const ADEdge &soEdge,
                        const Real soEdgeDot) {
  const Real valDot = foEdge.dw * soEdge.w + foEdge.w * soEdgeDot;

  if (foEdge.to == soEdge.to) {
    g_ADGraph->selfSoEdgesDot[foEdge.to] += Real(2.0) * valDot;
  } else {
    g_ADGraph->soEdgesDot[std::max(foEdge.to, soEdge.to)].Insert(
        std::min(foEdge.to, soEdge.to), valDot);
  }
}

inline Real GetAdjointDot(const AReal &i, const AReal &j) {
  if (i.varId == j.varId) {
    return g_ADGraph->selfSoEdgesDot[i.varId];
  }
  return g_ADGraph->soEdgesDot[std::max(i.varId, j.varId)].Query(
      std::min(i.varId, j.varId));
}

//==================================================
// Batched directional propagation scaffold.
//
// These helpers add graph storage and public accessors for multiple
// simultaneous directional tangents. They intentionally do not replace
// PropagateAdjointDirectional() yet.
//==================================================

inline void DisableBatchDirectionalSlotWorkspace() {
  g_ADGraph->useBatchDirectionalSlotWorkspace = false;
  g_ADGraph->batchSelfSlot.clear();
  g_ADGraph->batchSlotOuterInnerToSlot.clear();
  g_ADGraph->batchDirectionalSlotValues.clear();
}

inline void EnableBatchDirectionalSlotWorkspace(const int nSlots) {
  const int nDirections = g_ADGraph->nBatchDirections;
  g_ADGraph->useBatchDirectionalSlotWorkspace = true;

  g_ADGraph->batchSelfSlot.assign(g_ADGraph->vertices.size(), -1);
  g_ADGraph->batchSlotOuterInnerToSlot.clear();
  g_ADGraph->batchSlotOuterInnerToSlot.resize(g_ADGraph->vertices.size());

  g_ADGraph->batchDirectionalSlotValues.assign(
      static_cast<size_t>(nDirections),
      std::vector<Real>(static_cast<size_t>(nSlots), Real(0.0)));
}

inline void ClearBatchDirectionalSlotValues() {
  if (!g_ADGraph->useBatchDirectionalSlotWorkspace) {
    return;
  }
  for (auto &values : g_ADGraph->batchDirectionalSlotValues) {
    std::fill(values.begin(), values.end(), Real(0.0));
  }
}

inline void SetBatchDirectionalSelfSlot(const VertexId vertex, const int slot) {
  if (g_ADGraph->batchSelfSlot.size() < g_ADGraph->vertices.size()) {
    g_ADGraph->batchSelfSlot.assign(g_ADGraph->vertices.size(), -1);
  }
  g_ADGraph->batchSelfSlot[vertex] = slot;
}

inline void SetBatchDirectionalOffdiagSlot(const VertexId i, const VertexId j,
                                           const int slot) {
  const VertexId outer = std::max(i, j);
  const VertexId inner = std::min(i, j);

  if (g_ADGraph->batchSlotOuterInnerToSlot.size() <
      g_ADGraph->vertices.size()) {
    g_ADGraph->batchSlotOuterInnerToSlot.resize(g_ADGraph->vertices.size());
  }

  // Store slot+1 so Query()==0 means absent.
  g_ADGraph->batchSlotOuterInnerToSlot[outer].Insert(inner, Real(slot + 1));
}

inline bool AddBatchDirectionalSlotValue(const size_t direction,
                                         const VertexId i, const VertexId j,
                                         const Real value) {
  if (!g_ADGraph->useBatchDirectionalSlotWorkspace) {
    return false;
  }
  if (direction >= g_ADGraph->batchDirectionalSlotValues.size()) {
    return false;
  }

  int slot = -1;

  if (i == j) {
    if (i < g_ADGraph->batchSelfSlot.size()) {
      slot = g_ADGraph->batchSelfSlot[i];
    }
  } else {
    const VertexId outer = std::max(i, j);
    const VertexId inner = std::min(i, j);

    if (outer < g_ADGraph->batchSlotOuterInnerToSlot.size()) {
      const Real stored =
          g_ADGraph->batchSlotOuterInnerToSlot[outer].Query(inner);
      if (stored != Real(0.0)) {
        slot = static_cast<int>(stored) - 1;
      }
    }
  }

  if (slot < 0) {
    return false;
  }

  auto &values = g_ADGraph->batchDirectionalSlotValues[direction];
  if (slot >= static_cast<int>(values.size())) {
    return false;
  }

  values[static_cast<size_t>(slot)] += value;
  return true;
}

inline Real GetBatchDirectionalSlotValue(const size_t direction,
                                         const int slot) {
  return g_ADGraph
      ->batchDirectionalSlotValues[direction][static_cast<size_t>(slot)];
}

inline bool TryGetBatchDirectionalSlot(const VertexId i, const VertexId j,
                                       int &slot_out) {
  slot_out = -1;

  if (!g_ADGraph->useBatchDirectionalSlotWorkspace) {
    return false;
  }

  if (i == j) {
    if (i < g_ADGraph->batchSelfSlot.size()) {
      slot_out = g_ADGraph->batchSelfSlot[i];
      return slot_out >= 0;
    }
    return false;
  }

  const VertexId outer = std::max(i, j);
  const VertexId inner = std::min(i, j);

  if (outer >= g_ADGraph->batchSlotOuterInnerToSlot.size()) {
    return false;
  }

  const Real stored = g_ADGraph->batchSlotOuterInnerToSlot[outer].Query(inner);

  if (stored == Real(0.0)) {
    return false;
  }

  slot_out = static_cast<int>(stored) - 1;
  return slot_out >= 0;
}

inline bool TryGetBatchDirectionalSlotValue(const size_t direction,
                                            const VertexId i, const VertexId j,
                                            Real &value_out) {
  int slot = -1;
  if (!TryGetBatchDirectionalSlot(i, j, slot)) {
    return false;
  }

  if (direction >= g_ADGraph->batchDirectionalSlotValues.size()) {
    return false;
  }

  const auto &values = g_ADGraph->batchDirectionalSlotValues[direction];

  if (slot < 0 || slot >= static_cast<int>(values.size())) {
    return false;
  }

  value_out = values[static_cast<size_t>(slot)];
  return true;
}

inline void ReserveDirectionalBTreeStorage(const size_t reserve_per_tree) {
  for (auto &tree : g_ADGraph->soEdges) {
    tree.Reserve(reserve_per_tree);
  }
  for (auto &tree : g_ADGraph->soEdgesDot) {
    tree.Reserve(reserve_per_tree);
  }
  for (auto &trees_for_direction : g_ADGraph->soEdgesDotBatch) {
    for (auto &tree : trees_for_direction) {
      tree.Reserve(reserve_per_tree);
    }
  }
}

struct BatchEdgeSlotCoverageDiagnostic {
  std::size_t total_edges = 0;
  std::size_t mapped_edges = 0;
  std::size_t unmapped_edges = 0;
  double mapped_fraction = 0.0;
};

inline BatchEdgeSlotCoverageDiagnostic DiagnoseBatchEdgeSlotCoverage() {
  BatchEdgeSlotCoverageDiagnostic out;

  if (!g_ADGraph->useBatchDirectionalSlotWorkspace) {
    return out;
  }

  const std::size_t n = std::min(g_ADGraph->soEdges.size(),
                                 g_ADGraph->batchSlotOuterInnerToSlot.size());

  for (std::size_t outer = 0; outer < n; ++outer) {
    const auto &edges = g_ADGraph->soEdges[outer];
    auto &slot_map = g_ADGraph->batchSlotOuterInnerToSlot[outer];

    const std::size_t total = edges.Size();
    const std::size_t mapped = edges.CountMappedKeys(slot_map);

    out.total_edges += total;
    out.mapped_edges += mapped;
  }

  out.unmapped_edges = out.total_edges - out.mapped_edges;
  if (out.total_edges > 0) {
    out.mapped_fraction = static_cast<double>(out.mapped_edges) /
                          static_cast<double>(out.total_edges);
  }

  return out;
}

inline void PrintBatchEdgeSlotCoverageDiagnostic() {
  const auto d = DiagnoseBatchEdgeSlotCoverage();
  std::cerr << "batch edge slot coverage:" << " total=" << d.total_edges
            << " mapped=" << d.mapped_edges << " unmapped=" << d.unmapped_edges
            << " mapped_fraction=" << d.mapped_fraction << "\n";
}

struct IntermediateEdgeSlotRegistryDiagnostic {
  std::size_t slots = 0;
  std::size_t source_edges = 0;
};

inline IntermediateEdgeSlotRegistryDiagnostic
BuildIntermediateEdgeSlotRegistryFromSoEdges() {
  IntermediateEdgeSlotRegistryDiagnostic out;

  g_ADGraph->intermediateEdgeSlotRegistry.Clear();

  for (VertexId outer = 0;
       outer < static_cast<VertexId>(g_ADGraph->soEdges.size()); ++outer) {
    auto &tree = g_ADGraph->soEdges[outer];

    for (const auto &node : tree.nodes) {
      g_ADGraph->intermediateEdgeSlotRegistry.GetOrCreate(
          outer, static_cast<VertexId>(node.key));
      ++out.source_edges;
    }
  }

  g_ADGraph->intermediateEdgeSlotRegistryBuilt = true;
  out.slots = g_ADGraph->intermediateEdgeSlotRegistry.size();
  return out;
}

inline IntermediateEdgeSlotRegistryDiagnostic
GetIntermediateEdgeSlotRegistryDiagnostic() {
  IntermediateEdgeSlotRegistryDiagnostic out;
  out.slots = g_ADGraph->intermediateEdgeSlotRegistry.size();
  out.source_edges = g_ADGraph->intermediateEdgeSlotRegistry.size();
  return out;
}

inline void PrintIntermediateEdgeSlotRegistryDiagnostic() {
  const auto d = BuildIntermediateEdgeSlotRegistryFromSoEdges();
  std::cerr << "intermediate edge registry:" << " source_edges="
            << d.source_edges << " slots=" << d.slots << "\n";
}

inline std::uint64_t g_flat_intermediate_read_hit_count = 0;
inline std::uint64_t g_flat_intermediate_read_miss_count = 0;
inline std::uint64_t g_flat_intermediate_write_hit_count = 0;
inline std::uint64_t g_flat_intermediate_write_miss_count = 0;

inline std::vector<std::pair<VertexId, VertexId>>
    g_flat_intermediate_read_miss_samples;
inline std::vector<std::pair<VertexId, VertexId>>
    g_flat_intermediate_write_miss_samples;

inline void RecordFlatIntermediateMissSample(
    std::vector<std::pair<VertexId, VertexId>> &samples, const VertexId i,
    const VertexId j) {
  if (samples.size() < 12) {
    samples.emplace_back(std::max(i, j), std::min(i, j));
  }
}

inline void PrintFlatIntermediateMissSamples() {
  std::cerr << "flat intermediate read miss samples:";
  for (const auto &p : g_flat_intermediate_read_miss_samples) {
    std::cerr << " (" << p.first << "," << p.second << ")";
  }
  std::cerr << "\n";

  std::cerr << "flat intermediate write miss samples:";
  for (const auto &p : g_flat_intermediate_write_miss_samples) {
    std::cerr << " (" << p.first << "," << p.second << ")";
  }
  std::cerr << "\n";
}

inline void ResetFlatIntermediateDirectionalCounters() {
  g_flat_intermediate_read_hit_count = 0;
  g_flat_intermediate_read_miss_count = 0;
  g_flat_intermediate_write_hit_count = 0;
  g_flat_intermediate_write_miss_count = 0;

  g_flat_intermediate_read_miss_samples.clear();
  g_flat_intermediate_write_miss_samples.clear();
}

inline void PrintFlatIntermediateDirectionalCounters() {
  std::cerr << "flat intermediate:" << " read_hit="
            << g_flat_intermediate_read_hit_count
            << " read_miss=" << g_flat_intermediate_read_miss_count
            << " write_hit=" << g_flat_intermediate_write_hit_count
            << " write_miss=" << g_flat_intermediate_write_miss_count << "\n";

  PrintFlatIntermediateMissSamples();
}

inline void ClearFlatIntermediateDirectionalValues() {
  if (g_ADGraph->useFlatIntermediateDirectionalBackend) {
    g_ADGraph->flatIntermediateDirectionalValues.Clear();
  }
}

inline std::size_t EstimateFlatIntermediateDirectionalSlotCapacity() {
  std::size_t edge_count = 0;

  for (const auto &tree : g_ADGraph->soEdges) {
    edge_count += tree.Size();
  }

  const std::size_t base =
      std::max(edge_count, g_ADGraph->intermediateEdgeSlotRegistry.size());

  // Reverse propagation creates additional intermediate directional edges
  // beyond the initial soEdges snapshot. Over-allocate to avoid repeated
  // EnsureSlotsPreserve() realloc/copy in the hot loop.
  return base * 4 + 1024;
}

inline void EnableFlatIntermediateDirectionalBackend() {
  if (!g_ADGraph->intermediateEdgeSlotRegistryBuilt ||
      g_ADGraph->intermediateEdgeSlotRegistry.size() == 0) {
    BuildIntermediateEdgeSlotRegistryFromSoEdges();
  }

  g_ADGraph->useFlatIntermediateDirectionalBackend = true;
  g_ADGraph->flatIntermediateDirectionalValues.Resize(
      static_cast<size_t>(g_ADGraph->nBatchDirections),
      EstimateFlatIntermediateDirectionalSlotCapacity());
}

inline bool AddFlatIntermediateDirectionalValue(const size_t direction,
                                                const VertexId i,
                                                const VertexId j,
                                                const Real value) {
  if (!g_ADGraph->useFlatIntermediateDirectionalBackend) {
    ++g_flat_intermediate_write_miss_count;
    RecordFlatIntermediateMissSample(g_flat_intermediate_write_miss_samples, i,
                                     j);
    return false;
  }

  const std::size_t slot =
      g_ADGraph->intermediateEdgeSlotRegistry.GetOrCreate(i, j);

  g_ADGraph->flatIntermediateDirectionalValues.EnsureSlotsPreserve(slot + 1);
  g_ADGraph->flatIntermediateDirectionalValues.Add(direction, slot, value);

  ++g_flat_intermediate_write_hit_count;
  return true;
}

inline bool TryGetFlatIntermediateDirectionalValue(const size_t direction,
                                                   const VertexId i,
                                                   const VertexId j,
                                                   Real &value_out) {
  if (!g_ADGraph->useFlatIntermediateDirectionalBackend) {
    ++g_flat_intermediate_read_miss_count;
    RecordFlatIntermediateMissSample(g_flat_intermediate_read_miss_samples, i,
                                     j);
    return false;
  }

  std::size_t slot = 0;
  if (!g_ADGraph->intermediateEdgeSlotRegistry.TryGet(i, j, slot)) {
    ++g_flat_intermediate_read_miss_count;
    RecordFlatIntermediateMissSample(g_flat_intermediate_read_miss_samples, i,
                                     j);
    return false;
  }

  value_out = g_ADGraph->flatIntermediateDirectionalValues(direction, slot);
  ++g_flat_intermediate_read_hit_count;
  return true;
}
inline void ResizeDirectionalBatch(const int nDirections) {
  if (nDirections < 0) {
    throw std::invalid_argument(
        "ResizeDirectionalBatch: nDirections must be nonnegative");
  }

  g_ADGraph->nBatchDirections = nDirections;

  for (auto &v : g_ADGraph->vertices) {
    v.dotBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.wDotBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.soWDotBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.e1.dwBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.e2.dwBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.batchActiveDirectionMask = 0;
  }

  g_ADGraph->soEdgesDotBatch.resize(static_cast<size_t>(nDirections));
  g_ADGraph->selfSoEdgesDotBatch.resize(static_cast<size_t>(nDirections));

  for (int k = 0; k < nDirections; ++k) {
    g_ADGraph->soEdgesDotBatch[static_cast<size_t>(k)].resize(
        g_ADGraph->vertices.size());

    g_ADGraph->selfSoEdgesDotBatch[static_cast<size_t>(k)].assign(
        g_ADGraph->vertices.size(), Real(0.0));
  }

  if (g_ADGraph->useBatchDirectionalSlotWorkspace) {
    const size_t nSlots =
        g_ADGraph->batchDirectionalSlotValues.empty()
            ? 0
            : g_ADGraph->batchDirectionalSlotValues.front().size();
    g_ADGraph->batchDirectionalSlotValues.assign(
        static_cast<size_t>(nDirections), std::vector<Real>(nSlots, Real(0.0)));
  }
}

inline void ClearDirectionalBatch() {
  const int nDirections = g_ADGraph->nBatchDirections;

  for (auto &v : g_ADGraph->vertices) {
    v.dotBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.wDotBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.soWDotBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.e1.dwBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.e2.dwBatch.assign(static_cast<size_t>(nDirections), Real(0.0));
    v.batchActiveDirectionMask = 0;
  }

  for (int k = 0; k < nDirections; ++k) {
    auto &trees = g_ADGraph->soEdgesDotBatch[static_cast<size_t>(k)];
    for (auto &tree : trees) {
      tree.Clear();
    }

    g_ADGraph->selfSoEdgesDotBatch[static_cast<size_t>(k)].assign(
        g_ADGraph->vertices.size(), Real(0.0));
  }
}

inline void CheckDirectionalBatchIndex(const int k) {
  if (k < 0 || k >= g_ADGraph->nBatchDirections) {
    throw std::out_of_range("directional batch index out of range");
  }
}

inline void SetVertexDotBatch(const VertexId vertexId, const int k,
                              const Real value) {
  CheckDirectionalBatchIndex(k);

  if (vertexId >= g_ADGraph->vertices.size()) {
    throw std::out_of_range("SetVertexDotBatch: vertexId out of range");
  }

  auto &v = g_ADGraph->vertices[vertexId];
  if (v.dotBatch.size() != static_cast<size_t>(g_ADGraph->nBatchDirections)) {
    v.dotBatch.assign(static_cast<size_t>(g_ADGraph->nBatchDirections),
                      Real(0.0));
  }

  v.dotBatch[static_cast<size_t>(k)] = value;
}

inline Real GetVertexDotBatch(const VertexId vertexId, const int k) {
  CheckDirectionalBatchIndex(k);

  if (vertexId >= g_ADGraph->vertices.size()) {
    throw std::out_of_range("GetVertexDotBatch: vertexId out of range");
  }

  const auto &v = g_ADGraph->vertices[vertexId];

  if (v.dotBatch.size() <= static_cast<size_t>(k)) {
    return Real(0.0);
  }

  return v.dotBatch[static_cast<size_t>(k)];
}

inline void SetARealDotBatch(AReal &x, const int k, const Real value) {
  SetVertexDotBatch(x.varId, k, value);

  if (x.dotBatch.size() != static_cast<size_t>(g_ADGraph->nBatchDirections)) {
    x.dotBatch.assign(static_cast<size_t>(g_ADGraph->nBatchDirections),
                      Real(0.0));
  }

  x.dotBatch[static_cast<size_t>(k)] = value;
}

inline void PushEdgeDotBatchValue(const size_t direction, const VertexId fo_to,
                                  const VertexId so_to, const Real valDot) {
  if (direction >= g_ADGraph->selfSoEdgesDotBatch.size() ||
      direction >= g_ADGraph->soEdgesDotBatch.size()) {
    std::cerr << "PushEdgeDotBatchValue direction out of range: direction="
              << direction
              << " self size=" << g_ADGraph->selfSoEdgesDotBatch.size()
              << " edge size=" << g_ADGraph->soEdgesDotBatch.size() << "\\n";
    std::abort();
  }

  if (fo_to == so_to) {
    auto &selfDots = g_ADGraph->selfSoEdgesDotBatch[direction];
    if (fo_to >= selfDots.size()) {
      std::cerr << "PushEdgeDotBatchValue self index out of range: direction="
                << direction << " vertex=" << fo_to
                << " size=" << selfDots.size() << "\\n";
      std::abort();
    }

    if (!AddBatchDirectionalSlotValue(direction, fo_to, fo_to,
                                      Real(2.0) * valDot)) {
      selfDots[fo_to] += Real(2.0) * valDot;
    }
  } else {
    const VertexId outer = std::max(fo_to, so_to);
    const VertexId inner = std::min(fo_to, so_to);

    auto &trees = g_ADGraph->soEdgesDotBatch[direction];

    if (outer >= trees.size()) {
      std::cerr << "PushEdgeDotBatchValue tree index out of range: direction="
                << direction << " outer=" << outer
                << " trees.size=" << trees.size()
                << " vertices.size=" << g_ADGraph->vertices.size() << "\\n";
      std::abort();
    }

    if (!AddBatchDirectionalSlotValue(direction, outer, inner, valDot)) {
      if (!AddFlatIntermediateDirectionalValue(direction, outer, inner,
                                               valDot)) {
        trees[outer].Insert(inner, valDot);
      }
    }
  }
}

inline void PushEdgeDotBatch(const size_t direction, const ADEdge &foEdge,
                             const ADEdge &soEdge, const Real foEdgeDot,
                             const Real soEdgeDot) {
  const Real valDot = foEdgeDot * soEdge.w + foEdge.w * soEdgeDot;

  if (direction >= g_ADGraph->selfSoEdgesDotBatch.size() ||
      direction >= g_ADGraph->soEdgesDotBatch.size()) {
    std::cerr << "PushEdgeDotBatch direction out of range: direction="
              << direction
              << " self size=" << g_ADGraph->selfSoEdgesDotBatch.size()
              << " edge size=" << g_ADGraph->soEdgesDotBatch.size() << "\n";
    std::abort();
  }

  if (foEdge.to == soEdge.to) {
    auto &selfDots = g_ADGraph->selfSoEdgesDotBatch[direction];
    if (foEdge.to >= selfDots.size()) {
      std::cerr << "PushEdgeDotBatch self index out of range: direction="
                << direction << " vertex=" << foEdge.to
                << " size=" << selfDots.size() << "\n";
      std::abort();
    }
    if (!AddBatchDirectionalSlotValue(direction, foEdge.to, foEdge.to,
                                      Real(2.0) * valDot)) {
      selfDots[foEdge.to] += Real(2.0) * valDot;
    }
  } else {
    const VertexId outer = std::max(foEdge.to, soEdge.to);
    const VertexId inner = std::min(foEdge.to, soEdge.to);

    auto &trees = g_ADGraph->soEdgesDotBatch[direction];

    if (outer >= trees.size()) {
      std::cerr << "PushEdgeDotBatch tree index out of range: direction="
                << direction << " outer=" << outer
                << " trees.size=" << trees.size()
                << " vertices.size=" << g_ADGraph->vertices.size() << "\n";
      std::abort();
    }

    if (!AddBatchDirectionalSlotValue(direction, outer, inner, valDot)) {
      trees[outer].Insert(inner, valDot);
    }
  }
}

inline Real GetAdjointDotBatch(const AReal &i, const AReal &j, const int k) {
  CheckDirectionalBatchIndex(k);

  if (g_ADGraph->soEdgesDotBatch.size() <= static_cast<size_t>(k) ||
      g_ADGraph->selfSoEdgesDotBatch.size() <= static_cast<size_t>(k)) {
    return Real(0.0);
  }

  if (i.varId == j.varId) {
    const auto &diag = g_ADGraph->selfSoEdgesDotBatch[static_cast<size_t>(k)];

    if (diag.size() <= i.varId) {
      return Real(0.0);
    }

    return diag[i.varId];
  }

  auto &trees = g_ADGraph->soEdgesDotBatch[static_cast<size_t>(k)];

  const VertexId outer = std::max(i.varId, j.varId);
  const VertexId inner = std::min(i.varId, j.varId);

  if (trees.size() <= outer) {
    return Real(0.0);
  }

  return trees[outer].Query(inner);
}

inline void PropagateAdjointDirectional() {
  const VertexId n_vertices = static_cast<VertexId>(g_ADGraph->vertices.size());

  if (g_ADGraph->soEdges.size() < g_ADGraph->vertices.size()) {
    g_ADGraph->soEdges.resize(g_ADGraph->vertices.size());
  } else {
    for (int i = 0; i < (int)g_ADGraph->soEdges.size(); ++i)
      g_ADGraph->soEdges[i].Clear();
  }

  if (g_ADGraph->soEdgesDot.size() < g_ADGraph->vertices.size()) {
    g_ADGraph->soEdgesDot.resize(g_ADGraph->vertices.size());
  } else {
    for (int i = 0; i < (int)g_ADGraph->soEdgesDot.size(); ++i)
      g_ADGraph->soEdgesDot[i].Clear();
  }

  g_ADGraph->selfSoEdges.assign(g_ADGraph->vertices.size(), Real(0.0));
  g_ADGraph->selfSoEdgesDot.assign(g_ADGraph->vertices.size(), Real(0.0));

  for (VertexId vid = n_vertices - 1; vid > 0; --vid) {
    ADVertex &vertex = g_ADGraph->vertices[vid];
    ADEdge &e1 = vertex.e1;
    ADEdge &e2 = vertex.e2;

    if (e1.to == vid)
      continue;

    // Push sparse off-diagonal Hessian edges and directional edges.
    BTree &btree = g_ADGraph->soEdges[vid];
    BTree &btreeDot = g_ADGraph->soEdgesDot[vid];

    for (auto it = btree.nodes.begin(); it != btree.nodes.end(); ++it) {
      ADEdge soEdge(it->key, it->val);
      const Real soDot = btreeDot.Query(it->key);

      PushEdge(e1, soEdge);
      PushEdgeDot(e1, soEdge, soDot);

      if (e2.to != vid) {
        PushEdge(e2, soEdge);
        PushEdgeDot(e2, soEdge, soDot);
      }
    }

    // Push diagonal Hessian entry.
    const Real S = g_ADGraph->selfSoEdges[vid];
    const Real SDot = g_ADGraph->selfSoEdgesDot[vid];

    if (S != Real(0.0) || SDot != Real(0.0)) {
      g_ADGraph->selfSoEdges[e1.to] += e1.w * e1.w * S;
      g_ADGraph->selfSoEdgesDot[e1.to] +=
          Real(2.0) * e1.w * e1.dw * S + e1.w * e1.w * SDot;

      if (e2.to != vid) {
        g_ADGraph->selfSoEdges[e2.to] += e2.w * e2.w * S;
        g_ADGraph->selfSoEdgesDot[e2.to] +=
            Real(2.0) * e2.w * e2.dw * S + e2.w * e2.w * SDot;

        const Real cross = e1.w * e2.w * S;
        const Real crossDot =
            (e1.dw * e2.w + e1.w * e2.dw) * S + e1.w * e2.w * SDot;

        if (e1.to == e2.to) {
          g_ADGraph->selfSoEdges[e1.to] += Real(2.0) * cross;
          g_ADGraph->selfSoEdgesDot[e1.to] += Real(2.0) * crossDot;
        } else {
          g_ADGraph->soEdges[std::max(e1.to, e2.to)].Insert(
              std::min(e1.to, e2.to), cross);
          g_ADGraph->soEdgesDot[std::max(e1.to, e2.to)].Insert(
              std::min(e1.to, e2.to), crossDot);
        }
      }
    }

    // Create local second-order contribution and its directional derivative.

    const Real a = vertex.w;
    const Real aDot = vertex.wDot;

    if ((a != Real(0.0) || aDot != Real(0.0)) &&
        (vertex.soW != Real(0.0) || vertex.soWDot != Real(0.0))) {
      const Real create = a * vertex.soW;
      const Real createDot = aDot * vertex.soW + a * vertex.soWDot;

      if (e2.to == vid) {
        g_ADGraph->selfSoEdges[e1.to] += create;
        g_ADGraph->selfSoEdgesDot[e1.to] += createDot;
      } else if (e1.to == e2.to) {
        g_ADGraph->selfSoEdges[e1.to] += Real(2.0) * create;
        g_ADGraph->selfSoEdgesDot[e1.to] += Real(2.0) * createDot;
      } else {
        g_ADGraph->soEdges[std::max(e1.to, e2.to)].Insert(
            std::min(e1.to, e2.to), create);
        g_ADGraph->soEdgesDot[std::max(e1.to, e2.to)].Insert(
            std::min(e1.to, e2.to), createDot);
      }
    }

    // Propagate first-order adjoints and directional adjoints.
    if (a != Real(0.0) || aDot != Real(0.0)) {
      vertex.w = Real(0.0);
      vertex.wDot = Real(0.0);

      g_ADGraph->vertices[e1.to].w += a * e1.w;
      g_ADGraph->vertices[e1.to].wDot += aDot * e1.w + a * e1.dw;

      if (e2.to != vid) {
        g_ADGraph->vertices[e2.to].w += a * e2.w;
        g_ADGraph->vertices[e2.to].wDot += aDot * e2.w + a * e2.dw;
      }
    }
  }
}

// Lightweight diagnostics for batched directional propagation.

inline std::uint64_t g_batch_query_count = 0;
inline std::uint64_t g_batch_pushdot_count = 0;
inline std::uint64_t g_batch_insert_count = 0;

inline void ResetBatchDirectionalCounters() {
  g_batch_query_count = 0;
  g_batch_pushdot_count = 0;
  g_batch_insert_count = 0;
}

inline double g_batch_workspace_init_ms = 0.0;
inline double g_batch_total_ms = 0.0;

inline void ResetBatchWorkspaceTiming() {
  g_batch_workspace_init_ms = 0.0;
  g_batch_total_ms = 0.0;
}

inline void PropagateDirectionalBatchForwardReplay() {
  const int nDirections = g_ADGraph->nBatchDirections;
  if (nDirections <= 0) {
    return;
  }

  const size_t batchSize = static_cast<size_t>(nDirections);
  const VertexId n_vertices = static_cast<VertexId>(g_ADGraph->vertices.size());

  for (VertexId vid = 0; vid < n_vertices; ++vid) {
    ADVertex &v = g_ADGraph->vertices[vid];

    if (v.dotBatch.size() != batchSize) {
      v.dotBatch.assign(batchSize, Real(0.0));
    }
    if (v.wDotBatch.size() != batchSize) {
      v.wDotBatch.assign(batchSize, Real(0.0));
    }
    if (v.soWDotBatch.size() != batchSize) {
      v.soWDotBatch.assign(batchSize, Real(0.0));
    }
    if (v.e1.dwBatch.size() != batchSize) {
      v.e1.dwBatch.assign(batchSize, Real(0.0));
    }
    if (v.e2.dwBatch.size() != batchSize) {
      v.e2.dwBatch.assign(batchSize, Real(0.0));
    }

    std::fill(v.wDotBatch.begin(), v.wDotBatch.end(), Real(0.0));
    std::fill(v.soWDotBatch.begin(), v.soWDotBatch.end(), Real(0.0));
    std::fill(v.e1.dwBatch.begin(), v.e1.dwBatch.end(), Real(0.0));
    std::fill(v.e2.dwBatch.begin(), v.e2.dwBatch.end(), Real(0.0));

    if (v.op == OpCode::Independent) {
      continue;
    }

    std::fill(v.dotBatch.begin(), v.dotBatch.end(), Real(0.0));

    const VertexId left = v.left;
    const VertexId right = v.right;

    const bool has_left = left < g_ADGraph->vertices.size();
    const bool has_right = right < g_ADGraph->vertices.size();

    const Real lp = has_left ? g_ADGraph->vertices[left].primal : Real(0.0);
    const Real rp = has_right ? g_ADGraph->vertices[right].primal : Real(0.0);
    const Real c = v.constant;

    for (int k = 0; k < nDirections; ++k) {
      const size_t kk = static_cast<size_t>(k);

      const Real ld =
          (has_left && kk < g_ADGraph->vertices[left].dotBatch.size())
              ? g_ADGraph->vertices[left].dotBatch[kk]
              : Real(0.0);

      const Real rd =
          (has_right && kk < g_ADGraph->vertices[right].dotBatch.size())
              ? g_ADGraph->vertices[right].dotBatch[kk]
              : Real(0.0);

      switch (v.op) {
      case OpCode::Add:
        v.dotBatch[kk] = ld + rd;
        break;

      case OpCode::AddConstant:
        v.dotBatch[kk] = ld;
        break;

      case OpCode::Subtract:
        v.dotBatch[kk] = ld - rd;
        break;

      case OpCode::SubtractConstant:
        v.dotBatch[kk] = ld;
        break;

      case OpCode::ConstantSubtract:
        v.dotBatch[kk] = -ld;
        break;

      case OpCode::Multiply:
        v.dotBatch[kk] = ld * rp + lp * rd;
        if (v.e1.to != vid) {
          v.e1.dwBatch[kk] = rd;
        }
        if (v.e2.to != vid) {
          v.e2.dwBatch[kk] = ld;
        }
        break;

      case OpCode::MultiplyConstant:
        v.dotBatch[kk] = c * ld;
        if (v.e1.to != vid) {
          v.e1.dwBatch[kk] = Real(0.0);
        }
        break;

      case OpCode::Divide:
        if (rp != Real(0.0)) {
          const Real inv = Real(1.0) / rp;
          const Real inv2 = inv * inv;
          const Real inv3 = inv2 * inv;
          v.dotBatch[kk] = (ld * rp - lp * rd) * inv2;

          if (v.e1.to != vid) {
            v.e1.dwBatch[kk] = -rd * inv2;
          }
          if (v.e2.to != vid) {
            v.e2.dwBatch[kk] = (-ld * inv2) + (Real(2.0) * lp * rd * inv3);
          }
          v.soWDotBatch[kk] = -rd * inv2;
        }
        break;

      case OpCode::DivideConstant:
        if (c != Real(0.0)) {
          v.dotBatch[kk] = ld / c;
        }
        break;

      case OpCode::ConstantDivide:
        if (lp != Real(0.0)) {
          const Real inv = Real(1.0) / lp;
          const Real inv2 = inv * inv;
          const Real inv3 = inv2 * inv;
          v.dotBatch[kk] = -c * ld * inv2;
          if (v.e1.to != vid) {
            v.e1.dwBatch[kk] = Real(2.0) * c * ld * inv3;
          }
          v.soWDotBatch[kk] = -Real(6.0) * c * ld * inv3 * inv;
        }
        break;

      case OpCode::Exp: {
        const Real ev = std::exp(lp);
        v.dotBatch[kk] = ev * ld;
        if (v.e1.to != vid) {
          v.e1.dwBatch[kk] = ev * ld;
        }
        v.soWDotBatch[kk] = ev * ld;
        break;
      }

      case OpCode::Log:
        if (lp != Real(0.0)) {
          const Real inv = Real(1.0) / lp;
          const Real inv2 = inv * inv;
          v.dotBatch[kk] = ld * inv;
          if (v.e1.to != vid) {
            v.e1.dwBatch[kk] = -ld * inv2;
          }
          v.soWDotBatch[kk] = Real(2.0) * ld * inv2 * inv;
        }
        break;

      case OpCode::Sqrt:
        if (lp > Real(0.0)) {
          const Real root = std::sqrt(lp);
          const Real inv_root = Real(1.0) / root;
          v.dotBatch[kk] = Real(0.5) * inv_root * ld;
          if (v.e1.to != vid) {
            v.e1.dwBatch[kk] = -Real(0.25) * inv_root / lp * ld;
          }
          v.soWDotBatch[kk] = Real(0.375) * inv_root / (lp * lp) * ld;
        }
        break;

      case OpCode::Negate:
        v.dotBatch[kk] = -ld;
        break;

      case OpCode::Independent:
      default:
        break;
      }
    }
  }
}

inline BTree &EnsureBatchDotTreeSlot(const size_t direction,
                                     const VertexId vertex, const char *site) {
  if (direction >= g_ADGraph->soEdgesDotBatch.size()) {
    std::cerr << "soEdgesDotBatch direction out of range at " << site
              << ": direction=" << direction
              << " size=" << g_ADGraph->soEdgesDotBatch.size() << "\n";
    std::abort();
  }

  auto &trees = g_ADGraph->soEdgesDotBatch[direction];

  if (vertex >= trees.size()) {
    std::cerr << "soEdgesDotBatch vertex out of range at " << site
              << ": direction=" << direction << " vertex=" << vertex
              << " trees.size=" << trees.size()
              << " vertices.size=" << g_ADGraph->vertices.size() << "\n";
    std::abort();
  }

  return trees[vertex];
}

inline bool BatchDirectionHasLocalSignal(const ADVertex &vertex,
                                         const ADEdge &e1, const ADEdge &e2,
                                         const VertexId vid, const size_t kk) {
  const Real e1dw = kk < e1.dwBatch.size() ? e1.dwBatch[kk] : Real(0.0);
  const Real e2dw = kk < e2.dwBatch.size() ? e2.dwBatch[kk] : Real(0.0);
  const Real aDot =
      kk < vertex.wDotBatch.size() ? vertex.wDotBatch[kk] : Real(0.0);
  const Real soWDot =
      kk < vertex.soWDotBatch.size() ? vertex.soWDotBatch[kk] : Real(0.0);

  Real selfDot = Real(0.0);
  if (kk < g_ADGraph->selfSoEdgesDotBatch.size() &&
      vid < g_ADGraph->selfSoEdgesDotBatch[kk].size()) {
    selfDot = g_ADGraph->selfSoEdgesDotBatch[kk][vid];
  }

  return e1dw != Real(0.0) || (e2.to != vid && e2dw != Real(0.0)) ||
         aDot != Real(0.0) || soWDot != Real(0.0) || selfDot != Real(0.0);
}

inline void ComputeBatchActiveDirectionMasks(const int nDirections) {
  const int cappedDirections = std::min(nDirections, 64);

  for (VertexId vid = 0;
       vid < static_cast<VertexId>(g_ADGraph->vertices.size()); ++vid) {
    ADVertex &vertex = g_ADGraph->vertices[vid];
    ADEdge &e1 = vertex.e1;
    ADEdge &e2 = vertex.e2;

    std::uint64_t mask = 0;

    for (int k = 0; k < cappedDirections; ++k) {
      const size_t kk = static_cast<size_t>(k);
      if (BatchDirectionHasLocalSignal(vertex, e1, e2, vid, kk)) {
        mask |= (std::uint64_t(1) << kk);
      }
    }

    vertex.batchActiveDirectionMask = mask;
  }
}

inline bool BatchDirectionMaskHasSignal(const ADVertex &vertex,
                                        const ADVertex & /*unused*/,
                                        const ADEdge &e1, const ADEdge &e2,
                                        const VertexId vid, const size_t kk) {
  if (kk < 64) {
    return (vertex.batchActiveDirectionMask & (std::uint64_t(1) << kk)) != 0;
  }

  return BatchDirectionHasLocalSignal(vertex, e1, e2, vid, kk);
}

inline void PropagateAdjointDirectionalBatch() {
  ResetFlatIntermediateDirectionalCounters();

  const auto batch_total_start = std::chrono::steady_clock::now();

  const int nDirections = g_ADGraph->nBatchDirections;
  ResetBatchDirectionalCounters();
  if (nDirections <= 0)
    return;

  const VertexId n_vertices = static_cast<VertexId>(g_ADGraph->vertices.size());

  if (g_ADGraph->soEdges.size() < g_ADGraph->vertices.size()) {
    g_ADGraph->soEdges.resize(g_ADGraph->vertices.size());
  } else {
    for (int i = 0; i < (int)g_ADGraph->soEdges.size(); ++i)
      g_ADGraph->soEdges[i].Clear();
  }

  g_ADGraph->selfSoEdges.assign(g_ADGraph->vertices.size(), Real(0.0));

  g_ADGraph->soEdgesDotBatch.resize(static_cast<size_t>(nDirections));
  g_ADGraph->selfSoEdgesDotBatch.resize(static_cast<size_t>(nDirections));

  for (int k = 0; k < nDirections; ++k) {
    auto &edgesDot = g_ADGraph->soEdgesDotBatch[static_cast<size_t>(k)];

    if (edgesDot.size() < g_ADGraph->vertices.size()) {
      edgesDot.resize(g_ADGraph->vertices.size());
    } else {
      for (int i = 0; i < (int)edgesDot.size(); ++i)
        edgesDot[i].Clear();
    }

    g_ADGraph->selfSoEdgesDotBatch[static_cast<size_t>(k)].assign(
        g_ADGraph->vertices.size(), Real(0.0));
  }

  const size_t batchSize = static_cast<size_t>(nDirections);

  const auto batch_workspace_init_start = std::chrono::steady_clock::now();

  for (auto &vertex : g_ADGraph->vertices) {
    if (vertex.wDotBatch.size() != batchSize)
      vertex.wDotBatch.assign(batchSize, Real(0.0));
    else
      std::fill(vertex.wDotBatch.begin(), vertex.wDotBatch.end(), Real(0.0));

    if (vertex.soWDotBatch.size() != batchSize)
      vertex.soWDotBatch.assign(batchSize, Real(0.0));
    else
      std::fill(vertex.soWDotBatch.begin(), vertex.soWDotBatch.end(),
                Real(0.0));

    if (vertex.e1.dwBatch.size() != batchSize)
      vertex.e1.dwBatch.assign(batchSize, Real(0.0));
    else
      std::fill(vertex.e1.dwBatch.begin(), vertex.e1.dwBatch.end(), Real(0.0));

    if (vertex.e2.dwBatch.size() != batchSize)
      vertex.e2.dwBatch.assign(batchSize, Real(0.0));
    else
      std::fill(vertex.e2.dwBatch.begin(), vertex.e2.dwBatch.end(), Real(0.0));
  }

  const auto batch_workspace_init_end = std::chrono::steady_clock::now();
  g_batch_workspace_init_ms =
      std::chrono::duration<double, std::milli>(batch_workspace_init_end -
                                                batch_workspace_init_start)
          .count();

  PropagateDirectionalBatchForwardReplay();
  ClearBatchDirectionalSlotValues();
  ClearFlatIntermediateDirectionalValues();

  // Defensive storage normalization before reverse sweep. The directional
  // reverse pass inserts into soEdgesDotBatch[direction][vertex], so every
  // direction must own a full vector of BTree slots.
  g_ADGraph->soEdgesDotBatch.resize(static_cast<size_t>(nDirections));
  g_ADGraph->selfSoEdgesDotBatch.resize(static_cast<size_t>(nDirections));

  for (int k = 0; k < nDirections; ++k) {
    auto &edgeDots = g_ADGraph->soEdgesDotBatch[static_cast<size_t>(k)];
    if (edgeDots.size() < g_ADGraph->vertices.size()) {
      edgeDots.resize(g_ADGraph->vertices.size());
    }

    auto &selfDots = g_ADGraph->selfSoEdgesDotBatch[static_cast<size_t>(k)];
    if (selfDots.size() < g_ADGraph->vertices.size()) {
      selfDots.resize(g_ADGraph->vertices.size(), Real(0.0));
    }
  }

  EnableFlatIntermediateDirectionalBackend();

  ComputeBatchActiveDirectionMasks(nDirections);

  for (VertexId vid = n_vertices - 1; vid > 0; --vid) {
    ADVertex &vertex = g_ADGraph->vertices[vid];
    ADEdge &e1 = vertex.e1;
    ADEdge &e2 = vertex.e2;

    if (e1.to == vid)
      continue;

    BTree &btree = g_ADGraph->soEdges[vid];

    for (auto it = btree.nodes.begin(); it != btree.nodes.end(); ++it) {
      ADEdge soEdge(it->key, it->val);

      PushEdge(e1, soEdge);

      if (e2.to != vid) {
        PushEdge(e2, soEdge);
      }

      for (int k = 0; k < nDirections; ++k) {
        const size_t kk = static_cast<size_t>(k);
        Real soDot = Real(0.0);

        if (!TryGetBatchDirectionalSlotValue(kk, vid, it->key, soDot)) {
          if (g_ADGraph->useFlatIntermediateDirectionalBackend) {
            // A flat miss means no directional value was written for this edge.
            // Since all patched writes use the flat backend, leave soDot = 0.
            (void)TryGetFlatIntermediateDirectionalValue(
                kk, vid, static_cast<VertexId>(it->key), soDot);
          } else {
            BTree &btreeDot = g_ADGraph->soEdgesDotBatch[kk][vid];
            ++g_batch_query_count;
            soDot = btreeDot.Query(it->key);
          }
        }

        const Real e1dw_k = kk < e1.dwBatch.size() ? e1.dwBatch[kk] : Real(0.0);
        const Real e1ValDot = e1dw_k * soEdge.w + e1.w * soDot;
        if (e1ValDot != Real(0.0)) {
          ++g_batch_pushdot_count;
          PushEdgeDotBatchValue(kk, e1.to, soEdge.to, e1ValDot);
        }

        if (e2.to != vid) {
          const Real e2dw_k =
              kk < e2.dwBatch.size() ? e2.dwBatch[kk] : Real(0.0);
          const Real e2ValDot = e2dw_k * soEdge.w + e2.w * soDot;
          if (e2ValDot != Real(0.0)) {
            ++g_batch_pushdot_count;
            PushEdgeDotBatchValue(kk, e2.to, soEdge.to, e2ValDot);
          }
        }
      }
    }

    const Real S = g_ADGraph->selfSoEdges[vid];

    if (S != Real(0.0)) {
      g_ADGraph->selfSoEdges[e1.to] += e1.w * e1.w * S;

      if (e2.to != vid) {
        g_ADGraph->selfSoEdges[e2.to] += e2.w * e2.w * S;

        const Real cross = e1.w * e2.w * S;

        if (e1.to == e2.to) {
          g_ADGraph->selfSoEdges[e1.to] += Real(2.0) * cross;
        } else {
          g_ADGraph->soEdges[std::max(e1.to, e2.to)].Insert(
              std::min(e1.to, e2.to), cross);
        }
      }
    }

    for (int k = 0; k < nDirections; ++k) {
      const size_t kk = static_cast<size_t>(k);
      if (!BatchDirectionMaskHasSignal(vertex, vertex, e1, e2, vid, kk)) {
        continue;
      }

      const Real SDot = g_ADGraph->selfSoEdgesDotBatch[kk][vid];

      if (S != Real(0.0) || SDot != Real(0.0)) {
        const Real e1SelfDot =
            Real(2.0) * e1.w * e1.dwBatch[kk] * S + e1.w * e1.w * SDot;
        if (!AddBatchDirectionalSlotValue(kk, e1.to, e1.to, e1SelfDot)) {
          g_ADGraph->selfSoEdgesDotBatch[kk][e1.to] += e1SelfDot;
        }

        if (e2.to != vid) {
          const Real e2SelfDot =
              Real(2.0) * e2.w * e2.dwBatch[kk] * S + e2.w * e2.w * SDot;
          if (!AddBatchDirectionalSlotValue(kk, e2.to, e2.to, e2SelfDot)) {
            g_ADGraph->selfSoEdgesDotBatch[kk][e2.to] += e2SelfDot;
          }

          const Real crossDot =
              (e1.dwBatch[kk] * e2.w + e1.w * e2.dwBatch[kk]) * S +
              e1.w * e2.w * SDot;

          if (e1.to == e2.to) {
            if (!AddBatchDirectionalSlotValue(kk, e1.to, e1.to,
                                              Real(2.0) * crossDot)) {
              g_ADGraph->selfSoEdgesDotBatch[kk][e1.to] += Real(2.0) * crossDot;
            }
          } else {
            if (!AddBatchDirectionalSlotValue(kk, e1.to, e2.to, crossDot)) {
              ++g_batch_insert_count;
              EnsureBatchDotTreeSlot(kk, std::max(e1.to, e2.to), "crossDot")
                  .Insert(std::min(e1.to, e2.to), crossDot);
            }
          }
        }
      }
    }

    const Real a = vertex.w;

    if (a != Real(0.0) && vertex.soW != Real(0.0)) {
      const Real create = a * vertex.soW;

      if (e2.to == vid) {
        g_ADGraph->selfSoEdges[e1.to] += create;
      } else if (e1.to == e2.to) {
        g_ADGraph->selfSoEdges[e1.to] += Real(2.0) * create;
      } else {
        g_ADGraph->soEdges[std::max(e1.to, e2.to)].Insert(
            std::min(e1.to, e2.to), create);
      }
    }

    for (int k = 0; k < nDirections; ++k) {
      const size_t kk = static_cast<size_t>(k);
      if (!BatchDirectionMaskHasSignal(vertex, vertex, e1, e2, vid, kk)) {
        continue;
      }

      const Real aDot = vertex.wDotBatch[kk];

      if ((a != Real(0.0) || aDot != Real(0.0)) &&
          (vertex.soW != Real(0.0) || vertex.soWDotBatch[kk] != Real(0.0))) {
        const Real createDot = aDot * vertex.soW + a * vertex.soWDotBatch[kk];

        if (e2.to == vid) {
          if (!AddBatchDirectionalSlotValue(kk, e1.to, e1.to, createDot)) {
            g_ADGraph->selfSoEdgesDotBatch[kk][e1.to] += createDot;
          }
        } else if (e1.to == e2.to) {
          if (!AddBatchDirectionalSlotValue(kk, e1.to, e1.to,
                                            Real(2.0) * createDot)) {
            g_ADGraph->selfSoEdgesDotBatch[kk][e1.to] += Real(2.0) * createDot;
          }
        } else {
          ++g_batch_insert_count;
          EnsureBatchDotTreeSlot(kk, std::max(e1.to, e2.to), "createDot")
              .Insert(std::min(e1.to, e2.to), createDot);
        }
      }
    }

    if (a != Real(0.0)) {
      vertex.w = Real(0.0);

      g_ADGraph->vertices[e1.to].w += a * e1.w;

      if (e2.to != vid) {
        g_ADGraph->vertices[e2.to].w += a * e2.w;
      }
    }

    for (int k = 0; k < nDirections; ++k) {
      const size_t kk = static_cast<size_t>(k);
      if (!BatchDirectionMaskHasSignal(vertex, vertex, e1, e2, vid, kk)) {
        continue;
      }

      const Real aDot = vertex.wDotBatch[kk];

      if (a != Real(0.0) || aDot != Real(0.0)) {
        vertex.wDotBatch[kk] = Real(0.0);

        g_ADGraph->vertices[e1.to].wDotBatch[kk] +=
            aDot * e1.w + a * e1.dwBatch[kk];

        if (e2.to != vid) {
          g_ADGraph->vertices[e2.to].wDotBatch[kk] +=
              aDot * e2.w + a * e2.dwBatch[kk];
        }
      }
    }
  }

  const auto batch_total_end = std::chrono::steady_clock::now();
  g_batch_total_ms = std::chrono::duration<double, std::milli>(
                         batch_total_end - batch_total_start)
                         .count();
}

inline void PropagateAdjoint() {
  if (g_ADGraph->vertices.size() > g_ADGraph->soEdges.size()) {
    g_ADGraph->soEdges.resize(g_ADGraph->vertices.size());
  } else {
    for (int i = 0; i < (int)g_ADGraph->soEdges.size(); i++) {
      g_ADGraph->soEdges[i].Clear();
    }
  }
  g_ADGraph->selfSoEdges.resize(g_ADGraph->vertices.size(), Real(0.0));
  // Any chance for SSE/AVX parallism?

  for (VertexId vid = g_ADGraph->vertices.size() - 1; vid > 0; vid--) {
    ADVertex &vertex = g_ADGraph->vertices[vid];
    ADEdge &e1 = vertex.e1;
    ADEdge &e2 = vertex.e2;
    if (e1.to == vid) {
      continue;
    }

    // Pushing
    BTree &btree = g_ADGraph->soEdges[vid];
    std::vector<BTNode>::iterator it;
    if (e2.to == vid) {
      for (it = btree.nodes.begin(); it != btree.nodes.end(); it++) {
        ADEdge soEdge(it->key, it->val);
        PushEdge(e1, soEdge);
      }
    } else {
      for (it = btree.nodes.begin(); it != btree.nodes.end(); it++) {
        ADEdge soEdge(it->key, it->val);
        PushEdge(e1, soEdge);
        PushEdge(e2, soEdge);
      }
    }
    if (g_ADGraph->selfSoEdges[vid] != Real(0.0)) {
      g_ADGraph->selfSoEdges[e1.to] +=
          e1.w * e1.w * g_ADGraph->selfSoEdges[vid];
      if (e2.to != vid) {
        g_ADGraph->selfSoEdges[e2.to] +=
            e2.w * e2.w * g_ADGraph->selfSoEdges[vid];
        if (e1.to == e2.to) {
          g_ADGraph->selfSoEdges[e2.to] +=
              Real(2.0) * e1.w * e2.w * g_ADGraph->selfSoEdges[vid];
        } else {
          g_ADGraph->soEdges[std::max(e1.to, e2.to)].Insert(
              std::min(e1.to, e2.to),
              e1.w * e2.w * g_ADGraph->selfSoEdges[vid]);
        }
      }
    }

    // release memory?

    Real a = vertex.w;
    if (a != Real(0.0)) {
      // Creating
      if (vertex.soW != Real(0.0)) {
        if (e2.to == vid) { // single-edge
          g_ADGraph->selfSoEdges[e1.to] += a * vertex.soW;
        } else if (e1.to == e2.to) {
          g_ADGraph->selfSoEdges[e1.to] += Real(2.0) * a * vertex.soW;
        } else {
          g_ADGraph->soEdges[std::max(e1.to, e2.to)].Insert(
              std::min(e1.to, e2.to), a * vertex.soW);
        }
      }
      // Adjoint
      vertex.w = Real(0.0);
      g_ADGraph->vertices[e1.to].w += a * e1.w;
      if (e2.to != vid) {
        g_ADGraph->vertices[e2.to].w += a * e2.w;
      }
    }
  }
}

////////////////// Quadra third-order extension API //////////////////

typedef std::vector<std::vector<Real>> DenseMatrix;

struct ValueGradientHessian {
  Real value = Real(0.0);
  std::vector<Real> gradient;
  DenseMatrix hessian;
};

template <typename Func>
inline ValueGradientHessian
evaluate_value_gradient_hessian(Func &&f, const std::vector<Real> &x) {
  ADGraph graph;
  std::vector<AReal> ax;
  ax.reserve(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) {
    ax.push_back(AReal(x[i]));
  }

  AReal y = f(ax);
  SetAdjoint(y, Real(1.0));
  PropagateAdjoint();

  ValueGradientHessian out;
  out.value = y.val;
  out.gradient.resize(x.size());
  out.hessian.assign(x.size(), std::vector<Real>(x.size(), Real(0.0)));
  for (std::size_t i = 0; i < x.size(); ++i) {
    out.gradient[i] = GetAdjoint(ax[i]);
    for (std::size_t j = 0; j < x.size(); ++j) {
      out.hessian[i][j] = GetAdjoint(ax[i], ax[j]);
    }
  }
  return out;
}

struct ThirdOrderScalar {
  Real val;
  Real d1;
  Real d2;
  Real d3;

  ThirdOrderScalar()
      : val(Real(0.0)), d1(Real(0.0)), d2(Real(0.0)), d3(Real(0.0)) {}
  ThirdOrderScalar(const Real v)
      : val(v), d1(Real(0.0)), d2(Real(0.0)), d3(Real(0.0)) {}
  ThirdOrderScalar(const Real v, const Real direction)
      : val(v), d1(direction), d2(Real(0.0)), d3(Real(0.0)) {}
};

struct DirectionalDerivatives3 {
  Real value = Real(0.0);
  Real first = Real(0.0);  // df(x)[d]
  Real second = Real(0.0); // d^T H(x) d
  Real third = Real(0.0);  // D^3 f(x)[d,d,d]
};

inline ThirdOrderScalar make_third_order_seed(const Real value,
                                              const Real direction) {
  return ThirdOrderScalar(value, direction);
}

inline ThirdOrderScalar unary_chain(const ThirdOrderScalar &x, const Real value,
                                    const Real fp, const Real fpp,
                                    const Real fppp) {
  ThirdOrderScalar y;
  y.val = value;
  y.d1 = fp * x.d1;
  y.d2 = fpp * x.d1 * x.d1 + fp * x.d2;
  y.d3 = fppp * x.d1 * x.d1 * x.d1 + Real(3.0) * fpp * x.d1 * x.d2 + fp * x.d3;
  return y;
}

inline ThirdOrderScalar operator+(const ThirdOrderScalar &a,
                                  const ThirdOrderScalar &b) {
  ThirdOrderScalar y;
  y.val = a.val + b.val;
  y.d1 = a.d1 + b.d1;
  y.d2 = a.d2 + b.d2;
  y.d3 = a.d3 + b.d3;
  return y;
}
inline ThirdOrderScalar operator+(const ThirdOrderScalar &a, const Real b) {
  return a + ThirdOrderScalar(b);
}
inline ThirdOrderScalar operator+(const Real a, const ThirdOrderScalar &b) {
  return ThirdOrderScalar(a) + b;
}
inline ThirdOrderScalar &operator+=(ThirdOrderScalar &a,
                                    const ThirdOrderScalar &b) {
  return (a = a + b);
}
inline ThirdOrderScalar &operator+=(ThirdOrderScalar &a, const Real b) {
  return (a = a + b);
}

inline ThirdOrderScalar operator-(const ThirdOrderScalar &a,
                                  const ThirdOrderScalar &b) {
  ThirdOrderScalar y;
  y.val = a.val - b.val;
  y.d1 = a.d1 - b.d1;
  y.d2 = a.d2 - b.d2;
  y.d3 = a.d3 - b.d3;
  return y;
}
inline ThirdOrderScalar operator-(const ThirdOrderScalar &a, const Real b) {
  return a - ThirdOrderScalar(b);
}
inline ThirdOrderScalar operator-(const Real a, const ThirdOrderScalar &b) {
  return ThirdOrderScalar(a) - b;
}
inline ThirdOrderScalar operator-(const ThirdOrderScalar &x) {
  ThirdOrderScalar y;
  y.val = -x.val;
  y.d1 = -x.d1;
  y.d2 = -x.d2;
  y.d3 = -x.d3;
  return y;
}
inline ThirdOrderScalar &operator-=(ThirdOrderScalar &a,
                                    const ThirdOrderScalar &b) {
  return (a = a - b);
}
inline ThirdOrderScalar &operator-=(ThirdOrderScalar &a, const Real b) {
  return (a = a - b);
}

inline ThirdOrderScalar operator*(const ThirdOrderScalar &a,
                                  const ThirdOrderScalar &b) {
  ThirdOrderScalar y;
  y.val = a.val * b.val;
  y.d1 = a.d1 * b.val + a.val * b.d1;
  y.d2 = a.d2 * b.val + Real(2.0) * a.d1 * b.d1 + a.val * b.d2;
  y.d3 = a.d3 * b.val + Real(3.0) * a.d2 * b.d1 + Real(3.0) * a.d1 * b.d2 +
         a.val * b.d3;
  return y;
}
inline ThirdOrderScalar operator*(const ThirdOrderScalar &a, const Real b) {
  ThirdOrderScalar y;
  y.val = a.val * b;
  y.d1 = a.d1 * b;
  y.d2 = a.d2 * b;
  y.d3 = a.d3 * b;
  return y;
}
inline ThirdOrderScalar operator*(const Real a, const ThirdOrderScalar &b) {
  return b * a;
}
inline ThirdOrderScalar &operator*=(ThirdOrderScalar &a,
                                    const ThirdOrderScalar &b) {
  return (a = a * b);
}
inline ThirdOrderScalar &operator*=(ThirdOrderScalar &a, const Real b) {
  return (a = a * b);
}

inline ThirdOrderScalar Inv(const ThirdOrderScalar &x) {
  const Real invX = Real(1.0) / x.val;
  const Real invX2 = invX * invX;
  const Real invX3 = invX2 * invX;
  const Real invX4 = invX3 * invX;
  return unary_chain(x, invX, -invX2, Real(2.0) * invX3, -Real(6.0) * invX4);
}

inline ThirdOrderScalar operator/(const ThirdOrderScalar &a,
                                  const ThirdOrderScalar &b) {
  return a * Inv(b);
}
inline ThirdOrderScalar operator/(const ThirdOrderScalar &a, const Real b) {
  return a * (Real(1.0) / b);
}
inline ThirdOrderScalar operator/(const Real a, const ThirdOrderScalar &b) {
  return a * Inv(b);
}
inline ThirdOrderScalar &operator/=(ThirdOrderScalar &a,
                                    const ThirdOrderScalar &b) {
  return (a = a / b);
}
inline ThirdOrderScalar &operator/=(ThirdOrderScalar &a, const Real b) {
  return (a = a / b);
}

inline ThirdOrderScalar square(const ThirdOrderScalar &x) { return x * x; }

inline ThirdOrderScalar sqrt(const ThirdOrderScalar &x) {
  const Real sqrtX = std::sqrt(x.val);
  const Real invSqrtX = Real(1.0) / sqrtX;
  return unary_chain(x, sqrtX, Real(0.5) * invSqrtX,
                     -Real(0.25) * invSqrtX / x.val,
                     Real(0.375) * invSqrtX / (x.val * x.val));
}

inline ThirdOrderScalar pow(const ThirdOrderScalar &x, const Real a) {
  return unary_chain(x, std::pow(x.val, a), a * std::pow(x.val, a - Real(1.0)),
                     a * (a - Real(1.0)) * std::pow(x.val, a - Real(2.0)),
                     a * (a - Real(1.0)) * (a - Real(2.0)) *
                         std::pow(x.val, a - Real(3.0)));
}

inline ThirdOrderScalar exp(const ThirdOrderScalar &x) {
  const Real expX = std::exp(x.val);
  return unary_chain(x, expX, expX, expX, expX);
}

inline ThirdOrderScalar log(const ThirdOrderScalar &x) {
  const Real invX = Real(1.0) / x.val;
  return unary_chain(x, std::log(x.val), invX, -invX * invX,
                     Real(2.0) * invX * invX * invX);
}

inline ThirdOrderScalar sin(const ThirdOrderScalar &x) {
  return unary_chain(x, std::sin(x.val), std::cos(x.val), -std::sin(x.val),
                     -std::cos(x.val));
}

inline ThirdOrderScalar cos(const ThirdOrderScalar &x) {
  return unary_chain(x, std::cos(x.val), -std::sin(x.val), -std::cos(x.val),
                     std::sin(x.val));
}

inline ThirdOrderScalar tan(const ThirdOrderScalar &x) {
  const Real tanX = std::tan(x.val);
  const Real secX = Real(1.0) / std::cos(x.val);
  const Real sec2X = secX * secX;
  return unary_chain(x, tanX, sec2X, Real(2.0) * tanX * sec2X,
                     Real(2.0) * sec2X * sec2X +
                         Real(4.0) * tanX * tanX * sec2X);
}

inline ThirdOrderScalar asin(const ThirdOrderScalar &x) {
  const Real tmp = Real(1.0) / (Real(1.0) - x.val * x.val);
  const Real sqrtTmp = std::sqrt(tmp);
  return unary_chain(x, std::asin(x.val), sqrtTmp, x.val * sqrtTmp * tmp,
                     (Real(1.0) + Real(2.0) * x.val * x.val) * sqrtTmp * tmp *
                         tmp);
}

inline ThirdOrderScalar acos(const ThirdOrderScalar &x) {
  const Real tmp = Real(1.0) / (Real(1.0) - x.val * x.val);
  const Real sqrtTmp = std::sqrt(tmp);
  return unary_chain(x, std::acos(x.val), -sqrtTmp, -x.val * sqrtTmp * tmp,
                     -(Real(1.0) + Real(2.0) * x.val * x.val) * sqrtTmp * tmp *
                         tmp);
}

template <typename Func>
inline DirectionalDerivatives3
evaluate_directional_derivatives3(Func &&f, const std::vector<Real> &x,
                                  const std::vector<Real> &direction) {
  if (x.size() != direction.size()) {
    throw std::invalid_argument("evaluate_directional_derivatives3: x and "
                                "direction must have the same length");
  }
  std::vector<ThirdOrderScalar> ax;
  ax.reserve(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) {
    ax.push_back(make_third_order_seed(x[i], direction[i]));
  }
  ThirdOrderScalar y = f(ax);
  DirectionalDerivatives3 out;
  out.value = y.val;
  out.first = y.d1;
  out.second = y.d2;
  out.third = y.d3;
  return out;
}

template <typename Func>
inline Real third_directional_derivative(Func &&f, const std::vector<Real> &x,
                                         const std::vector<Real> &direction) {
  return evaluate_directional_derivatives3(std::forward<Func>(f), x, direction)
      .third;
}

template <typename Func>
inline DenseMatrix hessian_directional_derivative_central_difference(
    Func &&f, const std::vector<Real> &x, const std::vector<Real> &direction,
    Real eps = Real(1e-5)) {
  if (x.size() != direction.size()) {
    throw std::invalid_argument(
        "hessian_directional_derivative_central_difference: x and direction "
        "must have the same length");
  }
  std::vector<Real> xp = x;
  std::vector<Real> xm = x;
  for (std::size_t i = 0; i < x.size(); ++i) {
    xp[i] += eps * direction[i];
    xm[i] -= eps * direction[i];
  }
  ValueGradientHessian hp = evaluate_value_gradient_hessian(f, xp);
  ValueGradientHessian hm = evaluate_value_gradient_hessian(f, xm);
  DenseMatrix out = hp.hessian;
  for (std::size_t i = 0; i < out.size(); ++i) {
    for (std::size_t j = 0; j < out[i].size(); ++j) {
      out[i][j] = (hp.hessian[i][j] - hm.hessian[i][j]) / (Real(2.0) * eps);
    }
  }
  return out;
}

} // namespace had

#endif // HAD_QUADRA_H__
