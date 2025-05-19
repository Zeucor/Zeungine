#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Curve.hpp>
#include <zg/Registry.hpp>
#define PI acos(-1)
using namespace zg;
// struct CurveScene : zg::Scene
// {
// 	// std::shared_ptr<zg::entities::NDParametricCurve<2>> circle;
// 	// std::shared_ptr<zg::entities::NDParametricCurve<2>> spiral;
// 	// std::shared_ptr<zg::entities::NDParametricCurve<3>> helix;
// 	// std::shared_ptr<zg::entities::NDParametricCurve<3>> viviani;
// 	// std::shared_ptr<zg::entities::NDParametricCurve<3>> fibspiral;
// 	// std::shared_ptr<zg::entities::NDParametricCurve<3>> butterfly;
// 	zg::vp::VML vml;
// 	zg::vp::VFBLR vfblr;
// 	CurveScene(zg::Window& window) :
// 			Scene(window, {1, 1, 1}, glm::normalize(glm::vec3(0, 0, 1)), 80.f), vml(*this),
// 			vfblr(*this, zg::vp::VFBLR::WSADSC, 2)
// 	{
// 		// circle = std::make_shared<zg::entities::NDParametricCurve<2>>(
// 		// 	window, *this, glm::vec3(1, 0, 2), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1), zg::shaders::RuntimeConstants(), "", 0.5f, std::map<std::string, double>(), -PI,
// 		// 	PI + (PI / 20.f), PI / 20.f, "cos(t)", "sin(t)");
// 		// addEntity(circle);
// 		// spiral = std::make_shared<zg::entities::NDParametricCurve<2>>(
// 		// 	window, *this, glm::vec3(6, 0, 2), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1), zg::shaders::RuntimeConstants(), "", 0.5f, std::map<std::string, double>(), -PI,
// 		// 	PI, PI / 20.f, "t * cos(t)", "t * sin(t)");
// 		// addEntity(spiral);
// 		// helix = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 		// 	window, *this, glm::vec3(12, 0, 2), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1), zg::shaders::RuntimeConstants(), "", 0.5f, std::map<std::string, double>(),
// 		// 	-PI, PI, PI / 20.f, "cos(t)", "sin(t)", "t");
// 		// addEntity(helix);
// 		// viviani = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 		// 	window, *this, glm::vec3(-8, 0, 2), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1), zg::shaders::RuntimeConstants(), "", 0.5f, std::map<std::string, double>(),
// 		// 	-PI * 3, PI * 3, PI / 20.f, "(1 + cos(t))", "sin(t)", "2 * sin(t/2)");
// 		// addEntity(viviani);
// 		// fibspiral = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 		// 	window, *this, glm::vec3(-4, 0, -5), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1), zg::shaders::RuntimeConstants(), "", 0.5f, std::map<std::string, double>(),
// 		// 	-PI * 3, PI * 3, PI / 20.f, "cos(t) * exp(0.1 * t)", "sin(t) * exp(0.1 * t)", "0.1 * t");
// 		// addEntity(fibspiral);
// 		// butterfly = std::make_shared<zg::entities::NDParametricCurve<3>>(
// 		// 	window, *this, glm::vec3(-2, 0, -10), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec4(1), zg::shaders::RuntimeConstants(), "", 0.5f, std::map<std::string, double>(),
// 		// 	-PI * 3, PI * 3, PI / 20.f, "sin(t) * (exp(cos(t)) - 2 * cos(4 * t) - pow(sin(t / 12), 5))",
// 		// 	"cos(t) * (exp(cos(t)) - 2 * cos(4 * t) - pow(sin(t / 12), 5))", "sin(t) * cos(t)");
// 		// addEntity(butterfly);
// 		return;
// 	}
// };
int main()
{
	Registry registry;
	// zg::Window window("Curve Test", 640, 480, -1, -1);
	// window.runOnThread([](auto& window) { window.setScene(std::make_shared<CurveScene>(window)); });
	// window.run();
}
