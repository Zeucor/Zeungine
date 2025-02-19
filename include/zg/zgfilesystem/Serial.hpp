#pragma once
namespace zgfilesystem
{
    template<typename O_T, typename I_T>
    struct Serial;
    template<typename T, typename O_T, typename I_T>
    void serialize(Serial<O_T, I_T>& serial, const T &value);
    template<typename T, typename O_T, typename I_T>
    void deserialize(Serial<O_T, I_T>& serial, T &value);
    template<typename O_T, typename I_T>
    struct Serial
    {
        O_T& ot;
        I_T& it;
        Serial(O_T& _ot, I_T& _it):
            ot(_ot),
            it(_it)
        {};
        template<typename T>
        constexpr Serial& operator << (const T& value)
        {
            if constexpr (is_trivially_copyable<T>())
            {
                ot.write((const char *)&value, sizeof(value));
            }
            else if constexpr (requires { zgfilesystem::serialize(*this, value); })
            {
                zgfilesystem::serialize(*this, value);
            }
            else
            {
                static_assert("Unable to serialize T");
            }
            return *this;
        }
        template<typename T>
        constexpr Serial& operator >> (T& value)
        {
            if constexpr (is_trivially_copyable<T>())
            {
                it.read((char *)&value, sizeof(value));
            }
            else if constexpr (requires { zgfilesystem::deserialize(*this, value); })
            {
                zgfilesystem::deserialize(*this, value);
            }
            else
            {
                static_assert("Unable to deserialize T");
            }
            return *this;
        }
    };
}