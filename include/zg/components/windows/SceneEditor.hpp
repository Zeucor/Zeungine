#pragma once
#include "WindowComponent.hpp"
namespace zg::components::windows
{
    WindowComponentCreateInfo WindowEditorFactory();
    WindowComponentCreateInfo WindowPropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
    WindowComponentCreateInfo RegistryGraphFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
    WindowComponentCreateInfo ScenePropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
    WindowComponentCreateInfo EntityPropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale);
}