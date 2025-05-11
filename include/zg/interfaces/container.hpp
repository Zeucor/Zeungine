#pragma
#include <cstdint>
namespace zg::interfaces
{
    template<typename T>
    struct container
    {
        virtual ~container() = default;
        virtual size_t size() = 0;
        virtual T* data() = 0;
    };
}