#include <zg/components/windows/Editor.hpp>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
using namespace zg::components::windows;
using namespace zg;
SceneCreateInfo EditorSceneFactory()
{
    SceneCreateInfo info{
        .name = "Editor Scene",
        .cameraPosition = {0, 0, 10},
        .cameraDirection = {0, 0, -1},
        .cameraUp = {0, 1, 0},
        .projectionType = vp::Projection::TYPE::Orthographic,
        .orthoSize = {2, 2},
        .onAttachedFunction = [](auto& scene) {
            scene.clearColor = {0, 0, 0, 0};
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto hLayout_size = glm::vec3(2, 2, 1);
            auto hLayout_info = entities::ui::LayoutFactory("HLayout", entities::ui::LayoutDimension::Horizontal, {0, 0, 0}, hLayout_size, window.iRenderer, true);
            auto hLayout_tuple = scene.addEntity(hLayout_info);
            auto& hLayout = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(hLayout_tuple);
            auto vLayout_1_size = glm::vec3(1.5, 2, 1);
            auto vLayout_1_position = std::any_cast<glm::vec3>(hLayout.template setData<glm::vec3>("GetSubPosition", vLayout_1_size));
            auto vLayout_1_info = entities::ui::LayoutFactory("VLayout_1", entities::ui::LayoutDimension::Vertical, vLayout_1_position, vLayout_1_size, window.iRenderer, true);
            auto vLayout_1_tuple = hLayout.addChild(vLayout_1_info);
            auto& vLayout_1 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(vLayout_1_tuple);
            glm::vec3 plane_2_size(0.5, 0.5, 1);
            auto plane_2_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_2_size));
            auto plane_2_to_be_panel = entities::PlaneFactory(glm::vec4(0, 0, 1, 1), "Blue Plane", plane_2_position, rotate_identity, plane_2_size);
            vLayout_1.addChild(plane_2_to_be_panel);
            glm::vec3 plane_3_size(0.0, 1.0, 1);
            auto plane_3_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_3_size));
            auto plane_3_to_be_panel = entities::PlaneFactory(glm::vec4(1, 1, 1, 1), "No Size X Plane", plane_3_position, rotate_identity, plane_3_size);
            vLayout_1.addChild(plane_3_to_be_panel);
            glm::vec3 plane_4_size(1.5, 0.5, 1);
            auto plane_4_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_4_size));
            auto plane_4_to_be_panel = entities::PlaneFactory(glm::vec4(1, 1, 1, 1), "Greeen Plane", plane_4_position, rotate_identity, plane_4_size);
            vLayout_1.addChild(plane_4_to_be_panel);
            auto vLayout_2_size = glm::vec3(0.5, 2, 1);
            auto vLayout_2_position = std::any_cast<glm::vec3>(hLayout.template setData<glm::vec3>("GetSubPosition", vLayout_2_size));
            auto vLayout_2_info = entities::ui::LayoutFactory("VLayout_2", entities::ui::LayoutDimension::Vertical, vLayout_2_position, vLayout_2_size, window.iRenderer, true);
            auto vLayout_2_tuple = hLayout.addChild(vLayout_2_info);
            auto& vLayout_2 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(vLayout_2_tuple);
            glm::vec3 plane_1_size(0.5, 2, 1);
            auto plane_1_position = std::any_cast<glm::vec3>(vLayout_2.template setData<glm::vec3>("GetSubPosition", plane_1_size));
            auto plane_1_to_be_panel = entities::PlaneFactory(glm::vec4(1, 0, 0, 1), "Red Plane", plane_1_position, rotate_identity, plane_1_size);
            vLayout_2.addChild(plane_1_to_be_panel);
            // Setup HView / VView
            //  Add Panels
            //   Iterate Properties
        },
        .onDetachedFunction = [](auto& scene) {

        }
    };
    return info;
}
WindowComponentCreateInfo zg::components::windows::EditorFactory()
{
    WindowComponentCreateInfo info{
        .name = "Editor",
        .onAttachedFunction = [](auto& component) {
            auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
            auto editor_scene_tuple = window.addScene(EditorSceneFactory());
            auto& editor_scene = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(editor_scene_tuple);
            editor_scene.drawMode = SceneDrawMode::Single;
            // editor_scene.z += 5.f;
            component.template make<size_t>("EditorSceneID", std::get<KEY_ID_VECTOR_ID_INDEX>(editor_scene_tuple));
        },
        .onDetachedFunction = [](auto& component) {
            auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
            window.removeScene(component.template getData<size_t>("EditorSceneID"));
        }
    };
    return info;
}
EntityCreateInfo zg::entities::WindowPropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale)
{
    EntityCreateInfo info{};
    return info;
}
EntityCreateInfo zg::entities::RegistryGraphFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale)
{
    EntityCreateInfo info{};
    return info;
}
EntityCreateInfo zg::entities::ScenePropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale)
{
    EntityCreateInfo info{};
    return info;
}
EntityCreateInfo zg::entities::EntityPropertiesFactory(glm::vec3 position, glm::quat roation, glm::vec3 scale)
{
    EntityCreateInfo info{};
    return info;
}