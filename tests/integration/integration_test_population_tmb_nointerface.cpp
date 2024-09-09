#include "../../inst/include/distributions/distributions.hpp"
#include "../../inst/include/interface/interface.hpp"
#include "../../inst/include/common/data_object.hpp"
#include "../../inst/include/population_dynamics/fleet/fleet.hpp"
#include "../../inst/include/population_dynamics/population/population.hpp"

template<class Type>
Type objective_function<Type>::operator()(){

  DATA_INTEGER(fleet_num);
  DATA_INTEGER(survey_num);
  DATA_IVECTOR(yr);
  DATA_VECTOR(ages);
  //Curently not set up to run with data
 // DATA_VECTOR(observed_index_data); //nyears
  //DATA_VECTOR(observed_agecomp_data); //nyears x nages
  DATA_VECTOR(W_kg);
  DATA_SCALAR(ln_M);
  DATA_VECTOR(init_naa);

  PARAMETER_VECTOR(fleet_sel_A50); //nfleets
  PARAMETER_VECTOR(fleet_sel_slope); //nfleets
  PARAMETER_VECTOR(surv_sel_A50); //nsurveys
  PARAMETER_VECTOR(surv_sel_slope); //nsurveys
  PARAMETER_MATRIX(log_Fmort); //nyears x nfleets;
  PARAMETER(R0);
  PARAMETER(h);
  PARAMETER(logR_sd);
  PARAMETER(A50_mat);
  PARAMETER(slope_mat);

  int nyears = yr.size();
  int nages = ages.size();
  int nfleets = fleet_num;
  int nsurveys = survey_num;

  vector<Type> log_q(nyears);
  //Is log_q supposed to be 1 or 0?
  log_q.fill(1);

  fims_popdy::Population<Type> pop;



  for (int i = 0; i < nfleets; i++) {

    std::shared_ptr<fims_popdy::Fleet<Type> > f = std::make_shared<fims_popdy::Fleet<Type> >();
    f->Initialize(nyears, nages);
    //f->nyears = nyears;

    //f->observed_index_data = std::make_shared<fims_data_object::DataObject<Type> >(nyears);
    //f->observed_agecomp_data = std::make_shared<fims_data_object::DataObject<Type> >(nyears, nages);
    /*
     * To run with data, need to change 45 and 46 to:
     *     for(int y=0; y < nyears; y++){
     f->observed_index_data[y] = observed_index_data(y);
     for(int a=0; a < nages; a ++){
     int idx = y * nages + a;
     f->observed_agecomp_data[idx] = observed_agecomp_data(idx);

     }
     }
     **Note: this code chunk doesn't compile with error: no match for 'operator[]'
     **(operand types are 'std::shared_ptr<fims_data_object::DataObject<CppAD::AD<CppAD::AD<CppAD::AD<double> > > > >' and 'int')
     */


    //set up selectivity
    std::shared_ptr<fims_popdy::LogisticSelectivity<Type> > selectivity
      = std::make_shared<fims_popdy::LogisticSelectivity<Type> >();

    selectivity->inflection_point = fleet_sel_A50(i);
    selectivity->slope = fleet_sel_slope(i);
    f->selectivity = selectivity;

    for(int y = 0; y<nyears; y++){
      f->log_Fmort[y] = log_Fmort(y,i);
      f->log_q[y] = log_q(y);
      //Setting outside Prepare as Prepare() causing segfault
      f->Fmort[y] = exp(log_Fmort(y,i));
      f->q[y] = exp(log_q(y));
    }
  //Prepare function causing segfault
  //  f->Prepare(nyears);


    pop.fleets.push_back(f);

  }

  for (int i = 0; i < nsurveys; i++) {

    std::shared_ptr<fims_popdy::Fleet<Type> > s = std::make_shared<fims_popdy::Fleet<Type> >();
    s->Initialize(nyears, nages);



    //s->observed_index_data = std::make_shared<fims_data_object::DataObject<Type> >(nyears);
    //s->observed_agecomp_data = std::make_shared<fims_data_object::DataObject<Type> >(nyears, nages);

    //set up selectivity


    std::shared_ptr<fims_popdy::LogisticSelectivity<Type> > selectivity
      = std::make_shared<fims_popdy::LogisticSelectivity<Type> >();

    selectivity->inflection_point = surv_sel_A50(i);
    selectivity->slope = surv_sel_slope(i);
    s->selectivity = selectivity;
    for(int y = 0; y<nyears; y++){
      s->log_q[y] = log_q(y);
      s->q[y] = exp(log_q(y));
    }

    //Prepare function causing segfault
    //s->Prepare();
    pop.fleets.push_back(s);
  }

  pop.nfleets = pop.fleets.size();

  //initialize population
  pop.Initialize(nyears, 1, nages);

  //Set initial size to value from MCP (read in through R)
  for(int i=0; i < pop.nages; i++) {
    pop.log_init_naa[i] = log(init_naa[i]);
    pop.ages[i] = asDouble(ages(i));
  }
  for(int i=0; i<pop.nyears; i++){
    pop.years[i] = yr(i);
  }

  //Set recruitment
  std::shared_ptr<fims_popdy::SRBevertonHolt<Type> > rec =
    std::make_shared<fims_popdy::SRBevertonHolt<Type> >();
  rec->rzero = R0;
  rec->steep = h;
  /*the log_recruit_dev vector does not include a value for year == 0
    and is of length nyears - 1 where the first position of the vector
    corresponds to the second year of the time series.*/
  rec->log_recruit_devs.resize(nyears-1);
  std::fill(rec->log_recruit_devs.begin(), rec->log_recruit_devs.end(), 0.0);
  pop.recruitment = rec;

  //Set maturity
  std::shared_ptr<fims_popdy::LogisticMaturity<Type > > mat =
    std::make_shared<fims_popdy::LogisticMaturity<Type> >();
  mat->inflection_point = A50_mat;
  mat->slope = slope_mat;
  pop.maturity = mat;

  //set empirical growth
  std::shared_ptr<fims_popdy::EWAAgrowth<Type> > growth
    = std::make_shared<fims_popdy::EWAAgrowth<Type> > ();
  for (int i = 0; i < nages; i++) {
    growth->ewaa[ asDouble(ages(i))] = asDouble(W_kg(i))/1000;
  }
  pop.growth = growth;

  pop.Evaluate();

  vector<Type> pop_naa = pop.numbers_at_age;

  Type nll = 0;

  REPORT(pop_naa);

  return nll;
}


