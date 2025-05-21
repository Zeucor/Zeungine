#pragma once
#include "WindowComponent.hpp"
#include <zg/Entity.hpp>
namespace zg
{
    namespace components::windows
    {
        WindowComponentCreateInfo EditorFactory();
    }
    namespace entities
    {
        EntityCreateInfo WindowPropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
        EntityCreateInfo RegistryGraphFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
        EntityCreateInfo ScenePropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
        EntityCreateInfo EntityPropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
    }
}