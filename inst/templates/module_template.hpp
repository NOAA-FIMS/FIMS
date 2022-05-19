/*
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the 
 * source folder for reuse information.
 *
 * {{{ module_type }}} base file
 * The purpose of this file is to include any .hpp files within the 
 * subfolders so that only this file needs to included in the model.hpp file.
 * 
 * DEFINE guards for module_type module outline to define the 
 * module_type hpp file if not already defined.
 */
#define POPULATION_DYNAMICS_{{{ module_type }}}_HPP
#define POPULATION_DYNAMICS_{{{ module_type }}}_HPP


#include "functors/{{{module_type}}}_base.hpp"
#include "functors/{{{module_name}}}.hpp"

#endif /* FIMS_POPULATION_DYNAMICS_{{{module_type}}}_HPP */