#ifndef FIMS_EXECUTION_KERNEL_HPP
#define FIMS_EXECUTION_KERNEL_HPP

#include <functional>
#include <vector>

#include "forward_pass.hpp"

namespace fims {

// -----------------------------------------------------------------------
// param_index
//
// Returns the correct flat index for a parameter of any dimension given
// (age, year). The parameter's size determines which dimension it has:
//   1            → scalar: always return 0
//   n_ages       → by-age only: return age
//   n_years      → by-year only: return year
//   n_ages*n_years → full age×year: return age * n_years + year
//
// Call this inside any Calculate* function before indexing a parameter
// vector. The outcome is constant for any given model run, so the branch
// predictor handles it with negligible overhead.
// -----------------------------------------------------------------------
inline size_t param_index(size_t age, size_t year,
                          size_t n_years, size_t n_ages,
                          size_t param_size) {
  if (param_size == 1)               return 0;
  if (param_size == n_ages)          return age;
  if (param_size == n_years)         return year;
  /* param_size == n_ages * n_years */ return age * n_years + year;
}

// -----------------------------------------------------------------------
// ExecutionKernel<Type>
//
// One node in the typed execution plan, built from a KernelDescriptor
// in add_to_fims_tmb_internal<Type>(). Holds the actual closures that
// capture pointers to populations, fleets, and derived quantity vectors.
//
// PER_YEAR_AGE kernels:
//   - outer loop: age, inner loop: year  (matches FIMS index convention
//     i_age_year = age * n_years + year, so flat iteration is sequential)
//   - year_inits: closures called once per year at age==0 to zero-initialize
//     accumulating quantities (biomass, spawning_biomass, expected_index)
//   - steps: closures called per (age, year) element
//     signature: void(size_t i_age_year, size_t i_agem1_yearm1,
//                     size_t year, size_t age)
//     i_agem1_yearm1 is 0 when age==0 or year==0 (boundary; callee guards)
//
// PER_YEAR kernels:
//   - single loop over year
//   - year_steps signature: void(size_t year)
//
// SCALAR kernels:
//   - steps called once with (0, 0, 0, 0)
// -----------------------------------------------------------------------
template <typename Type>
struct ExecutionKernel {
  using YearInitFn = std::function<void(size_t)>;
  using StepFn = std::function<void(size_t, size_t, size_t, size_t)>;
  using YearStepFn = std::function<void(size_t)>;

  LoopDomain domain;
  std::vector<YearInitFn> year_inits;  // zero-init accumulators; PER_YEAR_AGE only
  std::vector<StepFn> steps;           // PER_YEAR_AGE element steps
  std::vector<YearStepFn> year_steps;  // PER_YEAR steps

  void execute(size_t n_years, size_t n_ages) const {
    if (domain == LoopDomain::PER_YEAR_AGE) {
      for (size_t age = 0; age < n_ages; age++) {
        for (size_t year = 0; year < n_years; year++) {
          size_t i = age * n_years + year;
          size_t i_prev =
              (age > 0 && year > 0) ? (age - 1) * n_years + (year - 1) : 0;

          if (age == 0) {
            for (const auto& init : year_inits) init(year);
          }

          for (const auto& step : steps) step(i, i_prev, year, age);
        }
      }
    } else if (domain == LoopDomain::PER_YEAR) {
      for (size_t year = 0; year < n_years; year++) {
        for (const auto& step : year_steps) step(year);
      }
    } else {  // SCALAR
      for (const auto& step : steps) step(0, 0, 0, 0);
    }
  }
};

// -----------------------------------------------------------------------
// Calculate* functions
//
// Element-wise functions called per (age, year) by the kernel loop.
// Each uses param_index to handle parameters of any dimension (scalar,
// by-age, by-year, or full age×year) without the caller needing to know
// which dimension was used at setup time.
//
// Shown here as free functions; in the actual implementation they are
// methods on FIMSModel<Type> with population and fleet pointers captured
// by closures when the kernel is built.
// -----------------------------------------------------------------------

// CalculateMortality
// Requires: natural_mortality (population->log_M), fishing_mortality (fleet->log_Fmort)
// Produces: mortality_Z
// DimRule:  BROADCAST_INPUTS — output size inherits from parameter dimensions
template <typename Type>
void CalculateMortality(
    std::shared_ptr<fims_popdy::Population<Type>>& population,
    std::vector<std::shared_ptr<fims_popdy::Fleet<Type>>>& fleets,
    std::map<std::string, fims::Vector<Type>>& dq,
    size_t i_age_year, size_t /*i_prev*/, size_t year, size_t age) {

  size_t ny = population->n_years;
  size_t na = population->n_ages;

  size_t m_idx = param_index(age, year, ny, na, population->log_M.size());
  dq["mortality_Z"][i_age_year] = fims_math::exp(population->log_M[m_idx]);

  for (auto& fleet : fleets) {
    if (!fleet->is_fishing_fleet) continue;
    size_t f_idx = param_index(age, year, ny, na, fleet->log_Fmort.size());
    Type s = fleet->selectivity->evaluate(population->ages[age]);
    dq["mortality_F"][i_age_year] +=
        fims_math::exp(fleet->log_Fmort[f_idx]) * s;
  }
  dq["mortality_Z"][i_age_year] += dq["mortality_F"][i_age_year];
}

// CalculateNumbersAA
// Requires: mortality_Z, initial_numbers_at_age (population->log_init_naa)
// Produces: numbers_at_age
// DimRule:  PER_AGE_YEAR_PLUS_ONE — cohort tracking always needs n_ages*(n_years+1)
// Note:     i_prev is 0 when age==0 or year==0; callee guards on both
template <typename Type>
void CalculateNumbersAA(
    std::shared_ptr<fims_popdy::Population<Type>>& population,
    std::map<std::string, fims::Vector<Type>>& dq,
    size_t i_age_year, size_t i_prev, size_t year, size_t age) {

  if (age == 0) {
    // Age-0 filled by recruitment module evaluate(), not here
    return;
  }
  if (year == 0) {
    dq["numbers_at_age"][i_age_year] =
        fims_math::exp(population->log_init_naa[age]);
    return;
  }
  dq["numbers_at_age"][i_age_year] =
      dq["numbers_at_age"][i_prev] *
      fims_math::exp(-dq["mortality_Z"][i_prev]);
  // Plus-group: add survivors from the same plus-group in year-1
  if (age == population->n_ages - 1) {
    dq["numbers_at_age"][i_age_year] +=
        dq["numbers_at_age"][i_prev + 1] *
        fims_math::exp(-dq["mortality_Z"][i_prev + 1]);
  }
}

// CalculateBiomass
// Requires: numbers_at_age, growth
// Produces: biomass  (accumulates +=; zeroed by year_init closure)
// DimRule:  PER_YEAR_PLUS_ONE
template <typename Type>
void CalculateBiomass(
    std::shared_ptr<fims_popdy::Population<Type>>& population,
    std::map<std::string, fims::Vector<Type>>& dq,
    size_t i_age_year, size_t /*i_prev*/, size_t year, size_t age) {
  dq["biomass"][year] +=
      dq["numbers_at_age"][i_age_year] *
      population->growth->evaluate(year, population->ages[age]);
}

// CalculateSpawningBiomass
// Requires: numbers_at_age, growth, proportion_mature_at_age, proportion_female
// Produces: spawning_biomass  (accumulates +=; zeroed by year_init closure)
// DimRule:  PER_YEAR_PLUS_ONE
template <typename Type>
void CalculateSpawningBiomass(
    std::shared_ptr<fims_popdy::Population<Type>>& population,
    std::map<std::string, fims::Vector<Type>>& dq,
    size_t i_age_year, size_t /*i_prev*/, size_t year, size_t age) {

  size_t ny = population->n_years;
  size_t na = population->n_ages;
  size_t pf_idx = param_index(age, year, ny, na, population->proportion_female.size());

  dq["spawning_biomass"][year] +=
      population->proportion_female[pf_idx] *
      dq["numbers_at_age"][i_age_year] *
      dq["proportion_mature_at_age"][i_age_year] *
      population->growth->evaluate(year, population->ages[age]);
}

// CalculateIndexNAA
// Requires: numbers_at_age, selectivity, catchability
// Produces: expected_index  (accumulates +=; zeroed by year_init closure)
// DimRule:  PER_YEAR
template <typename Type>
void CalculateIndexNAA(
    std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
    std::shared_ptr<fims_popdy::Population<Type>>& population,
    std::map<std::string, fims::Vector<Type>>& fleet_dq,
    std::map<std::string, fims::Vector<Type>>& pop_dq,
    size_t i_age_year, size_t /*i_prev*/, size_t year, size_t age) {

  size_t ny = population->n_years;
  size_t na = population->n_ages;
  size_t q_idx = param_index(age, year, ny, na, fleet->log_q.size());

  fleet_dq["index_numbers_at_age"][i_age_year] +=
      fims_math::exp(fleet->log_q[q_idx]) *
      fleet->selectivity->evaluate(population->ages[age]) *
      pop_dq["numbers_at_age"][i_age_year];
  fleet_dq["expected_index"][year] +=
      fleet_dq["index_numbers_at_age"][i_age_year];
}

// CalculateBiomassFromDepletion  (surplus production path)
// Requires: depletion, carrying_capacity
// Produces: biomass
// DimRule:  PER_YEAR_PLUS_ONE
// Note:     SP models have n_ages=1, so i_age_year == year; depletion is
//           indexed by year directly.
template <typename Type>
void CalculateBiomassFromDepletion(
    std::shared_ptr<fims_popdy::Population<Type>>& population,
    std::map<std::string, fims::Vector<Type>>& dq,
    size_t /*i_age_year*/, size_t /*i_prev*/, size_t year, size_t /*age*/) {

  size_t k_idx =
      param_index(0, year, population->n_years, 1,
                  population->log_carrying_capacity.size());
  dq["biomass"][year] =
      fims_math::exp(population->log_depletion[year]) *
      fims_math::exp(population->log_carrying_capacity[k_idx]);
}

// CalculateIndexFromBiomass  (surplus production path)
// Requires: biomass, catchability
// Produces: expected_index
// DimRule:  PER_YEAR
template <typename Type>
void CalculateIndexFromBiomass(
    std::shared_ptr<fims_popdy::Fleet<Type>>& fleet,
    std::map<std::string, fims::Vector<Type>>& pop_dq,
    std::map<std::string, fims::Vector<Type>>& fleet_dq,
    size_t /*i_age_year*/, size_t /*i_prev*/, size_t year, size_t /*age*/) {

  size_t q_idx =
      param_index(0, year, fleet->n_years, 1, fleet->log_q.size());
  fleet_dq["expected_index"][year] =
      pop_dq["biomass"][year] * fims_math::exp(fleet->log_q[q_idx]);
}

// -----------------------------------------------------------------------
// BuildKernel
//
// Translates one KernelDescriptor into a typed ExecutionKernel<Type>.
// Called from add_to_fims_tmb_internal<Type>() once per KernelDescriptor
// produced by run_forward_pass(), AFTER derived quantity vectors have
// already been initialized using KernelDescriptor::outputs.
//
// Responsibilities:
//   - Register year_inits closures that zero accumulating outputs at the
//     start of each year (biomass, spawning_biomass, expected_index)
//   - Register step closures that call the appropriate Calculate* function
//
// Not responsible for:
//   - Creating derived quantity vectors (done by add_to_fims_tmb_internal
//     using KernelDescriptor::outputs before BuildKernel is called)
//   - Registering quantities in variable_map (done by add_to_fims_tmb_internal)
// -----------------------------------------------------------------------
template <typename Type>
ExecutionKernel<Type> BuildKernel(
    const KernelDescriptor& descriptor,
    std::shared_ptr<fims_popdy::Population<Type>> population,
    std::vector<std::shared_ptr<fims_popdy::Fleet<Type>>> fleets) {

  ExecutionKernel<Type> kernel;
  kernel.domain = descriptor.domain;

  auto& pop_dq =
      fims_info::Information<Type>::GetInstance()
          ->GetPopulationDerivedQuantities(population->GetId());

  for (const auto& name : descriptor.calc_names) {

    if (name == "CalculateMortality") {
      kernel.steps.push_back(
          [population, fleets, &pop_dq](size_t i, size_t i_prev,
                                        size_t y, size_t a) {
            CalculateMortality(population, fleets, pop_dq, i, i_prev, y, a);
          });

    } else if (name == "CalculateNumbersAA") {
      kernel.steps.push_back(
          [population, &pop_dq](size_t i, size_t i_prev, size_t y, size_t a) {
            CalculateNumbersAA(population, pop_dq, i, i_prev, y, a);
          });

    } else if (name == "CalculateBiomass") {
      kernel.year_inits.push_back(
          [&pop_dq](size_t y) { pop_dq["biomass"][y] = Type(0); });
      kernel.steps.push_back(
          [population, &pop_dq](size_t i, size_t i_prev, size_t y, size_t a) {
            CalculateBiomass(population, pop_dq, i, i_prev, y, a);
          });

    } else if (name == "CalculateSpawningBiomass") {
      kernel.year_inits.push_back(
          [&pop_dq](size_t y) { pop_dq["spawning_biomass"][y] = Type(0); });
      kernel.steps.push_back(
          [population, &pop_dq](size_t i, size_t i_prev, size_t y, size_t a) {
            CalculateSpawningBiomass(population, pop_dq, i, i_prev, y, a);
          });

    } else if (name == "CalculateIndexNAA") {
      for (auto& fleet : fleets) {
        if (!fleet->is_survey_fleet) continue;
        auto& fleet_dq =
            fims_info::Information<Type>::GetInstance()
                ->GetFleetDerivedQuantities(fleet->GetId());
        kernel.year_inits.push_back(
            [&fleet_dq](size_t y) { fleet_dq["expected_index"][y] = Type(0); });
        kernel.steps.push_back(
            [fleet, population, &fleet_dq, &pop_dq](
                size_t i, size_t i_prev, size_t y, size_t a) {
              CalculateIndexNAA(fleet, population, fleet_dq, pop_dq,
                                i, i_prev, y, a);
            });
      }

    } else if (name == "CalculateBiomassFromDepletion") {
      kernel.year_inits.push_back(
          [&pop_dq](size_t y) { pop_dq["biomass"][y] = Type(0); });
      kernel.steps.push_back(
          [population, &pop_dq](size_t i, size_t i_prev, size_t y, size_t a) {
            CalculateBiomassFromDepletion(population, pop_dq, i, i_prev, y, a);
          });

    } else if (name == "CalculateIndexFromBiomass") {
      for (auto& fleet : fleets) {
        if (!fleet->is_survey_fleet) continue;
        auto& fleet_dq =
            fims_info::Information<Type>::GetInstance()
                ->GetFleetDerivedQuantities(fleet->GetId());
        kernel.year_inits.push_back(
            [&fleet_dq](size_t y) { fleet_dq["expected_index"][y] = Type(0); });
        kernel.steps.push_back(
            [fleet, &pop_dq, &fleet_dq](
                size_t i, size_t i_prev, size_t y, size_t a) {
              CalculateIndexFromBiomass(fleet, pop_dq, fleet_dq,
                                        i, i_prev, y, a);
            });
      }
    }
  }

  return kernel;
}

}  // namespace fims

#endif  // FIMS_EXECUTION_KERNEL_HPP
