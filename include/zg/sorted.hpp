#pragma once
#include "memory.hpp"
#include "interfaces/container.hpp"
#include "math/normal.hpp"
namespace zg
{
    struct sorter
    {
        virtual ~sorter() = default;
        template <typename ValueT, typename = std::enable_if_t<std::is_integral_v<ValueT> || std::is_floating_point_v<ValueT>>>
        size_t get_sorted_index(const ValueT& value)
        {
            auto& container_ref = *dynamic_cast<interfaces::container<ValueT>*>(this);
            auto size = container_ref.size();
            if (!size)
                return 0;
            auto data = container_ref.data();
            auto normu = math::normalizedMinMaxValue(data[0], value, data[size-1]);
            auto index = size_t(normu * size);
            if (index >= size)
                return size;
            auto currentindex = index;
            auto current_value_pointer = &data[currentindex];
            if (*current_value_pointer == value)
                return currentindex;
            auto directional = (*current_value_pointer < value) ? 1 : -1;
            if ((currentindex == 0 && directional == -1) ||
                (currentindex == size && directional == 1))
                return currentindex;
            for (currentindex += directional; currentindex > 0 && currentindex < size; currentindex += directional)
            {
                auto& current_value = data[currentindex];
                if (directional == 1)
                {
                    if (current_value < value)
                        continue;
                    else
                    {
                        currentindex--;
                        break;
                    }
                }
                else
                {
                    if (current_value > value)
                        continue;
                    break;
                }
            }
            return currentindex;
        }
    };
    template<typename ValueT>
    struct sorted_vector:
        interfaces::container<ValueT>,
        sorter
    {
    private:
        size_t m_size = 0;
        ValueT* m_data = 0;
    public:
        sorted_vector() = default;
        ~sorted_vector()
        {
            clear();
        }
        template<typename... Args>
        ValueT& emplace_wi_dupes(Args&&... args)
        {
            ValueT value(args...);
            auto index = get_sorted_index<ValueT>(value);
            m_size = allocate_amount_at_index<ValueT>(1, index, m_data, m_size);
            return (m_data[index] = value);
        }
        template<typename... Args>
        ValueT& emplace_no_dupes(Args&&... args)
        {
            ValueT value(args...);
            auto index = get_sorted_index<ValueT>(value);
            if (index < m_size)
            {
                auto& c_value = m_data[index];
                if (c_value == value)
                    return m_data[index];
            }
            else if (m_size && (index - 1) < m_size)
            {
                auto& c_value = m_data[index-1];
                if (c_value == value)
                    return m_data[index-1];
            }
            m_size = allocate_amount_at_index<ValueT>(1, index, m_data, m_size);
            return (m_data[index] = value);
        }
        bool erase(const ValueT& value)
        {
            auto index = get_sorted_index<ValueT>(value);
            if (index < m_size)
            {
                auto& c_value = m_data[index];
                if (c_value == value)
                {
                    m_size = allocate_amount_at_index<ValueT>(-1, index, m_data, m_size);
                    return true;
                }
            }
            return false;
        }
        void clear()
        {
            if (!m_data)
                return;
            free(m_data);
            m_data = 0;
            m_size = 0;
        }
        size_t size() override
        {
            return m_size;
        }
        ValueT* data() override
        {
            return m_data;
        }
    };
    template<typename KeyT, typename ValueT>
    struct sorted_map
    {

    };
    template<typename ValueT>
    struct sorted_set
    {

    };
}