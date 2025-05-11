#pragma once
#include <vector>
#include <map>
namespace zg
{
    struct Entity;
    struct Mesh;
    using TransparentDrawList = std::vector<std::pair<Entity*, Mesh*>>;
    using OpaqueDrawList = std::vector<std::pair<Entity*, Mesh*>>;
}