  /*! \file test_dlnorm_distribution.hpp
   * Specifies the negative log-likelihood of
   * the dlnorm distribution given data and parameters.
   * Creates singleton class that links this .hpp with
   * the test_dlnorm_distribution.cpp TMB verison of the model
   */
  #ifndef TEST_FLEET_INDEX_HPP
  #define TEST_FLEET_INDEX_HPP

  #include "../../../include/population_dynamics/fleet/fleet_nll.hpp"
  #include "../../../include/interface/interface.hpp"

  namespace fims {

    /**
     * @brief Model class defines and returns negative log-likelihood (nll)
     *
     * @tparam Type
     */
    template <typename Type>
    class Model {
      using DataVector = typename ModelTraits<Type>::DataVector;
      using Vector = typename ModelTraits<Type>::EigenVector;
      public:
      DataVector y; /*!< observation */
      Vector mean;  /*!< expected fleet index */
      Type logsd; /*!< standard deviation of the fleet observation error, must be strictly positive.*/

      // Initiate pointer to link .cpp to .hpp
      static Model<Type>* instance;
      
      ::objective_function<Type> *of;

      /** @brief Constructor.
       */
      Model(){}

      /**
       * @brief Create new singleton class
       *
       * @return Model<Type>*
       */
      static Model<Type>* getInstance(){
        return Model<Type>::instance;
      }

     /**
       * @brief Function that calculates the negative log-likelihood given the data and parameters
       *
       * @return negative log-likelihood (nll)
       */
      Type evaluate(){

        Type nll = 0;
        int n = y.size();

        fims::FleetIndexNLL<Type> nll_fleet_index;
        nll_fleet_index.log_obs_error = logsd;
        
        std::shared_ptr<fims::DataObject<Type>> index_data =
        std::make_shared<fims::DataObject<Type>>(n);
        nll_fleet_index.observed_index_data = index_data;

        nll_fleet_index.expected_index.resize(n);
        nll_fleet_index.expected_index = mean;
        Vector temp;
        temp.resize(n);
        for(int i =0; i < n; i++){
          nll_fleet_index.observed_index_data->at(i);
          temp[i] = nll_fleet_index.observed_index_data->at(i);
          nll = nll_fleet_index.evaluate();
        }
        REPORT_F(mean, of);
        REPORT_F(temp, of);

        return nll;
      }
    };

    /**
     * @brief Create new instance of Model
     *
     * @tparam Type
     */
    template<class Type>
    Model<Type>* Model<Type>::instance = new Model<Type>();
  }


  #endif  /* TEST_FLEET_NLL_HPP */


