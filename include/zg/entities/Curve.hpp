#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/glm.hpp>
#include <zg/math/MER.hpp>
#include <zg/utilities.hpp>
#include <zg/Registry.hpp>
namespace zg::entities
{
	namespace NDCurve
	{
		template <size_t N>
		size_t calculateCentralPointsCount(const std::vector<glm::vec<N, float>>& points);
		size_t calculateCentralPointsCount(double tStart, double tEnd, double tStep);
		template <size_t N>
		size_t getIndiceCount(size_t numPoints);
		template <size_t N>
		size_t getVertexCount(size_t numPoints);
		template <size_t N>
		size_t getIndiceCount(double tStart, double tEnd, double tStep);
		template <size_t N>
		size_t getVertexCount(double tStart, double tEnd, double tStep);
		template <size_t N>
		uint32_t getIndiceCount(const std::vector<glm::vec<N, float>>& points);
		template <size_t N>
		uint32_t getVertexCount(const std::vector<glm::vec<N, float>>& points);
		template <size_t N>
		std::vector<glm::vec3> getVertices(const std::vector<glm::vec<N, float>>& centralPoints, float radius, zg::FRONTFACE frontFace);
		template <size_t N>
		std::vector<uint32_t> getIndices(const std::vector<glm::vec<N, float>>& centralPoints, zg::FRONTFACE frontFace);
		// glm::vec<N, float> solveForT(double t)
		// {
		// 	*t_p = t;
		// 	size_t index = 0;
		// 	glm::vec<N, float> vec;
		// 	for (auto& equation : equations)
		// 		vec[index++] = zg::math::MathematicalEquationResolver::solve(equation, vars);
		// 	return vec;
		// }
	}

	template <size_t N = 3>
	EntityCreateInfo NDParametricCurveFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
													 const shaders::RuntimeConstants& constants, const std::string& name, float radius,
													 const std::vector<glm::vec<N, float>>& points, zg::FRONTFACE frontFace = IRenderer::DEFAULTFRONTFACE);
	template <size_t N = 3, typename... Args>
	EntityCreateInfo NDParametricCurveFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
													 const shaders::RuntimeConstants& constants, const std::string& name, float radius,
													 const std::map<std::string, double>& vars, double tStart, double tEnd, double tStep,
													 const std::string& t_equation, const Args&... args);
	template <size_t N = 3>
	EntityCreateInfo NDParametricCurveFactory(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
													 const shaders::RuntimeConstants& constants, const std::string& name, float radius,
													 const std::map<std::string, double>& vars, double tStart, double tEnd, double tStep,
													 const std::array<std::string, N>& equations);
	// struct NDParametricCurve : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<NDParametricCurve<N, float>>::id; }

	// private:
	// 	inline static size_t curvesCount = 0;

	// public:
	// 	Scene* scenePointer = 0;
	// 	glm::vec4 color = glm::vec4(1);
	// 	float radius = 0.75;
	// 	double tStart;
	// 	double tEnd;
	// 	double tStep;
	// 	std::array<std::string, N> equations;
	// 	std::vector<glm::vec4> colors;
	// 	std::vector<glm::vec3> normals = {};
	// 	NDParametricCurve(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
	// 										glm::vec4 color, const shaders::RuntimeConstants& constants, std::string_view name, float
	// radius, 										const std::map<std::string, double>& vars, const std::vector<glm::vec<N, float>>& points) : 			Entity(window,
	// scene, 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(points), {}, getVertexCount(points), {}, position, rotation, scale,
	// 						 (!name.empty()) ? name : ("Curve " + std::to_string(++curvesCount))),
	// 			scenePointer(&scene), color(color), radius(radius), vars(vars), tStart(tStart), tEnd(tEnd), tStep(tStep)
	// 	{
	// 		t_p = &this->vars["t"];
	// 		generateAndUpdateCurve(points);
	// 	}
	// 	template <typename... Args>
	// 	NDParametricCurve(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
	// 										glm::vec4 color, const shaders::RuntimeConstants& constants, std::string_view name, float
	// radius, 										const std::map<std::string, double>& vars, double tStart, double tEnd, double tStep, 										const std::string&
	// t_equation, const Args&... args) : 			Entity(window, scene, 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(tStart, tEnd, tStep), {}, getVertexCount(tStart, tEnd, tStep), {}, position, rotation,
	// 						 scale, (!name.empty()) ? name : ("Curve " + std::to_string(++curvesCount))),
	// 			scenePointer(&scene), color(color), radius(radius), vars(vars), tStart(tStart), tEnd(tEnd), tStep(tStep)
	// 	{
	// 		t_p = &this->vars["t"];
	// 		size_t index = 0;
	// 		addEquations(index, t_equation, args...);
	// 		generateAndUpdateCurve(tStart, tEnd, tStep);
	// 	}
	// 	NDParametricCurve(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
	// 										glm::vec4 color, const shaders::RuntimeConstants& constants, std::string_view name, float
	// radius, 										const std::map<std::string, double>& vars, double tStart, double tEnd, double tStep, 										const
	// std::array<std::string, N>& equations) : 			Entity(window, scene, 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(tStart, tEnd, tStep), {}, getVertexCount(tStart, tEnd, tStep), {}, position, rotation,
	// 						 scale, (!name.empty()) ? name : ("Curve " + std::to_string(++curvesCount))),
	// 			scenePointer(&scene), color(color), radius(radius), vars(vars), tStart(tStart), tEnd(tEnd), tStep(tStep),
	// 			equations(equations)
	// 	{
	// 		t_p = &this->vars["t"];
	// 		generateAndUpdateCurve(tStart, tEnd, tStep);
	// 	}

	// 	// --- Count and other methods remain the same as user provided ---


	// 	// --- generateAndUpdateCurve using Rotation Minimizing Frame (Double Reflection) ---
	// 	void generateAndUpdateCurve(double tStart, double tEnd, double tStep)
	// 	{
	// 		std::vector<glm::vec<N, float>> centralPoints;
	// 		std::vector<double> t_values; // Store t values if needed for debug

	// 		// Pre-calculate central points
	// 		const double loop_eps = tStep * 0.001;
	// 		for (double t = tStart; t <= tEnd + loop_eps; t += tStep)
	// 		{
	// 			centralPoints.push_back(solveForT(t));
	// 			t_values.push_back(t);
	// 		}

	// 		generateAndUpdateCurve(centralPoints);
	// 	} // End generateAndUpdateCurve metho

	// 	void setColor(glm::vec4 color) { updateElements("Color", colors); }

	// private:
	// 	template <typename... Args>
	// 	void addEquations(size_t index, const std::string& t_equation, const Args&... args)
	// 	{
	// 		equations[index] = t_equation;
	// 		addEquations(++index, args...);
	// 	}

	// 	void addEquations(size_t index) {}
	// };
} // namespace zg::entities
