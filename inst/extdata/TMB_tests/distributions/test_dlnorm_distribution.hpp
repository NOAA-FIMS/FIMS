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
     * @tparam Type 
     */
    template <typename Type>
    class Model {
      using DataVector = typename ModelTraits<Type>::DataVector;
      public:
      DataVector logy; /*!< observation */
      Type meanlog; /*!< mean of the distribution of log(x) */
      Type sdlog; /*!< standard deviation of the distribution of log(x) */

      // Initiate pointer to link .cpp to .hpp
      static Model<Type>* instance;

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
        int i;
        int n = logy.size();
        fims::Dlnorm<Type> nll_dlnorm;
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
     * @tparam Type 
     */
    template<class Type>
    Model<Type>* Model<Type>::instance = new Model<Type>();
  }


  #endif  /* TEST_DLNORM_DISTRIBUTION_HPP */


