  #ifndef TEST_DMULTINOM_LIKELIHOOD_HPP
  #define TEST_DMULTINOM_LIKELIHOOD_HPP

  #include "../../../include/likelihoods/likelihoods.hpp"
  #include "../../../include/interface/interface.hpp"

  namespace fims {
    template <typename T>
    class Model {
      using DataVector = typename ModelTraits<T>::DataVector;
      public:
      DataVector x;
      T p;

      static Model<T>* instance;

      Model(){}

      static Model<T>* getInstance(){
        return Model<T>::instance;
      }

      T evaluate(){

        T nll = 0;
        fims::Dmultinom<T> nll_dmultinom;
        nll -= nll_dmultinom.evaluate(x,p,true);
        return nll;
      }
    };

    template<class T>
    Model<T>* Model<T>::instance = new Model<T>();
  }


  #endif  /* TEST_DMULTINOM_LIKELIHOOD_HPP */


