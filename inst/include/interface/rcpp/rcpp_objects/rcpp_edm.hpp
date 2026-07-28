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

#include "../../../edm/edm.hpp"
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
 *
 * Uncertainty propagation follows the same pointer-based design as the value
 * fields. When an uncertainty series is supplied:
 *  - embedded_uncertainty mirrors embedded_values in layout
 *  - target_uncertainty mirrors target_values
 * Both are empty when no uncertainty series is provided.
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
   * @brief The flattened uncertainty matrix, stored row-major. Element
   * [row * n_cols + col] is sigma_{t - col * tau} for that row.
   * Empty when no uncertainty series is provided.
   */
  RealVector embedded_uncertainty;
  /**
   * @brief The target time-point uncertainties for each row (i.e., sigma_t),
   * dereferenced from the input_uncertainty series pointers.
   * Empty when no uncertainty series is provided.
   */
  RealVector target_uncertainty;

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
        target_values(other.target_values),
        embedded_uncertainty(other.embedded_uncertainty),
        target_uncertainty(other.target_uncertainty) {}

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
   * @brief Construct a delay embedding matrix from a time series vector
   * (no uncertainty propagation).
   *
   * @details Calls fims_edm::MakeDelayEmbedding. The resulting matrix values
   * and target values are stored in the interface fields. The
   * embedded_uncertainty and target_uncertainty fields remain empty.
   *
   * @param series An Rcpp::NumericVector representing the univariate time
   * series.
   * @param E The embedding dimension (must be > 0).
   * @param tau The time lag (must be > 0).
   */
  void construct(Rcpp::NumericVector series, uint32_t E, uint32_t tau) {
    fims::Vector<double> fims_series;
    fims_series.resize(series.size());
    for (R_xlen_t i = 0; i < series.size(); i++) {
      fims_series[i] = series[i];
    }

    fims_edm::DelayEmbeddingMatrix<double> embedding =
        fims_edm::MakeDelayEmbedding(fims_series, E, tau);

    this->embedding_dimension = E;
    this->time_lag = tau;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    this->embedded_values.resize(embedding.embedded_values.size());
    for (size_t i = 0; i < embedding.embedded_values.size(); i++) {
      this->embedded_values[i] = *embedding.embedded_values[i];
    }

    this->target_values.resize(embedding.target_values.size());
    for (size_t i = 0; i < embedding.target_values.size(); i++) {
      this->target_values[i] = *embedding.target_values[i];
    }

    // No uncertainty provided — leave uncertainty fields empty
    this->embedded_uncertainty.resize(0);
    this->target_uncertainty.resize(0);
  }

  /**
   * @brief Construct a delay embedding matrix from a time series vector,
   * propagating an accompanying uncertainty series.
   *
   * @details Same as construct() but also populates embedded_uncertainty and
   * target_uncertainty following the same pointer-based layout as the value
   * fields. @p uncertainty must have the same length as @p series.
   *
   * @param series An Rcpp::NumericVector representing the univariate time
   * series.
   * @param E The embedding dimension (must be > 0).
   * @param tau The time lag (must be > 0).
   * @param uncertainty An Rcpp::NumericVector of uncertainty values (e.g.,
   * standard deviations) with the same length as @p series.
   */
  void construct_with_uncertainty(Rcpp::NumericVector series, uint32_t E,
                                  uint32_t tau,
                                  Rcpp::NumericVector uncertainty) {
    fims::Vector<double> fims_series;
    fims_series.resize(series.size());
    for (R_xlen_t i = 0; i < series.size(); i++) {
      fims_series[i] = series[i];
    }

    fims::Vector<double> fims_uncertainty;
    fims_uncertainty.resize(uncertainty.size());
    for (R_xlen_t i = 0; i < uncertainty.size(); i++) {
      fims_uncertainty[i] = uncertainty[i];
    }

    fims_edm::DelayEmbeddingMatrix<double> embedding =
        fims_edm::MakeDelayEmbedding(fims_series, E, tau, fims_uncertainty);

    this->embedding_dimension = E;
    this->time_lag = tau;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    this->embedded_values.resize(embedding.embedded_values.size());
    for (size_t i = 0; i < embedding.embedded_values.size(); i++) {
      this->embedded_values[i] = *embedding.embedded_values[i];
    }

    this->target_values.resize(embedding.target_values.size());
    for (size_t i = 0; i < embedding.target_values.size(); i++) {
      this->target_values[i] = *embedding.target_values[i];
    }

    this->embedded_uncertainty.resize(embedding.embedded_uncertainty.size());
    for (size_t i = 0; i < embedding.embedded_uncertainty.size(); i++) {
      this->embedded_uncertainty[i] = *embedding.embedded_uncertainty[i];
    }

    this->target_uncertainty.resize(embedding.target_uncertainty.size());
    for (size_t i = 0; i < embedding.target_uncertainty.size(); i++) {
      this->target_uncertainty[i] = *embedding.target_uncertainty[i];
    }
  }

  /**
   * @brief Construct a delay embedding matrix from a time series, dropping
   * rows that contain the specified missing value (no uncertainty propagation).
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
    fims::Vector<double> fims_series;
    fims_series.resize(series.size());
    for (R_xlen_t i = 0; i < series.size(); i++) {
      fims_series[i] = series[i];
    }

    fims_edm::DelayEmbeddingMatrix<double> embedding =
        fims_edm::MakeDelayEmbeddingDropMissing(fims_series, E, tau,
                                                missing_value);

    this->embedding_dimension = E;
    this->time_lag = tau;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    this->embedded_values.resize(embedding.embedded_values.size());
    for (size_t i = 0; i < embedding.embedded_values.size(); i++) {
      this->embedded_values[i] = *embedding.embedded_values[i];
    }

    this->target_values.resize(embedding.target_values.size());
    for (size_t i = 0; i < embedding.target_values.size(); i++) {
      this->target_values[i] = *embedding.target_values[i];
    }

    // No uncertainty provided — leave uncertainty fields empty
    this->embedded_uncertainty.resize(0);
    this->target_uncertainty.resize(0);
  }

  /**
   * @brief Construct a delay embedding matrix from a time series, dropping
   * rows that contain the specified missing value, and propagating an
   * accompanying uncertainty series.
   *
   * @details Uncertainty rows are dropped alongside their corresponding value
   * rows. @p uncertainty must have the same length as @p series.
   *
   * @param series An Rcpp::NumericVector representing the univariate time
   * series.
   * @param E The embedding dimension (must be > 0).
   * @param tau The time lag (must be > 0).
   * @param missing_value The sentinel value representing missing data.
   * @param uncertainty An Rcpp::NumericVector of uncertainty values with the
   * same length as @p series.
   */
  void construct_drop_missing_with_uncertainty(
      Rcpp::NumericVector series, uint32_t E, uint32_t tau,
      double missing_value, Rcpp::NumericVector uncertainty) {
    fims::Vector<double> fims_series;
    fims_series.resize(series.size());
    for (R_xlen_t i = 0; i < series.size(); i++) {
      fims_series[i] = series[i];
    }

    fims::Vector<double> fims_uncertainty;
    fims_uncertainty.resize(uncertainty.size());
    for (R_xlen_t i = 0; i < uncertainty.size(); i++) {
      fims_uncertainty[i] = uncertainty[i];
    }

    fims_edm::DelayEmbeddingMatrix<double> embedding =
        fims_edm::MakeDelayEmbeddingDropMissing(fims_series, E, tau,
                                                missing_value, fims_uncertainty);

    this->embedding_dimension = E;
    this->time_lag = tau;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    this->embedded_values.resize(embedding.embedded_values.size());
    for (size_t i = 0; i < embedding.embedded_values.size(); i++) {
      this->embedded_values[i] = *embedding.embedded_values[i];
    }

    this->target_values.resize(embedding.target_values.size());
    for (size_t i = 0; i < embedding.target_values.size(); i++) {
      this->target_values[i] = *embedding.target_values[i];
    }

    this->embedded_uncertainty.resize(embedding.embedded_uncertainty.size());
    for (size_t i = 0; i < embedding.embedded_uncertainty.size(); i++) {
      this->embedded_uncertainty[i] = *embedding.embedded_uncertainty[i];
    }

    this->target_uncertainty.resize(embedding.target_uncertainty.size());
    for (size_t i = 0; i < embedding.target_uncertainty.size(); i++) {
      this->target_uncertainty[i] = *embedding.target_uncertainty[i];
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

/**
 * @brief Helper to map RealVector memory from DelayEmbeddingInterface to a transient
 * fims_edm::DelayEmbeddingMatrix<double> representation for functors.
 */
inline fims_edm::DelayEmbeddingMatrix<double> to_matrix(DelayEmbeddingInterface* de) {
  fims_edm::DelayEmbeddingMatrix<double> mat;
  mat.n_rows = de->target_values.size();
  mat.n_cols = mat.n_rows > 0 ? (de->embedded_values.size() / mat.n_rows) : 0;

  if (de->embedded_values.size() > 0 && de->embedded_values.storage_m) {
    mat.embedded_values.resize(de->embedded_values.size());
    for (size_t i = 0; i < de->embedded_values.size(); i++) {
      mat.embedded_values[i] = &((*de->embedded_values.storage_m)[i]);
    }
  }

  if (de->target_values.size() > 0 && de->target_values.storage_m) {
    mat.target_values.resize(de->target_values.size());
    for (size_t i = 0; i < de->target_values.size(); i++) {
      mat.target_values[i] = &((*de->target_values.storage_m)[i]);
    }
  }

  if (de->embedded_uncertainty.size() > 0 && de->embedded_uncertainty.storage_m) {
    mat.embedded_uncertainty.resize(de->embedded_uncertainty.size());
    for (size_t i = 0; i < de->embedded_uncertainty.size(); i++) {
      mat.embedded_uncertainty[i] = &((*de->embedded_uncertainty.storage_m)[i]);
    }
  }

  if (de->target_uncertainty.size() > 0 && de->target_uncertainty.storage_m) {
    mat.target_uncertainty.resize(de->target_uncertainty.size());
    for (size_t i = 0; i < de->target_uncertainty.size(); i++) {
      mat.target_uncertainty[i] = &((*de->target_uncertainty.storage_m)[i]);
    }
  }

  return mat;
}

/**
 * @brief Rcpp interface for Simplex projection.
 */
class SimplexProjectionInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E).
   */
  uint32_t embedding_dimension;
  /**
   * @brief The number of neighbors (k).
   */
  int32_t n_neighbors;

  /**
   * @brief The constructor.
   */
  SimplexProjectionInterface() : EDMInterfaceBase(), embedding_dimension(0), n_neighbors(0) {
    EDMInterfaceBase::live_objects[this->id] =
        std::make_shared<SimplexProjectionInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        EDMInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Copy constructor.
   */
  SimplexProjectionInterface(const SimplexProjectionInterface& other)
      : EDMInterfaceBase(other),
        embedding_dimension(other.embedding_dimension),
        n_neighbors(other.n_neighbors) {}

  /**
   * @brief The destructor.
   */
  virtual ~SimplexProjectionInterface() {}

  /**
   * @brief Get the ID of the object.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Predict the values using Simplex projection.
   * @param lib_id The ID of the library delay embedding.
   * @param test_id The ID of the test delay embedding.
   * @return A vector of predictions.
   */
  Rcpp::NumericVector predict(uint32_t lib_id, uint32_t test_id) {
    auto it_lib = EDMInterfaceBase::live_objects.find(lib_id);
    auto it_test = EDMInterfaceBase::live_objects.find(test_id);
    if (it_lib == EDMInterfaceBase::live_objects.end() ||
        it_test == EDMInterfaceBase::live_objects.end()) {
      throw std::invalid_argument("SimplexProjectionInterface::predict: invalid lib_id or test_id.");
    }

    DelayEmbeddingInterface* lib = dynamic_cast<DelayEmbeddingInterface*>(it_lib->second.get());
    DelayEmbeddingInterface* test = dynamic_cast<DelayEmbeddingInterface*>(it_test->second.get());
    if (!lib || !test) {
      throw std::invalid_argument("SimplexProjectionInterface::predict: live objects are not of type DelayEmbeddingInterface.");
    }

    fims_edm::DelayEmbeddingMatrix<double> lib_matrix = to_matrix(lib);
    fims_edm::DelayEmbeddingMatrix<double> test_matrix = to_matrix(test);

    fims_edm::SimplexProjection<double> sp;
    sp.library = &lib_matrix;
    sp.embedding_dimension = this->embedding_dimension;
    sp.n_neighbors = this->n_neighbors;

    sp.predict(test_matrix);

    Rcpp::NumericVector ret(sp.predictions.size());
    for (size_t i = 0; i < sp.predictions.size(); ++i) {
      ret[i] = sp.predictions[i];
    }
    return ret;
  }

  /**
   * @brief Finalizes the interface object.
   */
  virtual void finalize() {
    this->finalized = true;
  }

  /**
   * @brief Converts to JSON representation.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"EDM\",\n";
    ss << " \"module_type\": \"SimplexProjection\",\n";
    ss << " \"module_id\": " << this->id << ",\n";
    ss << " \"embedding_dimension\": " << this->embedding_dimension << ",\n";
    ss << " \"n_neighbors\": " << this->n_neighbors << "\n";
    ss << "}";
    return ss.str();
  }
};

/**
 * @brief Rcpp interface for S-Map projection.
 */
class SMapProjectionInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E).
   */
  uint32_t embedding_dimension;
  /**
   * @brief The S-Map localization parameter (theta).
   */
  double theta;
  /**
   * @brief The S-Map weighting kernel (exponential or gaussian).
   */
  std::string kernel;

  /**
   * @brief The constructor.
   */
  SMapProjectionInterface() : EDMInterfaceBase(), embedding_dimension(0), theta(1.0), kernel("exponential") {
    EDMInterfaceBase::live_objects[this->id] =
        std::make_shared<SMapProjectionInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        EDMInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Copy constructor.
   */
  SMapProjectionInterface(const SMapProjectionInterface& other)
      : EDMInterfaceBase(other),
        embedding_dimension(other.embedding_dimension),
        theta(other.theta),
        kernel(other.kernel) {}

  /**
   * @brief The destructor.
   */
  virtual ~SMapProjectionInterface() {}

  /**
   * @brief Get the ID of the object.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Predict the values using S-Map projection.
   * @param lib_id The ID of the library delay embedding.
   * @param test_id The ID of the test delay embedding.
   * @return A vector of predictions.
   */
  Rcpp::NumericVector predict(uint32_t lib_id, uint32_t test_id) {
    auto it_lib = EDMInterfaceBase::live_objects.find(lib_id);
    auto it_test = EDMInterfaceBase::live_objects.find(test_id);
    if (it_lib == EDMInterfaceBase::live_objects.end() ||
        it_test == EDMInterfaceBase::live_objects.end()) {
      throw std::invalid_argument("SMapProjectionInterface::predict: invalid lib_id or test_id.");
    }

    DelayEmbeddingInterface* lib = dynamic_cast<DelayEmbeddingInterface*>(it_lib->second.get());
    DelayEmbeddingInterface* test = dynamic_cast<DelayEmbeddingInterface*>(it_test->second.get());
    if (!lib || !test) {
      throw std::invalid_argument("SMapProjectionInterface::predict: live objects are not of type DelayEmbeddingInterface.");
    }

    fims_edm::DelayEmbeddingMatrix<double> lib_matrix = to_matrix(lib);
    fims_edm::DelayEmbeddingMatrix<double> test_matrix = to_matrix(test);

    fims_edm::SMapProjection<double> sm;
    sm.library = &lib_matrix;
    sm.embedding_dimension = this->embedding_dimension;
    sm.theta = this->theta;
    if (this->kernel == "gaussian") {
      sm.kernel = fims_edm::SMapKernel::kGaussian;
    } else {
      sm.kernel = fims_edm::SMapKernel::kExponential;
    }

    sm.predict(test_matrix);

    Rcpp::NumericVector ret(sm.predictions.size());
    for (size_t i = 0; i < sm.predictions.size(); ++i) {
      ret[i] = sm.predictions[i];
    }
    return ret;
  }

  /**
   * @brief Finalizes the interface object.
   */
  virtual void finalize() {
    this->finalized = true;
  }

  /**
   * @brief Converts to JSON representation.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"EDM\",\n";
    ss << " \"module_type\": \"SMapProjection\",\n";
    ss << " \"module_id\": " << this->id << ",\n";
    ss << " \"embedding_dimension\": " << this->embedding_dimension << ",\n";
    ss << " \"theta\": " << this->theta << ",\n";
    ss << " \"kernel\": \"" << this->kernel << "\"\n";
    ss << "}";
    return ss.str();
  }
};

/**
 * @brief Rcpp interface for GP-EDM projection.
 */
class GPEdmProjectionInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E).
   */
  uint32_t embedding_dimension;
  /**
   * @brief The ARD length-scale parameters (phi).
   */
  Rcpp::NumericVector phi;
  /**
   * @brief The signal variance parameter (sigma2).
   */
  double sigma2;
  /**
   * @brief The observation noise variance/nugget parameter (ve).
   */
  double ve;

  /**
   * @brief The constructor.
   */
  GPEdmProjectionInterface() : EDMInterfaceBase(), embedding_dimension(0), sigma2(1.0), ve(0.1) {
    EDMInterfaceBase::live_objects[this->id] =
        std::make_shared<GPEdmProjectionInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        EDMInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Copy constructor.
   */
  GPEdmProjectionInterface(const GPEdmProjectionInterface& other)
      : EDMInterfaceBase(other),
        embedding_dimension(other.embedding_dimension),
        phi(other.phi),
        sigma2(other.sigma2),
        ve(other.ve) {}

  /**
   * @brief The destructor.
   */
  virtual ~GPEdmProjectionInterface() {}

  /**
   * @brief Get the ID of the object.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Fits the optimized hyperparameters for GP-EDM.
   * @param lib_id The ID of the library delay embedding.
   * @return A list of optimized parameters.
   */
  Rcpp::List fit(uint32_t lib_id) {
    auto it_lib = EDMInterfaceBase::live_objects.find(lib_id);
    if (it_lib == EDMInterfaceBase::live_objects.end()) {
      throw std::invalid_argument("GPEdmProjectionInterface::fit: invalid lib_id.");
    }

    DelayEmbeddingInterface* lib = dynamic_cast<DelayEmbeddingInterface*>(it_lib->second.get());
    if (!lib) {
      throw std::invalid_argument("GPEdmProjectionInterface::fit: live object is not of type DelayEmbeddingInterface.");
    }
    fims_edm::DelayEmbeddingMatrix<double> lib_matrix = to_matrix(lib);

    fims_edm::GPEdmProjection<double> gp;
    gp.library = &lib_matrix;
    gp.embedding_dimension = this->embedding_dimension;
    gp.sigma2 = this->sigma2;
    gp.ve = this->ve;
    if (this->phi.size() > 0) {
      gp.phi.resize(this->phi.size());
      for (int i = 0; i < this->phi.size(); ++i) {
        gp.phi[i] = this->phi[i];
      }
    }

    gp.fit();

    this->sigma2 = gp.sigma2;
    this->ve = gp.ve;
    this->phi = Rcpp::NumericVector(gp.phi.size());
    for (size_t i = 0; i < gp.phi.size(); ++i) {
      this->phi[i] = gp.phi[i];
    }

    return Rcpp::List::create(
        Rcpp::Named("phi") = this->phi,
        Rcpp::Named("sigma2") = this->sigma2,
        Rcpp::Named("ve") = this->ve
    );
  }

  /**
   * @brief Predict the values using GP-EDM projection.
   * @param lib_id The ID of the library delay embedding.
   * @param test_id The ID of the test delay embedding.
   * @return A vector of predictions.
   */
  Rcpp::NumericVector predict(uint32_t lib_id, uint32_t test_id) {
    auto it_lib = EDMInterfaceBase::live_objects.find(lib_id);
    auto it_test = EDMInterfaceBase::live_objects.find(test_id);
    if (it_lib == EDMInterfaceBase::live_objects.end() ||
        it_test == EDMInterfaceBase::live_objects.end()) {
      throw std::invalid_argument("GPEdmProjectionInterface::predict: invalid lib_id or test_id.");
    }

    DelayEmbeddingInterface* lib = dynamic_cast<DelayEmbeddingInterface*>(it_lib->second.get());
    DelayEmbeddingInterface* test = dynamic_cast<DelayEmbeddingInterface*>(it_test->second.get());
    if (!lib || !test) {
      throw std::invalid_argument("GPEdmProjectionInterface::predict: live objects are not of type DelayEmbeddingInterface.");
    }

    fims_edm::DelayEmbeddingMatrix<double> lib_matrix = to_matrix(lib);
    fims_edm::DelayEmbeddingMatrix<double> test_matrix = to_matrix(test);

    fims_edm::GPEdmProjection<double> gp;
    gp.library = &lib_matrix;
    gp.embedding_dimension = this->embedding_dimension;
    gp.sigma2 = this->sigma2;
    gp.ve = this->ve;
    if (this->phi.size() > 0) {
      gp.phi.resize(this->phi.size());
      for (int i = 0; i < this->phi.size(); ++i) {
        gp.phi[i] = this->phi[i];
      }
    } else {
      gp.phi.assign(this->embedding_dimension, 0.1);
    }

    gp.predict(test_matrix);

    Rcpp::NumericVector ret(gp.predictions.size());
    for (size_t i = 0; i < gp.predictions.size(); ++i) {
      ret[i] = gp.predictions[i];
    }
    return ret;
  }

  /**
   * @brief Finalizes the interface object.
   */
  virtual void finalize() {
    this->finalized = true;
  }

  /**
   * @brief Converts to JSON representation.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"EDM\",\n";
    ss << " \"module_type\": \"GPEdmProjection\",\n";
    ss << " \"module_id\": " << this->id << ",\n";
    ss << " \"embedding_dimension\": " << this->embedding_dimension << ",\n";
    ss << " \"sigma2\": " << this->sigma2 << ",\n";
    ss << " \"ve\": " << this->ve << "\n";
    ss << "}";
    return ss.str();
  }
};

#endif
