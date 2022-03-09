#ifndef INST_INCL_INTERFACE_INTERFACE_HPP
#define INST_INCL_INTERFACE_INTERFACE_HPP
/*
* Interface file. Uses pre-processing macros
* to interface with multiple modeling platforms.
*/

//#define TMB_MODEL

//traits for interfacing with TMB
#ifdef TMB_MODEL

#define TMB_LIB_INIT R_init_FIMS
#include <TMB.hpp>

#endif

#endif
