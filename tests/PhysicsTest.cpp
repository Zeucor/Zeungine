#include <zg/Registry.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/EntityThirdPersonCamera.hpp>
#include <zg/components/scenes/GravityByAttraction.hpp>
#include <zg/components/scenes/GravityByVector.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
#include <zg/components/scenes/FXAA.hpp>
#include <zg/components/scenes/SMAA.hpp>
#include <zg/components/scenes/Bloom.hpp>
#include <zg/components/scenes/EdgeDetection.hpp>
#include <zg/components/scenes/DepthFog.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/entities/DeltaVisualizer.hpp>
#include <zg/entities/Frame.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/entities/Volume.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/math/Rotations.hpp>
#include <zg/physics/CollisionManifold.hpp>
using namespace zg;
shaders::RuntimeConstants commonShaderConstants({"Lighting", "DirectionalLightShadowMaps", "LightSpacePosition"});
glm::vec3 windowVisualizerPosition(56.8, 42, 57);
SceneCreateInfo PhysicsSceneFactory();
SceneCreateInfo HUDSceneFactory();
auto staticRigidBodyInfo =
	components::entities::RigidBodyFactory(components::entities::RigidBodyInfo{components::entities::BodyType::Static});
auto cubeRigidBodyInfo = components::entities::RigidBodyFactory(
	components::entities::RigidBodyInfo{components::entities::BodyType::Dynamic, 1.0f, 0.85f, 0.7f, true, false,
																			glm::vec<3, bool>(1, 0, 1), glm::vec<3, bool>(0)});
auto floorCreateInfo = entities::CubeFactory("Floor", {50, 40, 50}, {1, 0, 0, 0}, {1, 1, 1}, {20000, 0.5, 20000},
																						 {0.35, 0.45, 0.25, 1}, commonShaderConstants);
auto floorColliderInfo = components::entities::ColliderFactory(
	components::entities::ColliderInfo(
		std::make_shared<components::entities::BoxShapeData>(glm::vec3(20000, 0.5, 20000) / 2.f),
		components::entities::PhysicsMaterial{.friction=0.7f, .restitution=0.33f},
		false
	)
);
auto toxyCreateInfo = entities::CubeFactory("Toxy", {50, 47, 58}, {1, 0, 0, 0}, glm::vec3(2), glm::vec3(1), {0, 0, 1, 1}, commonShaderConstants);
auto toxyColliderInfo = components::entities::ColliderFactory(
	components::entities::ColliderInfo(
		std::make_shared<components::entities::BoxShapeData>(glm::vec3(2, 2, 2) / 2.f),
		components::entities::PhysicsMaterial{.friction=0.8f, .restitution=0.11f},
		false
	)
);
auto cubeCreateInfo = entities::CubeFactory("Cube", {53, 47, 58}, {1, 0, 0, 0}, glm::vec3(1), glm::vec3(1), {0.01, 1, 0.2, 1}, commonShaderConstants);
auto cubeColliderInfo = components::entities::ColliderFactory(
	components::entities::ColliderInfo(
		std::make_shared<components::entities::BoxShapeData>(glm::vec3(1, 1, 1) / 2.f),
		components::entities::PhysicsMaterial{.friction=0.21f, .restitution=0.55f},
		false
	)
);
int main()
{
	WindowCreateInfo windowCreateInfo{
		.title = "Physics Test",
		.windowWidth = 1920,
		.windowHeight = 1080,
		.windowX = 0,
		.windowY = 0,
		.borderless = true,
		.vsync = false,
		.framerate = 60, //Window::getScreenRefreshRate(1),
	};
	auto window_tuple = Registry::addWindow(windowCreateInfo);
	auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
	window.runOnThread([](auto& window){
		// window.attachComponent(zg::components::windows::FXAAFactory(0.0f, 0.00f, 32, 1.0f));
		// window.attachComponent(zg::components::windows::SMAAFactory(0.0f, 64, 64, 128));
		// window.attachComponent(zg::components::windows::EdgeDetectionFactory());
		// window.attachComponent(zg::components::windows::BloomFactory());
		window.addScene(PhysicsSceneFactory());
		window.addScene(HUDSceneFactory());
	});
	window.addKeyPressHandler(27, [&](auto pressed) {
		if (pressed)
			window.close();
	});
	window.run();
	return 0;
}
auto f = KEYCODE_UP;
auto b = KEYCODE_DOWN;
auto l = KEYCODE_LEFT;
auto r = KEYCODE_RIGHT;
auto s = 32;
SceneCreateInfo PhysicsSceneFactory()
{
	SceneCreateInfo info{
		.name = "PhysicsScene",
		.cameraPosition = {50, 50, 50},
		.cameraDirection = {0, -1, 1},
		.fov = 81.f,
		.onAttachedFunction = [](auto& scene)
		{
			auto& window = Registry::getWindow(scene.INDEX_STACK);
			scene.clearColor = {0, 0, 1, 1};
			glm::vec3 dldirection{1, -1, 1};
			dldirection = glm::normalize(dldirection);
			glm::vec3 dlup{0, 1, 0};
			scene.directionalLights.push_back({
				glm::vec3(20, 80, 20), // position
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
			// scene.attachComponent(components::scenes::GravityByAttractionFactory(0.000005f));
			// scene.attachComponent(components::scenes::GravityByVectorFactory(glm::vec3(0, -9.81, 0)));
			scene.attachComponent(components::scenes::PhysicsSceneFactory((long double)(1.0L / 40.0L)));
			// scene.attachComponent(components::scenes::ViewMouseControlFactory());
			// scene.attachComponent(
			// 	components::scenes::ViewQuadKeyControlFactory(components::scenes::KeyScheme::WSADSC, 5));
			scene.entities.reserve(5);
			auto floor_tuple = scene.addEntity(floorCreateInfo);
			auto& floor = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(floor_tuple);
			auto floor_index_stack = floor.INDEX_STACK;
			auto floor_rb_tuple = floor.attachComponent(staticRigidBodyInfo);
			auto& floor_rb = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(floor_rb_tuple);
			auto floor_rb_ID = floor_rb.ID;
			floor_rb.template setData<float>("Mass", 1000000.0f);
			floor.attachComponent(floorColliderInfo);
			// auto floorColliderCreateInfo = components::entities::ColliderFactory();
			// floor.attachComponent(floorColliderCreateInfo);
			auto toxy_tuple = scene.addEntity(toxyCreateInfo);
			auto& toxy = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(toxy_tuple);
			auto toxy_index_stack = toxy.INDEX_STACK;
			auto toxy_rb_tuple = toxy.attachComponent(cubeRigidBodyInfo);
			auto toxy_rb_ID = std::get<KEY_ID_VECTOR_VALUE_INDEX>(toxy_rb_tuple)->ID;
			toxy.attachComponent(toxyColliderInfo);
			auto cube1_tuple = scene.addEntity(cubeCreateInfo);
			auto& cube1 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(cube1_tuple);
			cube1.attachComponent(cubeRigidBodyInfo);
			cube1.attachComponent(cubeColliderInfo);
			cubeCreateInfo.position = {47, 47, 58};
			auto cube2_tuple = scene.addEntity(cubeCreateInfo);
			auto& cube2 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(cube2_tuple);
			cube2.attachComponent(cubeRigidBodyInfo);
			cube2.attachComponent(cubeColliderInfo);
			cubeCreateInfo.position = {50, 47, 54};
			auto cube3_tuple = scene.addEntity(cubeCreateInfo);
			auto& cube3 = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(cube3_tuple);
			cube3.attachComponent(cubeRigidBodyInfo);
			cube3.attachComponent(cubeColliderInfo);

			scene.attachComponent(components::scenes::EntityThirdPersonCameraFactory(*std::get<KEY_ID_VECTOR_VALUE_INDEX>(toxy_tuple)));

			// scene.attachComponent(components::scenes::DepthFogFactory());
			// cube controls
			{
				std::function<void()> onFrontTickFunction = [toxy_index_stack, toxy_rb_ID]()
				{
					auto& toxy = Registry::getEntity(toxy_index_stack);
					auto& toxy_rb = toxy.getComponentByID(toxy_rb_ID); 
					toxy_rb.template setData<glm::vec3>("applyLocalForceToCenter", glm::vec3(0, 0, 20));
				};
				std::function<void()> onBackTickFunction = [toxy_index_stack, toxy_rb_ID]()
				{
					auto& toxy = Registry::getEntity(toxy_index_stack);
					auto& toxy_rb = toxy.getComponentByID(toxy_rb_ID); 
					toxy_rb.template setData<glm::vec3>("applyLocalForceToCenter", glm::vec3(0, 0, -20));
				};
				std::function<void()> onLeftTickFunction = [toxy_index_stack, toxy_rb_ID]()
				{
					auto& toxy = Registry::getEntity(toxy_index_stack);
					auto& toxy_rb = toxy.getComponentByID(toxy_rb_ID); 
					toxy_rb.template setData<glm::vec3>("applyLocalForceToCenter", glm::vec3(20, 0, 0));
				};
				std::function<void()> onRightTickFunction = [toxy_index_stack, toxy_rb_ID]()
				{
					auto& toxy = Registry::getEntity(toxy_index_stack);
					auto& toxy_rb = toxy.getComponentByID(toxy_rb_ID); 
					toxy_rb.template setData<glm::vec3>("applyLocalForceToCenter", glm::vec3(-20, 0, 0));
				};
				std::function<void()> onSpaceTickFunction = [toxy_index_stack, floor_index_stack, floor_rb_ID, toxy_rb_ID]()
				{
					auto& toxy = Registry::getEntity(toxy_index_stack);
					auto& toxy_rb = toxy.getComponentByID(toxy_rb_ID);
					auto& floor = Registry::getEntity(floor_index_stack);
					auto& floor_rb = floor.getComponentByID(floor_rb_ID);
					physics::CollisionManifold* ManifoldPointer = 0;
					if (std::any_cast<bool>(toxy_rb.template setData<components::entities::EntityComponent*>("isTouching", &floor_rb)))
					{
						toxy_rb.template setData<glm::vec3>("applyLocalForceToCenter", glm::vec3(0, 150, 0));
					}
				};
				/*fID = */window.addKeyUpdateHandler(f, onFrontTickFunction);
				/*bID = */window.addKeyUpdateHandler(b, onBackTickFunction);
				/*lID = */window.addKeyUpdateHandler(l, onLeftTickFunction);
				/*rID = */window.addKeyUpdateHandler(r, onRightTickFunction);
				/*sID = */window.addKeyUpdateHandler(s, onSpaceTickFunction);
			}
		}
	};
	return info;
}
// 		// //
// 		// 	{
// commonShaderConstants);
// 		// 	cube4->attachComponent(cubeRigidBodyInfo);
// 		// 	cube4->attachComponent(components::entities::ColliderFactory(components::entities::ColliderInfo{
// 		// 		// std::make_shared<components::entities::MeshShapeData>(*cube4),
// 		// 		std::make_unique<components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
// 		// 		components::entities::PhysicsMaterial{0.30f, 0.7f}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
// 		// 	addEntity(cube4);
// 		// }
// 		// //
// 		// // walls
// 		// {
// 		// 	auto wall1 = std::make_shared<entities::Cube>(window, *this, glm::vec3(40, 43, 50), glm::quat(1, 0, 0, 0),
// 		// 																										glm::vec3(1), glm::vec3(1, 5, 20), commonShaderConstants);
// 		// 	auto wall1RigidBodyID = wall1->attachComponent(staticRigidBodyInfo);
// 		// 	auto& wall1RigidBody = wall1->getComponentByID(wall1RigidBodyID);
// 		// 	// wall1RigidBody.template getData<float>("Mass") = 2;
// 		// 	wall1->attachComponent(components::entities::ColliderFactory(components::entities::ColliderInfo{
// 		// 		std::make_unique<components::entities::BoxShapeData>(glm::vec3(1, 5, 20) / 2.f),
// 		// 		components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
// 		// 	addEntity(wall1);
// 		// 	auto wall2 = std::make_shared<entities::Cube>(window, *this, glm::vec3(60, 43, 50), glm::quat(1, 0, 0, 0),
// 		// 																										glm::vec3(1), glm::vec3(1, 5, 20), commonShaderConstants);
// 		// 	auto wall2RigidBodyID = wall2->attachComponent(staticRigidBodyInfo);
// 		// 	auto& wall2RigidBody = wall2->getComponentByID(wall2RigidBodyID);
// 		// 	// wall2RigidBody.template getData<float>("Mass") = 2;
// 		// 	wall2->attachComponent(components::entities::ColliderFactory(components::entities::ColliderInfo{
// 		// 		std::make_unique<components::entities::BoxShapeData>(glm::vec3(1, 5, 20) / 2.f),
// 		// 		components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
// 		// 	addEntity(wall2);
// 		// 	auto wall3 = std::make_shared<entities::Cube>(window, *this, glm::vec3(50, 43, 40), glm::quat(1, 0, 0, 0),
// 		// 																										glm::vec3(1), glm::vec3(20, 5, 1), commonShaderConstants);
// 		// 	auto wall3RigidBodyID = wall3->attachComponent(staticRigidBodyInfo);
// 		// 	auto& wall3RigidBody = wall3->getComponentByID(wall3RigidBodyID);
// 		// 	// wall3RigidBody.template getData<float>("Mass") = 2;
// 		// 	wall3->attachComponent(components::entities::ColliderFactory(components::entities::ColliderInfo{
// 		// 		std::make_unique<components::entities::BoxShapeData>(glm::vec3(20, 5, 1) / 2.f),
// 		// 		components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
// 		// 	addEntity(wall3);
// 		// 	auto wall4 = std::make_shared<entities::Cube>(window, *this, glm::vec3(50, 43, 60), glm::quat(1, 0, 0, 0),
// 		// 																										glm::vec3(1), glm::vec3(20, 5, 1), commonShaderConstants);
// 		// 	auto wall4RigidBodyID = wall4->attachComponent(staticRigidBodyInfo);
// 		// 	auto& wall4RigidBody = wall4->getComponentByID(wall4RigidBodyID);
// 		// 	// wall4RigidBody.template getData<float>("Mass") = 2;
// 		// 	wall4->attachComponent(components::entities::ColliderFactory(components::entities::ColliderInfo{
// 		// 		std::make_unique<components::entities::BoxShapeData>(glm::vec3(20, 5, 1) / 2.f),
// 		// 		components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
// 		// 	addEntity(wall4);
// 		// }
// 		// // visualizers
// 		// addEntity(std::make_shared<entities::DeltaVisualizer>(
// 		// 	window, *this, glm::vec2(5, 1), window.deltaTime, window.lastFrameDeltaTime, windowVisualizerPosition,
// 		// 	glm::angleAxis(glm::radians(180.f), glm::vec3(0, 1, 0)), glm::vec3(1)));
// 		// // volumes
// 		// {
// 		// 	auto _pi_ = acos(-1.f);

// 		// 	std::map<std::string, double> seashell_params = {
// 		// 		{"a", 0.2}, // Scale factor
// 		// 		{"b", 0.1}, // Exponential growth rate (tightness)
// 		// 		{"k", 3.0} // Number of twists/lobes factor
// 		// 	};
// 		// 	std::array<std::string, 3> seashell_eqs = {"a * exp(b*v) * cos(k*v) * cos(u)", "a * exp(b*v) * cos(k*v) *
// sin(u)",
// 		// 																						 "a * exp(b*v) * sin(k*v)"};
// 		// 	std::array<std::string, 3> seashell_normal_eqs = {
// 		// 		"cos(u) * (b * sin(k*v) + k * cos(k*v))", // Normal X component
// 		// 		"sin(u) * (b * sin(k*v) + k * cos(k*v))", // Normal Y component
// 		// 		"k * sin(k*v) - b * cos(k*v)" // Normal Z component
// 		// 	};
// 		// 	// Parametric Equations: Logarithmic Spiral Cone
// 		// 	// x = a * exp(b*v) * cos(k*v) * cos(u)
// 		// 	// y = a * exp(b*v) * cos(k*v) * sin(u)
// 		// 	// z = a * exp(b*v) * sin(k*v)
// 		// 	// auto seashell_volume = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(50, 45, 50), // Position
// 		// 	// 	glm::quat(1, 0, 0, 0), // Rotation
// 		// 	// 	glm::vec3(2.0f), // Scale
// 		// 	// 	glm::vec4(1.0, 0.5, 0.2, 1.0), // Orange color
// 		// 	// 	commonShaderConstants, "Seashell Volume", seashell_params,
// 		// 	// 	// U range: [0, 2*pi] (around the axis)
// 		// 	// 	0.0, 2.0 * _pi_, _pi_ / 5.f, // U range and step
// 		// 	// 	// V range: [0, 4*pi] (along the spiral length)
// 		// 	// 	0.0, 4.0 * _pi_, _pi_ / 5.f, // V range and step
// 		// 	// 	// Position Equations (using finite differencing for normals)
// 		// 	// 	seashell_eqs, seashell_normal_eqs);
// 		// 	// addEntity(seashell_volume);
// 		// 	// std::map<std::string, double> tanh_box_params = {
// 		// 	// 	{"rx", 0.5}, // Radius/half-extent x
// 		// 	// 	{"ry", 0.5}, // Radius/half-extent y
// 		// 	// 	{"rz", 0.5}, // Radius/half-extent z
// 		// 	// 	{"k", 10.0} // Sharpness factor (higher k = sharper edges)
// 		// 	// };
// 		// 	// cube5 = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(50, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1, 0, 1, 1),
// 		// 	// 	commonShaderConstants, "Box Volume",
// 		// 	// 	tanh_box_params, // Parameters map
// 		// 	// 	// U range (original)
// 		// 	// 	-_pi_, _pi_, _pi_ / 10.f,
// 		// 	// 	// V range (original)
// 		// 	// 	-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 10.f,
// 		// 	// 	// Parametric Equations using tanh for smooth box-like shape
// 		// 	// 	// Based on spherical mapping but clamped smoothly by tanh
// 		// 	// 	"rx * tanh(k * cos(v) * cos(u))", "ry * tanh(k * cos(v) * sin(u))", "rz * tanh(k * sin(v))"

// 		// 	// );
// 		// 	// addEntity(cube5);
// 		// 	//
// 		// 	// // 1. Standard Sphere
// 		// 	// std::map<std::string, double> sphere_params = {
// 		// 	// 	{"r", 1.0} // Radius
// 		// 	// };
// 		// 	// auto sphere_volume =
// 		// 	// 	std::make_shared<entities::NUVVolume<3, float>>(window, *this, glm::vec3(53, 43, 53), glm::quat(1, 0, 0,
// 		// 	// 0), 																											glm::vec3(1), glm::vec4(1.0, 0.0, 0.0, 1.0), //
// Red
// 		// 	// color 																											commonShaderConstants, "Sphere Volume",
// 		// 	// sphere_params,
// 		// 	// 																											// U range: [-pi, pi] (longitude)
// 		// 	// 																											-_pi_, _pi_, _pi_ / 6.5f,
// 		// 	// 																											// V range: [-pi/2, pi/2] (latitude)
// 		// 	// 																											-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 6.5f,
// 		// 	// 																											// Equations
// 		// 	// 																											"r * cos(v) * cos(u)", "r * cos(v) * sin(u)", "r *
// 		// 	// sin(v)"); addEntity(sphere_volume);

// 		// 	// 2. Torus (Donut)
// 		// 	// Note: A full torus typically requires v range [-pi, pi] or [0, 2*pi]
// 		// 	// std::map<std::string, double> torus_params = {
// 		// 	// 	{"R", 1.2}, // Major radius (center of tube to center of torus)
// 		// 	// 	{"r", 0.1} // Minor radius (radius of the tube)
// 		// 	// };
// 		// 	// std::array<std::string, 3> torus_eqs = {
// 		// 	// 	"(R + r * cos(v)) * cos(u)",
// 		// 	// 	"(R + r * cos(v)) * sin(u)",
// 		// 	// 	"r * sin(v)"
// 		// 	// };
// 		// 	// std::array<std::string, 3> torus_normal_eqs = {
// 		// 	// 	"cos(u) * cos(v)",
// 		// 	// 	"sin(u) * cos(v)",
// 		// 	// 	"sin(v)"
// 		// 	// };
// 		// 	// auto torus_volume = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(56, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1),
// 		// 	// 	glm::vec4(0.0, 1.0, 0.0, 1.0), // Green color
// 		// 	// 	commonShaderConstants, "Torus Volume", torus_params,
// 		// 	// 	// U range: [-pi, pi] (around the major circle)
// 		// 	// 	-_pi_, _pi_, _pi_ / 6.f,
// 		// 	// 	// V range: [-pi, pi] (around the minor circle - tube cross-section)
// 		// 	// 	// *** Adjust V range in constructor call if needed for full torus ***
// 		// 	// 	-_pi_, _pi_, _pi_ / 6.f,
// 		// 	// 	// Equations
// 		// 	// 	torus_eqs, torus_normal_eqs);
// 		// 	// addEntity(torus_volume);

// 		// 	// // 3. Twisted Ellipsoid
// 		// 	// std::map<std::string, double> twist_params = {
// 		// 	// 	{"rx", 1.0},
// 		// 	// 	{"ry", 0.4},
// 		// 	// 	{"rz", 0.7}, // Ellipsoid radii
// 		// 	// 	{"k", 0.8} // Twist factor (radians per unit of v)
// 		// 	// };
// 		// 	// auto twist_volume = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(47, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1),
// 		// 	// 	glm::vec4(0.0, 0.0, 1.0, 1.0), // Blue color
// 		// 	// 	commonShaderConstants, "Twisted Ellipsoid Volume", twist_params,
// 		// 	// 	// U range: [-pi, pi]
// 		// 	// 	-_pi_, _pi_, _pi_ / 4.f,
// 		// 	// 	// V range: [-pi/2, pi/2]
// 		// 	// 	-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 4.f,
// 		// 	// 	// Equations - applying rotation based on v
// 		// 	// 	"(rx * cos(v) * cos(u)) * cos(k*v) - (ry * cos(v) * sin(u)) * sin(k*v)", // Rotated x
// 		// 	// 	"(rx * cos(v) * cos(u)) * sin(k*v) + (ry * cos(v) * sin(u)) * cos(k*v)", // Rotated y
// 		// 	// 	"rz * sin(v)" // Original z
// 		// 	// );
// 		// 	// addEntity(twist_volume);

// 		// 	// // 4. Spiral Tube
// 		// 	// // Note: Requires a larger v range (e.g., [-2*pi, 2*pi]) for multiple turns
// 		// 	// std::map<std::string, double> spiral_params = {
// 		// 	// 	{"R_base", 2.0}, // Starting major radius
// 		// 	// 	{"r", 1.5}, // Tube radius
// 		// 	// 	{"spiral_factor", 1.0}, // How much R grows per unit of v
// 		// 	// 	{"height_factor", 2.0}, // How much z increases per unit of v
// 		// 	// 	{"n", 8.0} // Frequency multiplier for cross-section rotation (aesthetic)
// 		// 	// };
// 		// 	// auto spiral_volume = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(44, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1),
// 		// 	// 	glm::vec4(1.0, 1.0, 0.0, 1.0), // Yellow color
// 		// 	// 	commonShaderConstants, "Spiral Tube Volume", spiral_params,
// 		// 	// 	// U range: [-pi, pi] (around the tube cross-section)
// 		// 	// 	-_pi_, _pi_, _pi_ / 8.f,
// 		// 	// 	// V range: [-2*pi, 2*pi] (path of the spiral - adjust for length)
// 		// 	// 	// *** Adjust V range in constructor call for desired spiral length ***
// 		// 	// 	-2.0f * _pi_, 2.0f * _pi_, _pi_ / 16.f,
// 		// 	// 	// Equations
// 		// 	// 	"(R_base + spiral_factor * v + r * cos(u)) * cos(n*v)", // x: spiral path + tube offset, rotated
// 		// 	// 	"(R_base + spiral_factor * v + r * cos(u)) * sin(n*v)", // y: spiral path + tube offset, rotated
// 		// 	// 	"height_factor * v + r * sin(u)" // z: height + tube offset
// 		// 	// );
// 		// 	// addEntity(spiral_volume);


// 		// 	// // 5. "Star" Shape (Rippled Sphere)
// 		// 	// std::map<std::string, double> star_params = {
// 		// 	// 	{"base_r", 6.0}, // Average radius
// 		// 	// 	{"amp", 1.5}, // Amplitude of ripples
// 		// 	// 	{"n", 5.0}, // Frequency of ripples along u (longitude) - integer for closed loops
// 		// 	// 	{"m", 4.0} // Frequency of ripples along v (latitude) - integer for closed loops
// 		// 	// };
// 		// 	// auto star_volume = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(50, 43, 49), glm::quat(1, 0, 0, 0), glm::vec3(1),
// 		// 	// 	glm::vec4(1.0, 0.0, 1.0, 1.0), // Magenta color
// 		// 	// 	commonShaderConstants, "Star Volume", star_params,
// 		// 	// 	// U range: [-pi, pi]
// 		// 	// 	-_pi_, _pi_, _pi_ / 16.f,
// 		// 	// 	// V range: [-pi/2, pi/2]
// 		// 	// 	-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 16.f,
// 		// 	// 	// Equations - Modulate radius based on u and v
// 		// 	// 	"(base_r * (1.0 + amp * sin(n * u) * sin(m * v))) * cos(v) * cos(u)", // x = modulated_radius * sphere_x
// 		// 	// 	"(base_r * (1.0 + amp * sin(n * u) * sin(m * v))) * cos(v) * sin(u)", // y = modulated_radius * sphere_y
// 		// 	// 	"(base_r * (1.0 + amp * sin(n * u) * sin(m * v))) * sin(v)" // z = modulated_radius * sphere_z
// 		// 	// );
// 		// 	// addEntity(star_volume);

// 		// 	// // 6. Apple Shape (Rotated Cardioid-like)
// 		// 	// // Note: Requires v range [-pi, pi] or [0, 2*pi] for the full cardioid profile
// 		// 	// std::map<std::string, double> apple_params = {
// 		// 	// 	{"a", 2.0} // Scale factor for the cardioid profile
// 		// 	// };
// 		// 	// auto apple_volume = std::make_shared<entities::NUVVolume<3, float>>(
// 		// 	// 	window, *this, glm::vec3(50, 43, 46), glm::quat(1, 0, 0, 0), glm::vec3(1),
// 		// 	// 	glm::vec4(0.0, 1.0, 1.0, 1.0), // Cyan color
// 		// 	// 	commonShaderConstants, "Apple Volume", apple_params,
// 		// 	// 	// U range: [-pi, pi] (rotation around Z axis)
// 		// 	// 	-_pi_, _pi_, _pi_ / 16.f,
// 		// 	// 	// V range: [-pi, pi] (angle for the cardioid profile)
// 		// 	// 	// *** Adjust V range in constructor call if needed for full shape ***
// 		// 	// 	-_pi_, _pi_, _pi_ / 16.f,
// 		// 	// 	// Equations - Generate 2D cardioid in XZ cube5 (using v) and revolve around Z (using u)
// 		// 	// 	"(a * (1.0 - cos(v))) * cos(v) * cos(u)", // x = (profile_radius) * profile_x_component * cos(u)
// 		// 	// 	"(a * (1.0 - cos(v))) * cos(v) * sin(u)", // y = (profile_radius) * profile_x_component * sin(u)
// 		// 	// 	"(a * (1.0 - cos(v))) * sin(v)" // z = (profile_radius) * profile_z_component
// 		// 	// );
// 		// 	// addEntity(apple_volume);
// 		// }
// 		// //
// 		// attachComponent(components::scenes::EntityThirdPersonCameraFactory(*cube));
// 	}
// 	void preUpdate() override {}
// 	~PhysicsScene()
// 	{
// 		window.removeKeyUpdateHandler(f, fID);
// 		window.removeKeyUpdateHandler(b, bID);
// 		window.removeKeyUpdateHandler(l, lID);
// 		window.removeKeyUpdateHandler(r, rID);
// 		detachAllComponents();
// 	}
// };
SceneCreateInfo HUDSceneFactory()
{
	SceneCreateInfo info{
		.name = "HUD",
		.cameraPosition = {0, 0, -1},
		.cameraDirection = {0, 0, 1},
		.cameraUp = {0, 1, 0},
		.projectionType = zg::vp::Projection::TYPE::Orthographic,
		.orthoSize = {2, 2},
		.onAttachedFunction = [](auto& scene)
		{
			auto& window = Registry::getWindow(scene.INDEX_STACK);
            auto angle = glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
			scene.addEntity(entities::DeltaVisualizerFactory(glm::vec2(0.5, 0.5), window.deltaTime, window.lastFrameDeltaTime, glm::vec3(-0.75, 0.75, 0), angle));
		},
		.useBVH = false
	};
	return info;
}