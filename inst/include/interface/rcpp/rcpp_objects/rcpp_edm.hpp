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
 * @class SharedDouble
 * @brief A class that provides shared ownership of a double value.
 */
class SharedDouble {
 private:
  std::shared_ptr<double> value;

 public:
  SharedDouble() : value(std::make_shared<double>(0.0)) {}
  SharedDouble(double val) : value(std::make_shared<double>(val)) {}
  SharedDouble(const SharedDouble& other) : value(other.value) {}
  SharedDouble& operator=(const SharedDouble& other) {
    if (this != &other) {
      value = other.value;
    }
    return *this;
  }
  SharedDouble& operator=(const double& other) {
    *value = other;
    return *this;
  }
  operator double() const { return *value; }
  double get() const { return *value; }
  void set(double val) { *value = val; }
};

RCPP_EXPOSED_CLASS(SharedDouble)


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
  EDMInterfaceBase(const EDMInterfaceBase& other) 
        : id(other.id) {}

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
  SharedInt embedding_dimension;
  /**
   * @brief The time lag (tau) between successive coordinates in the
   * embedding.
   */
  SharedInt time_lag;
  /**
   * @brief TODO
   */
  SharedInt forecast_horizon;
  /**
   * @brief The number of rows in the constructed delay embedding matrix.
   */
  SharedInt n_rows;
  /**
   * @brief The number of columns in the constructed delay embedding matrix
   * (equal to embedding_dimension).
   */
  SharedInt n_cols;
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

  int get_embedding_dimension() const { return this->embedding_dimension.get(); }
  void set_embedding_dimension(int val) { this->embedding_dimension.set(val); }
  int get_time_lag() const { return this->time_lag.get(); }
  void set_time_lag(int val) { this->time_lag.set(val); }
  int get_n_rows() const { return this->n_rows.get(); }
  void set_n_rows(int val) { this->n_rows.set(val); }
  int get_n_cols() const { return this->n_cols.get(); }
  void set_n_cols(int val) { this->n_cols.set(val); }

  /**
   * @brief The constructor.
   */
  DelayEmbeddingInterface()
      : EDMInterfaceBase(),
        embedding_dimension(0),
        time_lag(0),
        forecast_horizon(0),
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
        forecast_horizon(other.forecast_horizon),
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
   * @param forecast_horizon Positive number of time steps ahead to predict.
   */
  void construct(Rcpp::NumericVector series, uint32_t E, uint32_t tau,
                 uint32_t forecast_horizon) {
    ConstructInternal(series, E, tau, forecast_horizon,
                      Rcpp::NumericVector(), false, 0.0);
  }

  /**
   * @brief Construct a delay embedding matrix from a time series vector,
   * propagating an accompanying uncertainty series.
   *
   * @details Same as construct() but also populates embedded_uncertainty and
   * target_uncertainty following the same pointer-based layout as the value
   * fields. @p uncertainty must have the same length as @p series.
   * Uses the forecast horizon stored by a previous construction call;
   * constructing a fresh object with this method raises an error.
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
    ConstructInternal(series, E, tau, this->forecast_horizon.get(),
                      uncertainty, false, 0.0);
  }

  /**
   * @brief Construct a delay embedding matrix from a time series, dropping
   * rows that contain the specified missing value (no uncertainty propagation).
   *
   * @param series An Rcpp::NumericVector representing the univariate time
   * series.
   * @param E The embedding dimension (must be > 0).
   * @param tau The time lag (must be > 0).
   * @param forecast_horizon Positive number of time steps ahead to predict.
   * @param missing_value The sentinel value representing missing data
   * (e.g., -999.0).
   */
  void construct_drop_missing(Rcpp::NumericVector series,
                              uint32_t E,
                              uint32_t tau,
                              uint32_t forecast_horizon,
                              double missing_value) {
    ConstructInternal(series, E, tau, forecast_horizon,
                      Rcpp::NumericVector(), true, missing_value);
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
   * @param forecast_horizon Positive number of time steps ahead to predict.
   * @param missing_value The sentinel value representing missing data.
   * @param uncertainty An Rcpp::NumericVector of uncertainty values with the
   * same length as @p series.
   */
  void construct_drop_missing_with_uncertainty(
      Rcpp::NumericVector series,
      uint32_t E,
      uint32_t tau,
      uint32_t forecast_horizon,
      double missing_value,
      Rcpp::NumericVector uncertainty) {
    ConstructInternal(series, E, tau, forecast_horizon,
                      uncertainty, true, missing_value);
  }

  /**
   * @brief Access an element of the delay embedding matrix by row and column.
   *
   * @param row Zero-based row index.
   * @param col Zero-based column index.
   * @return The value at (row, col) in the embedding matrix.
   */
  double at(uint32_t row, uint32_t col) {
    if (row >= (uint32_t)(int)this->n_rows || col >= (uint32_t)(int)this->n_cols) {
      throw std::invalid_argument(
          "DelayEmbeddingInterface::at() index out of bounds.");
    }
    return this->embedded_values[row * (int)this->n_cols + col];
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

 private:
  static fims::Vector<double> ToFimsVector(
      const Rcpp::NumericVector& values) {
    fims::Vector<double> result;
    result.resize(values.size());
    for (R_xlen_t i = 0; i < values.size(); ++i) {
      result[i] = values[i];
    }
    return result;
  }

  static void CopyEmbeddingValues(const std::vector<const double*>& source,
                                  RealVector& destination) {
    destination.resize(source.size());
    for (size_t i = 0; i < source.size(); ++i) {
      destination[i] = *source[i];
    }
  }

  void ConstructInternal(const Rcpp::NumericVector& series, uint32_t dimension,
                         uint32_t lag, uint32_t horizon,
                         const Rcpp::NumericVector& uncertainty,
                         bool drop_missing, double missing_value) {
    const auto values = ToFimsVector(series);
    const auto uncertainties = ToFimsVector(uncertainty);
    const auto embedding = drop_missing
        ? fims_edm::MakeDelayEmbeddingDropMissing(
              values, dimension, lag, horizon, missing_value, uncertainties)
        : fims_edm::MakeDelayEmbedding(
              values, dimension, lag, horizon, uncertainties);

    this->embedding_dimension = dimension;
    this->time_lag = lag;
    this->forecast_horizon = horizon;
    this->n_rows = embedding.n_rows;
    this->n_cols = embedding.n_cols;

    // Dereference while the temporary input storage is still alive.
    CopyEmbeddingValues(embedding.embedded_values, this->embedded_values);
    CopyEmbeddingValues(embedding.target_values, this->target_values);
    CopyEmbeddingValues(embedding.embedded_uncertainty,
                        this->embedded_uncertainty);
    CopyEmbeddingValues(embedding.target_uncertainty, this->target_uncertainty);
  }
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
 * @brief Keep both registered embeddings alive while prediction uses their views.
 */
struct EDMPredictionEmbeddings {
  std::shared_ptr<DelayEmbeddingInterface> library;
  std::shared_ptr<DelayEmbeddingInterface> test;
};

inline EDMPredictionEmbeddings get_edm_prediction_embeddings(
    uint32_t lib_id, uint32_t test_id, const std::string& context) {
  auto it_lib = EDMInterfaceBase::live_objects.find(lib_id);
  auto it_test = EDMInterfaceBase::live_objects.find(test_id);
  if (it_lib == EDMInterfaceBase::live_objects.end() ||
      it_test == EDMInterfaceBase::live_objects.end()) {
    throw std::invalid_argument(context + ": invalid lib_id or test_id.");
  }
  auto library = std::dynamic_pointer_cast<DelayEmbeddingInterface>(
      it_lib->second);
  auto test = std::dynamic_pointer_cast<DelayEmbeddingInterface>(
      it_test->second);
  if (!library || !test) {
    throw std::invalid_argument(
        context + ": live objects are not of type DelayEmbeddingInterface.");
  }
  return {library, test};
}

inline Rcpp::NumericVector edm_predictions_to_r(
    const std::vector<double>& predictions) {
  return Rcpp::NumericVector(predictions.begin(), predictions.end());
}

/**
 * @brief Copy evaluated predictions, leaving output unchanged without a model.
 */
inline void copy_edm_model_predictions(uint32_t id, RealVector& predictions) {
  const auto information = fims_info::Information<double>::GetInstance();
  const auto model = information->edm_models.find(id);
  if (model != information->edm_models.end()) {
    const auto& values = model->second->predictions;
    predictions.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      predictions[i] = values[i];
    }
  }
}

/**
 * @brief Copy one embedding into model-owned typed storage and bind its matrix.
 */
template <typename Type>
void populate_edm_matrix(
    const DelayEmbeddingInterface& source,
    fims_edm::DelayEmbeddingMatrix<Type>& matrix,
    std::vector<Type>& embedded_values_storage,
    std::vector<Type>& target_values_storage,
    std::vector<Type>& embedded_uncertainty_storage,
    std::vector<Type>& target_uncertainty_storage) {
  matrix.n_rows = source.target_values.storage_m->size();
  matrix.n_cols = matrix.n_rows > 0
      ? source.embedded_values.storage_m->size() / matrix.n_rows : 0;

  const auto copy_and_bind = [](const RealVector& values,
                                std::vector<Type>& storage,
                                std::vector<const Type*>& pointers) {
    storage.resize(values.storage_m->size());
    for (size_t i = 0; i < storage.size(); ++i) {
      storage[i] = Type((*values.storage_m)[i]);
    }
    // Bind only after resizing the model-owned backing storage.
    pointers.resize(storage.size());
    for (size_t i = 0; i < storage.size(); ++i) {
      pointers[i] = &storage[i];
    }
  };

  copy_and_bind(source.embedded_values, embedded_values_storage,
                matrix.embedded_values);
  copy_and_bind(source.target_values, target_values_storage,
                matrix.target_values);
  if (source.embedded_uncertainty.storage_m &&
      !source.embedded_uncertainty.storage_m->empty()) {
    copy_and_bind(source.embedded_uncertainty, embedded_uncertainty_storage,
                  matrix.embedded_uncertainty);
  }
  if (source.target_uncertainty.storage_m &&
      !source.target_uncertainty.storage_m->empty()) {
    copy_and_bind(source.target_uncertainty, target_uncertainty_storage,
                  matrix.target_uncertainty);
  }
}

/**
 * @brief Populate library and test matrices using their separate owned storage.
 */
template <typename Type>
void populate_edm_model_matrices(
    DelayEmbeddingInterface* de_lib,
    DelayEmbeddingInterface* de_test,
    std::shared_ptr<fims_edm::EDMModel<Type>>& model) {
  populate_edm_matrix(
      *de_lib, model->library_matrix, model->library_embedded_values_storage,
      model->library_target_values_storage,
      model->library_embedded_uncertainty_storage,
      model->library_target_uncertainty_storage);
  populate_edm_matrix(
      *de_test, model->test_matrix, model->test_embedded_values_storage,
      model->test_target_values_storage,
      model->test_embedded_uncertainty_storage,
      model->test_target_uncertainty_storage);
}

#ifdef TMB_MODEL
/**
 * @brief Validate embeddings, populate owned matrices, and register an EDM model.
 */
template <typename Type>
bool register_edm_model(
    uint32_t lib_id, uint32_t test_id,
    std::shared_ptr<fims_edm::EDMModel<Type>>& model,
    const std::string& context) {
  auto it_lib = EDMInterfaceBase::live_objects.find(lib_id);
  auto it_test = EDMInterfaceBase::live_objects.find(test_id);
  if (it_lib == EDMInterfaceBase::live_objects.end() ||
      it_test == EDMInterfaceBase::live_objects.end()) {
    FIMS_ERROR_LOG(context + ": invalid lib_de_id or test_de_id.");
    return false;
  }

  auto* library = dynamic_cast<DelayEmbeddingInterface*>(it_lib->second.get());
  auto* test = dynamic_cast<DelayEmbeddingInterface*>(it_test->second.get());
  if (!library || !test) {
    FIMS_ERROR_LOG(context + ": objects are not DelayEmbeddingInterface.");
    return false;
  }

  populate_edm_model_matrices<Type>(library, test, model);
  fims_info::Information<Type>::GetInstance()->edm_models[model->id] = model;
  return true;
}
#endif

/**
 * @brief Rcpp interface for Simplex projection.
 */
class SimplexProjectionInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E).
   */
  SharedInt embedding_dimension;
  /**
   * @brief The number of neighbors (k).
   */
  SharedInt n_neighbors;
  /**
   * @brief The library delay embedding ID.
   */
  SharedInt lib_de_id = 0;
  /**
   * @brief The test delay embedding ID.
   */
  SharedInt test_de_id = 0;
  /**
   * @brief The forecast horizon.
   */
  SharedInt forecast_horizon = 1;
  SharedInt time_exclusion_radius = 0;
  /**
   * @brief Prediction output vector.
   */
  RealVector predictions;

  int get_embedding_dimension() const { return this->embedding_dimension.get(); }
  void set_embedding_dimension(int val) { this->embedding_dimension.set(val); }
  int get_n_neighbors() const { return this->n_neighbors.get(); }
  void set_n_neighbors(int val) { this->n_neighbors.set(val); }
  int get_lib_de_id() const { return this->lib_de_id.get(); }
  void set_lib_de_id(int val) { this->lib_de_id.set(val); }
  int get_test_de_id() const { return this->test_de_id.get(); }
  void set_test_de_id(int val) { this->test_de_id.set(val); }
  int get_forecast_horizon() const { return this->forecast_horizon.get(); }
  void set_forecast_horizon(int val) { this->forecast_horizon.set(val); }
  int get_time_exclusion_radius() const { return this->time_exclusion_radius.get(); }
  void set_time_exclusion_radius(int val) { this->time_exclusion_radius.set(val); }

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
        n_neighbors(other.n_neighbors),
        lib_de_id(other.lib_de_id),
        test_de_id(other.test_de_id),
        forecast_horizon(other.forecast_horizon),
        time_exclusion_radius(other.time_exclusion_radius), 
        predictions(other.predictions) {}

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
    const auto embeddings = get_edm_prediction_embeddings(
        lib_id, test_id, "SimplexProjectionInterface::predict");
    auto lib_matrix = to_matrix(embeddings.library.get());
    auto test_matrix = to_matrix(embeddings.test.get());

    fims_edm::SimplexProjection<double> sp;
    sp.library = &lib_matrix;
    sp.embedding_dimension = this->embedding_dimension;
    sp.n_neighbors = this->n_neighbors;

    sp.predict(test_matrix);

    return edm_predictions_to_r(sp.predictions);
  }

  /**
   * @brief Finalizes the interface object.
   */
  virtual void finalize() {
    if (this->finalized) {
      FIMS_WARNING_LOG("SimplexProjection " + fims::to_string(this->id) +
                       " has been finalized already.");
    }
    this->finalized = true;

    copy_edm_model_predictions(this->id, this->predictions);
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

#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_edm::EDMModel<Type>> model =
        std::make_shared<fims_edm::EDMModel<Type>>();
    model->id = this->id;
    model->predictor_type = "simplex";
    model->embedding_dimension = this->embedding_dimension;
    model->n_neighbors = this->n_neighbors;
    model->forecast_horizon = this->forecast_horizon;

    return register_edm_model<Type>(
        this->lib_de_id, this->test_de_id, model, "SimplexProjectionInterface");
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
#endif
};

/**
 * @brief Rcpp interface for S-Map projection.
 */
class SMapProjectionInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E).
   */
  SharedInt embedding_dimension;
  /**
   * @brief The S-Map localization parameter (theta).
   */
  SharedDouble theta;
  /**
   * @brief The S-Map weighting kernel (exponential or gaussian).
   */
  SharedString kernel;
  /**
   * @brief The library delay embedding ID.
   */
  SharedInt lib_de_id = 0;
  /**
   * @brief The test delay embedding ID.
   */
  SharedInt test_de_id = 0;
  /**
   * @brief The forecast horizon.
   */
  SharedInt forecast_horizon = 1;
    /** @brief Exclude any points closer than or equal in time
   *  to the point in integer time. This is forward and backward in time.
   */
  SharedInt time_exclusion_radius = 0;
  /**
   * @brief Prediction output vector.
   */
  RealVector predictions;

  int get_embedding_dimension() const { return this->embedding_dimension.get(); }
  void set_embedding_dimension(int val) { this->embedding_dimension.set(val); }
  double get_theta() const { return this->theta.get(); }
  void set_theta(double val) { this->theta.set(val); }
  std::string get_kernel() const { return this->kernel.get(); }
  void set_kernel(std::string val) { this->kernel.set(val); }
  int get_lib_de_id() const { return this->lib_de_id.get(); }
  void set_lib_de_id(int val) { this->lib_de_id.set(val); }
  int get_test_de_id() const { return this->test_de_id.get(); }
  void set_test_de_id(int val) { this->test_de_id.set(val); }
  int get_forecast_horizon() const { return this->forecast_horizon.get(); }
  void set_forecast_horizon(int val) { this->forecast_horizon.set(val); }

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
        kernel(other.kernel),
        lib_de_id(other.lib_de_id),
        test_de_id(other.test_de_id),
        forecast_horizon(other.forecast_horizon),
        predictions(other.predictions) {}

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
    const auto embeddings = get_edm_prediction_embeddings(
        lib_id, test_id, "SMapProjectionInterface::predict");
    auto lib_matrix = to_matrix(embeddings.library.get());
    auto test_matrix = to_matrix(embeddings.test.get());

    fims_edm::SMapProjection<double> sm;
    sm.library = &lib_matrix;
    sm.embedding_dimension = this->embedding_dimension;
    sm.theta = this->theta;
    if (this->kernel.get() == "gaussian") {
      sm.kernel = fims_edm::SMapKernel::kGaussian;
    } else {
      sm.kernel = fims_edm::SMapKernel::kExponential;
    }

    sm.predict(test_matrix);

    return edm_predictions_to_r(sm.predictions);
  }

  /**
   * @brief Finalizes the interface object.
   */
  virtual void finalize() {
    if (this->finalized) {
      FIMS_WARNING_LOG("SMapProjection " + fims::to_string(this->id) +
                       " has been finalized already.");
    }
    this->finalized = true;

    copy_edm_model_predictions(this->id, this->predictions);
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

#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_edm::EDMModel<Type>> model =
        std::make_shared<fims_edm::EDMModel<Type>>();
    model->id = this->id;
    model->predictor_type = "smap";
    model->embedding_dimension = this->embedding_dimension;
    model->theta = this->theta;
    model->kernel = this->kernel;
    model->forecast_horizon = this->forecast_horizon;

    return register_edm_model<Type>(
        this->lib_de_id, this->test_de_id, model, "SMapProjectionInterface");
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
#endif
};

/**
 * @brief Rcpp interface for GP-EDM projection.
 */
class GPEdmProjectionInterface : public EDMInterfaceBase {
 public:
  /**
   * @brief The embedding dimension (E).
   */
  SharedInt embedding_dimension;
  /**
   * @brief The ARD length-scale parameters (phi).
   * @details Assign through set_phi() to keep the registered model copy in sync.
   */
  Rcpp::NumericVector phi;
  /**
   * @brief The signal variance parameter (sigma2).
   */
  SharedDouble sigma2;
  /**
   * @brief The observation noise variance/nugget parameter (ve).
   */
  SharedDouble ve;
  /**
   * @brief The library delay embedding ID.
   */
  SharedInt lib_de_id = 0;
  /**
   * @brief The test delay embedding ID.
   */
  SharedInt test_de_id = 0;
  /**
   * @brief The forecast horizon.
   */
  SharedInt forecast_horizon = 1;
  /**
   * @brief Prediction output vector.
   */
  RealVector predictions;

  int get_embedding_dimension() const { return this->embedding_dimension.get(); }
  void set_embedding_dimension(int val) { this->embedding_dimension.set(val); }
  Rcpp::NumericVector get_phi() const { return this->phi; }
  void set_phi(Rcpp::NumericVector values) {
    auto registered = std::dynamic_pointer_cast<GPEdmProjectionInterface>(
        EDMInterfaceBase::live_objects.at(this->id));
    if (!registered) {
      throw std::runtime_error(
          "GPEdmProjectionInterface::set_phi: registered object has wrong type.");
    }
    // Rcpp vector assignment replaces storage rather than updating the copy.
    this->phi = values;
    registered->phi = values;
  }
  double get_sigma2() const { return this->sigma2.get(); }
  void set_sigma2(double val) { this->sigma2.set(val); }
  double get_ve() const { return this->ve.get(); }
  void set_ve(double val) { this->ve.set(val); }
  int get_lib_de_id() const { return this->lib_de_id.get(); }
  void set_lib_de_id(int val) { this->lib_de_id.set(val); }
  int get_test_de_id() const { return this->test_de_id.get(); }
  void set_test_de_id(int val) { this->test_de_id.set(val); }
  int get_forecast_horizon() const { return this->forecast_horizon.get(); }
  void set_forecast_horizon(int val) { this->forecast_horizon.set(val); }

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
        ve(other.ve),
        lib_de_id(other.lib_de_id),
        test_de_id(other.test_de_id),
        forecast_horizon(other.forecast_horizon),
        predictions(other.predictions) {}

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
    Rcpp::NumericVector fitted_phi(gp.phi.size());
    for (size_t i = 0; i < gp.phi.size(); ++i) {
      fitted_phi[i] = gp.phi[i];
    }
    this->set_phi(fitted_phi);

    return Rcpp::List::create(
        Rcpp::Named("phi") = this->phi,
        Rcpp::Named("sigma2") = this->sigma2.get(),
        Rcpp::Named("ve") = this->ve.get()
    );
  }

  /**
   * @brief Predict the values using GP-EDM projection.
   * @param lib_id The ID of the library delay embedding.
   * @param test_id The ID of the test delay embedding.
   * @return A vector of predictions.
   */
  Rcpp::NumericVector predict(uint32_t lib_id, uint32_t test_id) {
    const auto embeddings = get_edm_prediction_embeddings(
        lib_id, test_id, "GPEdmProjectionInterface::predict");
    auto lib_matrix = to_matrix(embeddings.library.get());
    auto test_matrix = to_matrix(embeddings.test.get());

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

    return edm_predictions_to_r(gp.predictions);
  }

  /**
   * @brief Finalizes the interface object.
   */
  virtual void finalize() {
    if (this->finalized) {
      FIMS_WARNING_LOG("GPEdmProjection " + fims::to_string(this->id) +
                       " has been finalized already.");
    }
    this->finalized = true;

    copy_edm_model_predictions(this->id, this->predictions);
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

#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_edm::EDMModel<Type>> model =
        std::make_shared<fims_edm::EDMModel<Type>>();
    model->id = this->id;
    model->predictor_type = "gp_edm";
    model->embedding_dimension = this->embedding_dimension;
    model->sigma2 = this->sigma2;
    model->ve = this->ve;
    model->forecast_horizon = this->forecast_horizon;

    if (this->phi.size() > 0) {
      model->phi.resize(this->phi.size());
      for (int i = 0; i < this->phi.size(); ++i) {
        model->phi[i] = this->phi[i];
      }
    } else {
      model->phi.assign(this->embedding_dimension, 0.1);
    }

    return register_edm_model<Type>(
        this->lib_de_id, this->test_de_id, model, "GPEdmProjectionInterface");
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
    return true;
  }
#endif
};

#endif
