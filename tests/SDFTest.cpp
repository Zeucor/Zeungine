#include <zg/Window.hpp>
#include <zg/Registry.hpp>
#include <zg/crypto/Random.hpp>
#include <zg/entities/SDF.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/entities/sdf_mesh.hpp>
#include <zg/Spot.hpp>
#include <zg/components/windows/Editor.hpp>
using namespace zg;
using namespace zg::shaders;
SceneCreateInfo SDFSceneFactory();
RuntimeConstants commonShaderConstants({
    "Lighting", "DirectionalLightShadowMaps", "LightSpacePosition"
});
int main()
{
    Registry registry;
    // we are using SDFs so we need to declare a Registry
	ShaderFactory shader_factory;
    register_zg_shader_hooks();
    SDFRegistry sdf_rgy;
    register_zg_sdfs();
    WindowCreateInfo windowInfo{
        .title = "SDF Test",
        .windowWidth = 1366,
        .windowHeight = 768,
        .windowX = (1920 / 2) - (1366 / 2),
        .windowY = (1080 / 2) - (768 / 2),
        .borderless = false,
        .vsync = false,
        .framerate = 60
    };
    auto window_tuple = Registry::GetSingleton().addWindow(windowInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
    window.runOnThread([](auto& window){
        window.addScene(SDFSceneFactory());
        window.attachComponent(components::windows::EditorFactory());
    });
    window.registerHandler(EVENT_KEY_PRESS, [&](auto& event) {
        if (event.getValue() != 'q')
            return;
        auto& pressed = event.template castData<bool>();
        if (!pressed)
            return;
        window.close();
    });
    window.run();
}
SceneCreateInfo SDFSceneFactory()
{
    SceneCreateInfo info{
        .name = "SDF Scene",
        .cameraPosition = {10, 10, 10},
        .cameraDirection = glm::normalize(glm::vec3(-0.597868, -0.552368, -0.580898)),
        .fov = 81,
        .onAttachedFunction = [](auto& scene){
            scene.clearColor = {0.4, 0.3, 0.5, 1.0};
			glm::vec3 dldirection{0, -1, 0};
			dldirection = glm::normalize(dldirection);
			glm::vec3 dlup{0, 0, 1};
			scene.directionalLights.push_back({
				glm::vec3(0, 80, 00), // position
				dldirection, // direction
				dlup, // up
				glm::vec3(1.f, 1.f, 1.f), // color
				1.f, // intensity,
				1.f, // nearcube5
				364.f, // farcube5
				0.4f // ambientFactor
			});
			scene.directionalLightShadows.emplace_back(scene.INDEX_STACK, 0);
            Spot spot({1, 1, 1});
            auto color = []() -> glm::vec4 {
                return {
                    zg::crypto::Random::value<float>(0.1f, 1.0f),
                    zg::crypto::Random::value<float>(0.1f, 1.0f),
                    zg::crypto::Random::value<float>(0.1f, 1.0f),
                    1.f
                };
            };
            auto& sdf_rgy = SDFRegistry::GetSingleton();
            auto sdf_iter = sdf_rgy.begin();
            auto sdf_end = sdf_rgy.end();
            auto count = 0;
            for (;sdf_iter != sdf_end && count < 3; ++sdf_iter)
            {
                ++count;
                auto& key = sdf_iter->first;
                auto sdf_mesh = entities::sdf_mesh_factory(key, key, spot(12), rotate_identity, {6,6,6}, color(), commonShaderConstants);
                scene.addEntity(sdf_mesh);
            }
            scene.attachComponent(components::scenes::ViewMouseControlFactory());
            scene.attachComponent(components::scenes::ViewQuadKeyControlFactory(components::scenes::KeyScheme::WSADSC, 8));
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            scene.template make<size_t>("KeyID",
                window.registerHandler(EVENT_KEY_PRESS, [SCENE_INDEX_STACK = scene.INDEX_STACK](auto& event) {
                    if (event.getValue() != 'r')
                        return;
                    auto& pressed = event.template castData<bool>();
                    if (!pressed)
                        return;
                    auto& scene = Registry::GetSingleton().getScene(SCENE_INDEX_STACK);
                    auto& view = *scene.viewPointer;
                    view.direction = glm::normalize(glm::vec3(-1, -1, -1));
                    view.setDirty();
                })
            );
        },
        .onDetachedFunction = [](auto& scene){
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            window.deregisterHandler(EVENT_KEY_PRESS, scene.template getData<size_t>("KeyID"));
        }
    };
    return info;
}