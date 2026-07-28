/**
 * @file edm_predictor_base.hpp
 * @brief Declares the abstract base class for all EDM prediction functors.
 * @details All EDM prediction algorithms (Simplex Projection, S-map, GP-EDM)
 * inherit from EDMPredictorBase and override predict_one() and predict().
 *
 * Design notes:
 *  - The library (training manifold) is held as a const pointer into the
 *    caller-owned DelayEmbeddingMatrix so that TMB optimization updates to the
 *    underlying series are reflected automatically without rebuilding.
 *  - EuclideanDistance is a free function in the fims_edm namespace. It
 *    operates on raw pointers so it can be called inside AD-traced loops
 *    without relying on STL algorithms that are not available in TMB.
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_PREDICTOR_BASE_HPP
#define FIMS_EDM_PREDICTOR_BASE_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../../common/fims_vector.hpp"
#include "delay_embedding.hpp"
#include "../utilities/edm_distance_weights.hpp"

namespace fims_edm {

// SquaredEuclideanDistance, SimplexWeights, SMapWeights, and NormalizeWeights
// are provided by edm_distance_weights.hpp (included above).

/**
 * @brief Abstract base class for EDM prediction functors.
 *
 * @details Concrete subclasses must implement:
 *  - predict_one(): predict a single future value given a query embedding
 *    coordinate vector.
 *  - predict(): predict over all rows of a test DelayEmbeddingMatrix and
 *    populate the `predictions` field.
 *
 * The caller sets `library` to point at a training DelayEmbeddingMatrix before
 * calling either method. The caller is responsible for the lifetime of the
 * pointed-to matrix. Passing a null pointer or an incompatible matrix will
 * produce a std::runtime_error or std::invalid_argument.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 */
template <typename Type>
struct EDMPredictorBase {
  /**
   * @brief Pointer to the training manifold (library embedding).
   * Not owned by this struct; must outlive all calls to predict_one/predict.
   * Must be set by the caller before calling predict_one() or predict().
   */
  const DelayEmbeddingMatrix<Type>* library = nullptr;

  /** @brief Embedding dimension E. Must match library->n_cols. */
  size_t embedding_dimension = 0;

  /** @brief Time lag tau (informational; not used by base directly). */
  size_t time_lag = 1;

  /**
   * @brief Output predictions.
   * Populated by predict(). Element i corresponds to the predicted value for
   * test_embedding row i.
   */
  std::vector<Type> predictions;

  virtual ~EDMPredictorBase() = default;

  /**
   * @brief Predict the next value for a single query point.
   *
   * @param query_point A vector of length embedding_dimension containing the
   *   lagged coordinates [x_t, x_{t-tau}, ..., x_{t-(E-1)tau}] for the
   *   query time step.
   * @return Predicted value at the next time step.
   * @throws std::runtime_error if library is null.
   * @throws std::invalid_argument if query_point.size() != embedding_dimension.
   */
  virtual Type predict_one(const fims::Vector<Type>& query_point) = 0;

  /**
   * @brief Predict for all rows of a test embedding.
   *
   * Iterates over every row of @p test_embedding, extracts the lagged
   * coordinate vector, calls predict_one(), and stores results in
   * `predictions`. After this call, predictions.size() == test_embedding.n_rows.
   *
   * @param test_embedding The query manifold. n_cols must equal
   *   embedding_dimension (and library->n_cols).
   * @throws std::runtime_error if library is null.
   * @throws std::invalid_argument if dimensions do not match.
   */
  virtual void predict(const DelayEmbeddingMatrix<Type>& test_embedding) = 0;

 protected:
  /**
   * @brief Validate that the library pointer is set and dimensions are
   * consistent with embedding_dimension.
   * @throws std::runtime_error if library is null.
   * @throws std::invalid_argument if library->n_cols != embedding_dimension.
   */
  void ValidateLibrary() const {
    if (library == nullptr) {
      throw std::runtime_error(
          "EDMPredictorBase: library pointer is null. "
          "Set library before calling predict.");
    }
    if (library->n_cols != embedding_dimension) {
      throw std::invalid_argument(
          "EDMPredictorBase: library embedding dimension does not match "
          "embedding_dimension field.");
    }
  }

  /**
   * @brief Validate that a query point has the correct dimension.
   * @param query_point The query coordinate vector.
   * @throws std::invalid_argument if size does not match embedding_dimension.
   */
  void ValidateQueryPoint(const fims::Vector<Type>& query_point) const {
    if (query_point.size() != embedding_dimension) {
      throw std::invalid_argument(
          "EDMPredictorBase: query_point size does not match "
          "embedding_dimension.");
    }
  }
};

}  // namespace fims_edm

#endif  // FIMS_EDM_PREDICTOR_BASE_HPP
