/**
 * @file growth_derived_size_provider.hpp
 * @brief Defines a size provider that reads mean growth outputs and prepares
 * population-level size probabilities on the biological size grid.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_GROWTH_DERIVED_SIZE_PROVIDER_HPP
#define FIMS_POPULATION_DYNAMICS_GROWTH_DERIVED_SIZE_PROVIDER_HPP

#include <cstddef>
#include <memory>
#include <stdexcept>

#include "../growth/growth_model_adapter.hpp"
#include "size_distribution_provider_base.hpp"
#include "size_products.hpp"
#include "../../common/fims_math.hpp"

namespace fims_popdy {

template <typename Type>
class GrowthDerivedSizeProvider : public SizeDistributionProviderBase<Type> {
 public:
  GrowthDerivedSizeProvider() = default;

  explicit GrowthDerivedSizeProvider(
      std::shared_ptr<GrowthDerivedObservationBase<Type>> growth)
      : growth_source_(growth) {}

  const SizeGrid* TryGetSizeGrid() const override { return population_size_grid_; }

  void SetPopulationSizeGrid(const SizeGrid* population_size_grid) override {
    population_size_grid_ = population_size_grid;
    size_products_prepared_ = false;
  }

  void SetPopulationDimensions(std::size_t n_years,
                               std::size_t n_ages) override {
    n_years_ = n_years;
    n_ages_ = n_ages;
    size_products_prepared_ = false;
  }

  void PrepareSizeProducts() override {
    if (!population_size_grid_) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires a population size grid");
    }

    if (!population_size_grid_->IsConsistent()) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires a consistent population size grid");
    }

    if (population_size_grid_->n_bins == 0) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires at least one size bin");
    }

    if (n_years_ == 0 || n_ages_ == 0) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires nonzero population dimensions");
    }

    const GrowthProducts<Type>& growth_products = PreparedGrowthProducts();

    if (growth_products.n_years != n_years_ ||
        growth_products.n_ages != n_ages_) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider growth products do not match configured population dimensions");
    }

    if (growth_products.n_sexes != 1) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider currently requires pooled growth products with n_sexes == 1");
    }

    size_products_.ResizeProbSizeOnly(
        n_years_, n_ages_, population_size_grid_->n_bins);

    for (std::size_t year_index = 0; year_index < n_years_; ++year_index) {
      for (std::size_t age_index = 0; age_index < n_ages_; ++age_index) {
        Type row_sum = static_cast<Type>(0.0);

        for (std::size_t size_bin_index = 0;
             size_bin_index < population_size_grid_->n_bins;
             ++size_bin_index) {
          const Type bin_prob = PopulationSizeBinProb(
              growth_products, year_index, age_index, size_bin_index);
          size_products_.ProbSize(year_index, age_index, size_bin_index) =
              bin_prob;
          row_sum += bin_prob;
        }

        const Type safe_row_sum = fims_math::ad_max(
            row_sum, static_cast<Type>(1e-12));

        for (std::size_t size_bin_index = 0;
             size_bin_index < population_size_grid_->n_bins;
             ++size_bin_index) {
          size_products_.ProbSize(year_index, age_index, size_bin_index) /=
              safe_row_sum;
        }
      }
    }

    size_products_prepared_ = true;
  }

  void InvalidatePreparedSizeProducts() override { size_products_prepared_ = false; }

  const Type& MeanLAA(std::size_t year_index, std::size_t age_index) const override {
    return PreparedGrowthProducts().MeanLAA(year_index, age_index, 0);
  }

  const Type& SdLAA(std::size_t year_index, std::size_t age_index) const override {
    return PreparedGrowthProducts().SdLAA(year_index, age_index, 0);
  }

  const Type& ProbSize(std::size_t year_index,
                       std::size_t age_index,
                       std::size_t size_bin_index) const override {
    if (!size_products_prepared_) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires prepared size products");
    }

    return size_products_.ProbSize(year_index, age_index, size_bin_index);
  }

  void SetGrowth(
      std::shared_ptr<GrowthDerivedObservationBase<Type>> growth) {
    growth_source_ = growth;
    size_products_prepared_ = false;
  }

 private:
  Type PopulationSizeBinProb(const GrowthProducts<Type>& growth_products,
                             std::size_t year_index,
                             std::size_t age_index,
                             std::size_t size_bin_index) const {
    if (!population_size_grid_ || population_size_grid_->n_bins == 0) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires a population size grid before computing ProbSize");
    }

    const Type mean_laa = growth_products.MeanLAA(year_index, age_index, 0);
    const Type sd_laa = fims_math::ad_max(
        growth_products.SdLAA(year_index, age_index, 0),
        static_cast<Type>(1e-8));

    if (population_size_grid_->n_bins == 1) {
      return static_cast<Type>(1.0);
    }

    if (size_bin_index == 0) {
      const Type upper = static_cast<Type>(population_size_grid_->edges[1]);
      return fims_math::normalcdf(upper, mean_laa, sd_laa);
    }

    if (size_bin_index + 1 == population_size_grid_->n_bins) {
      const Type lower = static_cast<Type>(
          population_size_grid_->edges[population_size_grid_->n_bins - 1]);
      return static_cast<Type>(1.0) -
             fims_math::normalcdf(lower, mean_laa, sd_laa);
    }

    const Type lower =
        static_cast<Type>(population_size_grid_->edges[size_bin_index]);
    const Type upper =
        static_cast<Type>(population_size_grid_->edges[size_bin_index + 1]);

    return fims_math::normalcdf(upper, mean_laa, sd_laa) -
           fims_math::normalcdf(lower, mean_laa, sd_laa);
  }

  const GrowthProducts<Type>& PreparedGrowthProducts() const {
    if (!growth_source_) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires a linked growth object");
    }

    const GrowthProducts<Type>* growth_products =
        growth_source_->TryGetPreparedGrowthProducts();

    if (!growth_products) {
      throw std::runtime_error(
          "GrowthDerivedSizeProvider requires prepared growth products");
    }

    return *growth_products;
  }

  std::shared_ptr<GrowthDerivedObservationBase<Type>> growth_source_;
  const SizeGrid* population_size_grid_ = nullptr;
  std::size_t n_years_ = 0;
  std::size_t n_ages_ = 0;
  bool size_products_prepared_ = false;
  SizeProducts<Type> size_products_;
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_GROWTH_DERIVED_SIZE_PROVIDER_HPP */