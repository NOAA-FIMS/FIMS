/** \file def.hpp
 */

/*
 * File:   def.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 */
#ifndef DEF_HPP
#define DEF_HPP
#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

// The following rows initialize default log files for outputing model progress
// comments used to assist in diagnosing model issues and tracking progress.
// These files will only be created if a logs folder is added to the root model
// directory.
std::ofstream FIMS_LOG("logs/fims.log");   /**< Generic log file */
std::ofstream INFO_LOG("logs/info.log");   /**< Information.hpp log file */
std::ofstream ERROR_LOG("logs/error.log"); /**< Error tracking log file */
std::ofstream DATA_LOG("logs/data.log");   /**< Data input tracking log file */
std::ofstream MODEL_LOG("logs/model.log"); /**< Model.hpp log file */
std::ofstream FLEET_LOG("logs/fleet.log"); /**< Fleet module log file */
std::ofstream DISTRIBUTIONS_LOG("logs/distributions.log"); /**< Fleet module log file */
std::ofstream POPULATION_LOG(
    "logs/population.log"); /**< Populations module log file */
std::ofstream RECRUITMENT_LOG(
    "logs/recruitment.log");                 /**< Recruitment module log file */
std::ofstream GROWTH_LOG("logs/growth.log"); /**< Growth module log file */
std::ofstream MATURITY_LOG(
    "logs/maturity.log"); /**< Maturity module log file */
std::ofstream SELECTIVITY_LOG(
    "logs/selectivity.log"); /**< Selectivity module log file */
std::ofstream DEBUG_LOG(
    "logs/debug/debug.log"); /**< Development debugging log file */

#ifdef TMB_MODEL
// simplify access to singletons
#define TMB_FIMS_REAL_TYPE double
#define TMB_FIMS_FIRST_ORDER AD<TMB_FIMS_REAL_TYPE>
#define TMB_FIMS_SECOND_ORDER AD<TMB_FIMS_FIRST_ORDER>
#define TMB_FIMS_THIRD_ORDER AD<TMB_FIMS_SECOND_ORDER>
#endif

namespace fims {

/**
 * A static class for FIMS logging.
 */

class fims_log {
 public:
  static std::map<std::string, std::ofstream>
      FIMS_LOGS; /**< Map Log of files */
  /**
   * Static getter for retrieving a specific log file.
   */
  static std::ofstream& get(const std::string& l) {
    typename std::map<std::string, std::ofstream>::iterator it;
    it = fims_log::FIMS_LOGS.find(l);
    if (it == fims_log::FIMS_LOGS.end()) {
      std::ofstream& of = fims_log::FIMS_LOGS[l];
      of.open(l.c_str());
    }

    return fims_log::FIMS_LOGS[l];
  }
};

std::map<std::string, std::ofstream> fims_log::FIMS_LOGS;

}  // namespace fims

#endif /* TRAITS_HPP */
