#include <zg/Entity.hpp>
zg::Entity::SerializeMap* serializeMapPointer = 0;
zg::Entity::DeserializeMap* deserializeMapPointer = 0;
zg::Entity::SerializeMap& getSerializeMap()
{
    if (!serializeMapPointer)
    {
        serializeMapPointer = new zg::Entity::SerializeMap();
    }
    return *serializeMapPointer;
}
zg::Entity::DeserializeMap& getDeserializeMap()
{
    if (!deserializeMapPointer)
    {
        deserializeMapPointer = new zg::Entity::DeserializeMap();
    }
    return *deserializeMapPointer;
}
void zg::Entity::registerSerialize(const std::string& typeName, const SerializeFunction& function)
{
    getSerializeMap()[typeName] = function;
}
void zg::Entity::registerDeserialize(const std::string& typeName, const DeserializeFunction& function)
{
    getDeserializeMap()[typeName] = function;
}
zg::Entity::SerializeFunction zg::Entity::getSerialize(const std::string& typeName)
{
    auto& serializeMap = getSerializeMap();
    auto iter = serializeMap.find(typeName);
    if (iter == serializeMap.end())
        throw std::runtime_error("TypeName: " + typeName + " serialize function not found");
    return iter->second;
}
zg::Entity::DeserializeFunction zg::Entity::getDeserialize(const std::string& typeName)
{
    auto& deserializeMap = getDeserializeMap();
    auto iter = deserializeMap.find(typeName);
    if (iter == deserializeMap.end())
        throw std::runtime_error("TypeName: " + typeName + " deserialize function not found");
    return iter->second;
}
void zg::Entity::cleanupSerialize()
{
    if (serializeMapPointer)
        delete serializeMapPointer;
    if (deserializeMapPointer)
        delete deserializeMapPointer;
}
#include <zg/entities/Plane.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/entities/Cube.hpp>
#define REGISTER_ENTITY(TYPE) \
zg::Entity::registerSerialize(EntityTypeID<TYPE>::id, [](auto& serial, auto& pointer) -> Serial& {\
    return serialize(serial, std::dynamic_pointer_cast<TYPE>(pointer));\
});\
zg::Entity::registerDeserialize(EntityTypeID<TYPE>::id, [](auto& serial, auto& pointer) -> Serial& {\
    auto dynamic_pointer = std::dynamic_pointer_cast<TYPE>(pointer);\
    deserialize(serial, dynamic_pointer);\
    pointer = std::dynamic_pointer_cast<zg::Entity>(dynamic_pointer);\
    return serial;\
})
auto registered = ([]()->bool{
    // REGISTER_ENTITY(zg::entities::Plane);
    // REGISTER_ENTITY(zg::entities::NDParametricCurve<3, float>);
    // REGISTER_ENTITY(zg::entities::Cube);
    return true;
})();