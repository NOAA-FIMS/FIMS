/*
 * File:   rcpp_fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DATA_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DATA_HPP

#include "../../../common/information.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface for Data as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Data)
 *
 */
class DataInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  Rcpp::NumericVector observed_data; /**< The data */
  static uint32_t id_g; /**< static id of the DataInterfaceBase object */
  uint32_t id;          /**< local id of the DataInterfaceBase object */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, DataInterfaceBase*>
      live_objects; /**< map associating the ids of DataInterfaceBase to
        the objects */

  /** @brief constructor
   */
  DataInterfaceBase() {
    this->id = DataInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    DataInterfaceBase */
    DataInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  /** @brief destructor
   */
  virtual ~DataInterfaceBase() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() { return this->id; }

  /**@brief add_to_fims_tmb dummy method
   *
   */
  virtual bool add_to_fims_tmb() { return true; };
};
uint32_t DataInterfaceBase::id_g = 1;
std::map<uint32_t, DataInterfaceBase*> DataInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for age comp data as an S4 object. To instantiate
 * from R:
 * acomp <- new(fims$AgeComp)
 */
class AgeCompDataInterface : public DataInterfaceBase {
 public:
  int amax;                          /**< first dimension of the data */
  int ymax;                          /**< second dimension of the data */
  Rcpp::NumericVector age_comp_data; /**<the age composition data*/

  /**
   * @brief constructor
   */
  AgeCompDataInterface(int ymax = 0, int amax = 0) : DataInterfaceBase() {
    this->amax = amax;
    this->ymax = ymax;
  }

  /**
   * @brief destructor
   */
  virtual ~AgeCompDataInterface() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() { return this->id; }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_data_object::DataObject<Type>> age_comp_data =
        std::make_shared<fims_data_object::DataObject<Type>>(this->ymax,
                                                             this->amax);

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
   * @brief adds parameters to the model
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
 * @brief Rcpp interface for data as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Index)
 */
class IndexDataInterface : public DataInterfaceBase {
 public:
  int ymax;                       /**< second dimension of the data */
  Rcpp::NumericVector index_data; /**<the age composition data*/

  /**
   * @brief constructor
   */
  IndexDataInterface(int ymax = 0) : DataInterfaceBase() { this->ymax = ymax; }

  /**
   * @brief destructor
   */
  virtual ~IndexDataInterface() {}

  /** @brief get the ID of the interface base object
   **/
  virtual uint32_t get_id() { return this->id; }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_data_object::DataObject<Type>> data =
        std::make_shared<fims_data_object::DataObject<Type>>(this->ymax);

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
   *@brief function to add to TMB
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
