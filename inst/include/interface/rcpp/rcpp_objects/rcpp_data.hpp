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
class DataInterface : public FIMSRcppInterfaceBase {
 public:
  Rcpp::NumericVector observed_data; /*!< The data */
  static uint32_t id_g; /**< static id of the DataInterface object */
  uint32_t id;          /**< local id of the DataInterface object */

  /** @brief constructor
   */
  DataInterface() { this->id = DataInterface::id_g++; }

  /** @brief destructor
   */
  virtual ~DataInterface() {}
};

uint32_t DataInterface::id_g = 1;

/**
 * @brief Rcpp interface for age comp data as an S4 object. To instantiate
 * from R:
 * acomp <- new(fims$AgeComp)
 */
class AgeCompDataInterface : public DataInterface {
 public:
  int amax;                          /*!< first dimension of the data */
  int ymax;                          /*!< second dimension of the data */
  Rcpp::NumericVector age_comp_data; /*!<the age composition data*/

  /**
   * @brief constructor
   */
  AgeCompDataInterface(int amax = 0, int ymax = 0) : DataInterface() {}

  /**
   * @brief destructor
   */
  virtual ~AgeCompDataInterface() {}

  /**
   * @brief adds parameters to the model
   */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::DataObject<TMB_FIMS_REAL_TYPE>> age_comp_data =
        std::make_shared<fims::DataObject<TMB_FIMS_REAL_TYPE>>(this->amax,
                                                               this->ymax);
    std::shared_ptr<fims::DataObject<TMB_FIMS_FIRST_ORDER>> age_comp_data_1 =
        std::make_shared<fims::DataObject<TMB_FIMS_FIRST_ORDER>>(this->amax,
                                                                 this->ymax);
    std::shared_ptr<fims::DataObject<TMB_FIMS_SECOND_ORDER>> age_comp_data_2 =
        std::make_shared<fims::DataObject<TMB_FIMS_SECOND_ORDER>>(this->amax,
                                                                  this->ymax);
    std::shared_ptr<fims::DataObject<TMB_FIMS_THIRD_ORDER>> age_comp_data_3 =
        std::make_shared<fims::DataObject<TMB_FIMS_THIRD_ORDER>>(this->amax,
                                                                 this->ymax);

    age_comp_data->id = this->id;

    age_comp_data_1->id = this->id;

    age_comp_data_2->id = this->id;

    age_comp_data_3->id = this->id;

    for (int y = 0; y < ymax; y++) {
      for (int a = 0; a < amax; a++) {
        int index_ya = y * amax + a;
        age_comp_data->at(y, a) = this->observed_data[index_ya];
        age_comp_data_1->at(y, a) = this->observed_data[index_ya];
        age_comp_data_2->at(y, a) = this->observed_data[index_ya];
        age_comp_data_3->at(y, a) = this->observed_data[index_ya];
      }
    }

    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    d0->data_objects[this->id] = age_comp_data;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    d1->data_objects[this->id] = age_comp_data_1;
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    d2->data_objects[this->id] = age_comp_data_2;
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    d3->data_objects[this->id] = age_comp_data_3;

    return true;
  }
};

/**
 * @brief Rcpp interface for data as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Index)
 */
class IndexDataInterface : public DataInterface {
 public:
  int ymax;                       /*!< second dimension of the data */
  Rcpp::NumericVector index_data; /*!<the age composition data*/

  /**
   * @brief constructor
   */
  IndexDataInterface(int ymax = 0) : DataInterface() {}

  /**
   * @brief destructor
   */
  virtual ~IndexDataInterface() {}

  /**
   *@brief function to add to TMB
   */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::DataObject<TMB_FIMS_REAL_TYPE>> index_data =
        std::make_shared<fims::DataObject<TMB_FIMS_REAL_TYPE>>(this->ymax);
    std::shared_ptr<fims::DataObject<TMB_FIMS_FIRST_ORDER>> index_data_1 =
        std::make_shared<fims::DataObject<TMB_FIMS_FIRST_ORDER>>(this->ymax);
    std::shared_ptr<fims::DataObject<TMB_FIMS_SECOND_ORDER>> index_data_2 =
        std::make_shared<fims::DataObject<TMB_FIMS_SECOND_ORDER>>(this->ymax);
    std::shared_ptr<fims::DataObject<TMB_FIMS_THIRD_ORDER>> index_data_3 =
        std::make_shared<fims::DataObject<TMB_FIMS_THIRD_ORDER>>(this->ymax);

    index_data->id = this->id;

    index_data_1->id = this->id;

    index_data_2->id = this->id;

    index_data_3->id = this->id;

    index_data->id = this->id;

    for (int y = 0; y < ymax; y++) {
      index_data->at(y) = this->observed_data[y];
      index_data_1->at(y) = this->observed_data[y];
      index_data_2->at(y) = this->observed_data[y];
      index_data_3->at(y) = this->observed_data[y];
    }

    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    d0->data_objects[this->id] = index_data;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    d1->data_objects[this->id] = index_data_1;
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    d2->data_objects[this->id] = index_data_2;
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    d3->data_objects[this->id] = index_data_3;
    return true;
  }
};

#endif