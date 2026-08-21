/**
 * \file quadra.cpp
 * \brief Quadra backend linkage and R interface for the FIMS shared library.
 */

#ifdef QUADRA_MODEL

#include <memory>

#include <Rcpp.h>

#include <quadra/quadra.hpp>

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

}  // namespace fims_quadra

namespace {

Rcpp::List quadra_backend_test(const Rcpp::NumericVector &parameters) {
  if (parameters.size() != 2) {
    Rcpp::stop("Quadra backend test expects exactly two parameters.");
  }

  fims_quadra::reset_tape();
  std::vector<quadra::AD> x =
      quadra::to_ad(std::vector<double>{parameters[0], parameters[1]});
  quadra::AD objective = x[0] * x[0] + 3.0 * x[1];

  had::Forward(*had::g_ADGraph);
  had::ZeroAdjoints(*had::g_ADGraph);
  had::SetAdjoint(objective, 1.0);
  had::PropagateAdjoint();

  Eigen::VectorXd gradient = quadra::extract_gradient(x);
  return Rcpp::List::create(
      Rcpp::Named("objective") = quadra::value_of(objective),
      Rcpp::Named("gradient") =
          Rcpp::NumericVector::create(gradient[0], gradient[1]),
      Rcpp::Named("backend") = "Quadra");
}

}  // namespace

void register_quadra(Rcpp::Module &m) {
  Rcpp::function(
      "quadra_backend_test", &quadra_backend_test,
      "Verify that Quadra automatic differentiation is linked into FIMS.");
}

#endif  // QUADRA_MODEL
