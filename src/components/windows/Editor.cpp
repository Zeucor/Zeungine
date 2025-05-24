#include <zg/components/windows/Editor.hpp>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/TimingFunction.hpp>
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
            auto plane_2_tuple = vLayout_1.addChild(plane_2_to_be_panel);
            auto plane_2_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(plane_2_tuple);
            auto& plane_2 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(plane_2_tuple);
            plane_2.template make<bool>("Showing", true);
            plane_2.template make<float>("AnimationT", 0.f);
            plane_2.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});
            glm::vec3 plane_3_size(0.0, 1.0, 1);
            auto plane_3_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_3_size));
            auto plane_3_to_be_panel = entities::PlaneFactory(glm::vec4(1, 1, 1, 1), "No Size X Plane", plane_3_position, rotate_identity, plane_3_size);
            vLayout_1.addChild(plane_3_to_be_panel);
            glm::vec3 plane_4_size(1.5, 0.5, 1);
            auto plane_4_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_4_size));
            auto plane_4_to_be_panel = entities::PlaneFactory(glm::vec4(0, 1, 0, 1), "Greeen Plane", plane_4_position, rotate_identity, plane_4_size);
            auto plane_4_tuple = vLayout_1.addChild(plane_4_to_be_panel);
            auto plane_4_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(plane_4_tuple);
            auto& plane_4 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(plane_4_tuple);
            plane_4.template make<bool>("Showing", true);
            plane_4.template make<float>("AnimationT", 0.f);
            plane_4.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});
            auto vLayout_2_size = glm::vec3(0.5, 2, 1);
            auto vLayout_2_position = std::any_cast<glm::vec3>(hLayout.template setData<glm::vec3>("GetSubPosition", vLayout_2_size));
            auto vLayout_2_info = entities::ui::LayoutFactory("VLayout_2", entities::ui::LayoutDimension::Vertical, vLayout_2_position, vLayout_2_size, window.iRenderer, true);
            auto vLayout_2_tuple = hLayout.addChild(vLayout_2_info);
            auto& vLayout_2 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(vLayout_2_tuple);
            glm::vec3 plane_1_size(0.5, 2, 1);
            auto plane_1_position = std::any_cast<glm::vec3>(vLayout_2.template setData<glm::vec3>("GetSubPosition", plane_1_size));
            auto plane_1_to_be_panel = entities::PlaneFactory(glm::vec4(1, 0, 0, 1), "Red Plane", plane_1_position, rotate_identity, plane_1_size);
            auto plane_1_tuple = vLayout_2.addChild(plane_1_to_be_panel);
            auto plane_1_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(plane_1_tuple);
            auto& plane_1 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(plane_1_tuple);
            plane_1.template make<bool>("Showing", true);
            plane_1.template make<float>("AnimationT", 0.f);
            plane_1.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});
            // Setup HView / VView
            //  Add Panels
            //   Iterate Properties
            auto& keyBools = scene.template make<std::shared_ptr<std::array<bool, 5>>>("KeyBools", std::make_shared<std::array<bool, 5>>());
            scene.template make<size_t>("KeyPressID", window.addAnyKeyPressHandler([
                SCENE_INDEX_STACK = scene.INDEX_STACK,
                plane_1_ID,
                start_1_pos = plane_1_position,
                plane_2_ID,
                start_2_pos = plane_2_position,
                plane_4_ID,
                start_4_pos = plane_4_position,
                keyBools
            ](const auto& key, auto pressed) {
                auto key_index = -1;
                switch (key)
                {
                case KEYCODE_CTRL:
                    key_index = 0;
                    break;
                case KEYCODE_SHIFT:
                    key_index = 1;
                    break;
                case '1':
                    key_index = 2;
                    break;
                case '2':
                    key_index = 3;
                    break;
                case '3':
                    key_index = 4;
                    break;
                }
                if (key_index == -1)
                {
                    return;
                }
                auto& key_bools = *keyBools;
                key_bools[key_index] = pressed;
                std::cout << "Key_index: " << key_index << ", pressed: " << pressed << std::endl;
                if (!pressed)
                {
                    return;
                }
                auto& rgy = Registry::GetSingleton();
                auto& window = rgy.getWindow(SCENE_INDEX_STACK);
                bool ctrlShiftPressed = key_bools[0];
                if (!ctrlShiftPressed)
                {
                    return;
                }
                auto& scene = rgy.getScene(SCENE_INDEX_STACK);
                switch (key)
                {
                case '1':
                {
                    auto& plane_2 = rgy.getEntity(plane_2_ID);
                    auto& showing = plane_2.template getData<bool>("Showing");
                    showing = !showing;
                    auto& animationT = plane_2.template getData<float>("AnimationT");
                    auto& animationRunning = plane_2.template getData<std::shared_ptr<bool>>("AnimationRunning");
                    if (animationRunning && *animationRunning)
                    {
                        *animationRunning = false;
                    }
                    animationT = 0.f;
                    glm::vec3 end_2_pos = start_2_pos + glm::vec3(0.0, 0.5, 0);
                    glm::vec3 _start_2_pos = plane_2.position;
                    if (showing)
                    {
                        end_2_pos = start_2_pos;
                    }
                    animationRunning = StartTimingFunction(window, _start_2_pos, end_2_pos, &plane_2.position, animationT, 1, Easing::easeInCubic);
                    break;
                };
                case '2':
                {
                    auto& plane_4 = rgy.getEntity(plane_4_ID);
                    auto& showing = plane_4.template getData<bool>("Showing");
                    showing = !showing;
                    auto& animationT = plane_4.template getData<float>("AnimationT");
                    auto& animationRunning = plane_4.template getData<std::shared_ptr<bool>>("AnimationRunning");
                    if (animationRunning && *animationRunning)
                    {
                        *animationRunning = false;
                    }
                    animationT = 0.f;
                    glm::vec3 end_4_pos = start_4_pos + glm::vec3(0.0, -0.5, 0);
                    glm::vec3 _start_4_pos = plane_4.position;
                    if (showing)
                    {
                        end_4_pos = start_4_pos;
                    }
                    animationRunning = StartTimingFunction(window, _start_4_pos, end_4_pos, &plane_4.position, animationT, 1, Easing::easeInCubic);
                    break;
                };
                case '3':
                {
                    auto& plane_1 = rgy.getEntity(plane_1_ID);
                    auto& showing = plane_1.template getData<bool>("Showing");
                    showing = !showing;
                    auto& animationT = plane_1.template getData<float>("AnimationT");
                    auto& animationRunning = plane_1.template getData<std::shared_ptr<bool>>("AnimationRunning");
                    if (animationRunning && *animationRunning)
                    {
                        *animationRunning = false;
                    }
                    animationT = 0.f;
                    glm::vec3 end_1_pos = start_1_pos + glm::vec3(0.5, 0.0, 0);
                    glm::vec3 _start_1_pos = plane_1.position;
                    if (showing)
                    {
                        end_1_pos = start_1_pos;
                    }
                    animationRunning = StartTimingFunction(window, _start_1_pos, end_1_pos, &plane_1.position, animationT, 1, Easing::easeInCubic);
                    break;
                };
                }
            }));
        },
        .onDetachedFunction = [](auto& scene) {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            window.removeAnyKeyPressHandler(scene.template getData<size_t>("KeyPressID"));
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