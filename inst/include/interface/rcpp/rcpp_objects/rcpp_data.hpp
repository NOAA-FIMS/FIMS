/**
 * @file rcpp_data.hpp
 * @brief The Rcpp interface to declare different types of data, e.g.,
 * age-composition and index data. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DATA_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DATA_HPP

#include "rcpp_interface_base.hpp"
#include "common/information.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp data
 * interfaces. This type should be inherited and not called from R directly.
 */
class DataInterfaceBase : public FIMSRcppInterfaceBase {
 public:
 
  /**
   * @brief The name of the data object.
   */
  SharedString name; 

  /**
   * @brief The dimensions of the data bins, i.e. year, age, etc.
   */
  std::shared_ptr<std::vector<size_t>> dims;
 
  /**
   * @brief The names of the data bins, i.e. year, age, etc.
   */
  std::shared_ptr<std::vector<std::string>> dim_names;

  /**
   * @brief The vector of data that is being passed from R.
   */
  RealVector observed_data;
  
  /**
   * @brief The vector of uncertainty that is being passed from
   * R.
   */
  RealVector uncertainty;

  //TODO: need to add ability to index uncertainty values that have different
  //dimensions to the observed data such as variance-covariance matrices of 
  //multivariate data.

  //TODO: add objects to track the index values explicitly to facilitate
  //not having to rely on the order of the data in the observed_data vector
  //and to allow for missing values and subset options like not starting at 
  //age zero.

  /**
   * @brief The static id of the DataInterfaceBase object.
   */
  static uint32_t id_g;
  
  /**
   * @brief The local id of the DataInterfaceBase object.
   *
   */
  uint32_t id;
  
  /**
   * @brief The map associating the IDs of DataInterfaceBase to the objects.
   * This is a live object, which is an object that has been created and lives
   * in memory.
   */
  static std::map<uint32_t, std::shared_ptr<DataInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  DataInterfaceBase() {
    this->id = DataInterfaceBase::id_g++;
    this->dims = std::make_shared<std::vector<size_t>>();
    this->dim_names = std::make_shared<std::vector<std::string>>();
    /* Create instance of map: key is id and value is pointer to
    DataInterfaceBase */
    // DataInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Data Interface Base object
   *
   * @param other
   */
  DataInterfaceBase(const DataInterfaceBase &other)
      : name(other.name),
        dims(other.dims),
        dim_names(other.dim_names),
        observed_data(other.observed_data),
        uncertainty(other.uncertainty),
        id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~DataInterfaceBase() {}

  /**
   * @brief Get the ID for the child data interface objects to inherit.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Adds the parameters to the TMB model.
   */
  virtual bool add_to_fims_tmb() { return true; };
};

/**
 * @brief  The Rcpp interface to instantiate a data object from R:
 * data <- methods::new(Data).
 */
class DataInterface : public DataInterfaceBase {
 public:
  
  /**
   * @brief The constructor.
   * 
   * @param dims A vector of dimensions for the age-composition data.
   * @param dim_names A vector of dimension names corresponding to the dims vector.
   * @param name The name of the data object.
   */
  DataInterface(const std::string &name, const Rcpp::IntegerVector &dims, 
      const Rcpp::StringVector &dim_names) : DataInterfaceBase() {
    
    this->name.set(name);

    if(dim_names.size() != dims.size()) {
      FIMS_ERROR_LOG("DataInterface::DataInterface(Rcpp::IntegerVector, std::string, Rcpp::StringVector): `dim_names` length (" + std::to_string(dim_names.size()) + ") must equal the requested "
          "dims length (" + std::to_string(dims.size()) + "). Received length: " + std::to_string(dim_names.size()) + ".");
    }

    size_t nvals = 1;
    
    for(size_t i = 0; i < dims.size(); i++) {
      this->dims->push_back(static_cast<size_t>(dims[i]));
      this->dim_names->push_back(static_cast<std::string>(dim_names[i]));
      nvals *= static_cast<size_t>(dims[i]);
    }

    this->observed_data.resize(nvals);
    this->uncertainty.resize(nvals);

    DataInterfaceBase::live_objects[this->id] =
        std::make_shared<DataInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DataInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Data Interface object
   *
   * @param other
   */
  DataInterface(const DataInterface &other)
      : DataInterfaceBase(other) {}

  /**
   * @brief The destructor.
   */
  virtual ~DataInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() override { return this->id; }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * data interface with age-composition data. It also returns the ID, the rank
   * of 2, the dimensions by printing ymax and amax, followed by the data values
   * themselves. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\": \"" << this->name.get() << "\",\n";
    ss << " \"id\":" << this->id << ",\n";
    ss << " \"type\": \"data\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [" ;
    for (size_t i = 0; i < dim_names->size() - 1; i++) {
      ss << (*dim_names)[i] << ", ";
    }
    ss << (*dim_names)[dim_names->size() - 1] << "],\n";
    ss << "  \"dimensions\": ["; 
    for (size_t i = 0; i < dims->size() - 1; i++) {
      ss << (*dims)[i] << ", ";
    }
    ss << (*dims)[dims->size() - 1] << "],\n},\n";
    ss << " \"value\": [";
    for (size_t i = 0; i < observed_data.size() - 1; i++) {
      ss << observed_data[i] << ", ";
    }
    ss << observed_data[observed_data.size() - 1] << "],\n";
    ss << "\"uncertainty\":[ ";
    for (size_t i = 0; i < uncertainty.size() - 1; i++) {
      ss << uncertainty[i] << ", ";
    }
    ss << uncertainty[uncertainty.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_variable_object::VariableObject<Type>> new_object =
        std::make_shared<fims_variable_object::VariableObject<Type>>(
          this->name.get(), fims_variable_object::VariableKind::DETERMINISTIC, *(this->dims), *(this->dim_names));

    new_object->id = this->id;
    
    // NOTE: This setup assumes that observed data and uncertainty are equal 
    // length and also that they are both ordered correctly according to the 
    // dimension assumptions in FIMS for the specific data types.

    for (size_t i = 0; i < observed_data.size(); i++) {
      new_object->values[i] = observed_data[i];
      new_object->uncertainty[i] = uncertainty[i];
    }

    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->variable_objects[this->id] = new_object;

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
 * @brief  The Rcpp interface for AgeComp to instantiate the object from R:
 * acomp <- methods::new(AgeComp).
 */
class AgeCompDataInterface : public DataInterfaceBase {
 public:

  /**
   * @brief The constructor.
   * 
   * @param dims A vector of dimensions for the age-composition data.
   */
  AgeCompDataInterface(int ymax=0, int amax=0) : DataInterfaceBase() {
    
    this->name.set("AgeComp");

    
    this->dim_names->resize(0);
    this->dim_names->reserve(2);
    this->dim_names->push_back("year");
    this->dim_names->push_back("age");
    

    size_t nvals = static_cast<size_t>(ymax)*static_cast<size_t>(amax);
    
    this->dims->push_back(static_cast<size_t>(ymax));
    this->dims->push_back(static_cast<size_t>(amax));
      
    this->observed_data.resize(nvals);
    this->uncertainty.resize(nvals);

    DataInterfaceBase::live_objects[this->id] =
        std::make_shared<AgeCompDataInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DataInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Age Comp Data Interface object
   *
   * @param other
   */
  AgeCompDataInterface(const AgeCompDataInterface &other)
      : DataInterfaceBase(other) {}

  /**
   * @brief The destructor.
   */
  virtual ~AgeCompDataInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * data interface with age-composition data. It also returns the ID, the rank
   * of 2, the dimensions by printing ymax and amax, followed by the data values
   * themselves. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\": \"AgeComp\",\n";
    ss << " \"id\":" << this->id << ",\n";
    ss << " \"type\": \"data\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [";
    for (size_t i = 0; i < dim_names->size() - 1; i++) {
      ss << (*dim_names)[i] << ", ";
    }
    ss << (*dim_names)[dim_names->size() - 1] << "],\n";
    ss << "  \"dimensions\": ["; 
    for (size_t i = 0; i < dims->size() - 1; i++) {
      ss << (*dims)[i] << ", ";
    }
    ss << (*dims)[dims->size() - 1] << "],\n},\n";
    ss << " \"value\": [";
    for (size_t i = 0; i < observed_data.size() - 1; i++) {
      ss << observed_data[i] << ", ";
    }
    ss << observed_data[observed_data.size() - 1] << "],\n";
    ss << "\"uncertainty\":[ ";
    for (size_t i = 0; i < uncertainty.size() - 1; i++) {
      ss << uncertainty[i] << ", ";
    }
    ss << uncertainty[uncertainty.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_variable_object::VariableObject<Type>> age_comp_data =
        std::make_shared<fims_variable_object::VariableObject<Type>>(
          this->name.get(), fims_variable_object::VariableKind::DETERMINISTIC, 
          *(this->dims), *(this->dim_names));

    age_comp_data->id = this->id;

    // NOTE: This setup assumes that observed data and uncertainty are equal 
    // length and also that they are both ordered correctly according to the 
    // dimension assumptions in FIMS for the specific data types.
    // for age comps that is int i_age_year = y * amax + a;

    for (size_t i = 0; i < observed_data.size(); i++) {
      age_comp_data->values[i] = observed_data[i];
      age_comp_data->uncertainty[i] = uncertainty[i];
    }
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->variable_objects[this->id] = age_comp_data;

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
 * @brief The Rcpp interface for LengthComp to instantiate the object from R:
 * lcomp <- methods::new(LengthComp).
 */
class LengthCompDataInterface : public DataInterfaceBase {
 public:

  /**
   * @brief The constructor.
   */
  LengthCompDataInterface(int ymax, int lmax) : DataInterfaceBase() {
 
    this->name.set("LengthComp");

    
    this->dim_names->resize(0);
    this->dim_names->reserve(2);
    this->dim_names->push_back("year");
    this->dim_names->push_back("length");
    

    size_t nvals = static_cast<size_t>(ymax) * static_cast<size_t>(lmax);
    
    this->dims->push_back(static_cast<size_t>(ymax));
    this->dims->push_back(static_cast<size_t>(lmax));
    
    this->observed_data.resize(nvals);
    this->uncertainty.resize(nvals);

    DataInterfaceBase::live_objects[this->id] =
        std::make_shared<LengthCompDataInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DataInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Length Comp Data Interface object
   *
   * @param other
   */
  LengthCompDataInterface(const LengthCompDataInterface &other)
      : DataInterfaceBase(other) {}

  /**
   * @brief The destructor.
   */
  virtual ~LengthCompDataInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * data interface with length-composition data. It also returns the ID, the
   * rank of 2, the dimensions by printing ymax and lmax, followed by the data
   * values themselves. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\": \"LengthComp\",\n";
    ss << " \"id\":" << this->id << ",\n";
    ss << " \"type\": \"data\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [";
    for (size_t i = 0; i < dim_names->size() - 1; i++) {
      ss << (*dim_names)[i] << ", ";
    }
    ss << (*dim_names)[dim_names->size() - 1] << "],\n";
    ss << "  \"dimensions\": ["; 
    for (size_t i = 0; i < dims->size() - 1; i++) {
      ss << (*dims)[i] << ", ";
    }
    ss << (*dims)[dims->size() - 1] << "],\n},\n";
    ss << " \"value\": [";
    for (size_t i = 0; i < observed_data.size() - 1; i++) {
      ss << observed_data[i] << ", ";
    }
    ss << observed_data[observed_data.size() - 1] << "],\n";
    ss << "\"uncertainty\":[ ";
    for (size_t i = 0; i < uncertainty.size() - 1; i++) {
      ss << uncertainty[i] << ", ";
    }
    ss << uncertainty[uncertainty.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_variable_object::VariableObject<Type>> length_comp_data =
        std::make_shared<fims_variable_object::VariableObject<Type>>(
          this->name.get(), fims_variable_object::VariableKind::DETERMINISTIC, 
          *(this->dims), *(this->dim_names));

    length_comp_data->id = this->id;
    
    // NOTE: This setup assumes that observed data and uncertainty are equal 
    // length and also that they are both ordered correctly according to the 
    // dimension assumptions in FIMS for the specific data types.
    // for length comps that is int i_length_year = y * lmax + l;

    for (size_t i = 0; i < observed_data.size(); i++) {
      length_comp_data->values[i] = observed_data[i];
      length_comp_data->uncertainty[i] = uncertainty[i];
    }

    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();
    info->variable_objects[this->id] = length_comp_data;
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
 * @brief  The Rcpp interface for Index to instantiate the object from R:
 * fleet <- methods::new(Index).
 */
class IndexDataInterface : public DataInterfaceBase {
 public:

  /**
   * @brief The constructor.
   */
  IndexDataInterface(int ymax = 0) : DataInterfaceBase() {
    
    this->name.set("Index");

    
    this->dim_names->resize(0);
    this->dim_names->reserve(1);
    this->dim_names->push_back("year");
    
    size_t nvals = static_cast<size_t>(ymax);
    
    this->dims->push_back(static_cast<size_t>(ymax));
    
    this->observed_data.resize(nvals);
    this->uncertainty.resize(nvals);

    DataInterfaceBase::live_objects[this->id] =
        std::make_shared<IndexDataInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DataInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Index Data Interface object
   *
   * @param other
   */
  IndexDataInterface(const IndexDataInterface &other)
      : DataInterfaceBase(other){}

  /**
   * @brief The destructor.
   */
  virtual ~IndexDataInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * data interface with index data. It also returns the ID, the rank of 1, the
   * dimensions by printing ymax, followed by the data values themselves. This
   * string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\": \"Index\",\n";
    ss << " \"id\": " << this->id << ",\n";
    ss << " \"type\": \"data\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [" << "\"n_years\"" << "],\n";
    ss << "  \"dimensions\": [" << (*dims)[0] << "]\n},\n";
    ss << " \"value\": [";
    for (size_t i = 0; i < observed_data.size() - 1; i++) {
      ss << observed_data[i] << ", ";
    }
    ss << observed_data[observed_data.size() - 1] << "],\n";
    ss << "\"uncertainty\": [ ";
    for (size_t i = 0; i < uncertainty.size() - 1; i++) {
      ss << uncertainty[i] << ", ";
    }
    ss << uncertainty[uncertainty.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_variable_object::VariableObject<Type>> data =
        std::make_shared<fims_variable_object::VariableObject<Type>>(
          this->name.get(), fims_variable_object::VariableKind::DETERMINISTIC, 
          *(this->dims), *(this->dim_names));

    data->id = this->id;

    for (int y = 0; y < (*dims)[0]; y++) {
      data->values[y] = this->observed_data[y];
      data->uncertainty[y] = this->uncertainty[y];
    }

    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->variable_objects[this->id] = data;
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
 * @brief  The Rcpp interface for Landings to instantiate the object from R:
 * fleet <- methods::new(Landings).
 */
class LandingsDataInterface : public DataInterfaceBase {
 public:

  /**
   * @brief The constructor.
   */
  LandingsDataInterface(int ymax = 0) : DataInterfaceBase() {
    this->name.set("Landings");

    this->dim_names->resize(0);
    this->dim_names->reserve(1);
    this->dim_names->push_back("year");
    
    size_t nvals = static_cast<size_t>(ymax);
    
    this->dims->push_back(static_cast<size_t>(ymax));
    
    this->observed_data.resize(nvals);
    this->uncertainty.resize(nvals);

    DataInterfaceBase::live_objects[this->id] =
        std::make_shared<LandingsDataInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DataInterfaceBase::live_objects[this->id]);
  }

  /**
   * @brief Construct a new Landings Data Interface object
   *
   * @param other
   */
  LandingsDataInterface(const LandingsDataInterface &other)
      : DataInterfaceBase(other) {}

  /**
   * @brief The destructor.
   */
  virtual ~LandingsDataInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * data interface with landings data. It also returns the ID, the rank of 1,
   * the dimensions by printing ymax, followed by the data values themselves.
   * This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\": \"Landings\",\n";
    ss << " \"id\": " << this->id << ",\n";
    ss << " \"type\": \"data\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [" << "\"n_years\"" << "],\n";
    ss << "  \"dimensions\": [" << (*dims)[0] << "]\n},\n";
    ss << " \"value\": [";
    for (size_t i = 0; i < observed_data.size() - 1; i++) {
      ss << observed_data[i] << ", ";
    }
    ss << observed_data[observed_data.size() - 1] << "],\n";
    ss << "\"uncertainty\": [ ";
    for (size_t i = 0; i < uncertainty.size() - 1; i++) {
      ss << uncertainty[i] << ", ";
    }
    ss << uncertainty[uncertainty.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_variable_object::VariableObject<Type>> data =
        std::make_shared<fims_variable_object::VariableObject<Type>>(
          this->name.get(), fims_variable_object::VariableKind::DETERMINISTIC, 
          *(this->dims), *(this->dim_names));

    data->id = this->id;

    for (int y = 0; y < (*dims)[0]; y++) {
      data->values[y] = this->observed_data[y];
      data->uncertainty[y] = this->uncertainty[y];
    }

    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->variable_objects[this->id] = data;
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
