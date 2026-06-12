/**
 * @file rcpp_edm.hpp
 * @brief The Rcpp interface for EDM (Empirical Dynamic Modeling) delay
 * embedding utilities. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_EDM_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_EDM_HPP

#include "../../../edm/functors/delay_embedding.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp EDM
 * interfaces. This type should be inherited and not called from R directly.
 */
class EDMInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the EDMInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the EDMInterfaceBase object.
   */
  uint32_t id;
  /**
   * @brief The map associating the IDs of EDMInterfaceBase to the objects.
   */
  static std::map<uint32_t, std::shared_ptr<EDMInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  EDMInterfaceBase() { this->id = EDMInterfaceBase::id_g++; }

  /**
   * @brief Copy constructor.
   */
  EDMInterfaceBase(const EDMInterfaceBase& other) : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~EDMInterfaceBase() {}

  /**
   * @brief Get the ID for the child EDM interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;
};

/**
 * @brief Rcpp interface for delay embedding to instantiate the object from R:
 * delay_embedding <- methods::new(DelayEmbeddingMatrix).
 *
 * @details Wraps the C++ fims_edm::DelayEmbeddingMatrix and the
 * fims_edm::MakeDelayEmbedding / fims_edm::MakeDelayEmbeddingDropMissing
 * factory functions, exposing them to R through the FIMS Rcpp module system.
 */
class DelayEmbeddingInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E), i.e., the number of lagged
   * coordinates per row.
   */
  uint32_t embedding_dimension;
  /**
   * @brief The time lag (tau) between successive coordinates in the
   * embedding.
   */
  uint32_t time_lag;
  /**
   * @brief The number of rows in the constructed delay embedding matrix.
   */
  uint32_t n_rows;
  /**
   * @brief The number of columns in the constructed delay embedding matrix
   * (equal to embedding_dimension).
   */
  uint32_t n_cols;
  /**
   * @brief The flattened delay embedding matrix, stored row-major. Element
   * [row * n_cols + col] is the value of x_{t - col * tau} for that row.
   */
  RealVector embedded_values;
  /**
   * @brief The target time-point values for each row (i.e., x_t), dereferenced
   * from the original series pointers.
   */
  RealVector target_values;

  /**
   * @brief The constructor.
   */
  DelayEmbeddingInterface()
      : EDMInterfaceBase(),
        embedding_dimension(0),
        time_lag(0),
        n_rows(0),
        n_cols(0) {
    EDMInterfaceBase::live_objects[this->id] =
        std::make_shared<DelayEmbeddingInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        EDMInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Copy constructor.
   */
  DelayEmbeddingInterface(const DelayEmbeddingInterface& other)
      : EDMInterfaceBase(other),
        embedding_dimension(other.embedding_dimension),
        time_lag(other.time_lag),
        n_rows(other.n_rows),
        n_cols(other.n_cols),
        embedded_values(other.embedded_values),
        target_values(other.target_values) {}

  /**
   * @brief The destructor.
   */
  virtual ~DelayEmbeddingInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Construct a delay embedding matrix from a time series vector.
   *
   * @details Calls fims_edm::MakeDelayEmbedding with the provided series,
   * embedding dimension, and time lag. The resulting matrix values and
   * target indices are stored in the interface fields.
   *
   * @param series An Rcpp::NumericVector representing the univariate time
   * series.
   * @param E The embedding dimension (must be > 0).
   * @param tau The time lag (must be > 0).
   */
  void construct(Rcpp::NumericVector series, uint32_t E, uint32_t tau) {
    // Convert R vector to fims::Vector<double>
    fims::Vector<double> fims_series;
    fims_series.resize(series.size());
    for (R_xlen_t i = 0; i < series.size(); i++) {
      fims_series[i] = series[i];
    }

    // Build delay embedding
    fims_edm::DelayEmbeddingMatrix<double> embedding =
        fims_edm::MakeDelayEmbedding(fims_series, E, tau);

    // Store parameters
    this->embedding_dimension = E;
    this->time_lag = tau;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    // Copy flattened embedded values into RealVector (dereference pointers)
    this->embedded_values.resize(embedding.embedded_values.size());
    for (size_t i = 0; i < embedding.embedded_values.size(); i++) {
      this->embedded_values[i] = *embedding.embedded_values[i];
    }

    // Copy target values into RealVector (dereference pointers)
    this->target_values.resize(embedding.target_values.size());
    for (size_t i = 0; i < embedding.target_values.size(); i++) {
      this->target_values[i] = *embedding.target_values[i];
    }
  }

  /**
   * @brief Construct a delay embedding matrix from a time series, dropping
   * rows that contain the specified missing value.
   *
   * @param series An Rcpp::NumericVector representing the univariate time
   * series.
   * @param E The embedding dimension (must be > 0).
   * @param tau The time lag (must be > 0).
   * @param missing_value The sentinel value representing missing data
   * (e.g., -999.0).
   */
  void construct_drop_missing(Rcpp::NumericVector series, uint32_t E,
                              uint32_t tau, double missing_value) {
    // Convert R vector to fims::Vector<double>
    fims::Vector<double> fims_series;
    fims_series.resize(series.size());
    for (R_xlen_t i = 0; i < series.size(); i++) {
      fims_series[i] = series[i];
    }

    // Build delay embedding, dropping missing values
    fims_edm::DelayEmbeddingMatrix<double> embedding =
        fims_edm::MakeDelayEmbeddingDropMissing(fims_series, E, tau,
                                                missing_value);

    // Store parameters
    this->embedding_dimension = E;
    this->time_lag = tau;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    // Copy flattened embedded values into RealVector (dereference pointers)
    this->embedded_values.resize(embedding.embedded_values.size());
    for (size_t i = 0; i < embedding.embedded_values.size(); i++) {
      this->embedded_values[i] = *embedding.embedded_values[i];
    }

    // Copy target values into RealVector (dereference pointers)
    this->target_values.resize(embedding.target_values.size());
    for (size_t i = 0; i < embedding.target_values.size(); i++) {
      this->target_values[i] = *embedding.target_values[i];
    }
  }

  /**
   * @brief Access an element of the delay embedding matrix by row and column.
   *
   * @param row Zero-based row index.
   * @param col Zero-based column index.
   * @return The value at (row, col) in the embedding matrix.
   */
  double at(uint32_t row, uint32_t col) {
    if (row >= this->n_rows || col >= this->n_cols) {
      throw std::invalid_argument(
          "DelayEmbeddingInterface::at() index out of bounds.");
    }
    return this->embedded_values[row * this->n_cols + col];
  }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object.
   * @details For the delay embedding, there are no TMB-estimated parameters
   * to finalize; this is a no-op placeholder for consistency with the FIMS
   * interface pattern.
   */
  virtual void finalize() {
    if (this->finalized) {
      FIMS_WARNING_LOG("DelayEmbedding " + fims::to_string(this->id) +
                       " has been finalized already.");
    }
    this->finalized = true;
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A json-formatted string describing this delay embedding module.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"EDM\",\n";
    ss << " \"module_type\": \"DelayEmbedding\",\n";
    ss << " \"module_id\": " << this->id << ",\n";
    ss << " \"embedding_dimension\": " << this->embedding_dimension << ",\n";
    ss << " \"time_lag\": " << this->time_lag << ",\n";
    ss << " \"n_rows\": " << this->n_rows << ",\n";
    ss << " \"n_cols\": " << this->n_cols << "\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    // The delay embedding currently operates on observed (fixed) data and
    // does not register estimable parameters with the TMB model.
    // Future prediction functors (Simplex, S-map, GP-EDM) will register
    // their parameters here.
    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }

#endif
};

#endif
