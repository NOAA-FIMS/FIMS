#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_PRECISION_BUILDERS_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_PRECISION_BUILDERS_HPP

#include "rcpp_interface_base.hpp"
#include "../../../distributions/functors/precision_builders.hpp"

/**
 * @brief Rcpp interface for the Dynamic Structural Equation Model (DSEM).
 * @details Manages the RAM matrix and metadata used to assemble the joint 
 * precision matrix Q.
 */
class DSEMInterface : public FIMSRcppInterfaceBase {
public:
    /** @brief Global id counter used to assign unique interface ids. */
    static uint32_t id_g;

    /** @brief Live DSEM interface objects indexed by ID. */
    static std::map<uint32_t, std::shared_ptr<DSEMInterface>> live_objects;

    /** @brief Unique id for this interface instance. */
    uint32_t id;
    
    /** @brief The RAM specification matrix passed from R. */
    Rcpp::IntegerMatrix ram_matrix; 

    /** @brief Vector of path coefficients (beta_z). */
    VariableVector beta_z;
    
    /** @brief Total years */
    size_t n_time = 0;

    /** @brief Total variables (e.g., recruitment, temp) */
    size_t n_variables = 0;

    /**
     * @brief Constructor.
     * @details The object is created first, then registered explicitly by
     * calling register_object() to avoid copying the object into the registry 
     * during construction.
     */
     DSEMInterface() {
        this->id = DSEMInterface::id_g++;
     }

    /**
     * @brief Construct a new DSEMInterface object from another.
     * @param other Source object.
     */
    DSEMInterface(const DSEMInterface& other)
        : FIMSRcppInterfaceBase(other),
          id(other.id),
          ram_matrix(other.ram_matrix),
          beta_z(other.beta_z),
          n_time(other.n_time),
          n_variables(other.n_variables) {}

    /**
     * @brief Destructor.
     */
    virtual ~DSEMInterface() {}

    /**
     * @brief Register this object in the global live object map and interface list.
     * @return True on success.
     */
    void register_self(std::shared_ptr<DSEMInterface> self) {
        DSEMInterface::live_objects[this->id] = self;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(self);
    }

    
    /**
     * @brief Get the unique identifier for this interface instance.
     * @return Interface id.
     */
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
        
        // Set up beta_z in the builder and register parameters
        builder->beta_z.resize(this->beta_z.size());
        for (size_t i = 0; i < this->beta_z.size(); ++i) {
            builder->beta_z[i] = this->beta_z[i].initial_value_m;
            if (this->beta_z[i].estimation_type_m.get() == "fixed_effects") {
                std::stringstream ss;
                ss << "dsem." << this->id << ".beta_z." << this->beta_z[i].id_m;
                info->RegisterParameterName(ss.str());
                info->RegisterParameter(builder->beta_z[i]);
            }
            if (this->beta_z[i].estimation_type_m.get() == "random_effects") {
                std::stringstream ss;
                ss << "dsem." << this->id << ".beta_z." << this->beta_z[i].id_m;
                info->RegisterRandomEffectName(ss.str());
                info->RegisterRandomEffect(builder->beta_z[i]);
            }
        }

        // Copy the RAM matrix into structured RAMPath objects
        for (int r = 0; r < ram_matrix.nrow(); ++r) {
            typename fims_distributions::DSEMPrecisionMatrixBuilder<Type>::RAMPath path;
            path.type = ram_matrix(r, 0);       // Rho vs Gamma
            path.from = ram_matrix(r, 1);       // Origin
            path.to = ram_matrix(r, 2);         // Target
            path.beta_index = ram_matrix(r, 3); // Index into beta_z
            builder->paths.push_back(path);
        }

        // Register builder in Information Map (to be accessed by GMRF distribution)
        info->precision_builders[this->id] = builder; 
        return true;
    }

    /**
     * @brief Adds the object to the TMB model.
     * @return True on success.
     */
    virtual bool add_to_fims_tmb() {
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
        return true;
    }

    /**
     * @brief Copy optimized values back to the R-facing object.
     */
    virtual void finalize() {
        if (this->finalized) return;
        std::shared_ptr<fims_info::Information<double>> info =
            fims_info::Information<double>::GetInstance();
        auto it = info->precision_builders.find(this->id);
        if (it != info->precision_builders.end()) {
            auto builder = std::dynamic_pointer_cast<fims_distributions::DSEMPrecisionMatrixBuilder<double>>(it->second);
            for (size_t i = 0; i < this->beta_z.size(); ++i) {
                this->beta_z[i].final_value_m = builder->beta_z[i];
            }
        }
        this->finalized = true;
    }
#endif
};

uint32_t DSEMInterface::id_g = 1;
std::map<uint32_t, std::shared_ptr<DSEMInterface>> DSEMInterface::live_objects;

#endif 