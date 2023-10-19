  /*! \file test_dmultinom_distribution.hpp
   * Specifies the negative log-likelihood of
   * the dmultinom distribution given data and parameters.
   * Creates singleton class that links this .hpp with
   * the test_dmultinom_distribution.cpp TMB verison of the model
   */
  #ifndef TEST_FLEET_ACOMP_NLL_HPP
  #define TEST_FLEET_ACOMP_NLL_HPP

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

      using Vector = typename ModelTraits<Type>::EigenVector;
      public:
      Vector x; /*!< Vector of length K of integers */
      Vector p; /*!< Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1). */

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

        Type nll = 0.0;
        int n = x.size();
      
        fims::FleetAgeCompNLL<Type> nll_fac;
        nll_fac.catch_numbers_at_age.resize(n);
        nll_fac.nyears = 1;
        nll_fac.nages = 10;
        std::shared_ptr<fims::DataObject<Type>> age_comp_data =
        std::make_shared<fims::DataObject<Type>>(10, 1);

        nll_fac.observed_agecomp_data = age_comp_data;
        Vector obs;
        Vector p_set;
        obs.resize(n);
        p_set.resize(n);
        for(int i =0; i < n; i++){
          nll_fac.observed_agecomp_data->at(0, i);
          nll_fac.catch_numbers_at_age[i] = p[i];

          obs[i] = nll_fac.observed_agecomp_data->at(0,i);
          p_set[i] = nll_fac.catch_numbers_at_age[i];
        }
        
        
        REPORT_F(obs, of);
        REPORT_F(p_set, of);
        REPORT_F(n, of);
        nll = nll_fac.evaluate();
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


  #endif  /* TEST_DMULTINOM_DISTRIBUTION_HPP */


