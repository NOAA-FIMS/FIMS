  /*! \file test_dlnorm_distribution.hpp 
   * Specifies the negative log-likelihood of 
   * the dlnorm distribution given data and parameters. 
   * Creates singleton class that links this .hpp with 
   * the test_dlnorm_distribution.cpp TMB verison of the model
   */
  #ifndef TEST_DLNORM_DISTRIBUTION_HPP
  #define TEST_DLNORM_DISTRIBUTION_HPP

  #include "../../../include/distributions/distributions.hpp"
  #include "../../../include/interface/interface.hpp"

  namespace fims {
    
    /**
     * @brief Model class defines and returns negative log-likelihood (nll)
     * 
     * @tparam T 
     */
    template <typename T>
    class Model {
      using DataVector = typename ModelTraits<T>::DataVector;
      public:
      DataVector logy; /*!< observation */
      T meanlog; /*!< mean of the distribution of log(x) */
      T sdlog; /*!< standard deviation of the distribution of log(x) */

      // Initiate pointer to link .cpp to .hpp
      static Model<T>* instance;

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

        T nll = 0;
        int i;
        int n = logy.size();
        fims::Dlnorm<T> nll_dlnorm;
        nll_dlnorm.meanlog = meanlog;
        nll_dlnorm.sdlog = sdlog;
        for(i =0; i < n; i++){
          nll_dlnorm.x = logy[i];
          nll -= nll_dlnorm.evaluate(true);
        }
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


  #endif  /* TEST_DLNORM_DISTRIBUTION_HPP */


