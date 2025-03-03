#pragma once
#include <zg/system/headerplujplusdefines.hpp>
#include <chrono>
/*keys pressed and for a peaceful const purpose

*/
struct peaceccsc
{
    // i could always propose virtual ~peaceccsc() = delete;
                                virtual ~peaceccsc() = default;
    // template<T>
    // virtual T now();

    static LD_REAL now() { return NANO_TIMEPOINT_CAST(SYS_CLOCK::now()).time_since_epoch().count(); }
};
#define NOT_GONNA_JO_THIS_THIS_JONE(thingypeacemode) (void*)1;