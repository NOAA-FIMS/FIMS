#ifndef FIMS_FORWARD_PASS_HPP
#define FIMS_FORWARD_PASS_HPP

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace fims {

// Iteration structure of a calculation.
// PER_YEAR_AGE: for age { for year { step(i, i_prev, year, age) } }
//   - index convention matches existing FIMS: i = age * n_years + year
//   - age-outer order ensures (age-1, year-1) is already computed when
//     (age, year) runs, satisfying cohort recurrence dependencies
// PER_YEAR: for year { step(year) }
//   - post-processing after the main age loop (e.g., SSB ratio, recruitment)
// SCALAR: step called once per Evaluate()
enum class LoopDomain { PER_YEAR_AGE, PER_YEAR, SCALAR };

// Rule for computing a calculation's output vector size from model dimensions.
// This replaces the model-type-specific sizing blocks in add_to_fims_tmb_internal:
// instead of CatchAtAgeInterface hard-coding mortality_Z as n_ages*n_years and
// SurplusProductionInterface hard-coding biomass as n_years+1, each CalcDescriptor
// carries the rule that determines its output size for any model configuration.
//
// BROADCAST_INPUTS: output size = broadcast of all required_input sizes.
//   Scalar yields to anything; by-age and by-year combine to full age×year.
//   Used when output granularity inherits from inputs (e.g., mortality_Z).
// PER_YEAR:              output size = n_years
// PER_YEAR_PLUS_ONE:     output size = n_years + 1  (e.g., biomass, spawning_biomass)
// PER_AGE_YEAR:          output size = n_ages * n_years
// PER_AGE_YEAR_PLUS_ONE: output size = n_ages * (n_years + 1)  (e.g., numbers_at_age)
// SCALAR_OUT:            output size = 1  (e.g., reference points fmsy, bmsy)
enum class DimRule {
  BROADCAST_INPUTS,
  PER_YEAR,
  PER_YEAR_PLUS_ONE,
  PER_AGE_YEAR,
  PER_AGE_YEAR_PLUS_ONE,
  SCALAR_OUT
};

// Static metadata for one calculation block.
// required_inputs / produced_outputs are quantity names in the shared
// derived quantities map. The forward pass uses these to determine
// execution order; the calculation methods themselves use the map directly.
struct CalcDescriptor {
  std::string name;
  std::vector<std::string> required_inputs;
  std::vector<std::string> produced_outputs;
  LoopDomain domain;
  DimRule output_dim_rule;
};

// Resolved size and dimension metadata for one output derived quantity,
// computed by run_forward_pass() and passed to add_to_fims_tmb_internal()
// to initialize derived quantity vectors — no model-type knowledge needed.
struct ResolvedOutput {
  std::string name;
  size_t size;                        // total number of elements
  std::vector<int> dims;              // shape, e.g. {n_years, n_ages}
  std::vector<std::string> dim_names; // labels, e.g. {"n_years", "n_ages"}
};

// One resolved node in the execution plan.
// Multiple calc_names share one loop when fused (same domain,
// consecutive in execution order).
struct KernelDescriptor {
  std::vector<std::string> calc_names;
  LoopDomain domain;
  std::vector<ResolvedOutput> outputs; // resolved derived quantity sizes for
                                       // this kernel; consumed by
                                       // add_to_fims_tmb_internal to initialize
                                       // derived quantity vectors
};

// -----------------------------------------------------------------------
// broadcast_size
//
// Returns the output size when combining two quantities of potentially
// different dimensions. Scalar (size 1) yields to anything larger;
// by-age and by-year combine to full age×year.
// -----------------------------------------------------------------------
inline size_t broadcast_size(size_t a, size_t b,
                             size_t n_years, size_t n_ages) {
  size_t full = n_ages * n_years;
  if (a == full || b == full) return full;
  if ((a == n_ages && b == n_years) || (a == n_years && b == n_ages))
    return full;
  return std::max(a, b);
}

// -----------------------------------------------------------------------
// resolve_output
//
// Computes the size and dimension metadata for one output derived quantity
// given the calculation's DimRule and the sizes of its inputs as currently
// known in the forward pass.
// -----------------------------------------------------------------------
inline ResolvedOutput resolve_output(
    const std::string& output_name,
    DimRule rule,
    const std::unordered_map<std::string, size_t>& available_sizes,
    const std::vector<std::string>& required_inputs,
    size_t n_years, size_t n_ages) {

  ResolvedOutput out;
  out.name = output_name;

  switch (rule) {
    case DimRule::BROADCAST_INPUTS: {
      size_t sz = 1;
      for (const auto& input : required_inputs) {
        auto it = available_sizes.find(input);
        if (it != available_sizes.end())
          sz = broadcast_size(sz, it->second, n_years, n_ages);
      }
      out.size = sz;
      if (sz == 1) {
        out.dims = {1};
        out.dim_names = {"scalar"};
      } else if (sz == n_years) {
        out.dims = {(int)n_years};
        out.dim_names = {"n_years"};
      } else if (sz == n_ages) {
        out.dims = {(int)n_ages};
        out.dim_names = {"n_ages"};
      } else {
        out.dims = {(int)n_years, (int)n_ages};
        out.dim_names = {"n_years", "n_ages"};
      }
      break;
    }
    case DimRule::PER_YEAR:
      out.size = n_years;
      out.dims = {(int)n_years};
      out.dim_names = {"n_years"};
      break;
    case DimRule::PER_YEAR_PLUS_ONE:
      out.size = n_years + 1;
      out.dims = {(int)(n_years + 1)};
      out.dim_names = {"n_years+1"};
      break;
    case DimRule::PER_AGE_YEAR:
      out.size = n_ages * n_years;
      out.dims = {(int)n_years, (int)n_ages};
      out.dim_names = {"n_years", "n_ages"};
      break;
    case DimRule::PER_AGE_YEAR_PLUS_ONE:
      out.size = n_ages * (n_years + 1);
      out.dims = {(int)(n_years + 1), (int)n_ages};
      out.dim_names = {"n_years+1", "n_ages"};
      break;
    case DimRule::SCALAR_OUT:
      out.size = 1;
      out.dims = {1};
      out.dim_names = {"scalar"};
      break;
  }
  return out;
}

// -----------------------------------------------------------------------
// Static registry
//
// Add one entry per known calculation block. Inputs/outputs are quantity
// names shared through the derived quantities map. When multiple entries
// can produce the same output (e.g., "biomass" via NAA path vs. depletion
// path), whichever inputs are available at forward-pass time determines
// which one activates — the other stays dormant.
//
// The DimRule on each entry tells run_forward_pass() how to size the output
// derived quantity vector — replacing the model-type-specific initialization
// blocks that currently live in CatchAtAgeInterface and
// SurplusProductionInterface.
// -----------------------------------------------------------------------
inline const std::vector<CalcDescriptor>& GetCalcRegistry() {
  static const std::vector<CalcDescriptor> registry = {
      // Catch-at-age path -----------------------------------------------
      {
          "CalculateMortality",
          {"natural_mortality", "fishing_mortality"},
          {"mortality_Z"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::BROADCAST_INPUTS,  // scalar M + n_years Fmort → n_years output
      },
      {
          "CalculateNumbersAA",
          {"mortality_Z", "initial_numbers_at_age"},
          {"numbers_at_age"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::PER_AGE_YEAR_PLUS_ONE,  // cohort tracking needs n_years+1
      },
      {
          "CalculateBiomass",
          {"numbers_at_age", "growth"},
          {"biomass"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::PER_YEAR_PLUS_ONE,  // reduction over ages, one value per year
      },
      {
          "CalculateSpawningBiomass",
          {"numbers_at_age", "growth", "proportion_mature_at_age",
           "proportion_female"},
          {"spawning_biomass"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::PER_YEAR_PLUS_ONE,
      },
      {
          "CalculateIndexNAA",
          {"numbers_at_age", "selectivity", "catchability"},
          {"expected_index"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::PER_YEAR,
      },
      // Surplus production path -----------------------------------------
      // "biomass" and "expected_index" are the same output names as above;
      // whichever input set is satisfied first wins.
      {
          "CalculateBiomassFromDepletion",
          {"depletion", "carrying_capacity"},
          {"biomass"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::PER_YEAR_PLUS_ONE,
      },
      {
          "CalculateIndexFromBiomass",
          {"biomass", "catchability"},
          {"expected_index"},
          LoopDomain::PER_YEAR_AGE,
          DimRule::PER_YEAR,
      },
  };
  return registry;
}

// -----------------------------------------------------------------------
// run_forward_pass
//
// Greedy algorithm that resolves which calculations to run AND sizes all
// output derived quantities — replacing the model-type-specific
// initialization blocks in add_to_fims_tmb_internal().
//
// seeded_quantities: name → size for each quantity contributed by the
//   modules the user has instantiated (populations, fleets, depletion, etc.).
//   Size conventions: 1 = scalar, n_ages = by-age, n_years = by-year,
//   n_ages*n_years = full. Functional evaluators (growth, selectivity) are
//   registered with their effective size (e.g., n_ages for selectivity).
//
// Returns an ordered list of KernelDescriptors. Each carries:
//   - calc_names: which calculations share this kernel's loop
//   - domain: the loop structure
//   - outputs: ResolvedOutput entries with name, size, and dim metadata
//
// add_to_fims_tmb_internal() uses KernelDescriptor::outputs to initialize
// derived quantity vectors generically — no model-type knowledge needed.
// Typed execution closures are then built via BuildKernel<Type>().
// -----------------------------------------------------------------------
inline std::vector<KernelDescriptor> run_forward_pass(
    const std::unordered_map<std::string, size_t>& seeded_quantities,
    size_t n_years, size_t n_ages) {

  std::unordered_map<std::string, size_t> available = seeded_quantities;
  std::set<std::string> scheduled;
  std::vector<KernelDescriptor> execution_order;

  bool changed = true;
  while (changed) {
    changed = false;

    for (const auto& calc : GetCalcRegistry()) {
      if (scheduled.count(calc.name)) continue;

      bool inputs_ready = true;
      for (const auto& input : calc.required_inputs) {
        if (!available.count(input)) {
          inputs_ready = false;
          break;
        }
      }
      if (!inputs_ready) continue;

      // Resolve output size and dimension metadata before adding to plan
      std::vector<ResolvedOutput> resolved_outputs;
      for (const auto& output_name : calc.produced_outputs) {
        ResolvedOutput out = resolve_output(output_name, calc.output_dim_rule,
                                           available, calc.required_inputs,
                                           n_years, n_ages);
        resolved_outputs.push_back(out);
        available[output_name] = out.size;  // output is now available with known size
      }

      // Fuse with the previous kernel only when both are PER_YEAR_AGE.
      // PER_YEAR and SCALAR always start fresh kernels — different loop structure.
      bool fuse = !execution_order.empty() &&
                  execution_order.back().domain == LoopDomain::PER_YEAR_AGE &&
                  calc.domain == LoopDomain::PER_YEAR_AGE;

      if (fuse) {
        execution_order.back().calc_names.push_back(calc.name);
        for (auto& out : resolved_outputs)
          execution_order.back().outputs.push_back(out);
      } else {
        KernelDescriptor kd;
        kd.calc_names = {calc.name};
        kd.domain = calc.domain;
        kd.outputs = resolved_outputs;
        execution_order.push_back(kd);
      }

      scheduled.insert(calc.name);
      changed = true;
    }
  }

  return execution_order;
}

}  // namespace fims

#endif  // FIMS_FORWARD_PASS_HPP
