/**
 * @file fims_modules.hpp
 * @brief Rcpp module definitions. Allows for the use of
 * methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef SRC_FIMS_MODULES_HPP
#define SRC_FIMS_MODULES_HPP

#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_data.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_depletion.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_fleet.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_growth.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_likelihood.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_math.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_natural_mortality.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_recruitment.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp"
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"
#include "../inst/include/interface/rcpp/rcpp_interface.hpp"

RCPP_EXPOSED_CLASS(Parameter)
RCPP_EXPOSED_CLASS(ParameterVector)
RCPP_EXPOSED_CLASS(RealVector)
RCPP_EXPOSED_CLASS(SharedInt)
RCPP_EXPOSED_CLASS(SharedString)
RCPP_EXPOSED_CLASS(SharedReal)
RCPP_EXPOSED_CLASS(SharedBoolean)
RCPP_EXPOSED_CLASS(CatchAtAgePopulationDerivedQuantitiesInterface)
RCPP_EXPOSED_CLASS(CatchAtAgeFleetDerivedQuantitiesInterface)
RCPP_EXPOSED_CLASS(LikelihoodInterfaceBase)
RCPP_EXPOSED_CLASS(NormalLikelihoodInterface)
RCPP_EXPOSED_CLASS(LognormalLikelihoodInterface)
RCPP_EXPOSED_CLASS(GammaLikelihoodInterface)
RCPP_EXPOSED_CLASS(InvGammaLikelihoodInterface)
RCPP_EXPOSED_CLASS(MultinomialLikelihoodInterface)

/**
 * @brief Define fims C++ functions and classes exposed in R
 *
 * @details
 * The use of `RCPP_MODULE()` allows for exporting of specific C++ code to the
 * R environment, making it callable from R, i.e., providing declarative code
 * of what the module exposes to R.
 *
 * Each element included in the module should have a name, a pointer, and a
 * description separated by commas in that order. Elements can be functions or
 * classes. Within an element, both the name and the description should be
 * wrapped in quotes.
 *
 * The description is printed to the in R when users run `methods::show()` on
 * a function or class within the RCPP_MODULE, e.g., `methods::show(Parameter)`.
 * Thus, information in the description should either link to the relevant
 * doxygen documentation for the C++ class or function or exactly duplicate
 * what is written in the doxygen.
 *
 * Each of the functions included in this module should be exported by manually
 * exporting them in R/FIMS-package.R. Typically, R packages that use C++ would
 * take care a lot of this stuff automatically using `Rcpp::export` calls in the
 * C++ documentation but we cannot do that within FIMS because of how the
 * package is compiled.
 *
 */
RCPP_MODULE(fims) {
  Rcpp::function(
      "CreateTMBModel", &CreateTMBModel,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "set_fixed", &set_fixed_parameters,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "get_fixed", &get_fixed_parameters_vector,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "set_random", &set_random_parameters,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "get_random", &get_random_parameters_vector,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_parameter_names", &get_parameter_names,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_random_names", &get_random_names,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function("add_shared_prior", &setup_prior, 
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "clear", clear,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log", get_log,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log_errors", get_log_errors,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log_warnings", get_log_warnings,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "get_log_info", get_log_info,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "write_log", write_log,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "set_log_path", set_log_path,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "init_logging", init_logging,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "set_log_throw_on_error", set_log_throw_on_error,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "log_info", log_info,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "log_warning", log_warning,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      "log_error", log_error,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "logit", logit_rcpp,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");
  Rcpp::function(
      // TODO: fix the naming mismatch
      "inv_logit", inv_logit_rcpp,
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.");

  Rcpp::class_<Parameter>(
      "Parameter",
      "See https://noaa-fims.github.io/FIMS/doxygen/classParameter.html.")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::initial_value_m)
      .field("estimated_value", &Parameter::final_value_m)
      .field("id", &Parameter::id_m)
      .field("estimation_type", &Parameter::estimation_type_m);

  Rcpp::class_<ParameterVector>(
      "ParameterVector",
      "See https://noaa-fims.github.io/FIMS/doxygen/classParameterVector.html.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("add_prior", &ParameterVector::add_prior)
      .method("get", &ParameterVector::get)
      .method("set", &ParameterVector::set)
      .method("show", &ParameterVector::show)
      .method("at", &ParameterVector::at)
      .method("size", &ParameterVector::size)
      .method("resize", &ParameterVector::resize)
      .method("set_all_estimable", &ParameterVector::set_all_estimable)
      .method("set_all_random", &ParameterVector::set_all_random)
      .method("fill", &ParameterVector::fill)
      .method("get_id", &ParameterVector::get_id);

  Rcpp::class_<RealVector>(
      "RealVector",
      "See https://noaa-fims.github.io/FIMS/doxygen/classRealVector.html.")
      .constructor()
      .constructor<size_t>()
      .constructor<Rcpp::NumericVector, size_t>()
      .method("get", &RealVector::get)
      .method("set", &RealVector::set)
      .method("fromRVector", &RealVector::fromRVector)
      .method("toRVector", &RealVector::toRVector)
      .method("show", &RealVector::show)
      .method("at", &RealVector::at)
      .method("size", &RealVector::size)
      .method("resize", &RealVector::resize)
      .method("get_id", &RealVector::get_id);

  Rcpp::class_<SharedInt>(
      "SharedInt",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedInt.html.")
      .constructor()
      .constructor<int>()
      .method("get", &SharedInt::get)
      .method("set", &SharedInt::set);

  Rcpp::class_<SharedString>(
      "SharedString",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedString.html.")
      .constructor()
      .constructor<std::string>()
      .method("get", &SharedString::get)
      .method("set", &SharedString::set);

  Rcpp::class_<SharedBoolean>(
      "SharedBoolean",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedBoolean.html.")
      .constructor()
      .constructor<bool>()
      .method("get", &SharedBoolean::get)
      .method("set", &SharedBoolean::set);

  Rcpp::class_<SharedReal>(
      "SharedReal",
      "See https://noaa-fims.github.io/FIMS/doxygen/classSharedReal.html.")
      .constructor()
      .constructor<double>()
      .method("get", &SharedReal::get)
      .method("set", &SharedReal::set);

  Rcpp::class_<BevertonHoltRecruitmentInterface>(
      "BevertonHoltRecruitment",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classBevertonHoltRecruitmentInterface.html.")
      .constructor()
      .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep)
      .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero)
      .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs)
      .field("log_r", &BevertonHoltRecruitmentInterface::log_r)
      .field("log_expected_recruitment",
             &BevertonHoltRecruitmentInterface::log_expected_recruitment)
      .field("n_years", &BevertonHoltRecruitmentInterface::n_years)
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
      .method("SetRecruitmentProcessID",
              &BevertonHoltRecruitmentInterface::SetRecruitmentProcessID)
      .method("evaluate_mean",
              &BevertonHoltRecruitmentInterface::evaluate_mean);

  Rcpp::class_<LogDevsRecruitmentInterface>(
      "LogDevsRecruitmentProcess",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogDevsRecruitmentInterface.html.")
      .constructor()
      .method("get_id", &LogDevsRecruitmentInterface::get_id)
      .method("evaluate_process",
              &LogDevsRecruitmentInterface::evaluate_process);

  Rcpp::class_<LogRRecruitmentInterface>(
      "LogRRecruitmentProcess",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogRRecruitmentInterface.html.")
      .constructor()
      .method("get_id", &LogRRecruitmentInterface::get_id)
      .method("evaluate_process", &LogRRecruitmentInterface::evaluate_process);

  Rcpp::class_<FleetInterface>(
      "Fleet",
      "See https://noaa-fims.github.io/FIMS/doxygen/classFleetInterface.html.")
      .constructor()
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("n_ages", &FleetInterface::n_ages)
      .field("n_years", &FleetInterface::n_years)
      .field("n_lengths", &FleetInterface::n_lengths)
      .field("observed_landings_units",
             &FleetInterface::observed_landings_units)
      .field("observed_index_units", &FleetInterface::observed_index_units)
      .field("log_index_expected", &FleetInterface::log_index_expected)
      .field("log_landings_expected", &FleetInterface::log_landings_expected)
      .field("agecomp_expected", &FleetInterface::agecomp_expected)
      .field("lengthcomp_expected", &FleetInterface::lengthcomp_expected)
      .field("agecomp_proportion", &FleetInterface::agecomp_proportion)
      .field("lengthcomp_proportion", &FleetInterface::lengthcomp_proportion)
      .field("age_to_length_conversion",
             &FleetInterface::age_to_length_conversion)
      .field("mean_log_q", &FleetInterface::mean_log_q)
      .field("log_index_to_depletion_carrying_capacity_ratio",
             &FleetInterface::log_index_to_depletion_carrying_capacity_ratio)
      .method("get_id", &FleetInterface::get_id)
      .method("SetName", &FleetInterface::SetName)
      .method("GetName", &FleetInterface::GetName)
      .method("SetObservedAgeCompDataID",
              &FleetInterface::SetObservedAgeCompDataID)
      .method("GetObservedAgeCompDataID",
              &FleetInterface::GetObservedAgeCompDataID)
      .method("SetObservedLengthCompDataID",
              &FleetInterface::SetObservedLengthCompDataID)
      .method("GetObservedLengthCompDataID",
              &FleetInterface::GetObservedLengthCompDataID)
      .method("SetObservedIndexDataID", &FleetInterface::SetObservedIndexDataID)
      .method("GetObservedIndexDataID", &FleetInterface::GetObservedIndexDataID)
      .method("SetObservedLandingsDataID",
              &FleetInterface::SetObservedLandingsDataID)
      .method("GetObservedLandingsDataID",
              &FleetInterface::GetObservedLandingsDataID)
      .method("SetSelectivityID", &FleetInterface::SetSelectivityID);

  Rcpp::class_<AgeCompDataInterface>("AgeComp",
                                     "See "
                                     "https://noaa-fims.github.io/FIMS/doxygen/"
                                     "classAgeCompDataInterface.html.")
      .constructor<int, int>()
      .field("age_comp_data", &AgeCompDataInterface::age_comp_data)
      .method("get_id", &AgeCompDataInterface::get_id);

  Rcpp::class_<LengthCompDataInterface>(
      "LengthComp",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLengthCompDataInterface.html.")
      .constructor<int, int>()
      .field("length_comp_data", &LengthCompDataInterface::length_comp_data)
      .method("get_id", &LengthCompDataInterface::get_id);

  Rcpp::class_<LandingsDataInterface>(
      "Landings",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLandingsDataInterface.html.")
      .constructor<int>()
      .field("landings_data", &LandingsDataInterface::landings_data)
      .method("get_id", &LandingsDataInterface::get_id);

  Rcpp::class_<IndexDataInterface>(
      "Index",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classIndexDataInterface.html.")
      .constructor<int>()
      .field("index_data", &IndexDataInterface::index_data)
      .method("get_id", &IndexDataInterface::get_id);

  Rcpp::class_<PopulationInterface>(
      "Population",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classPopulationInterface.html.")
      .constructor()
      .method("get_id", &PopulationInterface::get_id, "get population ID")
      .field("n_ages", &PopulationInterface::n_ages, "number of ages")
      .field("n_fleets", &PopulationInterface::n_fleets, "number of fleets")
      .field("n_years", &PopulationInterface::n_years, "number of years")
      .field("n_lengths", &PopulationInterface::n_lengths, "number of lengths")
      .field("log_M", &PopulationInterface::log_M,
             "natural log of the natural mortality of the population")
      .field("log_f_multiplier", &PopulationInterface::log_f_multiplier,
             "natural log of the fishing mortality multiplier")
      .field("spawning_biomass_ratio",
             &PopulationInterface::spawning_biomass_ratio,
             "spawning biomass ratio")
      .field("log_init_naa", &PopulationInterface::log_init_naa,
             "natural log of the initial numbers at age")
      .field("ages", &PopulationInterface::ages,
             "vector of ages in the population; length n_ages")
      .method("SetMaturityID", &PopulationInterface::SetMaturityID,
              "Set the unique id for the Maturity object")
      .method("SetGrowthID", &PopulationInterface::SetGrowthID,
              "Set the unique id for the growth object")
      .method("SetRecruitmentID", &PopulationInterface::SetRecruitmentID,
              "Set the unique id for the Recruitment object")
      .method("SetDepletionID", &PopulationInterface::SetDepletionID,
              "Set the unique id for the Depletion object")
      .method("AddFleet", &PopulationInterface::AddFleet,
              "Set a unique fleet id to the list of fleets operating on this "
              "population")
      .method("SetName", &PopulationInterface::SetName,
              "Set the name of the population")
      .method("GetName", &PopulationInterface::GetName,
              "Get the name of the population");

  Rcpp::class_<LogisticMaturityInterface>(
      "LogisticMaturity",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogisticMaturityInterface.html.")
      .constructor()
      .field("inflection_point", &LogisticMaturityInterface::inflection_point)
      .field("slope", &LogisticMaturityInterface::slope)
      .method("get_id", &LogisticMaturityInterface::get_id)
      .method("evaluate", &LogisticMaturityInterface::evaluate);

  Rcpp::class_<LogisticSelectivityInterface>(
      "LogisticSelectivity",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogisticSelectivityInterface.html.")
      .constructor()
      .field("inflection_point",
             &LogisticSelectivityInterface::inflection_point)
      .field("slope", &LogisticSelectivityInterface::slope)
      .method("get_id", &LogisticSelectivityInterface::get_id)
      .method("evaluate", &LogisticSelectivityInterface::evaluate);

  Rcpp::class_<DoubleLogisticSelectivityInterface>(
      "DoubleLogisticSelectivity",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDoubleLogisticSelectivityInterface.html.")
      .constructor()
      .field("inflection_point_asc",
             &DoubleLogisticSelectivityInterface::inflection_point_asc)
      .field("slope_asc", &DoubleLogisticSelectivityInterface::slope_asc)
      .field("inflection_point_desc",
             &DoubleLogisticSelectivityInterface::inflection_point_desc)
      .field("slope_desc", &DoubleLogisticSelectivityInterface::slope_desc)
      .method("get_id", &DoubleLogisticSelectivityInterface::get_id)
      .method("evaluate", &DoubleLogisticSelectivityInterface::evaluate);

  Rcpp::class_<EWAAGrowthInterface>(
      "EWAAGrowth",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classEWAAGrowthInterface.html.")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages, "Ages for each age class.")
      .field("weights", &EWAAGrowthInterface::weights,
             "Weights for each age class.")
      .field("n_years", &EWAAGrowthInterface::n_years, "Number of years.")
      .method("get_id", &EWAAGrowthInterface::get_id)
      .method("evaluate", &EWAAGrowthInterface::evaluate);

  Rcpp::class_<PellaTomlinsonInterface>(
      "PTDepletion",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classPellaTomlinsonInterface.html.")
      .constructor()
      .field("log_growth_rate", &PellaTomlinsonInterface::log_growth_rate)
      .field("log_carrying_capacity", &PellaTomlinsonInterface::log_carrying_capacity)
      .field("log_shape", &PellaTomlinsonInterface::log_shape)
      .field("log_depletion", &PellaTomlinsonInterface::log_depletion)
      .field("log_init_depletion", &PellaTomlinsonInterface::log_init_depletion)
      .field("log_expected_depletion",
             &PellaTomlinsonInterface::log_expected_depletion)
      .field("n_years", &PellaTomlinsonInterface::n_years)
      .method("get_id", &PellaTomlinsonInterface::get_id)
      .method("evaluate_mean", &PellaTomlinsonInterface::evaluate_mean);

  Rcpp::class_<DnormDistributionsInterface>(
      "DnormDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDnormDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id)
      .method("evaluate", &DnormDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DnormDistributionsInterface::set_observed_data)
      .method("set_distribution_mean",
              &DnormDistributionsInterface::set_distribution_mean)
      .method("set_distribution_links",
              &DnormDistributionsInterface::set_distribution_links)
      .field("observed_values", &DnormDistributionsInterface::observed_values)
      .field("expected_values", &DnormDistributionsInterface::expected_values)
      .field("expected_mean", &DnormDistributionsInterface::expected_mean)
      .field("log_sd", &DnormDistributionsInterface::log_sd);

  Rcpp::class_<DlnormDistributionsInterface>(
      "DlnormDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDlnormDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id)
      .method("evaluate", &DlnormDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DlnormDistributionsInterface::set_observed_data)
      .method("set_distribution_links",
              &DlnormDistributionsInterface::set_distribution_links)
      .field("observed_values", &DlnormDistributionsInterface::observed_values)
      .field("expected_values", &DlnormDistributionsInterface::expected_values)
      .field("log_sd", &DlnormDistributionsInterface::log_sd);

  Rcpp::class_<DgammaDistributionsInterface>("DgammaDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDgammaDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DgammaDistributionsInterface::get_id)
      .method("evaluate", &DgammaDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DgammaDistributionsInterface::set_observed_data)
      .method("set_distribution_links",
              &DgammaDistributionsInterface::set_distribution_links)
      .field("observed_values", &DgammaDistributionsInterface::observed_values)
      .field("expected_values", &DgammaDistributionsInterface::expected_values)
      .field("log_sd", &DgammaDistributionsInterface::log_sd);
    
     Rcpp::class_<DinvgammaDistributionsInterface>(
      "DinvgammaDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDinvgammaDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DinvgammaDistributionsInterface::get_id)
      .method("evaluate", &DinvgammaDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DinvgammaDistributionsInterface::set_observed_data)
      .method("set_distribution_links",
              &DinvgammaDistributionsInterface::set_distribution_links)
      .field("observed_values",
             &DinvgammaDistributionsInterface::observed_values)
      .field("log_shape", &DinvgammaDistributionsInterface::log_shape)
      .field("log_scale", &DinvgammaDistributionsInterface::log_scale);

  Rcpp::class_<DmultinomDistributionsInterface>(
      "DmultinomDistribution",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classDmultinomDistributionsInterface.html.")
      .constructor()
      .method("get_id", &DmultinomDistributionsInterface::get_id)
      .method("evaluate", &DmultinomDistributionsInterface::evaluate)
      .method("set_observed_data",
              &DmultinomDistributionsInterface::set_observed_data)
      .method("set_distribution_links",
              &DmultinomDistributionsInterface::set_distribution_links)
      .method("set_note", &DmultinomDistributionsInterface::set_note)
      .field("observed_values",
             &DmultinomDistributionsInterface::observed_values)
      .field("expected_values",
             &DmultinomDistributionsInterface::expected_values)
      .field("dims", &DmultinomDistributionsInterface::dims);

  Rcpp::class_<LikelihoodInterfaceBase>(
      "LikelihoodBase",
      "Base class for likelihood component interfaces.")
      .method("set_role", &LikelihoodInterfaceBase::set_role)
      .method("set_real_input", &LikelihoodInterfaceBase::set_real_input)
      .method("set_parameter_input",
              &LikelihoodInterfaceBase::set_parameter_input)
      .field("observed_values", &LikelihoodInterfaceBase::observed_values)
      .field("expected_values", &LikelihoodInterfaceBase::expected_values)
      .field("real_input", &LikelihoodInterfaceBase::real_input)
      .field("nll_components", &LikelihoodInterfaceBase::nll_components);

  Rcpp::class_<NormalLikelihoodInterface>(
      "NormalLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classNormalLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &NormalLikelihoodInterface::get_id)
      .method("evaluate", &NormalLikelihoodInterface::evaluate)
      .field("log_sd", &NormalLikelihoodInterface::log_sd);

  Rcpp::class_<LognormalLikelihoodInterface>(
      "LognormalLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLognormalLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &LognormalLikelihoodInterface::get_id)
      .method("evaluate", &LognormalLikelihoodInterface::evaluate)
      .field("log_sd", &LognormalLikelihoodInterface::log_sd);

  Rcpp::class_<GammaLikelihoodInterface>(
      "GammaLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classGammaLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &GammaLikelihoodInterface::get_id)
      .method("evaluate", &GammaLikelihoodInterface::evaluate)
      .field("log_sd", &GammaLikelihoodInterface::log_sd);

  Rcpp::class_<InvGammaLikelihoodInterface>(
      "InvGammaLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classInvGammaLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &InvGammaLikelihoodInterface::get_id)
      .method("evaluate", &InvGammaLikelihoodInterface::evaluate)
      .field("log_shape", &InvGammaLikelihoodInterface::log_shape)
      .field("log_scale", &InvGammaLikelihoodInterface::log_scale);

  Rcpp::class_<MultinomialLikelihoodInterface>(
      "MultinomialLikelihood",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classMultinomialLikelihoodInterface.html.")
      .constructor()
      .derives<LikelihoodInterfaceBase>("LikelihoodBase")
      .method("get_id", &MultinomialLikelihoodInterface::get_id)
      .method("evaluate", &MultinomialLikelihoodInterface::evaluate)
      .field("dims", &MultinomialLikelihoodInterface::dims);

  Rcpp::class_<CatchAtAgePopulationDerivedQuantitiesInterface>(
      "CatchAtAgePopulationDerivedQuantities",
      "Catch-at-age population derived quantities exposed as RealVector "
      "members.")
      .constructor()
      .method("Initialize",
              &CatchAtAgePopulationDerivedQuantitiesInterface::Initialize)
      .method("Fill", &CatchAtAgePopulationDerivedQuantitiesInterface::Fill)
      .field("total_landings_weight",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 total_landings_weight)
      .field("total_landings_numbers",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 total_landings_numbers)
      .field("mortality_F",
             &CatchAtAgePopulationDerivedQuantitiesInterface::mortality_F)
      .field("mortality_M",
             &CatchAtAgePopulationDerivedQuantitiesInterface::mortality_M)
      .field("mortality_Z",
             &CatchAtAgePopulationDerivedQuantitiesInterface::mortality_Z)
      .field("numbers_at_age",
             &CatchAtAgePopulationDerivedQuantitiesInterface::numbers_at_age)
      .field("unfished_numbers_at_age",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 unfished_numbers_at_age)
      .field("biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::biomass)
      .field("spawning_biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 spawning_biomass)
      .field("unfished_biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 unfished_biomass)
      .field("unfished_spawning_biomass",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 unfished_spawning_biomass)
      .field("proportion_mature_at_age",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 proportion_mature_at_age)
      .field("expected_recruitment",
             &CatchAtAgePopulationDerivedQuantitiesInterface::
                 expected_recruitment)
      .field("sum_selectivity",
             &CatchAtAgePopulationDerivedQuantitiesInterface::sum_selectivity);

  Rcpp::class_<CatchAtAgeFleetDerivedQuantitiesInterface>(
      "CatchAtAgeFleetDerivedQuantities",
      "Catch-at-age fleet derived quantities exposed as RealVector members.")
      .constructor()
      .method("Initialize",
              &CatchAtAgeFleetDerivedQuantitiesInterface::Initialize)
      .method("Fill", &CatchAtAgeFleetDerivedQuantitiesInterface::Fill)
      .field("landings_numbers_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 landings_numbers_at_age)
      .field("landings_weight_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 landings_weight_at_age)
      .field("landings_numbers_at_length",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 landings_numbers_at_length)
      .field("landings_weight",
             &CatchAtAgeFleetDerivedQuantitiesInterface::landings_weight)
      .field("landings_numbers",
             &CatchAtAgeFleetDerivedQuantitiesInterface::landings_numbers)
      .field("landings_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::landings_expected)
      .field("log_landings_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 log_landings_expected)
      .field("agecomp_proportion",
             &CatchAtAgeFleetDerivedQuantitiesInterface::agecomp_proportion)
      .field("lengthcomp_proportion",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 lengthcomp_proportion)
      .field("index_numbers_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_numbers_at_age)
      .field("index_weight_at_age",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_weight_at_age)
      .field("index_numbers_at_length",
             &CatchAtAgeFleetDerivedQuantitiesInterface::
                 index_numbers_at_length)
      .field("index_weight",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_weight)
      .field("index_numbers",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_numbers)
      .field("index_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::index_expected)
      .field("log_index_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::log_index_expected)
      .field("catch_index",
             &CatchAtAgeFleetDerivedQuantitiesInterface::catch_index)
      .field("expected_catch",
             &CatchAtAgeFleetDerivedQuantitiesInterface::expected_catch)
      .field("expected_index",
             &CatchAtAgeFleetDerivedQuantitiesInterface::expected_index)
      .field("agecomp_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::agecomp_expected)
      .field("lengthcomp_expected",
             &CatchAtAgeFleetDerivedQuantitiesInterface::lengthcomp_expected);

  Rcpp::class_<CatchAtAgeInterface>(
      "CatchAtAge",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classCatchAtAgeInterface.html.")
      .constructor()
      .method("AddPopulation", &CatchAtAgeInterface::AddPopulation)
      .method("InitializePopulationDerivedQuantities",
              &CatchAtAgeInterface::InitializePopulationDerivedQuantities)
      .method("GetPopulationDerivedQuantities",
              &CatchAtAgeInterface::GetPopulationDerivedQuantities)
      .method("InitializeFleetDerivedQuantities",
              &CatchAtAgeInterface::InitializeFleetDerivedQuantities)
      .method("GetFleetDerivedQuantities",
              &CatchAtAgeInterface::GetFleetDerivedQuantities)
      .method("get_output", &CatchAtAgeInterface::to_json)
      .method("GetId", &CatchAtAgeInterface::get_id)
      .method("DoReporting", &CatchAtAgeInterface::DoReporting)
      .method("IsReporting", &CatchAtAgeInterface::IsReporting);

  Rcpp::class_<SurplusProductionInterface>(
      "SurplusProduction",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/classSurplusProductionInterface.html.")
      .constructor()
      .method("AddPopulation", &SurplusProductionInterface::AddPopulation)
      .method("GetId", &SurplusProductionInterface::get_id)
      .method("DoReporting", &SurplusProductionInterface::DoReporting)
      .method("IsReporting", &SurplusProductionInterface::IsReporting);
}

#endif /* SRC_FIMS_MODULES_HPP */
