#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DSEM_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DSEM_HPP

#include "rcpp_interface_base.hpp"
#include "../../../distributions/functors/precision_builders.hpp"

/**
 * @brief Rcpp interface for the Dynamic Structural Equation Model (DSEM).
 * @details Manages the RAM matrix and metadata used to assemble the joint 
 * precision matrix Q [Plan].
 */
class DSEMInterface : public FIMSRcppInterfaceBase {
public:
    static uint32_t id_g;
    uint32_t id;
    
    /** @brief The RAM specification matrix passed from R [Plan]. */
    Rcpp::IntegerMatrix ram_matrix; 
    /** @brief Fixed starting values for paths when not estimating. */
    Rcpp::NumericVector ram_start_values;
    
    size_t n_time = 0;      /**< Total years */
    size_t n_variables = 0; /**< Total variables (e.g., recruitment, temp) */

    DSEMInterface() {
        this->id = DSEMInterface::id_g++;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(
            std::make_shared<DSEMInterface>(*this));
    }

    virtual ~DSEMInterface() {}
    virtual uint32_t get_id() { return this->id; }

#ifdef TMB_MODEL
    /**
     * @brief Translates R-side RAM settings into C++ computational metadata.
     */
    template <typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims_info::Information<Type>> info = 
            fims_info::Information<Type>::GetInstance();
        
        // Instantiate the builder
        std::shared_ptr<fims_distributions::DSEMPrecisionMatrixBuilder<Type>> builder = 
            std::make_shared<fims_distributions::DSEMPrecisionMatrixBuilder<Type>>();
        
        builder->n_time = this->n_time;
        builder->n_variables = this->n_variables;
        
        // Copy the RAM matrix into structured RAMPath objects [Plan]
        for (int r = 0; r < ram_matrix.nrow(); ++r) {
            typename fims_distributions::DSEMPrecisionMatrixBuilder<Type>::RAMPath path;
            path.type = ram_matrix(r, 0);       // Rho vs Gamma
            path.from = ram_matrix(r, 1);       // Origin
            path.to = ram_matrix(r, 2);         // Target
            path.beta_index = ram_matrix(r, 3); // Index into beta_z
            path.start = static_cast<Type>(ram_start_values[r]);
            builder->paths.push_back(path);
        }

        // Register builder in Information Map (to be accessed by GMRF distribution)
        info->dsem_builders[this->id] = builder; 
        
        return true;
    }

    virtual bool add_to_fims_tmb() {
        this->add_to_fims_tmb_internal<double>();
        this->add_to_fims_tmb_internal<TMBad::ad_aug>();
        return true;
    }
#endif
};

uint32_t DSEMInterface::id_g = 1;
#endif 