#include "gtest/gtest.h"
#include "core/had_quadra.hpp"

DECLARE_ADGRAPH()

TEST(QuadraBatchWorkspace, PropagatesContiguousDirectionalState) {
  had::ADGraph graph;
  had::g_ADGraph = &graph;

  had::AReal x(3.0);
  had::AReal objective = x * x;

  had::ResizeDirectionalBatch(2);
  had::SetARealDotBatch(x, 0, 1.0);
  had::SetARealDotBatch(x, 1, -2.0);
  had::PropagateDirectionalBatchForwardReplay();

  EXPECT_DOUBLE_EQ(had::GetVertexDotBatch(objective.varId, 0), 6.0);
  EXPECT_DOUBLE_EQ(had::GetVertexDotBatch(objective.varId, 1), -12.0);

  had::ZeroAdjoints(graph);
  graph.vertices[objective.varId].w = 1.0;
  had::PropagateAdjoint();
  graph.vertices[objective.varId].w = 1.0;
  had::PropagateAdjointDirectionalBatch();

  EXPECT_DOUBLE_EQ(had::VertexWDotBatch(graph, x.varId, 0), 2.0);
  EXPECT_DOUBLE_EQ(had::VertexWDotBatch(graph, x.varId, 1), -4.0);
}

TEST(QuadraBatchWorkspace, KeepsOptionalBatchStateOutOfVertices) {
  EXPECT_LE(sizeof(had::ADVertex), 128u);
}
