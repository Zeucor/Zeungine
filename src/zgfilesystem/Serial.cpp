#include <zg/Serial.hpp>

template <>
Serial& deserialize(Serial& serial, std::string& str)
{
    auto size = str.size();
    serial >> size;
    str.resize(size);
    serial.readBytes(str.data(), size);
    return serial;
}
template<>
Serial& serialize(Serial& serial, const std::string& str)
{
    auto size = str.size();
    serial << size;
    serial.writeBytes(str.c_str(), size);
    return serial;
}
template <>
Serial& deserialize(Serial& serial, std::vector<std::string>& vec)
{
    auto size = vec.size();
    serial >> size;
    vec.resize(size);
    for (auto i = 0; i < size; ++i)
        serial >> vec[i];
    return serial;
}
template <>
Serial& serialize(Serial& serial, const std::vector<std::string>& vec)
{
    auto size = vec.size();
    serial << size;
    for (auto i = 0; i < size; ++i)
        serial << vec[i];
    return serial;
}
std::map<std::type_index, any_registry::any_serialize> any_registry::serializers;
std::map<std::type_index, any_registry::any_deserialize> any_registry::deserializers;
std::map<std::type_index, std::string> any_registry::type_to_stable_name;
std::map<std::string, std::type_index> any_registry::stable_name_to_type;
void any_registry::register_type(
    std::type_index type,
    const std::string& stable_name,
    const any_serialize& ser_func,
    const any_deserialize& deser_func)
{
    if (stable_name_to_type.count(stable_name))
    {
        return;
    }
    serializers[type] = ser_func;
    deserializers[type] = deser_func;
    type_to_stable_name[type] = stable_name;
    stable_name_to_type.emplace(stable_name, type);
}
template <>
Serial& serialize(Serial& serial, const std::any& any)
{
    if (!any.has_value()) {
        serial << false;
        return serial;
    }
    std::type_index type = std::type_index(any.type());
    auto ser_it = any_registry::serializers.find(type);
    if (ser_it == any_registry::serializers.end()) {
        serial << false;
        return serial;
    }
    auto name_it = any_registry::type_to_stable_name.find(type);
    if (name_it == any_registry::type_to_stable_name.end()) {
        serial << false;
        return serial;
    }
    serial << true;
    std::string stable_name = name_it->second;
    serial << stable_name;
    return ser_it->second(serial, any);
}

template <>
Serial& deserialize(Serial& serial, std::any& any)
{
    bool readBit = false;
    serial >> readBit;
    if (!readBit)
        return serial;
    std::string stable_name;
    serial >> stable_name;
    auto name_iter = any_registry::stable_name_to_type.find(stable_name);
    if (name_iter == any_registry::stable_name_to_type.end())
        throw std::runtime_error("no stable_name type found by: " + stable_name);
    auto& type = name_iter->second;
    auto deser_it = any_registry::deserializers.find(type);
    if (deser_it == any_registry::deserializers.end())
        throw std::runtime_error("no deserializer found for type: " + stable_name);
    return deser_it->second(serial, any);
    return serial;
}
#define ANY_REGISTRY__REGISTER_TYPE(TYPE, KEY) \
any_registry::register_type( \
    std::type_index(typeid(TYPE)),\
    KEY,\
    [](Serial& serial, const std::any& any) -> Serial& \
    {\
        return (serial << std::any_cast<const TYPE&>(any));\
    },\
    [](Serial& serial, std::any& any) -> Serial& \
    {\
        TYPE value;\
        serial >> value;\
        any = value;\
        return serial;\
    }\
)
bool te = ([](){
    any_registry::register_type(
        std::type_index(typeid(int)),
        "int",
        [](Serial& serial, const std::any& any) -> Serial&
        {
            return serial << std::any_cast<const int&>(any);
        },
        [](Serial& serial, std::any& any) -> Serial&
        {
            int value;
            serial >> value;
            any = value;
            return serial;
        }
    );
    any_registry::register_type(
        std::type_index(typeid(float)),
        "float",
        [](Serial& serial, const std::any& any) -> Serial&
        {
            return serial << std::any_cast<const float&>(any);
        },
        [](Serial& serial, std::any& any) -> Serial&
        {
            float value;
            serial >> value;
            any = value;
            return serial;
        }
    );
    any_registry::register_type(
        std::type_index(typeid(std::string)),
        "std_string",
        [](Serial& serial, const std::any& any) -> Serial&
        {
            return serial << std::any_cast<const std::string&>(any);
        },
        [](Serial& serial, std::any& any) -> Serial&
        {
            std::string value;
            serial >> value;
            any = value;
            return serial;
        }
    ); // Using "string" as stable name
    ANY_REGISTRY__REGISTER_TYPE(uint8_t, "uint8_t");
    ANY_REGISTRY__REGISTER_TYPE( int8_t, "int8_t");
    ANY_REGISTRY__REGISTER_TYPE(uint16_t, "uint16_t");
    ANY_REGISTRY__REGISTER_TYPE( int16_t, "int16_t");
    ANY_REGISTRY__REGISTER_TYPE(uint32_t, "uint32_t");
    // ANY_REGISTRY__REGISTER_TYPE( int32_t, "int32_t");
    ANY_REGISTRY__REGISTER_TYPE(uint64_t, "uint64_t");
    ANY_REGISTRY__REGISTER_TYPE( int64_t, "int64_t");
    // ANY_REGISTRY__REGISTER_TYPE(float, "float");
    ANY_REGISTRY__REGISTER_TYPE(double, "double");
    ANY_REGISTRY__REGISTER_TYPE(long double, "long_double");
    // ANY_REGISTRY__REGISTER_TYPE(std::string, "std_string");
    return true;
})();