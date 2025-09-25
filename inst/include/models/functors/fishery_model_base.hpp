/**
 * @file fishery_model_base.hpp
 * @brief Defines the base class for all fishery models within the FIMS
 * framework.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_MODELS_FISHERY_MODEL_BASE_HPP
#define FIMS_MODELS_FISHERY_MODEL_BASE_HPP

#include "../../common/model_object.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "../../population_dynamics/population/population.hpp"

/**
 * @brief The population dynamics of FIMS.
 *
 */
namespace fims_popdy
{

  /**
   * @brief Structure to hold dimension information for derived quantities.
   */
  struct DimensionInfo
  {
    std::string name;                    /*!< name of the derived quantity */
    int ndims;                           /*!< number of dimensions */
    fims::Vector<int> dims;              /*!< vector of dimensions */
    fims::Vector<std::string> dim_names; /*!< vector of dimension names */
    fims::Vector<double> se_values_m;    /*!< final values of the report vector */

    /**
     * @brief Default constructor for dimension information.
     */
    DimensionInfo() : ndims(0) {}

    /**
     * @brief Constructor with parameters.
     * @param name The name of the derived quantity.
     * @param dims A vector of integers representing the dimensions.
     * @param dim_names A vector of strings representing the names of the dimensions.
     */
    DimensionInfo(const std::string &name, const fims::Vector<int> &dims, const fims::Vector<std::string> &dim_names)
        : name(name), ndims(dims.size()), dims(dims), dim_names(dim_names) {}
  };

  /**
   * TMB SDreports are returned as a single vector, if there are multiple
   * populations or fleets then the report vectors are concatenated together.
   * This struct is used to store the name, id, and length of each report
   * vector so that they can be extracted from the single report vector.
   */
  struct UncertaintyReportInfo
  {
    /**
     * * name of the report vector
     */
    std::string name;
    /**
     * * id of the population or fleet the report is associated with
     */
    uint32_t id_m;
    /**
     * * starting index of the report vector in the overall report vector
     */
    size_t start_m;
    /**
     * * length of the report vector
     */
    size_t length_m;
    
    

    /**
     * @brief Default constructor for UncertaintyReportInfo.
     */
    UncertaintyReportInfo() : id_m(0), start_m(0), length_m(0) {}

    /**
     * @brief Constructor with parameters.
     * @param name The name of the report vector.
     * @param id The id of the population or fleet the report is associated with.
     * @param start The starting index of the report vector in the overall report vector.
     * @param length The length of the report vector.
     */
    UncertaintyReportInfo(const std::string &name, uint32_t id, size_t start, size_t length)
        : name(name), id_m(id), start_m(start), length_m(length) {}

    /**
     * @brief Copy constructor.
     * @param other The UncertaintyReportInfo object to copy from.
     */
    UncertaintyReportInfo(const UncertaintyReportInfo &other)
        : name(other.name), id_m(other.id_m), start_m(other.start_m), length_m(other.length_m) {}
  };

  template <typename Type>
  /**
   * @brief FisheryModelBase is a base class for fishery models in FIMS.
   *
   */
  class FisheryModelBase : public fims_model_object::FIMSObject<Type>
  {
    static uint32_t id_g; /*!< global id where unique id is drawn from for fishery model object*/
    uint32_t id;          /*!< unique identifier assigned for fishery model object */

  public:
#ifdef TMB_MODEL
    bool do_reporting = true; /*!< flag to control reporting of derived quantities */
#endif
    /**
     * @brief A string specifying the model type.
     *
     */
    std::string model_type_m;
    /**
     * @brief Unique identifier for the fishery model.
     *
     */
    std::set<uint32_t> population_ids;
    /**
     * @brief A vector of populations in the fishery model.
     *
     */
    std::vector<std::shared_ptr<fims_popdy::Population<Type>>> populations;
    /**
     * @brief A map of fleets in the fishery model, indexed by fleet id.
     * Unique instances to eliminate duplicate initialization.
     *
     */
    std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type>>> fleets;
    /**
     * @brief Fleet-based iterator.
     *
     */
    typedef typename std::map<uint32_t,
                              std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
        fleet_iterator;

    /**
     * @brief Type definitions for derived quantities and dimension information maps.
     */
    typedef typename std::map<uint32_t, std::map<std::string, fims::Vector<Type>>> DerivedQuantitiesMap;

    /**
     * @brief Iterator for the derived quantities map.
     */
    typedef typename DerivedQuantitiesMap::iterator DerivedQuantitiesMapIterator;

    /**
     * @brief Shared pointer for the fleet derived quantities map.
     */
    std::shared_ptr<DerivedQuantitiesMap> fleet_derived_quantities;

    /**
     * @brief Shared pointer for the population derived quantities map.
     */
    std::shared_ptr<DerivedQuantitiesMap> population_derived_quantities;

    /**
     * @brief Type definitions for dimension information maps.
     */
    typedef typename std::map<uint32_t, std::map<std::string, DimensionInfo>> DimensionInfoMap;

    /**
     * @brief Shared pointer for the fleet dimension information map.
     */
    std::shared_ptr<DimensionInfoMap> fleet_dimension_info;

    /**
     * @brief Shared pointer for the population dimension information map.
     */
    std::shared_ptr<DimensionInfoMap> population_dimension_info;

    /**
     * @brief Type definition for the uncertainty report information map.
     */
    typedef typename std::map<uint32_t, std::map<std::string, UncertaintyReportInfo>> UncertaintyReportInfoMap;

    /**
     * @brief Shared pointer for the uncertainty report information map.
     */
    std::shared_ptr<UncertaintyReportInfoMap> fleet_uncertainty_report_info;

    /**
     * @brief Shared pointer for the population uncertainty report information map.
     */
    std::shared_ptr<UncertaintyReportInfoMap> population_uncertainty_report_info;

#ifdef TMB_MODEL
    ::objective_function<Type> *of;
#endif
    /**
     * @brief Construct a new Fishery Model Base object.
     *
     */
    FisheryModelBase() : id(FisheryModelBase::id_g++)
    {
      fleet_derived_quantities = std::make_shared<DerivedQuantitiesMap>();
      population_derived_quantities = std::make_shared<DerivedQuantitiesMap>();
      fleet_dimension_info = std::make_shared<DimensionInfoMap>();
      population_dimension_info = std::make_shared<DimensionInfoMap>();
    }

    /**
     * @brief Construct a new Fishery Model Base object.
     *
     * @param other
     */
    FisheryModelBase(const FisheryModelBase &other) : id(other.id),
                                                      population_ids(other.population_ids),
                                                      populations(other.populations),
                                                      fleet_derived_quantities(other.fleet_derived_quantities),
                                                      population_derived_quantities(other.population_derived_quantities),
                                                      fleet_dimension_info(other.fleet_dimension_info),
                                                      population_dimension_info(other.population_dimension_info)
    {
    }

    /**
     * @brief Destroy the Fishery Model Base object.
     *
     */
    virtual ~FisheryModelBase() {}

    /**
     * @brief Show the Catch At Age model derived quantities.
     *
     * @param p
     */
    void ShowPopulation(std::shared_ptr<fims_popdy::Population<double>> &p)
    {
      std::cout << "\n\nCAA Model Derived Quantities:\n";
      typename fims_popdy::Population<double>::derived_quantities_iterator it;
      for (it = p->derived_quantities.begin(); it != p->derived_quantities.end();
           it++)
      {
        fims::Vector<double> &dq = (*it).second;
        std::cout << (*it).first << ":" << std::endl;
        for (int i = 0; i < dq.size(); i++)
        {
          std::cout << dq[i] << " ";
        }
        std::cout << std::endl
                  << std::endl;
      }
    }

    /**
     * @brief Show the populations in the fishery model.
     *
     */
    void Show()
    {
      for (size_t p = 0; p < this->populations.size(); p++)
      {
        this->ShowPopulation(this->populations[p]);
      }
    }

    /**
     * @brief Get the fleet dimension information.
     *
     * @return std::map<uint32_t, std::map<std::string, DimensionInfo>>
     */
    std::map<uint32_t, std::map<std::string, DimensionInfo>> &GetFleetDimensionInfo()
    {
      return *fleet_dimension_info;
    }

    /**
     * @brief Get the population dimension information.
     *
     * @return std::map<uint32_t, std::map<std::string, DimensionInfo>>
     */
    std::map<uint32_t, std::map<std::string, DimensionInfo>> &GetPopulationDimensionInfo()
    {
      return *population_dimension_info;
    }

    /**
     * @brief Get the fleet derived quantities.
     *
     * @return DerivedQuantitiesMap
     */
    DerivedQuantitiesMap &GetFleetDerivedQuantities()
    {
      return *fleet_derived_quantities;
    }

    /**
     * @brief Get the population derived quantities.
     *
     * @return DerivedQuantitiesMap
     */
    DerivedQuantitiesMap &GetPopulationDerivedQuantities()
    {
      return *population_derived_quantities;
    }

    /**
     * @brief Get the fleet derived quantities for a specified fleet.
     *
     * @param fleet_id The ID of the fleet.
     * @return std::map<std::string, fims::Vector<Type>>&
     */
    std::map<std::string, fims::Vector<Type>> &GetFleetDerivedQuantities(uint32_t fleet_id)
    {
      return (*fleet_derived_quantities)[fleet_id];
    }

    /**
     * @brief Get the population derived quantities for a specified population.
     *
     * @param population_id The ID of the population.
     * @return std::map<std::string, fims::Vector<Type>>&
     */
    std::map<std::string, fims::Vector<Type>> &GetPopulationDerivedQuantities(uint32_t population_id)
    {
      return (*population_derived_quantities)[population_id];
    }

    /**
     * @brief Get the fleet dimension information for a specified fleet.
     *
     * @param fleet_id The ID of the fleet.
     * @return std::map<std::string, DimensionInfo>
     */
    std::map<std::string, DimensionInfo> &GetFleetDimensionInfo(uint32_t fleet_id)
    {
      return (*fleet_dimension_info)[fleet_id];
    }

    /**
     * @brief Get the population dimension information for a specified population.
     *
     * @param population_id The ID of the population.
     * @return std::map<std::string, DimensionInfo>
     */
    std::map<std::string, DimensionInfo> &GetPopulationDimensionInfo(uint32_t population_id)
    {
      return (*population_dimension_info)[population_id];
    }

    /**
     * @brief Get the fleet uncertainty report information.
     */
    UncertaintyReportInfoMap &GetFleetUncertaintyReportInfo()
    {
      return *fleet_uncertainty_report_info;
    }

    /**
     * @brief Get the population uncertainty report information.
     */
    UncertaintyReportInfoMap &GetPopulationUncertaintyReportInfo()
    {
      return *population_uncertainty_report_info;
    }

    /**
     * @brief Get the fleet uncertainty report information for a specified fleet.
     *
     * @param fleet_id The ID of the fleet.
     * @return std::map<std::string, UncertaintyReportInfo>&
     */
    std::map<std::string, UncertaintyReportInfo> &GetFleetUncertaintyReportInfo(uint32_t fleet_id)
    {
      return (*fleet_uncertainty_report_info)[fleet_id];
    }

    /**
     * @brief Get the population uncertainty report information for a specified population.
     *
     * @param population_id The ID of the population.
     * @return std::map<std::string, UncertaintyReportInfo>&
     */
    std::map<std::string, UncertaintyReportInfo> &GetPopulationUncertaintyReportInfo(uint32_t population_id)
    {
      return (*population_uncertainty_report_info)[population_id];
    }


    /**
     * @brief Initialize a model.
     *
     */
    virtual void Initialize() {}

    /**
     * @brief Prepare the model.
     *
     */
    virtual void Prepare() {}

    /**
     * @brief Reset a vector from start to end with a value.
     *
     * @param v A vector to reset.
     * @param value The value you want to use for all elements in the
     * vector. The default is 0.0.
     */
    virtual void ResetVector(fims::Vector<Type> &v, Type value = 0.0)
    {
      std::fill(v.begin(), v.end(), value);
    }

    /**
     * @brief Evaluate the model.
     *
     */
    virtual void Evaluate() {}

    /**
     * @brief Report the model results via TMB.
     *
     */
    virtual void Report() {}

    /**
     * @brief Get the Id object.
     *
     * @return uint32_t
     */
    uint32_t GetId() { return this->id; }
  };

  template <typename Type>
  uint32_t FisheryModelBase<Type>::id_g = 0;

} // namespace fims_popdy
#endif
