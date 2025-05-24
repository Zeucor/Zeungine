#include <zg/components/windows/Editor.hpp>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/TimingFunction.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
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

            auto& robotoFile = scene.template make<std::shared_ptr<zgfilesystem::File>>("RobotoFile", std::make_shared<zgfilesystem::File>(
                zgfilesystem::File::getProgramDirectoryPath() / "fonts" / "Roboto" / "Roboto-Regular.ttf",
                enums::EFileLocation::Absolute,
                "r"
            ));
            auto& robotoFont = scene.template make<std::shared_ptr<fonts::freetype::FreetypeFont>>("RobotoFont", std::make_shared<fonts::freetype::FreetypeFont>(
                window.iRenderer,
                *robotoFile
            ));

            // main H Layout
            auto hLayout_size = glm::vec3(2, 2, 1);
            auto hLayout_info = entities::ui::LayoutFactory("HLayout", entities::ui::LayoutDimension::Horizontal, {0, 0, 0}, hLayout_size, window.iRenderer, true);
            auto hLayout_tuple = scene.addEntity(hLayout_info);
            auto& hLayout = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(hLayout_tuple);

            // left V Layout
            auto vLayout_1_size = glm::vec3(1.5, 2, 1);
            auto vLayout_1_position = std::any_cast<glm::vec3>(hLayout.template setData<glm::vec3>("GetSubPosition", vLayout_1_size));
            auto vLayout_1_info = entities::ui::LayoutFactory("VLayout_1", entities::ui::LayoutDimension::Vertical, vLayout_1_position, vLayout_1_size, window.iRenderer, true);
            auto vLayout_1_tuple = hLayout.addChild(vLayout_1_info);
            auto& vLayout_1 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(vLayout_1_tuple);

            // top "window" panel
            glm::vec3 window_panel_size(0.5, 0.5, 1);
            auto window_panel_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", window_panel_size));
            auto window_panel_to_be_panel = entities::PlaneFactory(glm::vec4(0.3, 0.3, 0.3, 0.88), "Window Panel", window_panel_position, rotate_identity, window_panel_size);
            auto window_panel_tuple = vLayout_1.addChild(window_panel_to_be_panel);
            auto window_panel_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(window_panel_tuple);
            auto& window_panel = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_panel_tuple);
            window_panel.template make<bool>("Showing", true);
            window_panel.template make<float>("AnimationT", 0.f);
            window_panel.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});
            
            auto& titleGlyphIDs = window_panel.template make<std::vector<size_t>>("TitleGlyphIDs");
            auto& titleCursorIndex = window_panel.template make<int64_t>("TitleCursorIndex");
            auto& titleCursorID = window_panel.template make<size_t>("TitleCursorID");
            auto& robotoFontRef = *robotoFont;
            std::string titleString("Window: ");
            titleString += window.title;
            auto& titleFontSize = window_panel.template make<float>("TitleFontSize", 42.f);
            auto& titleLineHeight = window_panel.template make<float>("TitleLineHeight", 0.0f);
            auto titleSize = robotoFontRef.stringSize(titleString, titleFontSize, titleLineHeight, {0, 0});
            glm::vec3 titleScale(2.f / window.windowWidth / 2.f, 2.f / window.windowHeight / 2.f, 1);
            auto scaledSize = titleSize * glm::vec2(titleScale);
            robotoFontRef.stringToEntity(titleString, glm::vec3{-window_panel_size.x, (window_panel_size.y / 2.f) - scaledSize.y, 0.1}, {1,1,1,1}, rotate_identity,
										titleScale, titleFontSize, titleLineHeight, scaledSize,
												enums::EBreakStyle::None, scene, window_panel,
												titleGlyphIDs, titleCursorIndex,
												titleCursorID);

            // middle "spacer" panel
            glm::vec3 plane_3_size(0.0, 1.0, 1);
            auto plane_3_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_3_size));
            auto plane_3_to_be_panel = entities::PlaneFactory(glm::vec4(1, 1, 1, 1), "Spacer Panel", plane_3_position, rotate_identity, plane_3_size);
            vLayout_1.addChild(plane_3_to_be_panel);

            // bottom "scene" panel
            glm::vec3 plane_4_size(1.5, 0.5, 1);
            auto plane_4_position = std::any_cast<glm::vec3>(vLayout_1.template setData<glm::vec3>("GetSubPosition", plane_4_size));
            auto plane_4_to_be_panel = entities::PlaneFactory(glm::vec4(0.3, 0.3, 0.3, 0.88), "Scene Panel", plane_4_position, rotate_identity, plane_4_size);
            auto plane_4_tuple = vLayout_1.addChild(plane_4_to_be_panel);
            auto plane_4_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(plane_4_tuple);
            auto& plane_4 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(plane_4_tuple);
            plane_4.template make<bool>("Showing", true);
            plane_4.template make<float>("AnimationT", 0.f);
            plane_4.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});

            // right V Layout
            auto vLayout_2_size = glm::vec3(0.5, 2, 1);
            auto vLayout_2_position = std::any_cast<glm::vec3>(hLayout.template setData<glm::vec3>("GetSubPosition", vLayout_2_size));
            auto vLayout_2_info = entities::ui::LayoutFactory("VLayout_2", entities::ui::LayoutDimension::Vertical, vLayout_2_position, vLayout_2_size, window.iRenderer, true);
            auto vLayout_2_tuple = hLayout.addChild(vLayout_2_info);
            auto& vLayout_2 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(vLayout_2_tuple);

            // right "entity" panel
            glm::vec3 plane_1_size(0.5, 2, 1);
            auto plane_1_position = std::any_cast<glm::vec3>(vLayout_2.template setData<glm::vec3>("GetSubPosition", plane_1_size));
            auto plane_1_to_be_panel = entities::PlaneFactory(glm::vec4(0.3, 0.3, 0.3, 0.88), "Entity Panel", plane_1_position, rotate_identity, plane_1_size);
            auto plane_1_tuple = vLayout_2.addChild(plane_1_to_be_panel);
            auto plane_1_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(plane_1_tuple);
            auto& plane_1 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(plane_1_tuple);
            plane_1.template make<bool>("Showing", true);
            plane_1.template make<float>("AnimationT", 0.f);
            plane_1.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});

            // Setup HView / VView  ✅
            //  Add Panels          ✅
            //   Iterate Properties ❎ at what point ? window props, scene props, selected entity & component props

            scene.template make<size_t>("KeyPressID", window.addAnyKeyPressHandler([
                SCENE_INDEX_STACK = scene.INDEX_STACK,
                plane_1_ID,
                start_1_pos = plane_1_position,
                window_panel_ID,
                start_2_pos = window_panel_position,
                plane_4_ID,
                start_4_pos = plane_4_position
            ](const auto& key, auto pressed) {
                if (!pressed)
                {
                    return;
                }
                auto& rgy = Registry::GetSingleton();
                auto& window = rgy.getWindow(SCENE_INDEX_STACK);
                bool ctrlPressed = window.mod & 1;
                if (!ctrlPressed)
                {
                    return;
                }
                auto& scene = rgy.getScene(SCENE_INDEX_STACK);
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
                switch (key)
                {
                case '1':
                {
                    animatePanel(window, window_panel_ID, start_2_pos, glm::vec3(0.0, 0.5, 0));
                    break;
                };
                case '2':
                {
                    animatePanel(window, plane_4_ID, start_4_pos, glm::vec3(0.0, -0.5, 0));
                    break;
                };
                case '3':
                {
                    animatePanel(window, plane_1_ID, start_1_pos, glm::vec3(0.5, 0.0, 0));
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