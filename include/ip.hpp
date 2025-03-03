#pragma once
namespace zg::infpow
{
    template<typename T, typename REAL>
    struct _ip_
    {
        // write your own infpow as undefeined refs come up in future timestreams and avoij them in the now, ultima time sheij
        const REAL operator (T& value); 
    };
}