#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/glm.hpp>
#include <zg/math/MER.hpp>
#include <zg/utilities.hpp>

namespace zg::entities
{
	template <size_t N = 3, typename Real = float, typename... Args>
	EntityCreateInfo NUVVolumeFactory(zg::Scene& scene_in, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
																		glm::vec4 color_in, const shaders::RuntimeConstants& constants,
																		std::string_view name, const std::map<std::string, double>& vars_in,
																		double uStart_in, double uEnd_in, double uStep_in, double vStart_in, double vEnd_in,
																		double vStep_in, const std::string& t_equation, const Args&... args);
	// struct NUVVolume : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<NUVVolume<N, Real>>::id; }

	// private:
	// 	inline static size_t curvesCount = 0;
	// 	inline static const double _PI_ = std::acos(-1.0);

	// public:
	// 	glm::vec4 color = glm::vec4(1);
	// 	std::vector<glm::vec4> colors;
	// 	std::vector<glm::vec3> normals;
	// 	std::map<std::string, double> vars = {};
	// 	double uStart;
	// 	double uEnd;
	// 	double uStep;
	// 	double vStart;
	// 	double vEnd;
	// 	double vStep;
	// 	double* u_p = 0;
	// 	double* v_p = 0;
	// 	std::array<std::string, N> equations;
	// 	std::optional<std::array<std::string, N>> normalEquations; // Optional analytical normal equations

	// 	// Constructor WITHOUT analytical normal equations (uses finite differencing)
	// 	template <typename... Args>
	// 	NUVVolume(zg::Window& window_in, ) :
	// 			Entity(window_in, scene_in,
	// 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(uStart_in, uEnd_in, uStep_in, vStart_in, vEnd_in, vStep_in), {},
	// 						 getvertexCount(uStart_in, uEnd_in, uStep_in, vStart_in, vEnd_in, vStep_in), {}, position, rotation,
	// 						 scale, (!name.empty()) ? std::string(name) : ("NUVVolume " + std::to_string(++curvesCount))),
	// 			color(color_in), vars(vars_in), uStart(uStart_in), uEnd(uEnd_in), uStep(uStep_in), vStart(vStart_in),
	// 			vEnd(vEnd_in), vStep(vStep_in), normalEquations(std::nullopt) // Explicitly set no normal equations
	// 	{
	// 		this->vars.try_emplace("u", 0.0);
	// 		this->vars.try_emplace("v", 0.0);
	// 		u_p = &this->vars["u"];
	// 		v_p = &this->vars["v"];
	// 		size_t index = 0;
	// 		addPositionEquations(index, t_equation, args...); // Use helper for position equations
	// 		generateMeshAndNormals();
	// 	}

	// 	// Constructor WITH analytical normal equations
	// 	NUVVolume(zg::Window& window_in, zg::Scene& scene_in, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
	// 						glm::vec4 color_in, const shaders::RuntimeConstants& constants, std::string_view name,
	// 						const std::map<std::string, double>& vars_in, double uStart_in, double uEnd_in, double uStep_in,
	// 						double vStart_in, double vEnd_in, double vStep_in, const std::array<std::string, N>& equations_in,
	// 						const std::array<std::string, N>& normal_equations_in) : // Added normal equations parameter
	// 			Entity(window_in, scene_in,
	// 						 zg::mergeVectors<std::string>(
	// 							 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
	// 						 getIndiceCount(uStart_in, uEnd_in, uStep_in, vStart_in, vEnd_in, vStep_in), {},
	// 						 getvertexCount(uStart_in, uEnd_in, uStep_in, vStart_in, vEnd_in, vStep_in), {}, position, rotation,
	// 						 scale, (!name.empty()) ? std::string(name) : ("NUVVolume " + std::to_string(++curvesCount))),
	// 			color(color_in), vars(vars_in), uStart(uStart_in), uEnd(uEnd_in), uStep(uStep_in), vStart(vStart_in),
	// 			vEnd(vEnd_in), vStep(vStep_in), equations(equations_in),
	// 			normalEquations(normal_equations_in) // Store the normal equations
	// 	{
	// 		this->vars.try_emplace("u", 0.0);
	// 		this->vars.try_emplace("v", 0.0);
	// 		u_p = &this->vars["u"];
	// 		v_p = &this->vars["v"];
	// 		generateMeshAndNormals();
	// 	}


	// 	void getUVCounts(double u_start, double u_end, double u_step, double v_start, double v_end, double v_step,
	// 									 size_t& uCount, size_t& vCount) const
	// 	{
	// 		if (u_step <= VEC_EPSILON || v_step <= VEC_EPSILON)
	// 		{
	// 			uCount = 0;
	// 			vCount = 0;
	// 			return;
	// 		}
	// 		uCount = static_cast<size_t>(std::round((u_end - u_start) / u_step)) + 1;
	// 		vCount = static_cast<size_t>(std::round((v_end - v_start) / v_step)) + 1;
	// 		if (std::abs(u_end - u_start) < VEC_EPSILON)
	// 			uCount = 1;
	// 		else if (u_end < u_start)
	// 			uCount = 0;
	// 		else if (uCount == 0)
	// 			uCount = 1;
	// 		if (std::abs(v_end - v_start) < VEC_EPSILON)
	// 			vCount = 1;
	// 		else if (v_end < v_start)
	// 			vCount = 0;
	// 		else if (vCount == 0)
	// 			vCount = 1;
	// 	}

	// 	size_t getvertexCount(double u_start, double u_end, double u_step, double v_start, double v_end,
	// 												 double v_step) const
	// 	{
	// 		size_t u, v;
	// 		getUVCounts(u_start, u_end, u_step, v_start, v_end, v_step, u, v);
	// 		return u * v;
	// 	}

	// 	size_t getIndiceCount(double u_start, double u_end, double u_step, double v_start, double v_end,
	// 												double v_step) const
	// 	{
	// 		size_t u, v;
	// 		getUVCounts(u_start, u_end, u_step, v_start, v_end, v_step, u, v);
	// 		if (u < 2 || v < 2)
	// 			return 0;
	// 		return (u - 1) * (v - 1) * 6;
	// 	}


	// 	void generateMeshAndNormals()
	// 	{
	// 		const double FINITE_DIFF_ABS = 1e-5;
	// 		const double du = FINITE_DIFF_ABS;
	// 		const double dv = FINITE_DIFF_ABS;

	// 		size_t uCount, vCount;
	// 		getUVCounts(uStart, uEnd, uStep, vStart, vEnd, vStep, uCount, vCount);

	// 		if (uCount == 0 || vCount == 0)
	// 		{
	// 			std::cerr << "Warning: NUVVolume - Zero points generated." << std::endl;
	// 			updateGpuData({}, {}, {}, {});
	// 			return;
	// 		}

	// 		size_t expectedCount = uCount * vCount;
	// 		std::vector<glm::vec3> calculated_vertices;
	// 		std::vector<glm::vec3> calculated_normals;
	// 		calculated_vertices.reserve(expectedCount);
	// 		calculated_normals.resize(expectedCount);

	// 		std::vector<uint32_t> calculated_indices;
	// 		size_t expectedIndices = getIndiceCount(uStart, uEnd, uStep, vStart, vEnd, vStep);
	// 		calculated_indices.reserve(expectedIndices);
	// 		auto& frontFace = window.iRenderer->frontFace;

	// 		if (uCount > 1 && vCount > 1)
	// 		{
	// 			for (size_t j = 0; j < vCount - 1; ++j)
	// 			{
	// 				for (size_t i = 0; i < uCount - 1; ++i)
	// 				{
	// 					uint32_t p00 = static_cast<uint32_t>(j * uCount + i);
	// 					uint32_t p10 = p00 + 1;
	// 					uint32_t p01 = static_cast<uint32_t>((j + 1) * uCount + i);
	// 					uint32_t p11 = p01 + 1;

	// 					if (frontFace == zg::CLOCKWISE)
	// 					{
	// 						calculated_indices.push_back(p00);
	// 						calculated_indices.push_back(p01);
	// 						calculated_indices.push_back(p11);
	// 						calculated_indices.push_back(p00);
	// 						calculated_indices.push_back(p11);
	// 						calculated_indices.push_back(p10);
	// 					}
	// 					else
	// 					{
	// 						calculated_indices.push_back(p00);
	// 						calculated_indices.push_back(p10);
	// 						calculated_indices.push_back(p11);
	// 						calculated_indices.push_back(p00);
	// 						calculated_indices.push_back(p11);
	// 						calculated_indices.push_back(p01);
	// 					}
	// 				}
	// 			}
	// 		}

	// 		uint32_t vertex_index = 0;
	// 		double uRange = uEnd - uStart;
	// 		double vRange = vEnd - vStart;
	// 		bool useAnalyticalNormals = normalEquations.has_value();

	//         for (size_t j = 0; j < vCount; ++j)
	//         {
	//             double v_ratio = (vCount > 1) ? static_cast<double>(j) / (vCount - 1) : 0.0;
	//             double v = vStart + v_ratio * (vEnd - vStart);

	//             for (size_t i = 0; i < uCount; ++i)
	//             {
	//                 double u_ratio = (uCount > 1) ? static_cast<double>(i) / (uCount - 1) : 0.0;
	//                 double u = uStart + u_ratio * (uEnd - uStart);

	//                 glm::vec<N, Real> point = solveForUV(u, v);
	//                 glm::vec3 current_pos(0.0f);
	//                 bool point_is_valid = true;
	//                 for (size_t dim = 0; dim < N; ++dim)
	//                     if (std::isnan(point[dim]) || std::isinf(point[dim]))
	//                     {
	//                         point_is_valid = false;
	//                         break;
	//                     }

	//                 if (point_is_valid)
	//                 {
	//                     if constexpr (N >= 3)
	//                         current_pos = glm::vec3(point);
	//                     else if constexpr (N == 2)
	//                         current_pos = glm::vec3(point.x, point.y, 0.0f);
	//                     calculated_vertices.push_back(current_pos);
	//                 }
	//                 else
	//                 {
	//                     calculated_vertices.push_back(current_pos);
	//                     calculated_normals[vertex_index] = glm::vec3(0.0f, 0.0f, 1.0f);
	//                     std::cerr << "Warning: NUVVolume - Invalid position generated at u=" << u << ", v=" << v
	//                                         << ". Using fallback." << std::endl;
	//                     ++vertex_index;
	//                     continue;
	//                 }

	//                 if (useAnalyticalNormals)
	//                 {
	//                     glm::vec<N, Real> normal_vec = solveForNormalUV(u, v);
	//                     glm::vec3 normal_dir(0.0f, 0.0f, 1.0f);

	//                     if constexpr (N >= 3)
	//                         normal_dir = glm::vec3(normal_vec);
	//                     else if constexpr (N == 2)
	//                         normal_dir = glm::vec3(normal_vec.x, normal_vec.y, 0.0f);

	//                     float norm_len_sq = glm::dot(normal_dir, normal_dir);
	//                     if (norm_len_sq > VEC_EPSILON_SQ)
	//                     {
	//                         calculated_normals[vertex_index] = normal_dir / std::sqrt(norm_len_sq);
	//                     }
	//                     else
	//                     {
	//                         calculated_normals[vertex_index] = glm::vec3(0.0f, 0.0f, 1.0f);
	//                         std::cerr << "Warning: NUVVolume - Analytical normal is zero vector at u=" << u << ", v="
	//                         << v
	//                                             << ". Using Z-up fallback." << std::endl;
	//                     }
	//                 }
	//                 ++vertex_index;
	//             }
	//         }
	//         if (!useAnalyticalNormals)
	//         {
	//             computeNormals(frontFace, calculated_indices, calculated_vertices, calculated_normals);
	//         }

	// 		size_t actualVertexCount = calculated_vertices.size();
	// 		std::vector<glm::vec4> calculated_colors(actualVertexCount, this->color);

	// 		// if (calculated_normals.size() != actualVertexCount)
	// 		// {
	// 		// 	size_t oldSize = calculated_normals.size();
	// 		// 	calculated_normals.resize(actualVertexCount, glm::vec3(0.0f, 0.0f, 1.0f));
	// 		// 	if (oldSize > actualVertexCount)
	// 		// 		calculated_normals.resize(actualVertexCount);
	// 		// 	std::cerr << "Warning: NUVVolume - Normal count mismatch (" << oldSize << ") vs position count ("
	// 		// 						<< actualVertexCount << "). Resized normals array." << std::endl;
	// 		// }
	// 		updateGpuData(calculated_vertices, calculated_normals, calculated_indices, calculated_colors);
	// 	}

	// 	// Solves parametric equations for given u, v (POSITION)
	// 	glm::vec<N, Real> solveForUV(double u, double v)
	// 	{
	// 		if (u_p)
	// 			*u_p = u;
	// 		else
	// 			vars["u"] = u;
	// 		if (v_p)
	// 			*v_p = v;
	// 		else
	// 			vars["v"] = v;
	// 		glm::vec<N, Real> vec;
	// 		if (equations.size() == N)
	// 		{
	// 			for (size_t index = 0; index < N; ++index)
	// 				vec[index] = static_cast<Real>(zg::math::MathematicalEquationResolver::solve(equations[index], vars));
	// 		}
	// 		else
	// 		{ /* ... error handling ... */
	// 		}
	// 		return vec;
	// 	}

	// 	// Solves parametric equations for given u, v (NORMAL) - NEW FUNCTION
	// 	glm::vec<N, Real> solveForNormalUV(double u, double v)
	// 	{
	// 		if (!normalEquations.has_value())
	// 		{
	// 			// Should not happen if called correctly, but return a default
	// 			std::cerr << "Error: solveForNormalUV called without normal equations!" << std::endl;
	// 			glm::vec<N, Real> default_norm;
	// 			if constexpr (N >= 3)
	// 				default_norm[2] = static_cast<Real>(1.0); // Z-up default
	// 			return default_norm;
	// 		}

	// 		// Update u,v in the shared vars map for the equation solver
	// 		if (u_p)
	// 			*u_p = u;
	// 		else
	// 			vars["u"] = u;
	// 		if (v_p)
	// 			*v_p = v;
	// 		else
	// 			vars["v"] = v;

	// 		glm::vec<N, Real> vec;
	// 		const auto& norm_eqs = normalEquations.value(); // Get the equations array

	// 		if (norm_eqs.size() == N)
	// 		{
	// 			for (size_t index = 0; index < N; ++index)
	// 			{
	// 				// Use the same solver, assuming it handles the normal functions (cos, sin etc.)
	// 				vec[index] = static_cast<Real>(zg::math::MathematicalEquationResolver::solve(norm_eqs[index], vars));
	// 			}
	// 		}
	// 		else
	// 		{
	// 			std::cerr << "Error: NUVVolume - normalEquations array size (" << norm_eqs.size()
	// 								<< ") does not match template parameter N (" << N << ")." << std::endl;
	// 			for (size_t i = 0; i < N; ++i)
	// 				vec[i] = static_cast<Real>(0.0);
	// 			if constexpr (N >= 3)
	// 				vec[2] = static_cast<Real>(1.0); // Z-up fallback
	// 		}
	// 		return vec;
	// 	}


	// 	// --- Other Methods (updateGpuData, preRender, setColor, updateColorBuffer) ---
	// 	// (Keep implementations, ensure they use base class members correctly if needed)
	// 	void updateGpuData(const std::vector<glm::vec3>& new_vertices, const std::vector<glm::vec3>& new_normals,
	// 										 const std::vector<uint32_t>& new_indices, const std::vector<glm::vec4>& new_colors)
	// 	{
	// 		this->vertices = new_vertices;
	// 		this->normals = new_normals;
	// 		this->indices = new_indices;
	// 		this->colors = new_colors;
	// 		updateIndices(this->indices);
	// 		updateElements("Color", this->colors);
	// 		updateElements("Position", this->vertices);
	// 		updateElements("Normal", this->normals);
	// 	}
	// 	bool preRender() override
	// 	{
	// 		const auto& model = getModelMatrix();
	// 		auto shader = addShader();
	// 		shader->bind(*this);
	// 		scene.entityPreRender(*this);
	// 		shader->setBlock("Model", *this, model);
	// 		shader->setBlock("View", *this,
	// 										 viewPointer ? viewPointer->matrix
	// 																 : (scene.viewPointer ? scene.viewPointer->matrix : glm::mat4(1.0f)));
	// 		shader->setBlock("Projection", *this,
	// 										 projectionPointer
	// 											 ? projectionPointer->matrix
	// 											 : (scene.projectionPointer ? scene.projectionPointer->matrix : glm::mat4(1.0f)));
	// 		shader->setBlock("CameraPosition", *this, scene.viewPointer ? scene.viewPointer->position : glm::vec3(0.0f),
	// 16); 		shader->unbind(); 		return true;
	// 	}
	// 	void updateColorBuffer() { updateElements("Color", this->colors); }
	// 	void setColor(const glm::vec4& new_color)
	// 	{
	// 		this->color = new_color;
	// 		if (this->colors.size() != this->vertices.size())
	// 		{
	// 			this->colors.resize(this->vertices.size());
	// 		}
	// 		std::fill(this->colors.begin(), this->colors.end(), this->color);
	// 		updateElements("Color", this->colors);
	// 	}


	// private:
	// 	// Helper for variadic constructor (Position Equations)
	// 	template <typename T, typename... Args>
	// 	void addPositionEquations(size_t index, const T& t_equation, const Args&... args)
	// 	{
	// 		if (index < N)
	// 		{
	// 			if constexpr (std::is_convertible_v<T, std::string_view>)
	// 				equations[index] = std::string(t_equation);
	// 			else
	// 				static_assert(std::is_convertible_v<T, std::string_view>, "Equation must be convertible to string_view");
	// 			if constexpr (sizeof...(args) > 0)
	// 				addPositionEquations(index + 1, args...);
	// 		}
	// 	}
	// };
} // namespace zg::entities
