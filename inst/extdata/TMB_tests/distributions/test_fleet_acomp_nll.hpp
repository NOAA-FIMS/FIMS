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
     * @tparam T
     */
    template <typename T>
    class Model {

      using Vector = typename ModelTraits<T>::EigenVector;
      public:
      Vector x; /*!< Vector of length K of integers */
      Vector p; /*!< Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1). */

      // Initiate pointer to link .cpp to .hpp
      static Model<T>* instance;

      
      ::objective_function<T> *of;

      /** @brief Constructor.
       */
      Model(){}

      /**
       * @brief Create new singleton class
       *
       * @return Model<T>*
       */
      static Model<T>* getInstance(){
        return Model<T>::instance;
      }

      /**
       * @brief Function that calculates the negative log-likelihood given the data and parameters
       *
       * @return negative log-likelihood (nll)
       */
      T evaluate(){

        T nll = 0.0;
        int n = x.size();
      
        fims::FleetAgeCompNLL<T> nll_fac;
        nll_fac.catch_numbers_at_age.resize(n);
        nll_fac.nyears = 1;
        nll_fac.nages = 10;
        std::shared_ptr<fims::DataObject<T>> age_comp_data =
        std::make_shared<fims::DataObject<T>>(10, 1);

        nll_fac.observed_agecomp_data = age_comp_data;
        Vector obs;
        Vector p_set;
        obs.resize(n);
        p_set.resize(n);
        for(int i =0; i < n; i++){
          nll_fac.observed_agecomp_data->set(0, i, x[i]);
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
     * @tparam T
     */
    template<class T>
    Model<T>* Model<T>::instance = new Model<T>();
  }


  #endif  /* TEST_DMULTINOM_DISTRIBUTION_HPP */


