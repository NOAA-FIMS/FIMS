/**
 * @file rcpp_fleet.hpp
 * @brief The Rcpp interface to declare different types of data, e.g.,
 * age-composition and index data. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DATA_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DATA_HPP

#include "../../../common/information.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp data
 * interfaces. This type should be inherited and not called from R directly.
 */
class DataInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The vector of data that is being passed from R.
   */
  Rcpp::NumericVector observed_data;
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
  static std::map<uint32_t, DataInterfaceBase*> live_objects;

  /**
   * @brief The constructor.
   */
  DataInterfaceBase() {
    this->id = DataInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    DataInterfaceBase */
    DataInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Data Interface Base object
   *
   * @param other
   */
  DataInterfaceBase(const DataInterfaceBase& other) :
  observed_data(other.observed_data), id(other.id) {
  }

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
// static id of the DataInterfaceBase object
uint32_t DataInterfaceBase::id_g = 1;
// local id of the DataInterfaceBase object map relating the ID of the
// DataInterfaceBase to the DataInterfaceBase objects
std::map<uint32_t, DataInterfaceBase*> DataInterfaceBase::live_objects;

/**
 * @brief  The Rcpp interface for AgeComp to instantiate the object from R:
 * acomp <- methods::new(AgeComp).
 */
class AgeCompDataInterface : public DataInterfaceBase {
 public:
  /**
   * @brief The first dimension of the data, which relates to the number of age
   * bins.
   */
  fims_int amax = 0;
  /**
   * @brief The second dimension of the data, which relates to the number of
   * time steps or years.
   */
  fims_int ymax = 0;
  /**
   * @brief The vector of age-composition data that is being passed from R.
   */
  RealVector age_comp_data;

  /**
   * @brief The constructor.
   */
  AgeCompDataInterface(int ymax = 0, int amax = 0) : DataInterfaceBase() {
    this->amax = amax;
    this->ymax = ymax;
    this->age_comp_data.resize(amax*ymax);

    FIMSRcppInterfaceBase::fims_interface_objects.push_back(std::make_shared<AgeCompDataInterface>(*this));
  }

  /**
   * @brief Construct a new Age Comp Data Interface object
   *
   * @param other
   */
  AgeCompDataInterface(const AgeCompDataInterface& other) :
  DataInterfaceBase(other), amax(other.amax), ymax(other.ymax), age_comp_data(other.age_comp_data) {}

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
    ss << " \"name\": \"data\",\n";
    ss << " \"type\" : \"AgeComp\",\n";
    ss << " \"id\":" << this->id << ",\n";
    ss << " \"rank\": " << 2 << ",\n";
    ss << " \"dimensions\": [" << this->ymax << "," << this->amax << "],\n";
    ss << " \"values\": [";
    for (R_xlen_t i = 0; i < age_comp_data.size() - 1; i++) {
        ss << age_comp_data[i] << ", ";
    }
    ss << age_comp_data[age_comp_data.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }
  

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_data_object::DataObject<Type>> age_comp_data =
        std::make_shared<fims_data_object::DataObject<Type>>(Type(this->ymax),
                                                             Type(this->amax));

    age_comp_data->id = this->id;
    for (int y = 0; y < ymax; y++) {
      for (int a = 0; a < amax; a++) {
        int i_age_year = y * amax + a;
        age_comp_data->at(y, a) = this->age_comp_data[i_age_year];
      }
    }

    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->data_objects[this->id] = age_comp_data;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

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
   * @brief The first dimension of the data, which relates to the number of
   * length bins.
   */
  fims_int lmax;
  /**
   * @brief The second dimension of the data, which relates to the number of
   * time steps or years.
   */
  fims_int ymax = 0;
  /**
   * @brief The vector of length-composition data that is being passed from R.
   */
  RealVector length_comp_data;

  /**
   * @brief The constructor.
   */
  LengthCompDataInterface(int ymax = 0, int lmax = 0) : DataInterfaceBase() {
    this->lmax = lmax;
    this->ymax = ymax;
    this->length_comp_data.resize(lmax*ymax);

    FIMSRcppInterfaceBase::fims_interface_objects.push_back(std::make_shared<LengthCompDataInterface>(*this));
  }

  /**
   * @brief Construct a new Length Comp Data Interface object
   *
   * @param other
   */
  LengthCompDataInterface(const LengthCompDataInterface& other) :
  DataInterfaceBase(other), lmax(other.lmax), ymax(other.ymax), length_comp_data(other.length_comp_data) {}

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
    ss << " \"name\": \"data\",\n";
    ss << " \"type\" : \"LengthComp\",\n";
    ss << " \"id\":" << this->id << ",\n";
    ss << " \"rank\": " << 2 << ",\n";
    ss << " \"dimensions\": [" << this->ymax << "," << this->lmax << "],\n";
    ss << " \"values\": [";
    for (R_xlen_t i = 0; i < length_comp_data.size() - 1; i++) {
        ss << length_comp_data[i] << ", ";
    }
    ss << length_comp_data[length_comp_data.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }
  
#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_data_object::DataObject<Type>> length_comp_data =
        std::make_shared<fims_data_object::DataObject<Type>>(Type(this->ymax),
                                                             Type(this->lmax));
    length_comp_data->id = this->id;
    for (int y = 0; y < ymax; y++) {
      for (int l = 0; l < lmax; l++) {
        int i_length_year = y * lmax + l;
        length_comp_data->at(y, l) = this->length_comp_data[i_length_year];
      }
    }
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();
    info->data_objects[this->id] = length_comp_data;
    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
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
   * @brief An integer that specifies the second dimension of the data.
   */
  fims_int ymax = 0;
  /**
   * @brief The vector of index data that is being passed from R.
   */
  RealVector index_data;

  /**
   * @brief The constructor.
   */
  IndexDataInterface(int ymax = 0) : DataInterfaceBase() {
    this->ymax = ymax;
    this->index_data.resize(ymax);
    
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(std::make_shared<IndexDataInterface>(*this));
  }

  /**
   * @brief Construct a new Index Data Interface object
   *
   * @param other
   */
  IndexDataInterface(const IndexDataInterface& other) :
  DataInterfaceBase(other), ymax(other.ymax), index_data(other.index_data) {}

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
    ss << " \"name\": \"data\",\n";
    ss << " \"type\": \"Index\",\n";
    ss << " \"id\": " << this->id << ",\n";
    ss << " \"rank\": " << 1 << ",\n";
    ss << " \"dimensions\": [" << this->ymax << "],\n";
    ss << " \"values\": [";
    for (R_xlen_t i = 0; i < index_data.size() - 1; i++) {
        ss << index_data[i] << ", ";
    }
    ss << index_data[index_data.size() - 1] << "]\n";
    ss << "}";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_data_object::DataObject<Type>> data =
        std::make_shared<fims_data_object::DataObject<Type>>(Type(this->ymax));

    data->id = this->id;

    for (int y = 0; y < ymax; y++) {
      data->at(y) = this->index_data[y];
    }

    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    info->data_objects[this->id] = data;
    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif
