#include "../inst/include/common/def.hpp"

namespace fims {
    std::shared_ptr<FIMSLog> FIMSLog::fims_log = 
    std::make_shared<FIMSLog>();
}