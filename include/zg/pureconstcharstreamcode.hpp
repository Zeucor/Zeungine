#pragma once
#include <zg/system/headerplujplusdefines.hpp>
#include <chrono>
/* 1 keys pressed and for a peaceful const purpose
   + 
*  6 = 3 + 3 
*/struct peaceccsc
{/*=
*     // i could always propose virtual ~peaceccsc() = delete;
                              */virtual ~peaceccsc() = default;
    // template<T>
    // virtual T now();

    static LD_REAL now() { return NANO_TIMEPOINT_CAST(SYS_CLOCK::now()).time_since_epoch().count(); }
    static LD_REAL findValue();
    // the higher must know of the context and evolve it
    // peace thruuout all operator contexts
};
#define GONNA_JO_THIS_THIS_JONE(thingypeacemode) (unsigned /* Signed whats the difference we're all headej forwards through time for a */ long long /* time */)17861513/*j++ to 17861514*/
