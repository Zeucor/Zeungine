#include <zg/components/windows/Editor.hpp>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/TimingFunction.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
using namespace zg::components::windows;
using namespace zg;
using namespace zg::entities::ui;
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

            auto& robotoFile = scene.template make<std::shared_ptr<zgfilesystem::File>>("RobotoFile", std::make_shared<zgfilesystem::File>(
                zgfilesystem::File::getProgramDirectoryPath() / "fonts" / "Roboto" / "Roboto-Regular.ttf",
                enums::EFileLocation::Absolute,
                "r"
            ));
            auto& robotoFont = scene.template make<std::shared_ptr<fonts::freetype::FreetypeFont>>("Font", std::make_shared<fonts::freetype::FreetypeFont>(
                window.iRenderer,
                *robotoFile
            ));

            static auto animatePanel = [](auto& window, auto ID, auto start_pos, auto start_end_diff){
                auto& rgy = Registry::GetSingleton();
                auto& panel = rgy.getEntity(ID);
                auto& showing = panel.template getData<bool>("Showing");
                showing = !showing;
                auto& animationT = panel.template getData<float>("AnimationT");
                auto& animationRunning = panel.template getData<std::shared_ptr<bool>>("AnimationRunning");
                if (animationRunning && *animationRunning)
                {
                    *animationRunning = false;
                }
                animationT = 0.f;
                glm::vec3 end_pos = start_pos + start_end_diff;
                glm::vec3 _start_pos = panel.position;
                if (showing)
                {
                    end_pos = start_pos;
                }
                animationRunning = StartTimingFunction(window, _start_pos, end_pos, &panel.position, animationT, 0.7, Easing::easeInCubic);
            };

            LayoutBuilder labu(scene);
            labu.add_layout("HLayout", LayoutDimension::Horizontal, {2, 2, 1})
                .add_layout("Window Layout", LayoutDimension::Vertical, {1.5, 2, 1})
                .add_panel("Window Panel", {1.0, 0.5, 1}, {0.3, 0.3, 0.3, 0.32})
                .add_key_press_handler('1', [](auto& window, auto& entity, auto pressed, auto start_position) {
                    if (!pressed)
                        return;
                    auto ctrlPressed = window.mod & 1;
                    if (!ctrlPressed)
                        return;
                    animatePanel(window, entity.ID, start_position, glm::vec3(0.0, 0.5, 0));
                })
                .add_layout("Window Panel Layout", LayoutDimension::Vertical, {1.0, 0.5, 1}, textures::BlendState::Text)
                .add_text("Window: " + window.title, 40.f)
                .add_text("W: " + std::to_string(uint32_t(window.windowWidth)) + ", H: " + std::to_string(uint32_t(window.windowHeight)), 28.f)
                .add_text("Borderless: " + std::to_string(window.borderless), 28.f)
                .add_text("Target FPS: " + std::to_string(window.framerate), 28.f)
                .move_up_stack() // above panel layout
                .move_up_stack() // above panel
                .add_layout("Spacer Layout", LayoutDimension::Vertical, {0.01, 1.0, 1})
                .move_up_stack() // above spacer
                .add_layout("Scene Layout", LayoutDimension::Horizontal, {1.5, 0.5, 1})
                .add_panel("Scene Panel", {1.5, 0.5, 1}, {0.3, 0.3, 0.3, 0.32})
                .add_key_press_handler('2', [](auto& window, auto& entity, auto pressed, auto start_position) {
                    if (!pressed)
                        return;
                    auto ctrlPressed = window.mod & 1;
                    if (!ctrlPressed)
                        return;
                    animatePanel(window, entity.ID, start_position, glm::vec3(0.0, -0.5, 0));
                })
                .add_layout("Scene Panel Layout", LayoutDimension::Vertical, {1.5, 0.5, 1}, textures::BlendState::Text)
                .add_text("Scene Graph", 40.f)
                .move_up_stack() // above scene panel layout
                .move_up_stack() // above scene panel
                .move_up_stack() // above scene layout
                .move_up_stack() // next root horizontal
                .add_layout("Entity Layout", LayoutDimension::Vertical, {0.5, 2, 1})
                .add_panel("Entity Panel", {0.5, 2, 1}, {0.3, 0.3, 0.3, 0.32})
                .add_key_press_handler('3', [](auto& window, auto& entity, auto pressed, auto start_position) {
                    if (!pressed)
                        return;
                    auto ctrlPressed = window.mod & 1;
                    if (!ctrlPressed)
                        return;
                    animatePanel(window, entity.ID, start_position, glm::vec3(0.5, 0.0, 0));
                })
                .add_layout("Entity Panel Layout", LayoutDimension::Vertical, {0.5, 2, 1}, textures::BlendState::Text)
                .add_text("Entity Graph", 40.f);

            // Setup HView / VView  ✅
            //  Add Panels          ✅
            //   Iterate Properties ❎ at what point ? window props, scene props, selected entity & component props

            // scene.template make<size_t>("KeyPressID", window.addAnyKeyPressHandler([
            //     SCENE_INDEX_STACK = scene.INDEX_STACK,
            //     plane_1_ID,
            //     start_1_pos = plane_1_position,
            //     window_panel_ID,
            //     start_2_pos = window_panel_position,
            //     plane_4_ID,
            //     start_4_pos = plane_4_position
            // ](const auto& key, auto pressed) {
            //     if (!pressed)
            //     {
            //         return;
            //     }
            //     auto& rgy = Registry::GetSingleton();
            //     auto& window = rgy.getWindow(SCENE_INDEX_STACK);
            //     bool ctrlPressed = window.mod & 1;
            //     if (!ctrlPressed)
            //     {
            //         return;
            //     }
            //     auto& scene = rgy.getScene(SCENE_INDEX_STACK);
            //     switch (key)
            //     {
            //     case '1':
            //     {
            //         animatePanel(window, window_panel_ID, start_2_pos, glm::vec3(0.0, 0.5, 0));
            //         break;
            //     };
            //     case '2':
            //     {
            //         animatePanel(window, plane_4_ID, start_4_pos, glm::vec3(0.0, -0.5, 0));
            //         break;
            //     };
            //     case '3':
            //     {
            //         animatePanel(window, plane_1_ID, start_1_pos, glm::vec3(0.5, 0.0, 0));
            //         break;
            //     };
            //     }
            // }));
        },
        .onDetachedFunction = [](auto& scene) {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            window.removeAnyKeyPressHandler(scene.template getData<size_t>("KeyPressID"));
        },
        .blendState = textures::BlendState::Layout
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