/*
 * File:   data_object.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov, andrea.havron@noaa.gov
 *
 * Created on March 24, 2022, 2:37 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained by
 * using this  software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express or
 * implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 */

#ifndef FIMS_COMMON_DATA_OBJECT_HPP
#define FIMS_COMMON_DATA_OBJECT_HPP

#include <vector>
#include <exception>
#include "model_object.hpp"


namespace fims {

    /**
     * Container to hold user supplied data.
     */
    template<typename Type>
    struct DataObject : public fims::ModelObject<Type> {
        static uint32_t id_g;
        std::vector<Type> data;
        size_t dimensions;
        size_t imax;
        size_t jmax;
        size_t kmax;
        size_t lmax;

        DataObject(size_t imax) :
        imax(imax), dimensions(1) {
            data.resize(imax);
            this->id = DataObject<Type>::id_g++;
        }

        DataObject(size_t imax, size_t jmax) :
        imax(imax), jmax(jmax), dimensions(2) {
            data.resize(imax * jmax);
            this->id = DataObject<Type>::id_g++;
        }

        DataObject(size_t imax, size_t jmax, size_t kmax) :
        imax(imax), jmax(jmax), kmax(kmax), dimensions(3) {
            data.resize(imax * jmax * kmax);
            this->id = DataObject<Type>::id_g++;
        }

        DataObject(size_t imax, size_t jmax, size_t kmax, size_t lmax) :
        imax(imax), jmax(jmax), kmax(kmax), lmax(lmax), dimensions(4) {
            data.resize(imax * jmax * kmax * lmax);
            this->id = DataObject<Type>::id_g++;
        }

        /**
         * Retrieve element from 1d data set. 
         * @param i
         * @return 
         */
        inline Type operator()(size_t i) {
            return data[i];
        }

        /**
         * Retrieve element from 1d data set. 
         * Throws an exception if index is out of bounds.
         * @param i
         * @return reference type
         */
        inline Type& at(size_t i) {
            if (i >= this->data.size()) {
                throw std::overflow_error("DataObject error:i index out of bounds");
            }
            return data[i];
        }

        /**
         * Retrieve element from 2d data set. 
         * @param i
         * @param j
         * @return 
         */
        inline const Type operator()(size_t i, size_t j) {
            return data[i * jmax + j];
        }

        /**
         * Retrieve element from 2d data set. 
         * Throws an exception if index is out of bounds.
         * @param i
         * @param j
         * @return reference type
         */
        inline Type& at(size_t i, size_t j) {
            if ((i * jmax + j) >= this->data.size()) {
                throw std::overflow_error("DataObject error: index out of bounds");
            }
            return data[i * jmax + j];
        }

        /**
         * Retrieve element from 3d data set. 
         * @param i
         * @param j
         * @param k
         * @return 
         */
        inline const Type operator()(size_t i, size_t j, size_t k) {
            return data[i * jmax * kmax + j * kmax + k];
        }

        /**
         * Retrieve element from 3d data set. 
         * Throws an exception if index is out of bounds.
         * @param i
         * @param j
         * @param k
         * @return reference type
         */
        inline Type& at(size_t i, size_t j, size_t k) {
            if ((i * jmax * kmax + j * kmax + k) >= this->data.size()) {
                throw std::overflow_error("DataObject error: index out of bounds");
            }
            return data[i * jmax * kmax + j * kmax + k];
        }

        /**
         * Retrieve element from 3d data set. 
         * @param i
         * @param j
         * @param k
         * @param l
         * @return 
         */
        inline const Type operator()(size_t i, size_t j, size_t k, size_t l) {
            return data[i * jmax * kmax * lmax + j * kmax * lmax + k * lmax + l];
        }

        /**
         * Retrieve element from 3d data set. 
         * Throws an exception if index is out of bounds.
         * @param i
         * @param j
         * @param k
         * @param l
         * @return reference type
         */
        inline Type& at(size_t i, size_t j, size_t k, size_t l) {
            if ((i * jmax * kmax * lmax + j * kmax * lmax + k * lmax + l) >= this->data.size()) {
                throw std::overflow_error("DataObject error: index out of bounds");
            }
            return data[i * jmax * kmax * lmax + j * kmax * lmax + k * lmax + l];
        }

        size_t get_dimensions() const {
            return dimensions;
        }

        size_t get_imax() const {
            return imax;
        }

        size_t get_jmax() const {
            return jmax;
        }

        size_t get_kmax() const {
            return kmax;
        }

        size_t get_lmax() const {
            return lmax;
        }


    };

    template<typename Type>
    uint32_t DataObject<Type>::id_g = 0;

}

#endif
