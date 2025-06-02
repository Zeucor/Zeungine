#include <zg/components/windows/Editor.hpp>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/TimingFunction.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/Escapes.hpp>
using namespace zg::components::windows;
using namespace zg;
using namespace zg::entities::ui;
SceneCreateInfo EditorSceneFactory(Window& window)
{
    SceneCreateInfo info{
        .name = "Editor Scene",
        .cameraPosition = {(const float&)window.windowWidth / 2.f, (const float&)window.windowHeight / 2.f, 10},
        .cameraDirection = {0, 0, -1},
        .cameraUp = {0, 1, 0},
        .projectionType = vp::Projection::TYPE::Orthographic,
        .orthoSize = {(const float&)window.windowWidth, (const float&)window.windowHeight},
        .onAttachedFunction = [](auto& scene) {
            scene.clearColor = {0, 0, 0, 0};
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            auto& robotoFile = scene.template make<std::shared_ptr<zgfilesystem::File>>("RobotoFile", std::make_shared<zgfilesystem::File>(
                zgfilesystem::File::getProgramDirectoryPath() / "fonts" / "Source Code Pro" / "SourceCodePro-Bold.ttf", // "Roboto" / "Roboto-Regular.ttf",// "Paul" / "Paul.ttf", // 
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

            LayoutBuilder labu(
                scene, // Adding root entity to scene (not an entity, this is an option)
                false // not using NDC (if true will resize textures to Window Res)
            );
            labu.add_layout("HLayout", LayoutDimension::Horizontal, {(const float&)window.windowWidth, (const float&)window.windowHeight, 1});
            labu.add_layout("LeftVLayout", LayoutDimension::Vertical, {(const float&)window.windowWidth * 0.75, (const float&)window.windowHeight, 1});
            labu.add_layout_with(
                "Window Layout",
                LayoutDimension::Horizontal,
                {(const float&)window.windowWidth * 0.5, (const float&)window.windowHeight * 0.25, 1},
                [window_ID = window.ID](auto& labu) {
                    auto& window = Registry::GetSingleton().getWindow(window_ID);
                    labu.add_panel("Window Info Panel", {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight * 0.25, 1}, {0.3, 0.3, 0.3, 0.32})
                    .add_key_press_handler('1', [](auto& window, auto& entity, auto pressed, auto start_position) {
                        if (!pressed)
                            return;
                        auto ctrlPressed = window.mod & 1;
                        if (!ctrlPressed)
                            return;
                        auto boundingSize = entity.getBoundingSize();
                        animatePanel(window, entity.ID, start_position, glm::vec3(0.0, boundingSize.y, 0));
                    });
                    labu.add_layout_with(
                        "Window Panel Layout",
                        LayoutDimension::Vertical,
                        {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight * 0.25, 1},
                        [window_ID = window.ID](auto& labu) {
                            auto& window = Registry::GetSingleton().getWindow(window_ID);
                            labu
                            .add_text(Escapes::Color({1, 1, 1}) + Escapes::SmallerFont + "Window:" + Escapes::ResetAttr + " " + Escapes::Color({0.937, 1.00, 0.0500}) + *window.title, 20.f)
                            .add_text(Escapes::Color({1, 1, 1}) + Escapes::SmallerFont + "W:" + Escapes::ResetAttr + " " + Escapes::Color({0.670, 0.703, 1.00}) + std::to_string(uint32_t((const float&)window.windowWidth)) + Escapes::Color({1, 1, 1}) + ", H: " + Escapes::Color({0.670, 0.703, 1.00}) + std::to_string(uint32_t((const float&)window.windowHeight)), 14.f)
                            .add_text(Escapes::Color({1, 1, 1}) + Escapes::SmallerFont + "Borderless:" + Escapes::ResetAttr + " " + Escapes::Color({0, 1, 0}) + std::to_string(window.borderless), 14.f)
                            .add_text(Escapes::Color({1, 1, 1}) + Escapes::SmallerFont + "Target FPS:" + Escapes::ResetAttr + " " + Escapes::Color({0, 1, 0}) + std::to_string(*window.framerate), 14.f)
                            // fps (formatted color)
                            .add_observed_T_formatted([window_ID = window.ID](auto& fps) {
                                auto& window = Registry::GetSingleton().getWindow(window_ID);
                                std::string string;
                                auto& window_framerate = *window.framerate;
                                if (fps <= (window_framerate / 2.f))
                                    string += Escapes::FG_Red;
                                else if (fps < (window_framerate))
                                    string += Escapes::FG_Yellow;
                                else if (fps >= (window_framerate * 2.0))
                                    string += Escapes::FG_BrightGreen;
                                else
                                    string += Escapes::FG_Green;
                                {
                                    std::ostringstream stream;
                                    stream.setf(std::ios::fixed, std::ios::floatfield);
                                    stream << std::setprecision(2);
                                    stream << fps;
                                    string += stream.str();
                                }
                                return (Escapes::SmallerFont + Escapes::Color({1, 1, 1}) + "FPS: " + Escapes::ResetAttr + string);
                            }, window.fps, 14.f)
                            // draw count
                            .add_layout("Draw Count Layout", LayoutDimension::Horizontal, {(const float&)window.windowWidth * 0.25, 0, 1}, textures::BlendState::Layout, true)
                            .add_text(Escapes::SmallerFont + Escapes::Color({1, 1, 1}) + "Draw Count: ", 14.f)
                            .add_observed_T(window.drawCount, 14.f)
                            .fit_layout_to_children()
                            .move_up_stack()
                            // triangle count
                            .add_observed_T_formatted([window_ID = window.ID](auto& count) {
                                auto& window = Registry::GetSingleton().getWindow(window_ID);
                                std::string string;
                                string += Escapes::SmallerFont +
                                    Escapes::Color({1, 1, 1}) +
                                    "Triangle Count: " +
                                    Escapes::ResetAttr +
                                    Escapes::Color({0.750, 0.771, 1.00}) +
                                    std::to_string(count);
                                return string;
                            }, window.triangleCount, 14.f);
                        },
                        textures::BlendState::Text);
                    labu.move_up_stack(); // above window info panel
    
                    labu.add_panel("Window Scenes Info Panel", {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight * 0.25, 1}, {0.7, 0.3, 0.3, 0.32})
                        .add_key_press_handler('1', [](auto& window, auto& entity, auto pressed, auto start_position) {
                            if (!pressed)
                                return;
                            auto ctrlPressed = window.mod & 1;
                            if (!ctrlPressed)
                                return;
                            auto boundingSize = entity.getBoundingSize();
                            animatePanel(window, entity.ID, start_position, glm::vec3(0.0, boundingSize.y, 0));
                        });
    
                    labu.add_layout_with(
                        "Window Scenes Panel Layout",
                        LayoutDimension::Vertical,
                        {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight * 0.25, 1},
                        [window_ID = window.ID](auto& labu) {
                            auto& rgy = Registry::GetSingleton();
                            auto& window = rgy.getWindow(window_ID);
                            labu.add_text("Scenes", 20.f);
                            for (auto& scene : window.scenes)
                            {
                                labu.add_layout_with(
                                    "Scene " + scene.name + " Layout",
                                    LayoutDimension::Vertical, {0, 0, 1},
                                    [scene_ID = scene.ID](auto& labu) {
                                        auto& scene = Registry::GetSingleton().getScene(scene_ID);
                                        labu.add_text(Escapes::SmallerFont + std::to_string(scene.ID) + ": " + Escapes::ResetAttr + scene.name, 14.f);
                                        labu.add_layout_with(
                                            "Scene " + scene.name + " Layout EC",
                                            LayoutDimension::Horizontal, {0, 0, 1},
                                            [entitiesCount = scene.entitiesCount](auto& labu) mutable {
                                                labu.add_text(" " + Escapes::SmallerFont + "Entity Count: ", 16.f);
                                                labu.add_observed_T(entitiesCount, 16.f);
                                            },
                                            textures::BlendState::Text, true);
                                        labu.add_layout_with(
                                            "Scene " + scene.name + " Layout cEID",
                                            LayoutDimension::Horizontal, {0, 0, 1},
                                            [currentClickedEntityID = scene.currentClickedEntityID](auto& labu) mutable {
                                                labu.add_text(" " + Escapes::SmallerFont + "cEntity ID: ", 16.f);
                                                labu.add_observed_T(currentClickedEntityID, 16.f);
                                            },
                                            textures::BlendState::Text, true);
                                        labu.add_layout_with("Scene " + scene.name + " Layout hEID",
                                            LayoutDimension::Horizontal, {0, 0, 1},
                                            [currentHoveredEntityID = scene.currentHoveredEntityID](auto& labu) mutable {
                                                labu.add_text(" " + Escapes::SmallerFont + "hEntity ID: ", 16.f);
                                                labu.add_observed_T(currentHoveredEntityID, 16.f);
                                            },
                                            textures::BlendState::Text, true);
                                    },
                                    textures::BlendState::Text, true);
                            }
                        });
                    labu.move_up_stack(); // above scenes info panel
                }
            );
            
            labu.add_layout("Spacer Layout", LayoutDimension::Vertical, {1, (const float&)window.windowHeight * 0.5, 1})
                .move_up_stack() // above spacer
                .add_layout("Scene Layout", LayoutDimension::Horizontal, {(const float&)window.windowWidth * 0.75, (const float&)window.windowHeight * 0.25, 1})
                .add_panel("Scene Panel", {(const float&)window.windowWidth * 0.75, (const float&)window.windowHeight * 0.25, 1}, {0.3, 0.3, 0.3, 0.32})
                .add_key_press_handler('2', [](auto& window, auto& entity, auto pressed, auto start_position) {
                    if (!pressed)
                        return;
                    auto ctrlPressed = window.mod & 1;
                    if (!ctrlPressed)
                        return;
                    auto boundingSize = entity.getBoundingSize();
                    animatePanel(window, entity.ID, start_position, glm::vec3(0.0, -boundingSize.y, 0));
                })
                .add_layout("Scene Panel Layout", LayoutDimension::Vertical, {(const float&)window.windowWidth * 0.75, (const float&)window.windowHeight * 0.25, 1}, textures::BlendState::Text);
            labu.add_text(Escapes::SmallerFont + "Scene: " + Escapes::ResetAttr + Escapes::Color({0.360, 0.413, 1.00}) + window.scenes.begin()->name, 24.f);
            labu.move_up_stack() // above scene panel layout
                .move_up_stack() // above scene panel
                .move_up_stack() // above scene layout
                .move_up_stack() // next root horizontal
                .add_layout("Entity Layout", LayoutDimension::Vertical, {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight, 1})
                .add_panel("Entity Panel", {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight, 1}, {0.3, 0.3, 0.3, 0.32})
                .add_key_press_handler('3', [](auto& window, auto& entity, auto pressed, auto start_position) {
                    if (!pressed)
                        return;
                    auto ctrlPressed = window.mod & 1;
                    if (!ctrlPressed)
                        return;
                    auto boundingSize = entity.getBoundingSize();
                    animatePanel(window, entity.ID, start_position, glm::vec3(boundingSize.x, 0.0, 0));
                })
                .add_layout("Entity Panel Layout", LayoutDimension::Vertical, {(const float&)window.windowWidth * 0.25, (const float&)window.windowHeight, 1}, textures::BlendState::Text)
                .add_text(Escapes::SmallerFont + "Entity Graph", 20.f);
        },
        .onDetachedFunction = [](auto& scene) {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            // window.removeAnyKeyPressHandler(scene.template getData<size_t>("KeyPressID"));
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
            auto editor_scene_tuple = window.addScene(EditorSceneFactory(window));
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