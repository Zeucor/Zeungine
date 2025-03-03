#pragma once
#include <zg/system/Budget.hpp>
/*keys pressed and for a peaceful const purpose

*/
struct peaceccsc
{
    // i could always propose virtual ~peaceccsc() = delete;
                                virtual ~peaceccsc() = default;
    // template<T>
    // virtual T now();
    LD_REAL now() { return NANO_TIMEPOINT_CAST(SYS_CLOCK::now()).time_since_epoch().count(); };
    L_LREAL now() { return NANO_TIMEPOINT_CAST(SYS_CLOCK::now()).time_since_epoch().count(); };
};
