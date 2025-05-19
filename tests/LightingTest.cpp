#include <array>
#include <zg/Entity.hpp>
#include <zg/Logger.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/crypto/Random.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/entities/SkyBox.hpp>
#include <zg/images/ImageLoader.hpp>
#include <zg/lights/DirectionalLight.hpp>
#include <zg/lights/Lights.hpp>
#include <zg/math/Rotations.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Registry.hpp>
using namespace zg;
using namespace zg::shaders;
// #define PI acos(-1)
// glm::vec3 centerPos = glm::vec3(1024.f) * 8.0f;
// float angleStep = PI * 2.0f / 8.0f; // Place 8 curves per rotation
// float radiusScale = 7.0f; // How much the radius increases per step
// float currentAngle = 0.0f;
// float currentRadius = 0.0f; // Start at the center for the very first one slightly offset
// float zOffsetStep = 2.0f; // Slightly change Z for each curve
// RuntimeConstants commonShaderConstants({"Lighting", "DirectionalLightShadowMaps", "LightSpacePosition",
// 																								 "Fog"});
// float commonLineThickness = 0.5f;
// float commonDt = PI / 64.f; // Default step size (can be overridden)
// struct TestTriangle;
// struct TestScene : Scene
// {
// 	std::shared_ptr<vp::VML> vml;
// 	std::shared_ptr<vp::VFBLR> vfblr;
// 	// std::shared_ptr<zg::entities::NDParametricCurve<3>> sun;
// 	// std::shared_ptr<entities::Cube> cube;
// 	explicit TestScene(Window& _window);
// 	~TestScene();
// 	void prePreRender() override
// 	{
// 		auto& directionalLight = directionalLights[0];
// 		float angle = (2 * window.deltaTime) * 8;
// 		directionalLight.position =
// 			zg::math::Rotations::Vec3AroundVec3(directionalLight.position, centerPos, {0, 0, angle});
// 		directionalLight.direction =
// 			zg::math::Rotations::Vec3AroundVec3(directionalLight.direction, {0, 0, 0}, {0, 0, -angle});
// 		// sun->position = directionalLight.position;
// 		// auto valpi = cos(PI * 1) * 0.5 + 0.5;
// 		auto val = cos(PI * glm::clamp<float>((directionalLight.direction.y + 1), 0, 1)) * 0.5 + 0.5;
// 		clearColor = {val, val, val, 1};
// 		// glm::normalize((glm::vec3(8) * 8.f) - directionalLight.position);
// 		// // directionalLight.lookAt = zg::math::Rotations::Vec3AroundVec3(directionalLight.direction, glm::quat(1, 0, 0, 0), {0,
// 		// // angle, 0});
// 		// std::cout << "Position: " << glm::to_string(directionalLight.position)
// 		//        << ", Direction: " << glm::to_string(directionalLight.direction) << std::endl;
// 		directionalLightShadows[0].update();
// 	}
// };
// auto calculatePosition = [](int index)
// {
// 	if (index == 0)
// 	{ // Special case for the first one near the center
// 		return centerPos + glm::vec3(5.f, 0.f, 0.f); // Small offset from true center
// 	}
// 	currentAngle = index * angleStep;
// 	currentRadius = index * radiusScale;
// 	return centerPos +
// 		glm::vec3(currentRadius * cos(currentAngle), index * zOffsetStep, currentRadius * sin(currentAngle));
// };
// TestScene::TestScene(Window& window) :
// 		Scene(
// 			window, centerPos + glm::vec3(10, 10, 10), glm::normalize(glm::vec3(-1, -1, -1)),
// 			81.f /*, {{textures::Framebuffer::AttachmentType::Color|*, textures::Framebuffer::AttachmentType::Depth*|}}*/)
// {
// 	std::ifstream sceneFile(zgfilesystem::File::getProgramDirectoryPath() / "lighting.scene", std::ios::binary);
// 	Serial sceneSerial(sceneFile);
// 	sceneSerial >> *this;
// 	// if (sun)
// 	// {
// 	// 	goto _hooks;
// 	// }
// 	// {
// 	// 	vml = std::make_shared<zg::vp::VML>(*this);
// 	// 	vfblr = std::make_shared<zg::vp::VFBLR>(*this, vp::VFBLR::WSADSC, 8.f);
// 	// 	auto dldirection = glm::normalize(zg::math::Rotations::Vec3AroundVec3({0, 0, 1}, {0, 0, 0}, {90, 0, 0}));
// 	// 	auto dlup = glm::normalize(zg::math::Rotations::Vec3AroundVec3({0, 1, 0}, {0, 0, 0}, {90, 0, 0}));
// 	// 	directionalLights.push_back({
// 	// 		centerPos + glm::vec3(0, 72, 0), // position
// 	// 		dldirection,
// 	// 		dlup,
// 	// 		{1, 1, 1}, // color
// 	// 		1.f, // intensity,
// 	// 		1.f, // nearPlane
// 	// 		364.f, // farPlane
//     //         0.2f // ambientFactor
// 	// 	});
// 	// 	auto& dl = directionalLights[0];
// 	// 	// dls.update();
// 	// 	directionalLightShadows.emplace_back(window, directionalLights[0]);
// 	// 	sun = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 		window, *this, dl.position, glm::quat(1, 0, 0, 0), glm::vec3(4.3), glm::vec4(0.944, 1.00, 0.760, 1),
// 	// 		RuntimeConstants(), "Sun", commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt,
// 	// 		"cos(8*t) * cos(t)", // Combination of angular frequencies
// 	// 		"sin(8*t) * cos(t)",
// 	// 		"sin(t)" // Controls latitude based on main angle t
// 	// 	);
// 	// 	sun->affectedByShadows = false;
// 	// 	addEntity(sun);

// 	// 	using namespace zg::crypto;
// 	// 	for (char j = 0; j < 3; j++)
// 	// 	{
// 	// 		char rand = Random::value<short>(1, 30);
// 	// 		// --- Curve Instantiations ---

// 	// 		if (rand > 29)
// 	// 		{
// 	// 			// 1. Simple Helix
// 	// 			auto simpleHelix = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 4.0f * PI, commonDt, "2*cos(t)",
// 	// 				"2*sin(t)", "0.5*t");
// 	// 			addEntity(simpleHelix);
// 	// 		}
// 	// 		else if (rand > 28)
// 	// 		{
// 	// 			// 2. Conical Helix (Expanding Radius)
// 	// 			auto conicalHelix = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 8.0f * PI,
// 	// 				PI / 128.f, // Smaller dt for larger range
// 	// 				"0.5 * t * cos(t)", "0.5 * t * sin(t)", "0.5 * t");
// 	// 			addEntity(conicalHelix);
// 	// 		}
// 	// 		else if (rand > 27)
// 	// 		{
// 	// 			// 3. Toroidal Spiral (5 wraps minor / 1 wrap major)
// 	// 			auto toroidalSpiral1 = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt,
// 	// 				"(5 + 1.5 * cos(5 * t)) * cos(t)", "(5 + 1.5 * cos(5 * t)) * sin(t)", "1.5 * sin(5 * t)");
// 	// 			addEntity(toroidalSpiral1);
// 	// 		}
// 	// 		else if (rand > 26)
// 	// 		{
// 	// 			// 4. Trefoil Knot
// 	// 			auto trefoilKnot = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "sin(t) + 2 * sin(2 * t)",
// 	// 				"cos(t) - 2 * cos(2 * t)", "-sin(3 * t)");
// 	// 			addEntity(trefoilKnot);
// 	// 		}
// 	// 		else if (rand > 25)
// 	// 		{
// 	// 			// 5. Spherical Spiral (Loxodrome-like)
// 	// 			auto sphericalSpiral = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.01f, PI - 0.01f,
// 	// 				PI / 128.f, // Avoid poles t=0, t=PI if parser struggles
// 	// 				"sin(t) * cos(15 * t)", "sin(t) * sin(15 * t)", "cos(t)");
// 	// 			addEntity(sphericalSpiral);
// 	// 		}
// 	// 		else if (rand > 24)
// 	// 		{
// 	// 			// 6. Viviani's Curve (Sphere/Cylinder Intersection)
// 	// 			auto viviani = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 4.0f * PI, commonDt, "1 * (1 + cos(t))", "1 * sin(t)",
// 	// 				"2 * sin(t / 2)");
// 	// 			addEntity(viviani);
// 	// 		}
// 	// 		else if (rand > 23)
// 	// 		{
// 	// 			// 7. 3D Lissajous (Freq 1,2,3)
// 	// 			auto lissajous3D_123 = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "sin(t)",
// 	// 				"sin(2 * t + acos(-1) / 3)", "cos(3 * t)");
// 	// 			addEntity(lissajous3D_123);
// 	// 		}
// 	// 		else if (rand > 22)
// 	// 		{
// 	// 			// 8. "Butterfly" 3D
// 	// 			auto butterfly3D = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scale carefully
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 12.0f * PI, PI / 256.f, // Needs high resolution & range
// 	// 				"sin(t) * (exp(cos(t)) - 2 * cos(4 * t) - pow(sin(t / 12), 5))",
// 	// 				"cos(t) * (exp(cos(t)) - 2 * cos(4 * t) - pow(sin(t / 12), 5))",
// 	// 				"cos(2*t)" // Use cos for z variation
// 	// 			);
// 	// 			addEntity(butterfly3D);
// 	// 		}
// 	// 		else if (rand > 21)
// 	// 		{
// 	// 			// 9. Figure-Eight Knot (Listing's Knot)
// 	// 			auto figureEightKnot = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "(2 + cos(2 * t)) * cos(3 * t)",
// 	// 				"(2 + cos(2 * t)) * sin(3 * t)", "sin(4 * t)");
// 	// 			addEntity(figureEightKnot);
// 	// 		}
// 	// 		else if (rand > 20)
// 	// 		{
// 	// 			// 10. Elliptical Helix
// 	// 			auto ellipticalHelix = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 6.0f * PI, commonDt, "4 * cos(t)",
// 	// 				"2 * sin(t)",
// 	// 				"0.6 * t" // Different radii for x and y
// 	// 			);
// 	// 			addEntity(ellipticalHelix);
// 	// 		}
// 	// 		else if (rand > 19)
// 	// 		{
// 	// 			// 11. Toroidal Spiral (3 wraps minor / 7 wrap major) - Higher Frequencies
// 	// 			auto toroidalSpiral2 = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI,
// 	// 				PI / 128.f, // Needs more resolution
// 	// 				"(6 + 1 * cos(3 * t)) * cos(7 * t)", "(6 + 1 * cos(3 * t)) * sin(7 * t)", "1 * sin(3 * t)");
// 	// 			addEntity(toroidalSpiral2);
// 	// 		}
// 	// 		else if (rand > 18)
// 	// 		{
// 	// 			// 12. Archimedean Spiral Raised Cosine Wave
// 	// 			auto archimedeanRaised = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 10.0f * PI, PI / 128.f,
// 	// 				"0.2 * t * cos(t)", "0.2 * t * sin(t)",
// 	// 				"2 * cos(t)" // Spirals out while oscillating vertically
// 	// 			);
// 	// 			addEntity(archimedeanRaised);
// 	// 		}
// 	// 		else if (rand > 17)
// 	// 		{
// 	// 			// 13. Lemniscate of Gerono based curve (Figure 8 in XY plane, raised)
// 	// 			auto lemniscateGerono3D = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "sin(t)", "sin(t) * cos(t)",
// 	// 				"cos(2*t)" // z oscillates twice per loop
// 	// 			);
// 	// 			addEntity(lemniscateGerono3D);
// 	// 		}
// 	// 		else if (rand > 16)
// 	// 		{
// 	// 			// 14. Helix on a Cone Surface
// 	// 			auto helixOnCone = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.1f, 10.0f,
// 	// 				0.05f, // Using linear t range for simplicity
// 	// 				"t * cos(8 * acos(-1) * t / 10)", // x = r * cos(angle), r=t, angle proportional to t
// 	// 				"t * sin(8 * acos(-1) * t / 10)", // y = r * sin(angle)
// 	// 				"t" // z = r (cone z=sqrt(x^2+y^2))
// 	// 			);
// 	// 			addEntity(helixOnCone);
// 	// 		}
// 	// 		else if (rand > 15)
// 	// 		{
// 	// 			// 15. Astroid Curve based 3D shape
// 	// 			auto astroid3D = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "pow(cos(t), 3)", "pow(sin(t), 3)",
// 	// 				"cos(4*t)" // z oscillates 4 times
// 	// 			);
// 	// 			addEntity(astroid3D);
// 	// 		}
// 	// 		else if (rand > 14)
// 	// 		{
// 	// 			// 16. Cinquefoil Knot (5-petal)
// 	// 			auto cinquefoilKnot = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "(2 + cos(5 * t / 2)) * cos(t)",
// 	// 				"(2 + cos(5 * t / 2)) * sin(t)", "sin(5 * t / 2)");
// 	// 			addEntity(cinquefoilKnot);
// 	// 		}
// 	// 		else if (rand > 13)
// 	// 		{
// 	// 			// 17. Twisted Cubic Curve (Simple polynomial)
// 	// 			auto twistedCubic = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), -5.0f, 5.0f,
// 	// 				0.1f, // Polynomials often look good over symmetric range
// 	// 				"t", "t*t", "t*t*t");
// 	// 			addEntity(twistedCubic);
// 	// 		}
// 	// 		else if (rand > 12)
// 	// 		{
// 	// 			// 18. Lissajous 3D (Freq 3,2,1)
// 	// 			auto lissajous3D_321 = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "cos(3 * t)",
// 	// 				"cos(2 * t + acos(-1) / 2)", "sin(t)");
// 	// 			addEntity(lissajous3D_321);
// 	// 		}
// 	// 		else if (rand > 11)
// 	// 		{
// 	// 			// 19. Damped Oscillation Helix
// 	// 			auto dampedHelix = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 10.0f * PI, PI / 128.f,
// 	// 				"exp(-0.1*t) * 5 * cos(2*t)", // Radius decreases exponentially
// 	// 				"exp(-0.1*t) * 5 * sin(2*t)", "0.4 * t");
// 	// 			addEntity(dampedHelix);
// 	// 		}
// 	// 		else if (rand > 10)
// 	// 		{
// 	// 			// 20. Saddle Quadric Surface Curve (Hyperbolic Paraboloid)
// 	// 			auto saddleCurve = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), -2.0f * PI, 2.0f * PI, commonDt,
// 	// 				"2 * t * cos(t)", // X follows spiral-like path
// 	// 				"3 * t * sin(t)", // Y follows related spiral
// 	// 				"(2*t*cos(t))^2 / 4 - (3*t*sin(t))^2 / 9" // z = x^2/a^2 - y^2/b^2
// 	// 			);
// 	// 			addEntity(saddleCurve);
// 	// 		}
// 	// 		else if (rand > 9)
// 	// 		{
// 	// 			// 21. Bill's Curve (Example from Graphics Gems)
// 	// 			auto billsCurve = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 12.0f * PI, PI / 128.f, // Needs range
// 	// 				"cos(t) + 0.5 * cos(7 * t) + 0.3 * sin(17 * t)", "sin(t) + 0.5 * sin(7 * t) + 0.3 * cos(17 * t)",
// 	// 				"0.2 * sin(3 * t)");
// 	// 			addEntity(billsCurve);
// 	// 		}
// 	// 		else if (rand > 8)
// 	// 		{
// 	// 			// 22. Spiral on a Paraboloid
// 	// 			auto paraboloidSpiral = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 4.0f * PI, PI / 128.f,
// 	// 				"sqrt(t) * cos(4*t)", // Use sqrt(t) for radius to match z=x^2+y^2
// 	// 				"sqrt(t) * sin(4*t)",
// 	// 				"t" // z = r^2 = t
// 	// 			);
// 	// 			addEntity(paraboloidSpiral);
// 	// 		}
// 	// 		else if (rand > 7)
// 	// 		{
// 	// 			// 23. Dennis' Curve (Another Graphics Gems example)
// 	// 			auto dennisCurve = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 10.0f * PI, PI / 128.f,
// 	// 				"t * cos(t)", // X based on t
// 	// 				"t * sin(t)", // Y based on t
// 	// 				"sin(5 * t)" // Z oscillates
// 	// 			);
// 	// 			addEntity(dennisCurve);
// 	// 		}
// 	// 		else if (rand > 6)
// 	// 		{
// 	// 			// 24. Torus Knot (p=2, q=3) - Simpler than Trefoil formula
// 	// 			auto torusKnot23 = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "(2 + cos(3 * t)) * cos(2 * t)",
// 	// 				"(2 + cos(3 * t)) * sin(2 * t)", "sin(3 * t)");
// 	// 			addEntity(torusKnot23);
// 	// 		}
// 	// 		else if (rand > 5)
// 	// 		{
// 	// 			// 25. Twisted Heart Curve 3D
// 	// 			auto twistedHeart3D = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5), glm::vec4(0.9, 0.2, 0.2, 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt,
// 	// 				"16 * pow(sin(t), 3)", "13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t)",
// 	// 				"2 * sin(t+acos(-1)/2)" // Add simple z oscillation
// 	// 			);
// 	// 			addEntity(twistedHeart3D);
// 	// 		}
// 	// 		else if (rand > 4)
// 	// 		{
// 	// 			// 26. Sine Wave on Sphere Surface
// 	// 			auto sphereSineWave = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt,
// 	// 				"cos(8*t) * cos(t)", // Combination of angular frequencies
// 	// 				"sin(8*t) * cos(t)",
// 	// 				"sin(t)" // Controls latitude based on main angle t
// 	// 			);
// 	// 			addEntity(sphereSineWave);
// 	// 		}
// 	// 		else if (rand > 3)
// 	// 		{
// 	// 			// 27. Logarithmic Spiral Raised Sine Wave
// 	// 			auto logSpiralRaised = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 6.0f * PI, PI / 128.f,
// 	// 				"0.5 * exp(0.15*t) * cos(t)", // Radius grows exponentially
// 	// 				"0.5 * exp(0.15*t) * sin(t)",
// 	// 				"3 * sin(t)" // Vertical oscillation
// 	// 			);
// 	// 			addEntity(logSpiralRaised);
// 	// 		}
// 	// 		else if (rand > 2)
// 	// 		{
// 	// 			// 28. Hypotrochoid based 3D curve
// 	// 			auto hypotrochoid3D = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", commonLineThickness, std::map<std::string, double>(), 0.0f, 10.0f * PI,
// 	// 				PI / 128.f, // Needs range for inner loops
// 	// 				"(5 - 2) * cos(t) + 3 * cos((5 - 2) / 2.0 * t)", // R=5, r=2, d=3
// 	// 				"(5 - 2) * sin(t) - 3 * sin((5 - 2) / 2.0 * t)",
// 	// 				"cos(3*t)" // Z oscillation
// 	// 			);
// 	// 			addEntity(hypotrochoid3D);
// 	// 		}
// 	// 		else if (rand > 1)
// 	// 		{
// 	// 			// 29. Lissajous 3D (Higher Frequencies 4,5,6)
// 	// 			auto lissajous3D_456 = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, PI / 128.f, // More resolution needed
// 	// 				"cos(4 * t)", "sin(5 * t + acos(-1) / 4)", "cos(6 * t)");
// 	// 			addEntity(lissajous3D_456);
// 	// 		}
// 	// 		else if (rand > 0)
// 	// 		{
// 	// 			// 30. Möbius Strip inspired curve (Not a surface, but traces path)
// 	// 			auto mobiusPath = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 	// 				window, *this, calculatePosition(j), glm::quat(1, 0, 0, 0), glm::vec3(0.5),
// 	// 				glm::vec4(Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), Random::value<float>(0.2, 0.9), 1),
// 	// 				commonShaderConstants, "", // Scaled up
// 	// 				commonLineThickness, std::map<std::string, double>(), 0.0f, 2.0f * PI, commonDt, "(1 + 0.5 * cos(t / 2.0)) * cos(t)",
// 	// 				"(1 + 0.5 * cos(t / 2.0)) * sin(t)",
// 	// 				"0.5 * sin(t / 2.0)" // Parameterization of center line of a Möbius strip
// 	// 			);
// 	// 			addEntity(mobiusPath);
// 	// 		}
// 	// 	}

// 	// 	cube = std::make_shared<entities::Cube>((Window&)window, *this, centerPos + glm::vec3(0, 2, 0), // position
// 	// 																					glm::vec3(0, 0, 0), // rotation
// 	// 																					glm::vec3(1, 1, 1), // scale
// 	// 																					glm::vec3(6.7, .9, 11.8), // size
// 	// 																					RuntimeConstants({"Lighting", "DirectionalLightShadowMaps",
// 	// 																																		 //  "SpotLightShadowMaps",
// 	// 																																		 //  "PointLightShadowMaps",
// 	// 																																		 "LightSpacePosition", "Fog"}));
// 	// 	addEntity(cube);

// 	// 	addEntity(std::make_shared<entities::Cube>((Window&)window, *this, centerPos, glm::vec3(0, 0, 0),
// 	// 																						 glm::vec3(1, 1, 1), glm::vec3(16 * 8, 1.3, 32 * 8),
// 	// 																						 RuntimeConstants({"Lighting", "DirectionalLightShadowMaps",
// 	// 																																				// "SpotLightShadowMaps",
// 	// 																																				// "PointLightShadowMaps",
// 	// 																																				"LightSpacePosition", "Fog"})));
// 	// 	// auto &shader = cube->addedShader();
// 	// 	// rotateLightPosition(window, *this, pointLights[0], pointLightShadows[0], glm::radians(180.0f), *shader);
// 	// 	// shader->bind(*this);
// 	// 	// shader->setUniform("fogDensity", 0.035f);
// 	// 	// shader->setUniform("fogColor", glm::vec4(0, 0, 0, 1));
// 	// 	// shader->unbind();
// 	// 	// auto programDir = zgfilesystem::File::getProgramDirectoryPath();
// 	// 	// auto skybox = std::make_shared<entities::SkyBox>(
// 	// 	//     (Window &)window,
// 	// 	//     *this,
// 	// 	//     std::vector<std::string_view>({(programDir / "images" / "skybox" / "right.jpg").string(), (programDir /
// 	// 	//     "images" / "skybox" / "left.jpg").string(), (programDir / "images" / "skybox" / "top.jpg").string(),
// 	// 	//                                    (programDir / "images" / "skybox" / "bottom.jpg").string(), (programDir /
// 	// 	//                                    "images" / "skybox" / "front.jpg").string(), (programDir / "images" / "skybox"
// 	// 	//                                    / "back.jpg").string()}));
// 	// 	// addEntity(skybox);
// 	// 	// pointLights.push_back({{5, 10, 0},
// 	// 	//                        {1, 1, 1},
// 	// 	//                        1.0,
// 	// 	//                        10000,
// 	// 	//                        1.f,
// 	// 	//                        250.f});
// 	// 	// pointLightShadows.emplace_back(window, pointLights[0]);
// 	// 	// dls.lookAtSet = true;
// 	// 	// dls.lookAt = glm::vec3(8,8,8) * 8.0f;
// 	// 	// dls.update();
// 	// 	// spotLights.push_back({{0, 25, -20},                        // position
// 	// 	//                       glm::normalize(glm::vec3(0, -1, 1)), // direction
// 	// 	//                       {0.0f, 0.0f, 1.0f},                  // color
// 	// 	//                       1.0f,                                // intensity
// 	// 	//                       glm::cos(glm::radians(25.0f)),       // cutoff
// 	// 	//                       glm::cos(glm::radians(50.0f)),       // outerCutoff
// 	// 	//                       1.f,
// 	// 	//                       250.f});
// 	// 	// spotLightShadows.emplace_back(window, spotLights[0]);
// 	// }
// _hooks:
// 	window.addKeyUpdateHandler(20,
// 														 [&]()
// 														 {
// 															 viewPointer->position.x -= 1.f * window.deltaTime;
// 															 viewPointer->update();
// 														 });
// 	window.addKeyUpdateHandler(19,
// 														 [&]()
// 														 {
// 															 viewPointer->position.x += 1.f * window.deltaTime;
// 															 viewPointer->update();
// 														 });
// 	window.addKeyUpdateHandler(17,
// 														 [&]()
// 														 {
// 															 viewPointer->position.y += 1.f * window.deltaTime;
// 															 viewPointer->update();
// 														 });
// 	window.addKeyUpdateHandler(18,
// 														 [&]()
// 														 {
// 															 viewPointer->position.y -= 1.f * window.deltaTime;
// 															 viewPointer->update();
// 														 });
// 	window.addKeyPressHandler(27,
// 														[&](const auto& pressed)
// 														{
// 															if (!pressed)
// 																window.close();
// 														});
// };
// TestScene::~TestScene()
// {
// 	std::ofstream sceneFile(zgfilesystem::File::getProgramDirectoryPath() / "lighting.scene", std::ios::binary);
// 	Serial sceneSerial(sceneFile);
// 	sceneSerial << *this;
// }
int main()
{
	Registry registry;
	ShaderFactory shader_factory;
	register_zg_shader_hooks();
	// Window window("Window", 1280, 720, -1, -1);
	// window.runOnThread([](auto& window) { window.setScene(std::make_shared<TestScene>(window)); });
	// window.run();
};

// template <>
// Serial& deserialize(Serial& serial, TestScene& scene)
// {
// 	serial.setContextPointer("Window", &scene.window);
// 	serial.setContextPointer("Scene", &scene);
// 	serial >> scene.vml;
// 	serial >> scene.vfblr;
// 	serial >> (zg::Scene&)scene;
// 	// scene.sun = std::dynamic_pointer_cast<zg::entities::NDParametricCurve<3>>(scene.getEntityByName("Sun"));
// 	return serial;
// }
// template <>
// Serial& serialize(Serial& serial, const TestScene& scene)
// {
// 	serial << scene.vml;
// 	serial << scene.vfblr;
// 	serial << (const zg::Scene&)scene;
// 	return serial;
// }
