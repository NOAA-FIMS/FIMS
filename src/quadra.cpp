/**
 * \file quadra.cpp
 * \brief Quadra backend linkage for the FIMS shared library.
 */

#ifdef QUADRA_MODEL

#include <memory>

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

#endif  // QUADRA_MODEL
