  /*! \file test_dmultinom_distribution.hpp 
   * Specifies the negative log-likelihood of 
   * the dmultinom distribution given data and parameters. 
   * Creates singleton class that links this .hpp with 
   * the test_dmultinom_distribution.cpp TMB verison of the model
   */
  #ifndef TEST_DMULTINOM_DISTRIBUTION_HPP
  #define TEST_DMULTINOM_DISTRIBUTION_HPP

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
      using Vector = typename ModelTraits<Type>::EigenVector;
      public:
      Vector x; /*!< Vector of length K of integers */
      Vector p; /*!< Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1). */

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
        fims::Dmultinom<Type> nll_dmultinom;
        nll_dmultinom.x = x;
        nll_dmultinom.p = p;
        nll -= nll_dmultinom.evaluate(true);
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


