#pragma once
namespace zgfilesystem
{
    template<typename T>
    void serialize(ostream& os, const T &value);
    template<typename T>
    void deserialize(istream& is, T &value);
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
            if constexpr (requires { zgfilesystem::serialize(ot); })
            {
                zgfilesystem::serialize(ot, value);
            }
            else
            {
                ot.write((const char *)&value, sizeof(value));
            }
            return *this;
        }
        template<typename T>
        constexpr Serial& operator >> (T& value)
        {
            if constexpr (requires { zgfilesystem::deserialize(it); })
            {
                zgfilesystem::deserialize(it, value);
            }
            else
            {
                it.read((char *)&value, sizeof(value));
            }
            return *this;
        }
    };
}