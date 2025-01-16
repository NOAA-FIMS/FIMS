/**
 * @file rcpp_growth.hpp
 * @brief The Rcpp interface to declare different types of growth, e.g.,
 * empirical weight-at-age data. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP

#include "../../../population_dynamics/growth/growth.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp growth
 * interfaces. This type should be inherited and not called from R directly.
 */
class GrowthInterfaceBase : public FIMSRcppInterfaceBase {
public:
    /**
     * @brief The static id of the GrowthInterfaceBase object.
     */
    static uint32_t id_g;
    /**
     * @brief The local id of the GrowthInterfaceBase object.
     */
    uint32_t id;
    /**
     * @brief The map associating the IDs of GrowthInterfaceBase to the objects.
     * This is a live object, which is an object that has been created and lives
     * in memory.
     */
    static std::map<uint32_t, GrowthInterfaceBase*> live_objects;

    /**
     * @brief The constructor.
     */
    GrowthInterfaceBase() {
        this->id = GrowthInterfaceBase::id_g++;
        /* Create instance of map: key is id and value is pointer to
        GrowthInterfaceBase */
        GrowthInterfaceBase::live_objects[this->id] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    /**
     * @brief The destructor.
     */
    virtual ~GrowthInterfaceBase() {
    }

    /**
     * @brief Get the ID for the child growth interface objects to inherit.
     */
    virtual uint32_t get_id() = 0;

    /**
     * @brief A method for each child growth interface object to inherit so
     * each growth option can have an evaluate() function.
     */
    virtual double evaluate(double age) = 0;
};
// static id of the GrowthInterfaceBase object
uint32_t GrowthInterfaceBase::id_g = 1;
// local id of the GrowthInterfaceBase object map relating the ID of the
// GrowthInterfaceBase to the GrowthInterfaceBase objects
std::map<uint32_t, GrowthInterfaceBase*> GrowthInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for EWAAgrowth to instantiate the object from R:
 * ewaa <- methods::new(EWAAgrowth). Where, EWAA stands for empirical weight at
 * age and growth is not actually estimated.
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
public:
    /**
     * @brief Weights (mt) for each age class.
     */
    std::vector<double> weights;
    /**
     * @brief Ages (years) for each age class.
     */
    std::vector<double> ages;
    /**
     * @brief A map of empirical weight-at-age values. TODO: describe this
     * parameter better.
     */
    std::map<double, double> ewaa;
    /**
     * @brief Have weight and age vectors been set? The default is false.
     */
    bool initialized = false;

    /**
     * @brief The constructor.
     */
    EWAAGrowthInterface() : GrowthInterfaceBase() {
    }

    /**
     * @brief The destructor.
     */
    virtual ~EWAAGrowthInterface() {
    }

    /**
     * @brief Gets the ID of the interface base object.
     * @return The ID.
     */
    virtual uint32_t get_id() {
        return this->id;
    }

    /**
     * @brief Create a map of input numeric vectors.
     * @param weights Type vector of weights.
     * @param ages Type vector of ages.
     * @return std::map<T, T>.
     */
    inline std::map<double, double> make_map(std::vector<double> ages,
            std::vector<double> weights) {
        std::map<double, double> mymap;
        for (uint32_t i = 0; i < ages.size(); i++) {
            mymap.insert(std::pair<double, double>(ages[i], weights[i]));
        }
        return mymap;
    }

    /**
     * @brief Evaluate the growth using empirical weight at age.
     * @param age Age. TODO: Document this better.
     * @details This can be called from R using ewaagrowth.evaluate(age).
     */
    virtual double evaluate(double age) {
        fims_popdy::EWAAgrowth<double> EWAAGrowth;

        if (initialized == false) {
            this->ewaa = make_map(this->ages, this->weights);
            // Check that ages and weights vector are the same length
            if (this->ages.size() != this->weights.size()) {
                Rcpp::stop("ages and weights must be the same length");
            }
            initialized = true;
        } else {
            Rcpp::stop("this empirical weight at age object is already initialized");
        }
        EWAAGrowth.ewaa = this->ewaa;
        return EWAAGrowth.evaluate(age);
    }

    /**
     * @brief Converts the data to json representation for the output.
     * @return A string is returned specifying that the module relates to the
     * growth interface with empirical weight at age. It also returns the ID, the
     * rank of 1, the dimensions, age bins, and the calculated values themselves.
     * This string is formatted for a json file.
     */
    virtual std::string to_json() {
        std::stringstream ss;

        ss << "\"module\" : {\n";
        ss << " \"name\": \"growth\",\n";
        ss << " \"type\" : \"EWAA\",\n";
        ss << " \"id\":" << this->id << ",\n";
        ss << " \"rank\": " << 1 << ",\n";
        ss << " \"dimensions\": [" << this->weights.size() << "],\n";

        ss << " \"ages\": [";
        for (size_t i = 0; i < ages.size() - 1; i++) {
            ss << ages[i] << ", ";
        }
        ss << ages[ages.size() - 1] << "],\n";

        ss << " \"values\": [";
        for (size_t i = 0; i < weights.size() - 1; i++) {
            ss << weights[i] << ", ";
        }
        ss << weights[weights.size() - 1] << "]\n";
        ss << "}";
        return ss.str();
    }

#ifdef TMB_MODEL

    template <typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims_info::Information<Type> > info =
                fims_info::Information<Type>::GetInstance();

        std::shared_ptr<fims_popdy::EWAAgrowth<Type> > ewaa_growth =
                std::make_shared<fims_popdy::EWAAgrowth<Type> >();

        // set relative info
        ewaa_growth->id = this->id;
        ewaa_growth->ewaa = make_map(this->ages, this->weights); // this->ewaa;
        // add to Information
        info->growth_models[ewaa_growth->id] = ewaa_growth;

        return true;
    }

    /**
     * @brief Adds the parameters to the TMB model.
     * @return A boolean of true.
     */
    virtual bool add_to_fims_tmb() {
#ifdef TMBAD_FRAMEWORK
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
#endif
        return true;
    }

#endif
};

#endif
