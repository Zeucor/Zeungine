#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/EntityThirdPersonCamera.hpp>
#include <zg/components/scenes/GravityByAttraction.hpp>
#include <zg/components/scenes/GravityByVector.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/entities/Volume.hpp>
#include <zg/math/Rotations.hpp>
#include <zg/physics/CollisionManifold.hpp>
#include <zg/vp/VFBLR.hpp>
#include <zg/vp/VML.hpp>
#include <zg/entities/Frame.hpp>
zg::shaders::RuntimeConstants commonShaderConstants({"Lighting", "DirectionalLightShadowMaps", "LightSpacePosition"});
struct PhysicsScene : zg::Scene
{
	std::shared_ptr<zg::components::scenes::EntityThirdPersonCamera> thirdPerson;
	// zg::vp::VML vml;
	// zg::vp::VFBLR vfblr;
	std::shared_ptr<zg::entities::Cube> floor;
	std::shared_ptr<zg::components::entities::RigidBody> floorRigidBody;
	std::shared_ptr<zg::entities::Cube> cube;
	std::shared_ptr<zg::components::entities::RigidBody> cubeRigidBody;
	std::shared_ptr<zg::entities::Cube> cube2;
	std::shared_ptr<zg::components::entities::RigidBody> cube2RigidBody;
	std::shared_ptr<zg::entities::Cube> cube3;
	std::shared_ptr<zg::components::entities::RigidBody> cube3RigidBody;
	std::shared_ptr<zg::entities::Cube> cube4;
	std::shared_ptr<zg::components::entities::RigidBody> cube4RigidBody;
	std::shared_ptr<zg::entities::Cube> wall1;
	std::shared_ptr<zg::components::entities::RigidBody> wall1RigidBody;
	std::shared_ptr<zg::entities::Cube> wall2;
	std::shared_ptr<zg::components::entities::RigidBody> wall2RigidBody;
	std::shared_ptr<zg::entities::Cube> wall3;
	std::shared_ptr<zg::components::entities::RigidBody> wall3RigidBody;
	std::shared_ptr<zg::entities::Cube> wall4;
	std::shared_ptr<zg::components::entities::RigidBody> wall4RigidBody;
	std::shared_ptr<zg::entities::NDParametricCurve<2, float>> curve1;
	std::vector<glm::vec2> curve1Points;
	size_t curve1PointsIndex = 0;
	zg::UniqueIdentifier curve1ID = 0;
	std::shared_ptr<zg::entities::NUVVolume<3, float>> cube5;
	std::shared_ptr<zg::entities::Frame> frame;
	zg::UniqueIdentifier frameID;
	zg::UniqueIdentifier fID = 0;
	zg::UniqueIdentifier bID = 0;
	zg::UniqueIdentifier lID = 0;
	zg::UniqueIdentifier rID = 0;
	zg::UniqueIdentifier sID = 0;
	int f = 0;
	int b = 0;
	int l = 0;
	int r = 0;
	int s = 0;
	PhysicsScene(zg::Window& window) : Scene(window, {50, 50, 50}, {0, -1, 1}, 81.f)
	// vml(*this),
	// vfblr(*this, zg::vp::VFBLR::KeyScheme::WSADSC, 8.f)
	{
		clearColor = {0, 0, 1, 1};
		{
			// auto dldirection = glm::normalize(zg::math::Rotations::Vec3AroundVec3({0, 0, 1}, {0, 0, 0}, {90, 0, 0}));
			// dldirection = glm::normalize(zg::math::Rotations::Vec3AroundVec3(dldirection, {0, 0, 0}, {0, 90, 90}));
			// auto dlup = glm::normalize(zg::math::Rotations::Vec3AroundVec3({0, 1, 0}, {0, 0, 0}, {90, 0, 0}));
			// dlup = glm::normalize(zg::math::Rotations::Vec3AroundVec3(dlup, {0, 0, 0}, {0, 90, 0}));
			glm::vec3 dldirection{1, -1, 1};
			dldirection = glm::normalize(dldirection);
			glm::vec3 dlup{0, 1, 0};
			directionalLights.push_back({
				glm::vec3(20, 80, 20), // position
				dldirection, // direction
				dlup, // up
				glm::vec3(1.f, 1.f, 1.f), // color
				1.f, // intensity,
				1.f, // nearcube5
				364.f, // farcube5
				0.4f // ambientFactor
			});
			auto& dl = directionalLights[0];
			directionalLightShadows.emplace_back(window, directionalLights[0]);
		}
		// addComponent(std::make_shared<zg::components::scenes::GravityByAttraction>(0.000005f));
		// addComponent(std::make_shared<zg::components::scenes::GravityByVector>(glm::vec3(0, -9.81, 0)));
		// addComponent(std::make_shared<zg::components::scenes::PhysicsScene>(*this));
		//
		{
			floor = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 40, 50), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(2000, 0.5, 2000), commonShaderConstants);
			// floorRigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*floor, zg::components::entities::BodyType::Static});
			// floorRigidBody->setMass(1000000);
			// floor->addComponent(floorRigidBody);
			// floor->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*floor, std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(2000, 0.5, 2000) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{0.30f, 0.7f}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(floor);
		}
		// cube
		{
			cube = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 47, 58), glm::quat(1, 0, 0, 0),
																									glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
			// cubeRigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*cube, zg::components::entities::BodyType::Dynamic, 1.0f, 0.85f, 0.7f,
			// 																					true, false, glm::vec<3, bool>(1, 0, 1), glm::vec<3, bool>(0)});
			// cube->addComponent(cubeRigidBody);
			// cube->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*cube,
			// 	// std::make_shared<zg::components::entities::MeshShapeData>(*cube),
			// 	std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{0.30f, 0.7f}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(cube);
			// cube2
			cube2 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(53, 47, 58), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
			// cube2RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*cube2, zg::components::entities::BodyType::Dynamic});
			// cube2->addComponent(cube2RigidBody);
			// cube2->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*cube2,
			// 	// std::make_shared<zg::components::entities::MeshShapeData>(*cube2),
			// 	std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{0.30f, 0.7f}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(cube2);
			// cube3
			cube3 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(47, 47, 58), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
			// cube3RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*cube3, zg::components::entities::BodyType::Dynamic});
			// cube3->addComponent(cube3RigidBody);
			// cube3->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*cube3,
			// 	// std::make_shared<zg::components::entities::MeshShapeData>(*cube3),
			// 	std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{0.30f, 0.7f}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(cube3);
			// cube4
			cube4 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 47, 54), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
			// cube4RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*cube4, zg::components::entities::BodyType::Dynamic});
			// cube4->addComponent(cube4RigidBody);
			// cube4->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*cube4,
			// 	// std::make_shared<zg::components::entities::MeshShapeData>(*cube4),
			// 	std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{0.30f, 0.7f}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(cube4);
		}
		//
		// walls
		{
			wall1 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(40, 43, 50), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(1, 5, 20), commonShaderConstants);
			// wall1RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*wall1, zg::components::entities::BodyType::Static});
			// wall1RigidBody->setMass(2);
			// wall1->addComponent(wall1RigidBody);
			// wall1->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*wall1, std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1, 5, 20) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(wall1);
			wall2 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(60, 43, 50), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(1, 5, 20), commonShaderConstants);
			// wall2RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*wall2, zg::components::entities::BodyType::Static});
			// wall2RigidBody->setMass(2);
			// wall2->addComponent(wall2RigidBody);
			// wall2->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*wall2, std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1, 5, 20) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(wall2);
			wall3 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 43, 40), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(20, 5, 1), commonShaderConstants);
			// wall3RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*wall3, zg::components::entities::BodyType::Static});
			// wall3RigidBody->setMass(2);
			// wall3->addComponent(wall3RigidBody);
			// wall3->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*wall3, std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(20, 5, 1) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(wall3);
			wall4 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 43, 60), glm::quat(1, 0, 0, 0),
																									 glm::vec3(1), glm::vec3(20, 5, 1), commonShaderConstants);
			// wall4RigidBody = std::make_shared<zg::components::entities::RigidBody>(
			// 	zg::components::entities::RigidBodyInfo{*wall4, zg::components::entities::BodyType::Static});
			// wall4RigidBody->setMass(2);
			// wall4->addComponent(wall4RigidBody);
			// wall4->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
			// 	*wall4, std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(20, 5, 1) / 2.f),
			// 	zg::components::entities::PhysicsMaterial{}, glm::vec3(0), glm::quat(1, 0, 0, 0), false}));
			addEntity(wall4);
		}
		// volumes
		{
			auto _pi_ = acos(-1.f);

			std::map<std::string, double> seashell_params = {
				{"a", 0.2}, // Scale factor
				{"b", 0.1}, // Exponential growth rate (tightness)
				{"k", 3.0} // Number of twists/lobes factor
			};
			std::array<std::string, 3> seashell_eqs = {"a * exp(b*v) * cos(k*v) * cos(u)", "a * exp(b*v) * cos(k*v) * sin(u)",
																								 "a * exp(b*v) * sin(k*v)"};
			std::array<std::string, 3> seashell_normal_eqs = {
				"cos(u) * (b * sin(k*v) + k * cos(k*v))", // Normal X component
				"sin(u) * (b * sin(k*v) + k * cos(k*v))", // Normal Y component
				"k * sin(k*v) - b * cos(k*v)" // Normal Z component
			};
			// Parametric Equations: Logarithmic Spiral Cone
			// x = a * exp(b*v) * cos(k*v) * cos(u)
			// y = a * exp(b*v) * cos(k*v) * sin(u)
			// z = a * exp(b*v) * sin(k*v)
			auto seashell_volume = std::make_shared<zg::entities::NUVVolume<3, float>>(
				window, *this, glm::vec3(50, 45, 50), // Position
				glm::quat(1, 0, 0, 0), // Rotation
				glm::vec3(2.0f), // Scale
				glm::vec4(1.0, 0.5, 0.2, 1.0), // Orange color
				commonShaderConstants, "Seashell Volume", seashell_params,
				// U range: [0, 2*pi] (around the axis)
				0.0, 2.0 * _pi_, _pi_ / 5.f, // U range and step
				// V range: [0, 4*pi] (along the spiral length)
				0.0, 4.0 * _pi_, _pi_ / 5.f, // V range and step
				// Position Equations (using finite differencing for normals)
				seashell_eqs, seashell_normal_eqs);
			addEntity(seashell_volume);
			// std::map<std::string, double> tanh_box_params = {
			// 	{"rx", 0.5}, // Radius/half-extent x
			// 	{"ry", 0.5}, // Radius/half-extent y
			// 	{"rz", 0.5}, // Radius/half-extent z
			// 	{"k", 10.0} // Sharpness factor (higher k = sharper edges)
			// };
			// cube5 = std::make_shared<zg::entities::NUVVolume<3, float>>(
			// 	window, *this, glm::vec3(50, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1, 0, 1, 1),
			// 	commonShaderConstants, "Box Volume",
			// 	tanh_box_params, // Parameters map
			// 	// U range (original)
			// 	-_pi_, _pi_, _pi_ / 10.f,
			// 	// V range (original)
			// 	-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 10.f,
			// 	// Parametric Equations using tanh for smooth box-like shape
			// 	// Based on spherical mapping but clamped smoothly by tanh
			// 	"rx * tanh(k * cos(v) * cos(u))", "ry * tanh(k * cos(v) * sin(u))", "rz * tanh(k * sin(v))"

			// );
			// addEntity(cube5);
			//
			// // 1. Standard Sphere
			// std::map<std::string, double> sphere_params = {
			// 	{"r", 1.0} // Radius
			// };
			// auto sphere_volume =
			// 	std::make_shared<zg::entities::NUVVolume<3, float>>(window, *this, glm::vec3(53, 43, 53), glm::quat(1, 0, 0,
			// 0), 																											glm::vec3(1), glm::vec4(1.0, 0.0, 0.0, 1.0), // Red
			// color 																											commonShaderConstants, "Sphere Volume",
			// sphere_params,
			// 																											// U range: [-pi, pi] (longitude)
			// 																											-_pi_, _pi_, _pi_ / 6.5f,
			// 																											// V range: [-pi/2, pi/2] (latitude)
			// 																											-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 6.5f,
			// 																											// Equations
			// 																											"r * cos(v) * cos(u)", "r * cos(v) * sin(u)", "r *
			// sin(v)"); addEntity(sphere_volume);

			// 2. Torus (Donut)
			// Note: A full torus typically requires v range [-pi, pi] or [0, 2*pi]
			// std::map<std::string, double> torus_params = {
			// 	{"R", 1.2}, // Major radius (center of tube to center of torus)
			// 	{"r", 0.1} // Minor radius (radius of the tube)
			// };
			// std::array<std::string, 3> torus_eqs = {
			// 	"(R + r * cos(v)) * cos(u)",
			// 	"(R + r * cos(v)) * sin(u)",
			// 	"r * sin(v)"
			// };
			// std::array<std::string, 3> torus_normal_eqs = {
			// 	"cos(u) * cos(v)",
			// 	"sin(u) * cos(v)",
			// 	"sin(v)"
			// };
			// auto torus_volume = std::make_shared<zg::entities::NUVVolume<3, float>>(
			// 	window, *this, glm::vec3(56, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1),
			// 	glm::vec4(0.0, 1.0, 0.0, 1.0), // Green color
			// 	commonShaderConstants, "Torus Volume", torus_params,
			// 	// U range: [-pi, pi] (around the major circle)
			// 	-_pi_, _pi_, _pi_ / 6.f,
			// 	// V range: [-pi, pi] (around the minor circle - tube cross-section)
			// 	// *** Adjust V range in constructor call if needed for full torus ***
			// 	-_pi_, _pi_, _pi_ / 6.f,
			// 	// Equations
			// 	torus_eqs, torus_normal_eqs);
			// addEntity(torus_volume);

			// // 3. Twisted Ellipsoid
			// std::map<std::string, double> twist_params = {
			// 	{"rx", 1.0},
			// 	{"ry", 0.4},
			// 	{"rz", 0.7}, // Ellipsoid radii
			// 	{"k", 0.8} // Twist factor (radians per unit of v)
			// };
			// auto twist_volume = std::make_shared<zg::entities::NUVVolume<3, float>>(
			// 	window, *this, glm::vec3(47, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1),
			// 	glm::vec4(0.0, 0.0, 1.0, 1.0), // Blue color
			// 	commonShaderConstants, "Twisted Ellipsoid Volume", twist_params,
			// 	// U range: [-pi, pi]
			// 	-_pi_, _pi_, _pi_ / 4.f,
			// 	// V range: [-pi/2, pi/2]
			// 	-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 4.f,
			// 	// Equations - applying rotation based on v
			// 	"(rx * cos(v) * cos(u)) * cos(k*v) - (ry * cos(v) * sin(u)) * sin(k*v)", // Rotated x
			// 	"(rx * cos(v) * cos(u)) * sin(k*v) + (ry * cos(v) * sin(u)) * cos(k*v)", // Rotated y
			// 	"rz * sin(v)" // Original z
			// );
			// addEntity(twist_volume);

			// // 4. Spiral Tube
			// // Note: Requires a larger v range (e.g., [-2*pi, 2*pi]) for multiple turns
			// std::map<std::string, double> spiral_params = {
			// 	{"R_base", 2.0}, // Starting major radius
			// 	{"r", 1.5}, // Tube radius
			// 	{"spiral_factor", 1.0}, // How much R grows per unit of v
			// 	{"height_factor", 2.0}, // How much z increases per unit of v
			// 	{"n", 8.0} // Frequency multiplier for cross-section rotation (aesthetic)
			// };
			// auto spiral_volume = std::make_shared<zg::entities::NUVVolume<3, float>>(
			// 	window, *this, glm::vec3(44, 43, 53), glm::quat(1, 0, 0, 0), glm::vec3(1),
			// 	glm::vec4(1.0, 1.0, 0.0, 1.0), // Yellow color
			// 	commonShaderConstants, "Spiral Tube Volume", spiral_params,
			// 	// U range: [-pi, pi] (around the tube cross-section)
			// 	-_pi_, _pi_, _pi_ / 8.f,
			// 	// V range: [-2*pi, 2*pi] (path of the spiral - adjust for length)
			// 	// *** Adjust V range in constructor call for desired spiral length ***
			// 	-2.0f * _pi_, 2.0f * _pi_, _pi_ / 16.f,
			// 	// Equations
			// 	"(R_base + spiral_factor * v + r * cos(u)) * cos(n*v)", // x: spiral path + tube offset, rotated
			// 	"(R_base + spiral_factor * v + r * cos(u)) * sin(n*v)", // y: spiral path + tube offset, rotated
			// 	"height_factor * v + r * sin(u)" // z: height + tube offset
			// );
			// addEntity(spiral_volume);


			// // 5. "Star" Shape (Rippled Sphere)
			// std::map<std::string, double> star_params = {
			// 	{"base_r", 6.0}, // Average radius
			// 	{"amp", 1.5}, // Amplitude of ripples
			// 	{"n", 5.0}, // Frequency of ripples along u (longitude) - integer for closed loops
			// 	{"m", 4.0} // Frequency of ripples along v (latitude) - integer for closed loops
			// };
			// auto star_volume = std::make_shared<zg::entities::NUVVolume<3, float>>(
			// 	window, *this, glm::vec3(50, 43, 49), glm::quat(1, 0, 0, 0), glm::vec3(1),
			// 	glm::vec4(1.0, 0.0, 1.0, 1.0), // Magenta color
			// 	commonShaderConstants, "Star Volume", star_params,
			// 	// U range: [-pi, pi]
			// 	-_pi_, _pi_, _pi_ / 16.f,
			// 	// V range: [-pi/2, pi/2]
			// 	-(_pi_ / 2.f), (_pi_ / 2.f), _pi_ / 16.f,
			// 	// Equations - Modulate radius based on u and v
			// 	"(base_r * (1.0 + amp * sin(n * u) * sin(m * v))) * cos(v) * cos(u)", // x = modulated_radius * sphere_x
			// 	"(base_r * (1.0 + amp * sin(n * u) * sin(m * v))) * cos(v) * sin(u)", // y = modulated_radius * sphere_y
			// 	"(base_r * (1.0 + amp * sin(n * u) * sin(m * v))) * sin(v)" // z = modulated_radius * sphere_z
			// );
			// addEntity(star_volume);

			// // 6. Apple Shape (Rotated Cardioid-like)
			// // Note: Requires v range [-pi, pi] or [0, 2*pi] for the full cardioid profile
			// std::map<std::string, double> apple_params = {
			// 	{"a", 2.0} // Scale factor for the cardioid profile
			// };
			// auto apple_volume = std::make_shared<zg::entities::NUVVolume<3, float>>(
			// 	window, *this, glm::vec3(50, 43, 46), glm::quat(1, 0, 0, 0), glm::vec3(1),
			// 	glm::vec4(0.0, 1.0, 1.0, 1.0), // Cyan color
			// 	commonShaderConstants, "Apple Volume", apple_params,
			// 	// U range: [-pi, pi] (rotation around Z axis)
			// 	-_pi_, _pi_, _pi_ / 16.f,
			// 	// V range: [-pi, pi] (angle for the cardioid profile)
			// 	// *** Adjust V range in constructor call if needed for full shape ***
			// 	-_pi_, _pi_, _pi_ / 16.f,
			// 	// Equations - Generate 2D cardioid in XZ cube5 (using v) and revolve around Z (using u)
			// 	"(a * (1.0 - cos(v))) * cos(v) * cos(u)", // x = (profile_radius) * profile_x_component * cos(u)
			// 	"(a * (1.0 - cos(v))) * cos(v) * sin(u)", // y = (profile_radius) * profile_x_component * sin(u)
			// 	"(a * (1.0 - cos(v))) * sin(v)" // z = (profile_radius) * profile_z_component
			// );
			// addEntity(apple_volume);
		}
		// frame
		{
			auto angle = glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
			frame = std::make_shared<zg::entities::Frame>(
				window, *this, glm::vec3(55, 50, 58), angle, glm::vec3(1), glm::vec2(5, 5), 0.1f, glm::vec4(1, 1, 1, 1), commonShaderConstants,
				"Window Curve Frame");
			frameID = addEntity(frame);
		}
		// cube controls
		{
			f = KEYCODE_UP;
			b = KEYCODE_DOWN;
			l = KEYCODE_LEFT;
			r = KEYCODE_RIGHT;
			s = 32;
			std::function<void()> onFrontTickFunction = [&]() { cubeRigidBody->applyLocalForceToCenter({0, 0, 30}); };
			std::function<void()> onBackTickFunction = [&]() { cubeRigidBody->applyLocalForceToCenter({0, 0, -30}); };
			std::function<void()> onLeftTickFunction = [&]() { cubeRigidBody->applyLocalForceToCenter({30, 0, 0}); };
			std::function<void()> onRightTickFunction = [&]() { cubeRigidBody->applyLocalForceToCenter({-30, 0, 0}); };
			std::function<void()> onSpaceTickFunction = [&]()
			{
				zg::physics::CollisionManifold* ManifoldPointer = 0;
				if (cubeRigidBody->isTouching(*floorRigidBody, ManifoldPointer) ||
						cubeRigidBody->isTouching(*cube2RigidBody, ManifoldPointer) ||
						cubeRigidBody->isTouching(*cube3RigidBody, ManifoldPointer) ||
						cubeRigidBody->isTouching(*cube4RigidBody, ManifoldPointer))
				{
					cubeRigidBody->applyLocalForceToCenter({0, 500, 0});
				}
			};
			//
			fID = window.addKeyUpdateHandler(f, onFrontTickFunction);
			bID = window.addKeyUpdateHandler(b, onBackTickFunction);
			lID = window.addKeyUpdateHandler(l, onLeftTickFunction);
			rID = window.addKeyUpdateHandler(r, onRightTickFunction);
			sID = window.addKeyUpdateHandler(s, onSpaceTickFunction);
		}
		//
		thirdPerson = std::make_shared<zg::components::scenes::EntityThirdPersonCamera>(*this, *cube);
		addComponent(thirdPerson);
	}
	void preUpdate() override
	{
		// curves (hehe)
		{
			if (!curve1ID)
			{
				curve1Points.resize(50);
				auto angle = glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
				curve1 = std::make_shared<zg::entities::NDParametricCurve<2, float>>(
					window, *this, glm::vec3(55, 50, 58), angle, glm::vec3(1), glm::vec4(1, 1, 1, 1), commonShaderConstants,
					"Window Curve", 0.1f, std::map<std::string, double>(), curve1Points);
				curve1ID = addEntity(curve1);
			}
			static constexpr float step = 0.1f;
			auto curve1PointsData = curve1Points.data();
			if (curve1PointsIndex >= 50)
			{
				curve1Points.erase(curve1Points.begin());
				for (auto i = 0; i < curve1PointsIndex; i++)
				{
					auto& p = curve1PointsData[i];
					p.x -= step;
				}
				curve1Points.push_back({});
				curve1PointsIndex--;
			}
			float lastFrameDelta = window.lastFrameDeltaTime;
			auto lastX = curve1PointsIndex ? curve1Points[curve1PointsIndex - 1].x : -2.5;
			auto thisY = (std::min)(1.0f, (float)window.deltaTime / lastFrameDelta);
			std::cout << "lastFrameDelta: " << lastFrameDelta << ", deltaTime: " << window.deltaTime << ", thisY: " << thisY << std::endl;
			auto point = glm::vec2(lastX + step, thisY);
			curve1PointsData[curve1PointsIndex++] = point;
			curve1->generateAndUpdateCurve(curve1Points);
		}
	}
	~PhysicsScene()
	{
		window.removeKeyUpdateHandler(f, fID);
		window.removeKeyUpdateHandler(b, bID);
		window.removeKeyUpdateHandler(l, lID);
		window.removeKeyUpdateHandler(r, rID);
	}
};
int main()
{
	zg::Window window("Physics Test", 1920, 1080, -1, -1, true, false, 50);
	window.runOnThread([](auto& window) { window.setScene(std::make_shared<PhysicsScene>(window)); });
	window.addKeyPressHandler(27,
														[&](auto pressed)
														{
															if (pressed)
																window.close();
														});
	window.run();
	return 0;
}
