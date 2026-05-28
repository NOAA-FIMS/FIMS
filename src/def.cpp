/**
 * \file def.cpp
 * \brief Implementation of definitions and static members for the FIMSLog
 * class.
 */

#include "../inst/include/common/def.hpp"

namespace fims {
std::shared_ptr<FIMSLog> FIMSLog::fims_log = std::make_shared<FIMSLog>();
}