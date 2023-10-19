  /*! \file test_dnorm_distribution.hpp 
   * Specifies the negative log-likelihood of 
   * the dnorm distribution given data and parameters. 
   * Creates singleton class that links this .hpp with 
   * the test_dnorm_distribution.cpp TMB verison of the model
   */
  #ifndef TEST_DNORM_DISTRIBUTION_HPP
  #define TEST_DNORM_DISTRIBUTION_HPP

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
      DataVector y; /*!< observation */
      Type mean;  /*!< mean of the normal distribution */
      Type sd; /*!< standard deviation of the normal distribution, must be strictly positive.*/

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
        int n = y.size();
        fims::Dnorm<Type> nll_dnorm;
        nll_dnorm.mean = mean;
        nll_dnorm.sd = sd;
        for(i =0; i < n; i++){
          nll_dnorm.x = y[i];
          nll -= nll_dnorm.evaluate(true);
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


  #endif  /* TEST_DNORM_DISTRIBUTION_HPP */


