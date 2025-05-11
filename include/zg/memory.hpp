#pragma once
#include <stdexcept>
#include <cstdint>
namespace zg
{
    size_t& ensure_allocated_size(size_t*& size_pointer);
    template <typename T>
    std::pair<size_t, size_t> move_pointer(long amount, size_t index, T* pointer, size_t current_size)
    {
        auto n_s_idx        = index + (amount * -1);
        auto src            = amount > 0 ? &pointer[index] : &pointer[n_s_idx];
        auto dest           = amount > 0 ? &pointer[index + amount] : &pointer[index];
        auto mve_amt        = amount > 0 ? current_size - index : current_size - n_s_idx;
        auto copy_buf       = (T*)malloc(mve_amt * sizeof(T));
        auto mve_amt_bytes  = mve_amt;
        memcpy(copy_buf, src, mve_amt_bytes);
        memcpy(dest, copy_buf, mve_amt_bytes);
        delete[] copy_buf;
        if (amount < 0)
        {
            auto new_size = current_size + amount;
            return {new_size, current_size - new_size};
        }
        return {index, amount};
    }
    /**
     * @brief allocates [amount](T) at an index
     * @param amount can be negative or positive, if positive allocates/resizes current_pointer to fit the new amount
     * @param index must be exactly |less than or equal| to currentSize, offset into the pointer where you would like new memory
     * @param current_pointer if null, will be allocated, if set will be reallocated
     * @param current_size_pointer if null, will be allocated, if set will be updated
     * @return as all values are passed in/out through arguments, this function returns void
     */
    template<typename T>
    size_t allocate_amount_at_index(long amount, size_t index, T*& current_pointer, size_t current_size = 0)
    {
        if (!amount)
            return 0;
        constexpr auto size_of_type = sizeof(T);
        long new_size = current_size + amount;
        if (new_size < 0)
            throw std::range_error("new_size is below 0. must be at 0 or above");
        size_t byte_new_size = new_size * size_of_type;
        size_t low_index = index;
        size_t amt_count = amount * size_of_type;
        if (current_pointer)
        {
            bool moved = false, realloced = false;
            if (amount < 0)
                goto _move;
        _realloc:
            current_pointer = (T*)realloc(current_pointer, byte_new_size);
            realloced = true;
            if (!moved && index < current_size)
            {
        _move:
                auto apir = move_pointer(amount, index, current_pointer, current_size);
                low_index = apir.first;
                amt_count = apir.second;
                if (!moved)
                {
                    moved = true;
                    if (!realloced)
                        goto _realloc;
                }
            }
        }
        if (!current_pointer)
        {
            current_pointer = (T*)malloc(amt_count);
        }
        memset(&current_pointer[low_index], 0, amt_count * size_of_type);
        return new_size;
    }
    struct chunk_mem_allocator
    {

    };
}
#define CMA zg::indexed_memory_allocator