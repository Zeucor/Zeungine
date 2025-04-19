#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/glm.hpp>
#include <zg/math/MER.hpp>
#include <zg/utilities.hpp>

namespace zg::entities
{
	template <size_t N = 3, typename Real = float>
	EntityCreateInfo NDParametricCurveFactory(zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
													 const shaders::RuntimeConstants& constants, std::string_view name, float radius,
													 const std::map<std::string, double>& vars, const std::vector<glm::vec<N, Real>>& points);
	template <size_t N = 3, typename Real = float, typename... Args>
	EntityCreateInfo NDParametricCurveFactory(zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
													 const shaders::RuntimeConstants& constants, std::string_view name, float radius,
													 const std::map<std::string, double>& vars, double tStart, double tEnd, double tStep,
													 const std::string& t_equation, const Args&... args);
	template <size_t N = 3, typename Real = float>
	EntityCreateInfo NDParametricCurveFactory(zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color,
													 const shaders::RuntimeConstants& constants, std::string_view name, float radius,
													 const std::map<std::string, double>& vars, double tStart, double tEnd, double tStep,
													 const std::array<std::string, N>& equations);
	// struct NDParametricCurve : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<NDParametricCurve<N, Real>>::id; }

	// private:
	// 	inline static size_t curvesCount = 0;

	// public:
	// 	Scene* scenePointer = 0;
	// 	glm::vec4 color = glm::vec4(1);
	// 	float radius = 0.75;
	// 	std::map<std::string, double> vars = {{"t", 0}};
	// 	double tStart;
	// 	double tEnd;
	// 	double tStep;
	// 	double* t_p = 0;
	// 	std::array<std::string, N> equations;
	// 	std::vector<glm::vec4> colors;
	// 	std::vector<glm::vec3> normals = {};
	// 	NDParametricCurve(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
	// 										glm::vec4 color, const shaders::RuntimeConstants& constants, std::string_view name, float
	// radius, 										const std::map<std::string, double>& vars, const std::vector<glm::vec<N, Real>>& points) : 			Entity(window,
	// scene, 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(points), {}, getvertexCount(points), {}, position, rotation, scale,
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
	// 						 getIndiceCount(tStart, tEnd, tStep), {}, getvertexCount(tStart, tEnd, tStep), {}, position, rotation,
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
	// 						 getIndiceCount(tStart, tEnd, tStep), {}, getvertexCount(tStart, tEnd, tStep), {}, position, rotation,
	// 						 scale, (!name.empty()) ? name : ("Curve " + std::to_string(++curvesCount))),
	// 			scenePointer(&scene), color(color), radius(radius), vars(vars), tStart(tStart), tEnd(tEnd), tStep(tStep),
	// 			equations(equations)
	// 	{
	// 		t_p = &this->vars["t"];
	// 		generateAndUpdateCurve(tStart, tEnd, tStep);
	// 	}

	// 	// --- Count and other methods remain the same as user provided ---
	// 	size_t calculateCentralPointsCount(const std::vector<glm::vec<N, Real>>& points) { return points.size(); }

	// 	size_t calculateCentralPointsCount(double tStart, double tEnd, double tStep)
	// 	{
	// 		if (tStep <= 0)
	// 			return 0;
	// 		return static_cast<size_t>(std::floor((tEnd - tStart) / tStep + VEC_EPSILON)) + 1;
	// 	}

	// 	size_t getIndiceCount(size_t numPoints)
	// 	{
	// 		if (numPoints < 2)
	// 			return 0;
	// 		size_t numSegments = numPoints - 1;
	// 		size_t indiceCount = 0;
	// 		if constexpr (N == 2)
	// 		{
	// 			indiceCount = (numSegments + 1) * 6; // Segments + closing segment
	// 		}
	// 		else if constexpr (N == 3)
	// 		{
	// 			const int circleSegments = 16;
	// 			indiceCount = (numSegments + 1) * circleSegments * 6; // Segments + closing segment
	// 		}
	// 		return indiceCount;
	// 	}

	// 	size_t getvertexCount(size_t numPoints)
	// 	{
	// 		size_t vertexCount = 0;
	// 		if constexpr (N == 2)
	// 		{
	// 			vertexCount = numPoints * 2;
	// 		}
	// 		else if constexpr (N == 3)
	// 		{
	// 			const int circleSegments = 16;
	// 			vertexCount = numPoints * circleSegments;
	// 		}
	// 		return vertexCount;
	// 	}

	// 	size_t getIndiceCount(double tStart, double tEnd, double tStep)
	// 	{
	// 		size_t numPoints = calculateCentralPointsCount(tStart, tEnd, tStep);
	// 		return getIndiceCount(numPoints);
	// 	}

	// 	size_t getvertexCount(double tStart, double tEnd, double tStep)
	// 	{
	// 		size_t numPoints = calculateCentralPointsCount(tStart, tEnd, tStep);
	// 		return getvertexCount(numPoints);
	// 	}

	// 	uint32_t getIndiceCount(const std::vector<glm::vec<N, Real>>& points)
	// 	{
	// 		size_t numPoints = calculateCentralPointsCount(points);
	// 		return getIndiceCount(numPoints);
	// 	}
	// 	uint32_t getvertexCount(const std::vector<glm::vec<N, Real>>& points)
	// 	{
	// 		size_t numPoints = calculateCentralPointsCount(points);
	// 		return getvertexCount(numPoints);
	// 	}
	// 	void generateAndUpdateCurve(const std::vector<glm::vec<N, Real>>& centralPoints)
	// 	{
	// 		const float VEC_EPSILON_SQ = VEC_EPSILON * VEC_EPSILON;

	// 		// Ensure member vectors are clear
	// 		indices.clear();
	// 		vertices.clear();
	// 		colors.clear();
	// 		normals.clear();

	// 		// Use references for convenience
	// 		std::vector<unsigned int>& indices = this->indices;
	// 		std::vector<glm::vec3>& vertices = vertices;
	// 		std::vector<glm::vec4>& vertex_colors = colors; // Use member colors vector
	// 		std::vector<glm::vec3>& vertex_normals = normals; // Use member normals vector

	// 		vertices.reserve(vertexCount);
	// 		vertex_normals.reserve(vertexCount);
	// 		vertex_colors.reserve(vertexCount);
	// 		indices.reserve(indiceCount);


	// 		if (centralPoints.size() < 2)
	// 		{
	// 			std::cerr << "WARN: Need at least 2 central points for curve generation.\n";
	// 			// Handle degenerate case (e.g., create a single ring or nothing)
	// 			return;
	// 		}

	// 		auto& frontFace = window.iRenderer->frontFace;

	// 		if constexpr (N == 2)
	// 		{
	// 			// --- 2D Implementation (Unchanged from user code, check indexing if needed) ---
	// 			for (size_t i = 0; i < centralPoints.size(); i++)
	// 			{
	// 				glm::vec2 p = centralPoints[i];
	// 				glm::vec2 tangent;
	// 				if (centralPoints.size() == 1)
	// 				{
	// 					tangent = glm::vec2(1.0f, 0.0f);
	// 				}
	// 				else if (i == 0)
	// 				{
	// 					tangent = glm::normalize(glm::vec2(centralPoints[i + 1]) - p);
	// 				}
	// 				else if (i == centralPoints.size() - 1)
	// 				{
	// 					tangent = glm::normalize(p - glm::vec2(centralPoints[i - 1]));
	// 				}
	// 				else
	// 				{
	// 					tangent = glm::normalize(glm::vec2(centralPoints[i + 1]) - glm::vec2(centralPoints[i - 1]));
	// 				}
	// 				// Handle potential zero tangent (normalize might return NaN)
	// 				if (glm::any(glm::isnan(tangent)) || glm::length2(tangent) <= VEC_EPSILON_SQ)
	// 				{
	// 					if (i > 0)
	// 						tangent = glm::normalize(p - glm::vec2(centralPoints[i - 1])); // Try backward
	// 					if (glm::any(glm::isnan(tangent)) || glm::length2(tangent) <= VEC_EPSILON_SQ)
	// 						tangent = glm::vec2(1.0f, 0.0f); // Default
	// 				}
	// 				glm::vec2 normal = glm::vec2(-tangent.y, tangent.x);
	// 				glm::vec3 v1 = glm::vec3(p + radius * normal, 0.0f);
	// 				glm::vec3 v2 = glm::vec3(p - radius * normal, 0.0f);
	// 				vertices.push_back(v1);
	// 				vertex_colors.push_back(color);
	// 				vertices.push_back(v2);
	// 				vertex_colors.push_back(color);
	// 				// Normals for 2D
	// 				vertex_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	// 				vertex_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	// 				if (i > 0)
	// 				{
	// 					int idx = (int)i * 2;
	// 					// Original user indexing for 2D
	// 					if (frontFace == zg::COUNTERCLOCKWISE)
	// 					{
	// 						indices.push_back(idx - 2);
	// 						indices.push_back(idx - 1);
	// 						indices.push_back(idx);
	// 						indices.push_back(idx);
	// 						indices.push_back(idx - 1);
	// 						indices.push_back(idx + 1);
	// 					}
	// 					else
	// 					{
	// 						indices.push_back(idx);
	// 						indices.push_back(idx - 1);
	// 						indices.push_back(idx - 2);
	// 						indices.push_back(idx + 1);
	// 						indices.push_back(idx - 1);
	// 						indices.push_back(idx);
	// 					}
	// 				}
	// 			}
	// 			// Closing for 2D
	// 			if (centralPoints.size() > 1)
	// 			{
	// 				int firstRowIdx = 0;
	// 				int lastRowIdx = (centralPoints.size() - 1) * 2;
	// 				if (frontFace == zg::COUNTERCLOCKWISE)
	// 				{
	// 					indices.push_back(lastRowIdx);
	// 					indices.push_back(lastRowIdx + 1);
	// 					indices.push_back(firstRowIdx);
	// 					indices.push_back(firstRowIdx);
	// 					indices.push_back(lastRowIdx + 1);
	// 					indices.push_back(firstRowIdx + 1);
	// 				}
	// 				else
	// 				{
	// 					indices.push_back(firstRowIdx);
	// 					indices.push_back(lastRowIdx + 1);
	// 					indices.push_back(lastRowIdx);
	// 					indices.push_back(firstRowIdx + 1);
	// 					indices.push_back(lastRowIdx + 1);
	// 					indices.push_back(firstRowIdx);
	// 				}
	// 			}
	// 		}
	// 		// --- N == 3 Case (Tube Curve with RMF) ---
	// 		else if constexpr (N == 3)
	// 		{
	// 			const int circleSegments = 16;
	// 			std::vector<glm::vec3> tangents(centralPoints.size());
	// 			std::vector<glm::vec3> normals_frame(centralPoints.size()); // RMF normal
	// 			std::vector<glm::vec3> binormals_frame(centralPoints.size()); // RMF binormal

	// 			// --- Calculate Tangents ---
	// 			for (size_t i = 0; i < centralPoints.size(); ++i)
	// 			{
	// 				glm::vec3 T;
	// 				glm::vec3 p_curr = glm::vec3(centralPoints[i]);
	// 				if (centralPoints.size() == 1)
	// 				{
	// 					T = glm::vec3(1.0f, 0.0f, 0.0f);
	// 				}
	// 				else if (i == 0)
	// 				{
	// 					T = glm::vec3(centralPoints[i + 1]) - p_curr;
	// 				}
	// 				else if (i == centralPoints.size() - 1)
	// 				{
	// 					T = p_curr - glm::vec3(centralPoints[i - 1]);
	// 				}
	// 				else
	// 				{
	// 					T = glm::vec3(centralPoints[i + 1]) - glm::vec3(centralPoints[i - 1]);
	// 				} // Central difference

	// 				// Normalize tangent, handle zero length robustly
	// 				float tangent_len_sq = glm::length2(T);
	// 				if (tangent_len_sq <= VEC_EPSILON_SQ)
	// 				{
	// 					if (i > 0)
	// 						T = p_curr - glm::vec3(centralPoints[i - 1]); // Try backward diff
	// 					tangent_len_sq = glm::length2(T);
	// 					if (tangent_len_sq <= VEC_EPSILON_SQ)
	// 					{
	// 						T = glm::vec3(1.0f, 0.0f, 0.0f);
	// 						std::cerr << "WARN: Zero tangent at i=" << i << ". Using default.\n";
	// 					}
	// 					else
	// 					{
	// 						T = glm::normalize(T);
	// 					}
	// 				}
	// 				else
	// 				{
	// 					T = glm::normalize(T);
	// 				}
	// 				tangents[i] = T;
	// 			}

	// 			// --- Initialize First Frame (i=0) ---
	// 			glm::vec3 T0 = tangents[0];
	// 			glm::vec3 N0;
	// 			glm::vec3 _B0_;
	// 			{ // Scope for initial frame calculation
	// 				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // Initial up vector
	// 				glm::vec3 potential_N = glm::cross(T0, up);
	// 				if (glm::length2(potential_N) <= VEC_EPSILON_SQ)
	// 				{ // If T0 is parallel to Y-up
	// 					up = glm::vec3(1.0f, 0.0f, 0.0f); // Try X-up
	// 					potential_N = glm::cross(T0, up);
	// 					if (glm::length2(potential_N) <= VEC_EPSILON_SQ)
	// 					{ // If T0 also parallel to X-up (e.g., T0 is Z-aligned)
	// 						up = glm::vec3(0.0f, 1.0f, 0.0f); // Use Y-up again, N will be X-aligned
	// 						potential_N = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), up); // Arbitrary N if T0 is zero (shouldn't
	// happen) 						if (glm::length2(T0) <= VEC_EPSILON_SQ) 							potential_N = glm::vec3(1.0f, 0.0f, 0.0f); // Handle zero T0
	// 					}
	// 				}
	// 				N0 = glm::normalize(potential_N);
	// 				_B0_ = glm::normalize(glm::cross(T0, N0));
	// 				// Ensure orthogonality if needed (might slightly change N0/_B0_)
	// 				// N0 = glm::normalize(glm::cross(_B0_, T0));
	// 			}
	// 			normals_frame[0] = N0;
	// 			binormals_frame[0] = _B0_;

	// 			// --- Propagate Frame using Double Reflection (RMF) ---
	// 			for (size_t i = 1; i < centralPoints.size(); ++i)
	// 			{
	// 				const glm::vec3& T_prev = tangents[i - 1];
	// 				const glm::vec3& N_prev = normals_frame[i - 1];
	// 				const glm::vec3& B_prev = binormals_frame[i - 1];
	// 				const glm::vec3& T_curr = tangents[i];

	// 				// Reflection vectors
	// 				glm::vec3 v1 = T_curr - T_prev;
	// 				glm::vec3 v2 = T_curr + T_prev;
	// 				float v2_len_sq = glm::length2(v2);

	// 				glm::vec3 N_curr;
	// 				glm::vec3 B_curr;

	// 				// Check for degenerate reflection (T_curr == -T_prev)
	// 				if (v2_len_sq <= VEC_EPSILON_SQ)
	// 				{
	// 					// Reflect across an arbitrary axis orthogonal to T_curr
	// 					// E.g., reflect N across T_curr, B across T_curr
	// 					N_curr = -N_prev + 2.0f * glm::dot(N_prev, T_curr) * T_curr;
	// 					B_curr = -B_prev + 2.0f * glm::dot(B_prev, T_curr) * T_curr;
	// 					std::cerr << "WARN: RMF reflection failed (T_curr == -T_prev) at i=" << i << ". Using simple
	// reflection.\n";
	// 				}
	// 				else
	// 				{
	// 					// Double reflection formula
	// 					float dot_v2_N = glm::dot(v2, N_prev);
	// 					float dot_v2_B = glm::dot(v2, B_prev);
	// 					float scale = 2.0f / v2_len_sq;

	// 					N_curr = N_prev - scale * dot_v2_N * v2;
	// 					B_curr = B_prev - scale * dot_v2_B * v2;
	// 				}

	// 				// --- Orthonormalize the new frame (T_curr, N_curr, B_curr) to prevent drift ---
	// 				N_curr = glm::normalize(N_curr - glm::dot(N_curr, T_curr) * T_curr);
	// 				B_curr = glm::normalize(glm::cross(T_curr, N_curr)); // Recalculate B from T and N

	// 				normals_frame[i] = N_curr;
	// 				binormals_frame[i] = B_curr;
	// 			}

	// 			// --- Generate Vertices and Indices using the RMF frame ---
	// 			for (size_t i = 0; i < centralPoints.size(); ++i)
	// 			{
	// 				glm::vec3 center = glm::vec3(centralPoints[i]);
	// 				const glm::vec3& normal_frame = normals_frame[i];
	// 				const glm::vec3& B = binormals_frame[i];
	// 				for (int j = 0; j < circleSegments; ++j)
	// 				{
	// 					float angle = (float)j / (float)circleSegments * glm::two_pi<float>();
	// 					glm::vec3 offset = radius * (cos(angle) * normal_frame + sin(angle) * B);
	// 					vertices.push_back(center + offset);
	// 					vertex_colors.push_back(color);
	// 					vertex_normals.push_back(glm::normalize(offset));
	// 					if (i > 0)
	// 					{
	// 						int prevRow = (i - 1) * circleSegments;
	// 						int currRow = i * circleSegments;
	// 						int nextJ = (j + 1) % circleSegments;
	// 						if (frontFace == zg::COUNTERCLOCKWISE)
	// 						{
	// 							indices.push_back(prevRow + nextJ);
	// 							indices.push_back(currRow + j);
	// 							indices.push_back(prevRow + j);
	// 							indices.push_back(currRow + nextJ);
	// 							indices.push_back(currRow + j);
	// 							indices.push_back(prevRow + nextJ);
	// 						}
	// 						else
	// 						{
	// 							indices.push_back(prevRow + j);
	// 							indices.push_back(currRow + j);
	// 							indices.push_back(prevRow + nextJ);
	// 							indices.push_back(currRow + j);
	// 							indices.push_back(prevRow + nextJ);
	// 							indices.push_back(currRow + nextJ);
	// 						}
	// 					}
	// 				}
	// 			}

	// 			// --- Add section to close the tube ---
	// 			if (centralPoints.size() > 1)
	// 			{
	// 				int firstRow = 0;
	// 				int lastRow = (centralPoints.size() - 1) * circleSegments;
	// 				for (int j = 0; j < circleSegments; ++j)
	// 				{
	// 					int nextJ = (j + 1) % circleSegments;
	// 					if (frontFace == zg::COUNTERCLOCKWISE)
	// 					{
	// 						indices.push_back(lastRow + nextJ);
	// 						indices.push_back(firstRow + j);
	// 						indices.push_back(lastRow + j);
	// 						indices.push_back(firstRow + nextJ);
	// 						indices.push_back(firstRow + j);
	// 						indices.push_back(lastRow + nextJ);
	// 					}
	// 					else
	// 					{
	// 						indices.push_back(lastRow + j);
	// 						indices.push_back(firstRow + j);
	// 						indices.push_back(lastRow + nextJ);
	// 						indices.push_back(lastRow + nextJ);
	// 						indices.push_back(firstRow + j);
	// 						indices.push_back(firstRow + nextJ);
	// 					}
	// 				}
	// 			}
	// 		} // End if constexpr (N == 3)
	// 		// computeNormals(window.iRenderer->frontFace, indices, vertices, normals);
	// 		updateIndices(indices);
	// 		updateElements("Color", colors);
	// 		updateElements("Position", vertices);
	// 		updateElements("Normal", normals);
	// 	}

	// 	glm::vec<N, float> solveForT(double t)
	// 	{
	// 		*t_p = t;
	// 		size_t index = 0;
	// 		glm::vec<N, float> vec;
	// 		for (auto& equation : equations)
	// 			vec[index++] = zg::math::MathematicalEquationResolver::solve(equation, vars);
	// 		return vec;
	// 	}

	// 	// --- generateAndUpdateCurve using Rotation Minimizing Frame (Double Reflection) ---
	// 	void generateAndUpdateCurve(double tStart, double tEnd, double tStep)
	// 	{
	// 		std::vector<glm::vec<N, Real>> centralPoints;
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

	// 	bool preRender() override
	// 	{
	// 		const auto& model = getModelMatrix();
	// 		auto shader = addShader();
	// 		shader->bind(*this);
	// 		scenePointer->entityPreRender(*this);
	// 		shader->setBlock("Model", *this, model);
	// 		shader->setBlock("View", *this, viewPointer ? viewPointer->matrix : scenePointer->viewPointer->matrix);
	// 		shader->setBlock("Projection", *this,
	// 										 projectionPointer ? projectionPointer->matrix : scenePointer->projectionPointer->matrix);
	// 		shader->setBlock("CameraPosition", *this, scenePointer->viewPointer->position, 16);
	// 		shader->unbind();
	// 		return true;
	// 	}
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
