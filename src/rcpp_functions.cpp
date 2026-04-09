
 #include "../inst/include/interface/rcpp/rcpp_interface.hpp"
 #include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
 
#include <Rcpp.h>

void register_functions(Rcpp::Module& m) {

  Rcpp::function("CreateTMBModel", &CreateTMBModel);
  Rcpp::function("set_fixed", &set_fixed_parameters);
  Rcpp::function("get_fixed", &get_fixed_parameters_vector);
  Rcpp::function("set_random", &set_random_parameters);
  Rcpp::function("get_random", &get_random_parameters_vector);

  Rcpp::function("get_parameter_names", &get_parameter_names);
  Rcpp::function("get_random_names", &get_random_names);

  Rcpp::function("clear", clear);

  Rcpp::function("get_log", get_log);
  Rcpp::function("get_log_errors", get_log_errors);
  Rcpp::function("get_log_warnings", get_log_warnings);
  Rcpp::function("get_log_info", get_log_info);

  Rcpp::function("write_log", write_log);
  Rcpp::function("set_log_path", set_log_path);
  Rcpp::function("init_logging", init_logging);
  Rcpp::function("set_log_throw_on_error", set_log_throw_on_error);

  Rcpp::function("log_info", log_info);
  Rcpp::function("log_warning", log_warning);
  Rcpp::function("log_error", log_error);

  Rcpp::function("logit", logit_rcpp);
  Rcpp::function("inv_logit", inv_logit_rcpp);
}