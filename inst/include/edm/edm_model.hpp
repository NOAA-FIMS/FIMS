/**
 * @file edm_model.hpp
 * @brief EDMModel — a FIMS module that wraps the EDM prediction functors
 *        (SimplexProjection, SMapProjection, GPEdmProjection) and exposes
 *        their predictions as a fims::Vector<Type> registered in
 *        Information<Type>::variable_map for use as expected values in
 *        DensityComponentBase data likelihoods.
 *
 * @details EDMModel<Type> inherits from fims_model_object::FIMSObject<Type>
 *          and follows the same lifecycle as other FIMS modules:
 *          - Created and populated by an Rcpp interface class via
 *            add_to_fims_tmb_internal<Type>().
 *          - Stored in Information<Type>::edm_models.
 *          - predictions is registered in Information<Type>::variable_map so
 *            DensityComponentBase::SetupData() can link it as
 *            data_expected_values.
 *          - Evaluate() is called from Model<Type>::Evaluate() before the
 *            data-likelihood loop.
 *
 * @note GP-EDM hyperparameters (phi, sigma2, ve) are stored as double (not
 *       Type) because the Rprop optimizer that fits them runs outside the
 *       CppAD tape. Predictions are still computed in Type arithmetic, so
 *       gradients flow through the prediction step.
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_EDM_MODEL_HPP
#define FIMS_EDM_EDM_MODEL_HPP

#include <memory>
#include <string>
#include <vector>

#include "../common/fims_vector.hpp"
#include "../common/model_object.hpp"
#include "functors/gp_edm_projection.hpp"
#include "functors/simplex_projection.hpp"
#include "functors/smap_projection.hpp"

namespace fims_edm {

/**
 * @brief FIMS module that runs an EDM prediction functor and exposes its
 *        output as a fims::Vector<Type> for use in the joint NLL.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 */
template <typename Type>
struct EDMModel : public fims_model_object::FIMSObject<Type> {
 public:
  /**
   * @brief Static global ID counter shared across all EDMModel instances.
   */
  static uint32_t id_g;

  /**
   * @brief Predictor type: "simplex", "smap", or "gp_edm".
   */
  std::string predictor_type = "simplex";

  /**
   * @brief Library embedding matrix (owned; filled by Rcpp interface).
   */
  DelayEmbeddingMatrix<Type> library_matrix;

  /**
   * @brief Test embedding matrix (owned; filled by Rcpp interface).
   */
  DelayEmbeddingMatrix<Type> test_matrix;

  /**
   * @brief Prediction output vector.
   *
   * @details Registered in Information<Type>::variable_map so that a
   *          DensityComponentBase with input_type == "data" can link to it
   *          as data_expected_values via SetupData().
   */
  fims::Vector<Type> predictions;

  // --- Simplex / SMap hyperparameters ---

  /**
   * @brief Embedding dimension (E). Must match library_matrix.n_cols.
   */
  size_t embedding_dimension = 3;

  /**
   * @brief Number of nearest neighbours (Simplex only).
   */
  size_t n_neighbors = 4;

  /**
   * @brief S-Map localisation parameter theta (SMap only).
   */
  double theta = 1.0;

  /**
   * @brief S-Map kernel type: "exponential" or "gaussian" (SMap only).
   */
  std::string kernel = "exponential";

  /**
   * @brief Forecast horizon (steps ahead). Default 1.
   */
  size_t forecast_horizon = 1;

  // --- GP-EDM hyperparameters (double, not Type — fitted by Rprop pre-pass) ---

  /**
   * @brief ARD inverse length-scale parameters, one per embedding dimension.
   *        Stored as double because Rprop runs outside the CppAD tape.
   */
  std::vector<double> phi;

  /**
   * @brief GP signal variance. Stored as double (Rprop pre-pass).
   */
  double sigma2 = 1.0;

  /**
   * @brief GP observation noise variance (nugget). Stored as double.
   */
  double ve = 0.1;

  /**
   * @brief Default constructor. Assigns a unique ID.
   */
  EDMModel() { this->id = EDMModel<Type>::id_g++; }

  /**
   * @brief Evaluate: run the chosen predictor and populate predictions[].
   *
   * @details Called from Model<Type>::Evaluate() before the data-likelihood
   *          loop so that predictions are available when DensityComponentBase
   *          evaluates the NLL contribution.
   */
  void Evaluate() {
    if (predictor_type == "simplex") {
      SimplexProjection<Type> pred;
      pred.library = &library_matrix;
      pred.embedding_dimension = this->embedding_dimension;
      pred.n_neighbors = static_cast<int>(this->n_neighbors);
      pred.forecast_horizon = this->forecast_horizon;
      pred.predict(test_matrix);

      predictions.resize(pred.predictions.size());
      for (size_t i = 0; i < pred.predictions.size(); ++i) {
        predictions[i] = pred.predictions[i];
      }

    } else if (predictor_type == "smap") {
      SMapProjection<Type> pred;
      pred.library = &library_matrix;
      pred.embedding_dimension = this->embedding_dimension;
      pred.theta = this->theta;
      if (this->kernel == "gaussian") {
        pred.kernel = SMapKernel::kGaussian;
      } else {
        pred.kernel = SMapKernel::kExponential;
      }
      pred.forecast_horizon = this->forecast_horizon;
      pred.predict(test_matrix);

      predictions.resize(pred.predictions.size());
      for (size_t i = 0; i < pred.predictions.size(); ++i) {
        predictions[i] = pred.predictions[i];
      }

    } else if (predictor_type == "gp_edm") {
      GPEdmProjection<Type> pred;
      pred.library = &library_matrix;
      pred.embedding_dimension = this->embedding_dimension;
      // phi/sigma2/ve are double — fitted by Rprop before CreateTMBModel().
      // They are used here as fixed constants in the AD-traced prediction.
      pred.phi = this->phi;
      pred.sigma2 = this->sigma2;
      pred.ve = this->ve;
      pred.predict(test_matrix);

      predictions.resize(pred.predictions.size());
      for (size_t i = 0; i < pred.predictions.size(); ++i) {
        predictions[i] = pred.predictions[i];
      }
    }
  }
};

/**
 * @brief Static ID counter initialised to 0.
 */
template <typename Type>
uint32_t EDMModel<Type>::id_g = 0;

}  // namespace fims_edm

#endif  // FIMS_EDM_EDM_MODEL_HPP
