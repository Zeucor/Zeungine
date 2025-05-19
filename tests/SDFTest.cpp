#include <zg/Window.hpp>
#include <zg/Registry.hpp>
#include <zg/entities/SDF.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/entities/sdf_mesh.hpp>
#include <zg/Spot.hpp>
using namespace zg;
using namespace zg::shaders;
SceneCreateInfo SDFSceneFactory();
RuntimeConstants commonShaderConstants({
    "Lighting", "DirectionalLightShadowMaps", "LightSpacePosition"
});
int main()
{
    // Registry rgy
    // we are using SDFs so we need to declare a Registry
	ShaderFactory shader_factory;
    register_zg_shader_hooks();
    SDFRegistry sdf_rgy;
    register_zg_sdfs();
    WindowCreateInfo windowInfo{
        .title = "SDF Test",
        .windowWidth = 1920,
        .windowHeight = 1080,
        .windowX = 0,
        .windowY = 0,
        .borderless = true,
        .vsync = false,
        .framerate = 144 / 2
    };
    auto window_tuple = Registry::addWindow(windowInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
    window.runOnThread([](auto& window){
        window.addScene(SDFSceneFactory());
    });
    window.addKeyPressHandler('q', [&](auto pressed){
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
			// auto& dl = scene.directionalLights[0];
			scene.directionalLightShadows.emplace_back(scene.INDEX_STACK, 0);
            // auto cubeInfo = entities::SDFFactory("Cube", glm::vec4(0.2, 0.1, 0.95, 1.0), "Cube", {14, 0, 0}, rotate_identity, {1, 1, 1}, commonShaderConstants);
            // scene.addEntity(cubeInfo);
            // auto torusInfo = entities::SDFFactory("Torus", glm::vec4(0.2, 0.1, 0.95, 1.0), "Torus", {18, 0, 0}, rotate_identity, {1, 1, 1}, commonShaderConstants);
            // scene.addEntity(torusInfo);
            // auto cylinderInfo = entities::SDFFactory("Cylinder", glm::vec4(0.2, 0.1, 0.95, 1.0), "Cylinder", {7, 0, 7}, rotate_identity, {1, 1, 1}, commonShaderConstants);
            // scene.addEntity(cylinderInfo);
            // // auto coneInfo = entities::SDFFactory("Cone", glm::vec4(0.2, 0.1, 0.95, 1.0), "Cone", {14, 0, 7}, rotate_identity, {1, 1, 1}, commonShaderConstants);
            // // scene.addEntity(coneInfo);
            // auto hexprisInfo = entities::SDFFactory("HexagonalPrism", glm::vec4(0.2, 0.1, 0.95, 1.0), "HexagonalPrism", {18, 0, 7}, rotate_identity, {1, 1, 1}, commonShaderConstants);
            // scene.addEntity(hexprisInfo);
            // auto sphere_sdf_mesh  = entities::sdf_mesh_factory("Sphere", "Sphere", {7, 0, 0}, rotate_identity, {1, 1, 1}, glm::vec4(0, 0, 1, 1), commonShaderConstants);
            // scene.addEntity(sphere_sdf_mesh);
            Spot spot({1, 1, 1});
            // auto sphere_sdf_mesh = entities::sdf_mesh_factory("Sphere", "Sphere", spot(2), rotate_identity, {1, 1, 1}, glm::vec4(0.944, 1.00, 0.440, 1.0), commonShaderConstants);
            // scene.addEntity(sphere_sdf_mesh);
            auto torus_sdf_mesh  = entities::sdf_mesh_factory("Torus", "Torus", spot(4), rotate_identity, {3, 3, 3}, glm::vec4(.87, .13, .2, 1), commonShaderConstants);
            scene.addEntity(torus_sdf_mesh);
            // auto cylinder_sdf_mesh  = entities::sdf_mesh_factory("Cylinder", "Cylinder", spot(6), rotate_identity, {3, 3, 3}, glm::vec4(.97, .9, .4, 1), commonShaderConstants);
            // scene.addEntity(cylinder_sdf_mesh);
            // auto rounded_cube_sdf_mesh = entities::sdf_mesh_factory("RoundedCube", "RoundedCube", spot(2), rotate_identity, {1, 1, 1}, glm::vec4(.74, .64, .92, 1), commonShaderConstants);
            // scene.addEntity(rounded_cube_sdf_mesh);
            // auto cone_sdf_mesh = entities::sdf_mesh_factory("Cone", "Cone", spot(8), rotate_identity, {4, 4, 4}, glm::vec4(.82, .9, .77, 1), commonShaderConstants);
            // scene.addEntity(cone_sdf_mesh);
            // auto hexagonal_prism_sdf_mesh = entities::sdf_mesh_factory("HexagonalPrism", "HexagonalPrism", spot(8), rotate_identity, {4, 4, 4}, glm::vec4(.9, .8, .7, 1), commonShaderConstants);
            // scene.addEntity(hexagonal_prism_sdf_mesh);
            // auto ellipsoid_sdf_mesh = entities::sdf_mesh_factory("Ellipsoid", "Ellipsoid", spot(8), rotate_identity, {4, 4, 4}, {0.7, 0.7, 0.9, 0.9}, commonShaderConstants);
            // scene.addEntity(ellipsoid_sdf_mesh);
            // auto capsule_sdf_mesh = entities::sdf_mesh_factory("Capsule", "Capsule", spot(6), rotate_identity, {3, 3, 3}, {0.88, 0.78, 0.92, 1}, commonShaderConstants);
            // scene.addEntity(capsule_sdf_mesh);
            // auto flower_sdf_mesh = entities::sdf_mesh_factory("Flower", "Flower", spot(12), rotate_identity, {4, 4, 4}, {0.34, 0.88, 0.99, 1}, commonShaderConstants);
            // scene.addEntity(flower_sdf_mesh);
            // auto WobblySphere_sdf_mesh = entities::sdf_mesh_factory("WobblySphere", "WobblySphere", spot(6), rotate_identity, {4, 4, 4}, {0.34, 0.88, 0.99, 1}, commonShaderConstants);
            // scene.addEntity(WobblySphere_sdf_mesh);
            auto HelixoidDonut_sdf_mesh = entities::sdf_mesh_factory("HelixoidDonut", "HelixoidDonut", spot(4), rotate_identity, {3, 3, 3}, {0.34, 0.88, 0.99, 1}, commonShaderConstants);
            scene.addEntity(HelixoidDonut_sdf_mesh);
            // auto SphericalHarmonicsInspiredBlob_sdf_mesh = entities::sdf_mesh_factory("SphericalHarmonicsInspiredBlob", "SphericalHarmonicsInspiredBlob", spot(6), rotate_identity, {4, 4, 4}, {0.34, 0.88, 0.99, 1}, commonShaderConstants);
            // scene.addEntity(SphericalHarmonicsInspiredBlob_sdf_mesh);
            // auto cube2Info = entities::CubeFactory("Cube", {14, 0, 0}, rotate_identity, {1, 1, 1}, {1, 1, 0, 1}, commonShaderConstants);
            // scene.addEntity(cube2Info);
            scene.attachComponent(components::scenes::ViewMouseControlFactory());
            scene.attachComponent(components::scenes::ViewQuadKeyControlFactory(components::scenes::KeyScheme::WSADSC, 8));
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            scene.template make<size_t>("rID", window.addKeyPressHandler('r', [&, SCENE_INDEX_STACK = scene.INDEX_STACK](auto pressed){
                if (!pressed)
                    return;
                auto& scene = Registry::getScene(SCENE_INDEX_STACK);
                scene.viewPointer->direction = glm::normalize(glm::vec3(-1, -1, -1));
                scene.viewPointer->setDirty();
            }));
        },
        .onDetachedFunction = [](auto& scene){
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            window.removeKeyPressHandler('r', scene.template getData<size_t>("rID"));
        },
        .preUpdateFunction = [](auto& scene){
            // std::cout << "scene.view.direction: " << glm::to_string(scene.viewPointer->direction) << std::endl;
        }
    };
    return info;
}