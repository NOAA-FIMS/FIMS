#include "../inst/include/common/def.hpp"

std::shared_ptr<fims::FIMSLog> fims::FIMSLog::fims_log =
std::make_shared<fims::FIMSLog>(); /**< Create a log object for writing log entries to a file. */