#pragma once

#include <algorithm>
#include <cstdint>
constexpr int RAY_PACKET_WIDTH = 16;
// #undef __AVX__
// #undef __AVX512__
#include <intrin.h>
#include <limits>
#include <vector>
#include <zg/KeyIDVector.hpp>
#include <zg/glm.hpp>
#include <zg/Entity.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Plane.hpp>
namespace std
{
	template <typename UserDataT>
	struct hash<std::pair<const UserDataT, const glm::vec3>>
	{
		size_t operator()(const std::pair<const UserDataT, const glm::vec3>& pair) const
		{
			return std::hash<UserDataT>{}(pair.first) ^ std::hash<glm::vec3>{}(pair.second);
		}
	};
} // namespace std
namespace zg::exp::raytracing
{
	template <typename TriangleT, typename UserDataT>
	class BVH
	{
	public:
		struct RayPacket
		{
			__m512 originX;
			__m512 originY;
			__m512 originZ;
			__m512 dirX;
			__m512 dirY;
			__m512 dirZ;
			__m512 invDirX;
			__m512 invDirY;
			__m512 invDirZ;
			__mmask16 validMask;
		};

		struct PacketHitInfo
		{
			__m512 t;
			__m512i triangleIndex;
			__mmask16 hitMask;
			UserDataT data[RAY_PACKET_WIDTH];
			glm::vec2 uv[RAY_PACKET_WIDTH];
		};

		struct AABB
		{
			glm::vec3 min;
			glm::vec3 max;

			AABB()
			{
				reset();
			}

			void reset()
			{
				min = glm::vec3(std::numeric_limits<float>::max());
				max = glm::vec3(-std::numeric_limits<float>::max());
			}

			void expand(const glm::vec3& point)
			{
				min = glm::min(min, point);
				max = glm::max(max, point);
			}

			void expand(const AABB& other)
			{
				min = glm::min(min, other.min);
				max = glm::max(max, other.max);
			}

			bool intersect(const glm::vec3& origin, const glm::vec3& invDir, const glm::ivec3& sign, float tMin,
										 float tMax) const
			{
				float t0x = (min.x - origin.x) * invDir.x;
				float t1x = (max.x - origin.x) * invDir.x;
				float t0y = (min.y - origin.y) * invDir.y;
				float t1y = (max.y - origin.y) * invDir.y;
				float t0z = (min.z - origin.z) * invDir.z;
				float t1z = (max.z - origin.z) * invDir.z;

				float txmin = sign.x ? t1x : t0x;
				float txmax = sign.x ? t0x : t1x;
				float tymin = sign.y ? t1y : t0y;
				float tymax = sign.y ? t0y : t1y;
				float tzmin = sign.z ? t1z : t0z;
				float tzmax = sign.z ? t0z : t1z;

				tMin = zg::max(tMin, zg::max(txmin, zg::max(tymin, tzmin)));
				tMax = zg::min(tMax, zg::min(txmax, zg::min(tymax, tzmax)));

				return tMax >= tMin;
			}

			__mmask16 intersect_packet(const RayPacket& rays, __m512 tMin, __m512 tMax) const
			{
				__m512 minX = _mm512_set1_ps(min.x);
				__m512 minY = _mm512_set1_ps(min.y);
				__m512 minZ = _mm512_set1_ps(min.z);
				__m512 maxX = _mm512_set1_ps(max.x);
				__m512 maxY = _mm512_set1_ps(max.y);
				__m512 maxZ = _mm512_set1_ps(max.z);

				__m512 t0x = _mm512_mul_ps(_mm512_sub_ps(minX, rays.originX), rays.invDirX);
				__m512 t1x = _mm512_mul_ps(_mm512_sub_ps(maxX, rays.originX), rays.invDirX);
				__m512 tminX = _mm512_min_ps(t0x, t1x);
				__m512 tmaxX = _mm512_max_ps(t0x, t1x);

				__m512 t0y = _mm512_mul_ps(_mm512_sub_ps(minY, rays.originY), rays.invDirY);
				__m512 t1y = _mm512_mul_ps(_mm512_sub_ps(maxY, rays.originY), rays.invDirY);
				__m512 tminY = _mm512_min_ps(t0y, t1y);
				__m512 tmaxY = _mm512_max_ps(t0y, t1y);

				__m512 t0z = _mm512_mul_ps(_mm512_sub_ps(minZ, rays.originZ), rays.invDirZ);
				__m512 t1z = _mm512_mul_ps(_mm512_sub_ps(maxZ, rays.originZ), rays.invDirZ);
				__m512 tminZ = _mm512_min_ps(t0z, t1z);
				__m512 tmaxZ = _mm512_max_ps(t0z, t1z);

				__m512 tMinFinal = _mm512_max_ps(_mm512_max_ps(tminX, tminY), _mm512_max_ps(tminZ, tMin));
				__m512 tMaxFinal = _mm512_min_ps(_mm512_min_ps(tmaxX, tmaxY), _mm512_min_ps(tmaxZ, tMax));

				return _mm512_cmp_ps_mask(tMinFinal, tMaxFinal, _CMP_LE_OQ);
			}
		};

		struct BVHNode
		{
			AABB bounds;
			int left = -1;
			int right = -1;
			int triangleIndex = -1;
		};

		struct TriangleKeyWrapper
		{
			UserDataT userData;
			glm::vec3 point;

			bool operator<(const TriangleKeyWrapper& other) const
			{
				if (userData < other.userData)
				{
					return true;
				}
				if (userData > other.userData)
				{
					return false;
				}

				if (point.x < other.point.x)
				{
					return true;
				}
				if (point.x > other.point.x)
				{
					return false;
				}
				if (point.y < other.point.y)
				{
					return true;
				}
				if (point.y > other.point.y)
				{
					return false;
				}
				return point.z < other.point.z;
			}
		};

		struct TriangleWithData
		{
			TriangleT triangle;
			UserDataT data;
			glm::vec3 centroid;
			AABB bounds;
			int mesh_index = -1;
			TriangleWithData& update(const TriangleT& tri)
			{
				triangle = tri;
				const auto& v0 = triangle[0];
				const auto& v1 = triangle[1];
				const auto& v2 = triangle[2];
				centroid = ((v0 + v1 + v2) / 3.0f);
				bounds.reset();
				bounds.expand(v0);
				bounds.expand(v1);
				bounds.expand(v2);
				return *this;
			}
			glm::vec2 get_texture_uv(glm::vec2 bary_uv, const std::array<glm::vec2, 3>& tri_uvs)
			{
				// Extract barycentric coordinates u and v
				float u = bary_uv.x;
				float v = bary_uv.y;

				// Calculate the third barycentric coordinate w
				// w = 1 - u - v
				float w = 1.0f - u - v;

				// Interpolate the texture UV coordinates
				// The formula for barycentric interpolation is:
				// P = w * A + u * B + v * C
				// Where A, B, C are the attributes (in this case, UV coordinates) of the triangle vertices.
				// In many graphics contexts, the barycentric coordinates u, v, w map to the
				// influence of vertices B, C, A respectively.
				// So, tri_uvs[0] is often associated with w (vertex A),
				// tri_uvs[1] with u (vertex B), and tri_uvs[2] with v (vertex C).
				glm::vec2 interpolated_uv = (tri_uvs[0] * w) + (tri_uvs[1] * u) + (tri_uvs[2] * v);

				return interpolated_uv;
			}
		};

		struct StackNode
		{
			int index;
			float tMin, tMax;
		};

		BVH() = default;

		BVH(const BVH& other):
			triangles(other.triangles),
			nodes(other.nodes),
			built(other.built),
			changed(other.changed)
		{}

		BVH& operator=(const BVH& other)
		{
			triangles = other.triangles;
			nodes = other.nodes;
			built = other.built;
			changed = other.changed;
			return *this;
		}

		void addTriangle(const TriangleT& tri, const UserDataT& data = UserDataT())
		{
			const auto& v0 = tri[0];
			const auto& v1 = tri[1];
			const auto& v2 = tri[2];
			TriangleWithData twd{tri, data, ((v0 + v1 + v2) / 3.0f)};
			twd.bounds.expand(v0);
			twd.bounds.expand(v1);
			twd.bounds.expand(v2);
			triangles.emplace_back(twd);
			return;
		}

		TriangleWithData& getTriangle(int index)
		{
			if (index >= 0 && index < triangles.size())
				return triangles[index];
			throw std::range_error("index is out of bounds");
		}

		void addEntity(Entity& entity)
		{
			if (entity.addToBVH)
			{
				size_t meshIndex = 0;
				for (auto& meshID : entity.meshIDs)
				{
					auto& mesh = Registry::GetSingleton().getMesh(meshID);
					auto& m_indices = mesh.indices;
					auto& m_vertices = mesh.vertices;
					std::vector<uint32_t> t_indices;
					std::vector<glm::vec3> t_vertices;
					auto p_indices = &m_indices;
					auto p_vertices = &m_vertices;
					if (m_indices.empty())
					{
						if (mesh.info.shapeType >= ShapeType::PlaneXZ_Center && mesh.info.shapeType <= ShapeType::PlaneXY_BottomLeft)
						{
							t_indices = entities::getPlaneIndices((entities::PlaneType)mesh.info.shapeType);
							t_vertices = entities::getPlaneVertices((entities::PlaneType)mesh.info.shapeType);
						}
						else if (mesh.info.shapeType == ShapeType::Box)
						{
							t_indices = entities::getCubeIndices();
							t_vertices = entities::getCubeVertices();
						}
						p_indices = &t_indices;
						p_vertices = &t_vertices;
					}
					auto& indices = *p_indices;
					auto& vertices = *p_vertices;
					auto indices_data = indices.data();
					auto vertices_data = vertices.data();
					auto& model = entity.getModelMatrix();
					auto indiceCount = indices.size();
					for (size_t i = 0, c = 0; i < indiceCount; c++, i += 3)
					{
						auto i0 = indices_data[i + 0];
						auto i1 = indices_data[i + 1];
						auto i2 = indices_data[i + 2];
						auto v0 = vertices_data[i0];
						auto v1 = vertices_data[i1];
						auto v2 = vertices_data[i2];
						v0 = glm::vec3(model * glm::vec4(v0, 1.0f));
						v1 = glm::vec3(model * glm::vec4(v1, 1.0f));
						v2 = glm::vec3(model * glm::vec4(v2, 1.0f));
						if constexpr (std::is_same_v<UserDataT, std::pair<size_t, size_t>>)
							addTriangle({{v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}}, {entity.ID, meshIndex});
						if constexpr (std::is_same_v<UserDataT, size_t>)
							addTriangle({{v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}}, entity.ID);
					}
					meshIndex++;
				}
			}
		}

		void updateEntity(Entity& entity)
		{
			if (entity.addToBVH)
			{
				size_t meshIndex = 0;
				for (auto& meshID : entity.meshIDs)
				{
					auto& mesh = Registry::GetSingleton().getMesh(meshID);
					std::vector<size_t> indices;
					size_t indicesCount = 0;
					auto triangles_size = triangles.size();
					auto triangles_data = triangles.data();
					for (size_t i = 0; i < triangles_size; ++i)
					{
						if constexpr (std::is_same_v<UserDataT, std::pair<size_t, size_t>>)
						{
						if (triangles_data[i].data.first == entity.ID && triangles_data[i].data.second == meshIndex)
								++indicesCount;
						}
						else if constexpr (std::is_same_v<UserDataT, size_t>)
						{
							if (triangles_data[i].data == entity.ID)
								++indicesCount;
						}
					}
					indices.reserve(indicesCount);
					for (size_t i = 0; i < triangles_size; ++i)
					{
						if constexpr (std::is_same_v<UserDataT, std::pair<size_t, size_t>>)
						{
						if (triangles_data[i].data.first == entity.ID && triangles_data[i].data.second == meshIndex)
								indices.push_back(i);
						}
						else if constexpr (std::is_same_v<UserDataT, size_t>)
						{
							if (triangles_data[i].data == entity.ID)
								indices.push_back(i);
						}
					}
					auto indiceCount = mesh.indices.size();
					auto& m_indices = mesh.indices;
					auto& m_vertices = mesh.vertices;
					std::vector<uint32_t> t_indices;
					std::vector<glm::vec3> t_vertices;
					auto p_indices = &m_indices;
					auto p_vertices = &m_vertices;
					if (m_indices.empty())
					{
						if (mesh.info.shapeType >= ShapeType::PlaneXZ_Center && mesh.info.shapeType <= ShapeType::PlaneXY_BottomLeft)
						{
							t_indices = entities::getPlaneIndices((entities::PlaneType)mesh.info.shapeType);
							t_vertices = entities::getPlaneVertices((entities::PlaneType)mesh.info.shapeType);
						}
						else if (mesh.info.shapeType == ShapeType::Box)
						{
							t_indices = entities::getCubeIndices();
							t_vertices = entities::getCubeVertices();
						}
						p_indices = &t_indices;
						p_vertices = &t_vertices;
					}
					auto& meshIndices = *p_indices;
					auto& meshVertices = *p_vertices;
					if (meshIndices.empty())
						return;
					auto indices_size = meshIndices.size();
					auto indices_data = meshIndices.data();
					auto vertices_data = meshVertices.data();
					auto& model = entity.getModelMatrix();
					for (size_t i = 0, c = 0; i < indices_size; c++, i += 3)
					{
						auto& triangleID = indices[c];
						auto i0 = indices_data[i + 0];
						auto i1 = indices_data[i + 1];
						auto i2 = indices_data[i + 2];
						auto v0 = vertices_data[i0];
						auto v1 = vertices_data[i1];
						auto v2 = vertices_data[i2];
						v0 = glm::vec3(model * glm::vec4(v0, 1.0f));
						v1 = glm::vec3(model * glm::vec4(v1, 1.0f));
						v2 = glm::vec3(model * glm::vec4(v2, 1.0f));
						if constexpr (std::is_same_v<UserDataT, std::pair<size_t, size_t>>)
							triangles_data[triangleID].update({{v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}});
						if constexpr (std::is_same_v<UserDataT, size_t>)
							triangles_data[triangleID].update({{v0.x, v0.y, v0.z}, {v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}});
					}
					meshIndex++;
				}
			}
			built = false;
			changed = true;
		}
		void removeEntity(Scene& scene, Entity& entity)
		{
			if (entity.addToBVH)
			{
				size_t removalIndex = 0;
				auto entityID = entity.ID;
				triangles.erase(
					std::remove_if(
						triangles.begin(),
						triangles.end(),
						[&](const auto& triangle) -> bool
						{
							if constexpr (std::is_same_v<UserDataT, std::pair<size_t, size_t>>)
							{
								if (triangle.data.first == entityID)
								{
									removalIndex++;
									return true;
								}
							}
							else if constexpr (std::is_same_v<UserDataT, size_t>)
							{
								if (triangle.data == entityID)
								{
									removalIndex++;
									return true;
								}
							}
							return false;
						}
					),
					triangles.end()
				);
			}
			changed = true;
			built = false;
		}

		void build()
		{
			{
				std::unique_lock lock(build_mutex);
				if (building)
				{
					build_cv.wait(lock, [&]() {
						return !building && built;
					});
					return;
				}
				else
				{
					building = true;
				}
			}
			nodes.clear();
			auto triangles_size = triangles.size();
			std::vector<int> indices(triangles_size);
			auto indices_size = indices.size();
			auto indices_data = indices.data();
			for (auto i = 0; i < indices_size; ++i)
			{
				indices_data[i] = i;
			}
			nodes.reserve(triangles_size * 2);
			int rootIndex = buildRecursive(indices);
			if (rootIndex != 0)
			{
				std::rotate(nodes.begin(), nodes.begin() + rootIndex, nodes.begin() + rootIndex + 1);
				fixNodeIndices(0);
			}
			{
				std::unique_lock lock(build_mutex);
				building = false;
				built = true;
				changed = false;
			}
			build_cv.notify_all();
		}

		bool intersect_glm(
			const glm::vec3& origin,
			const glm::vec3& dir,
			float& outT,
			int& outIndex,
			UserDataT& outData,
			glm::vec2& uv
		) const
		{
			if (!built && changed)
				build();
			glm::vec3 invDir = 1.0f / dir;
			glm::ivec3 sign = glm::lessThan(invDir, glm::vec3(0.0f));
			float closestT = std::numeric_limits<float>::max();
			bool hit = false;
			StackNode stack[64];
			int stackPtr = 1;
			stack[0] = {0, 0.0f, closestT};
			auto nodesData = nodes.data();
			auto triangles_data = triangles.data();

			while (stackPtr > 0)
			{
				const auto& sn = stack[--stackPtr];
				const auto& node = nodesData[sn.index];
				if (!node.bounds.intersect(origin, invDir, sign, sn.tMin, sn.tMax))
					continue;
				if (node.triangleIndex >= 0)
				{
					const auto& t = triangles_data[node.triangleIndex];
					const auto& v0 = t.triangle[0];
					const auto& v1 = t.triangle[1];
					const auto& v2 = t.triangle[2];
					glm::vec3 edge1 = v1 - v0;
					glm::vec3 edge2 = v2 - v0;
					glm::vec3 pvec = glm::cross(dir, edge2);
					float det = glm::dot(edge1, pvec);
					if (fabs(det) < 1e-5f)
						continue;
					float invDet = 1.0f / det;
					glm::vec3 tvec = origin - v0;
					float u = glm::dot(tvec, pvec) * invDet;
					if (u < 0.0f || u > 1.0f)
						continue;
					glm::vec3 qvec = glm::cross(tvec, edge1);
					float v = glm::dot(dir, qvec) * invDet;
					if (v < 0.0f || u + v > 1.0f)
						continue;
					float tHit = glm::dot(edge2, qvec) * invDet;
					if (tHit > 0.0f && tHit < closestT)
					{
						closestT = tHit;
						outT = tHit;
						outIndex = node.triangleIndex;
						outData = t.data;
						uv.x = u;
						uv.y = v;
						hit = true;
					}
				}
				else
				{
					stack[stackPtr++] = {node.left, sn.tMin, sn.tMax};
					stack[stackPtr++] = {node.right, sn.tMin, sn.tMax};
				}
			}
			return hit;
		}

		bool intersect_packet_glm(
			const RayPacket& rays,
			PacketHitInfo& outHit
		) const
		{
			if (!built && changed)
				build();
			for (auto i = 0; i < RAY_PACKET_WIDTH; i++)
				outHit.triangleIndex.m512i_i32[i] = -1;
			bool hit = false;
			for (int i = 0; i < RAY_PACKET_WIDTH; ++i)
			{
				glm::vec3 origin(rays.originX.m512_f32[i], rays.originY.m512_f32[i], rays.originZ.m512_f32[i]);
				glm::vec3 direction(rays.dirX.m512_f32[i], rays.dirY.m512_f32[i], rays.dirZ.m512_f32[i]);
				if (intersect_glm(origin, direction, outHit.t.m512_f32[i], outHit.triangleIndex.m512i_i32[i], outHit.data[i], outHit.uv[i]) && !hit)
					hit = true;
			}
			return hit;
		}

		bool intersect(const glm::vec3& origin, const glm::vec3& dir, float& outT, int& outIndex, UserDataT& outData, glm::vec2& uv) const
		{
			if (!built && changed)
				build();
#ifdef __AVX__
			__m128 orgX = _mm_set1_ps(origin.x);
			__m128 orgY = _mm_set1_ps(origin.y);
			__m128 orgZ = _mm_set1_ps(origin.z);
			__m128 dirX = _mm_set1_ps(dir.x);
			__m128 dirY = _mm_set1_ps(dir.y);
			__m128 dirZ = _mm_set1_ps(dir.z);

			glm::vec3 invDir = 1.0f / dir;
			glm::ivec3 sign = glm::lessThan(invDir, glm::vec3(0.0f));
			float closestT = std::numeric_limits<float>::max();
			bool hit = false;

			StackNode stack[64];
			int stackPtr = 1;
			stack[0] = {0, 0.0f, closestT};

			auto nodesData = nodes.data();
			auto triangles_data = triangles.data();

			while (stackPtr > 0)
			{
				const auto& sn = stack[--stackPtr];
				const auto& node = nodesData[sn.index];

				if (!node.bounds.intersect(origin, invDir, sign, sn.tMin, sn.tMax))
					continue;

				if (node.triangleIndex >= 0)
				{
					const auto& t = triangles_data[node.triangleIndex];
					auto v0 = t.triangle[0];
					auto v1 = t.triangle[1];
					auto v2 = t.triangle[2];

					__m128 v0x = _mm_set1_ps(v0.x);
					__m128 v0y = _mm_set1_ps(v0.y);
					__m128 v0z = _mm_set1_ps(v0.z);

					__m128 edge1x = _mm_set1_ps(v1.x - v0.x);
					__m128 edge1y = _mm_set1_ps(v1.y - v0.y);
					__m128 edge1z = _mm_set1_ps(v1.z - v0.z);
					__m128 edge2x = _mm_set1_ps(v2.x - v0.x);
					__m128 edge2y = _mm_set1_ps(v2.y - v0.y);
					__m128 edge2z = _mm_set1_ps(v2.z - v0.z);

					__m128 hx = _mm_sub_ps(_mm_mul_ps(dirY, edge2z), _mm_mul_ps(dirZ, edge2y));
					__m128 hy = _mm_sub_ps(_mm_mul_ps(dirZ, edge2x), _mm_mul_ps(dirX, edge2z));
					__m128 hz = _mm_sub_ps(_mm_mul_ps(dirX, edge2y), _mm_mul_ps(dirY, edge2x));

					__m128 a = _mm_add_ps(_mm_add_ps(_mm_mul_ps(edge1x, hx), _mm_mul_ps(edge1y, hy)), _mm_mul_ps(edge1z, hz));
					float af = _mm_cvtss_f32(a);
					if (af > -1e-5f && af < 1e-5f)
						continue;

					float f = 1.0f / af;
					__m128 fvec = _mm_set1_ps(f);

					__m128 sx = _mm_sub_ps(orgX, v0x);
					__m128 sy = _mm_sub_ps(orgY, v0y);
					__m128 sz = _mm_sub_ps(orgZ, v0z);

					__m128 u =
						_mm_mul_ps(fvec, _mm_add_ps(_mm_add_ps(_mm_mul_ps(sx, hx), _mm_mul_ps(sy, hy)), _mm_mul_ps(sz, hz)));
					float uf = _mm_cvtss_f32(u);
					if (uf < 0.0f || uf > 1.0f)
						continue;

					__m128 qx = _mm_sub_ps(_mm_mul_ps(sy, edge1z), _mm_mul_ps(sz, edge1y));
					__m128 qy = _mm_sub_ps(_mm_mul_ps(sz, edge1x), _mm_mul_ps(sx, edge1z));
					__m128 qz = _mm_sub_ps(_mm_mul_ps(sx, edge1y), _mm_mul_ps(sy, edge1x));

					__m128 v =
						_mm_mul_ps(fvec, _mm_add_ps(_mm_add_ps(_mm_mul_ps(dirX, qx), _mm_mul_ps(dirY, qy)), _mm_mul_ps(dirZ, qz)));
					float vf = _mm_cvtss_f32(v);
					if (vf < 0.0f || (uf + vf) > 1.0f)
						continue;

					__m128 tHit = _mm_mul_ps(
						fvec, _mm_add_ps(_mm_add_ps(_mm_mul_ps(edge2x, qx), _mm_mul_ps(edge2y, qy)), _mm_mul_ps(edge2z, qz)));
					float thitf = _mm_cvtss_f32(tHit);
					if (thitf > 0.0f && thitf < closestT)
					{
						closestT = thitf;
						outT = thitf;
						outIndex = node.triangleIndex;
						outData = t.data;
						uv.x = u.m128_f32[0];
						uv.y = v.m128_f32[0];
						hit = true;
					}
					continue;
				}

				int left = node.left;
				int right = node.right;

				// Optional: put closer child first (favors early exits)
				if (left >= 0)
					stack[stackPtr++] = {left, sn.tMin, sn.tMax};
				if (right >= 0)
					stack[stackPtr++] = {right, sn.tMin, sn.tMax};
			}

			return hit;
#else
			return intersect_glm(origin, dir, outT, outIndex, outData);
#endif
		}

		bool intersect_packet(const RayPacket& rays, PacketHitInfo& outHit)
		{
			if (!built && changed)
				build();
#ifdef __AVX512__
			__m512 packet_tMin_overall = _mm512_set1_ps(0.0f);
			StackNode stack[64];
			int stackPtr = 0;

			outHit.t = _mm512_set1_ps(std::numeric_limits<float>::max());
			outHit.triangleIndex = _mm512_set1_epi32(-1);
			outHit.hitMask = 0;
			auto nodes_data = nodes.data();
			auto triangles_data = triangles.data();
			if (!nodes.empty())
			{
				stack[stackPtr++] = {0};
			}
			while (stackPtr > 0)
			{
				auto& node = nodes_data[stack[--stackPtr].index];
				__m512 current_tMin_for_aabb = _mm512_set1_ps(0.0f);
				__m512 current_tMax_for_aabb = outHit.t;
				__mmask16 hit_aabb_mask = node.bounds.intersect_packet(rays, current_tMin_for_aabb, current_tMax_for_aabb);
				hit_aabb_mask = _kand_mask16(hit_aabb_mask, rays.validMask);
				if (hit_aabb_mask == 0)
					continue;
				if (node.triangleIndex >= 0)
				{
					auto& tri = triangles_data[node.triangleIndex];
					auto tri_v0 = tri.triangle[0];
					auto tri_v1 = tri.triangle[1];
					auto tri_v2 = tri.triangle[2];

					__m512 v0x = _mm512_set1_ps(tri_v0.x);
					__m512 v0y = _mm512_set1_ps(tri_v0.y);
					__m512 v0z = _mm512_set1_ps(tri_v0.z);

					__m512 edge1x = _mm512_set1_ps(tri_v1.x - tri_v0.x);
					__m512 edge1y = _mm512_set1_ps(tri_v1.y - tri_v0.y);
					__m512 edge1z = _mm512_set1_ps(tri_v1.z - tri_v0.z);

					__m512 edge2x = _mm512_set1_ps(tri_v2.x - tri_v0.x);
					__m512 edge2y = _mm512_set1_ps(tri_v2.y - tri_v0.y);
					__m512 edge2z = _mm512_set1_ps(tri_v2.z - tri_v0.z);

					// pvec = cross(ray.dir, edge2)
					__m512 pvec_x = _mm512_sub_ps(_mm512_mul_ps(rays.dirY, edge2z), _mm512_mul_ps(rays.dirZ, edge2y));
					__m512 pvec_y = _mm512_sub_ps(_mm512_mul_ps(rays.dirZ, edge2x), _mm512_mul_ps(rays.dirX, edge2z));
					__m512 pvec_z = _mm512_sub_ps(_mm512_mul_ps(rays.dirX, edge2y), _mm512_mul_ps(rays.dirY, edge2x));

					// det = dot(edge1, pvec)
					__m512 det = _mm512_add_ps(_mm512_mul_ps(edge1x, pvec_x),
																		 _mm512_add_ps(_mm512_mul_ps(edge1y, pvec_y), _mm512_mul_ps(edge1z, pvec_z)));

					// Check determinant: if det is close to zero, ray is parallel to triangle or backface culling.
					// Using 1e-5f as epsilon. Adjust if necessary.
					// _CMP_GT_OQ for a > epsilon (culls back-faces and parallel).
					// For two-sided, use _mm512_abs_ps(det) and compare with epsilon.
					__mmask16 det_mask = _mm512_cmp_ps_mask(det, _mm512_set1_ps(1e-5f), _CMP_GT_OQ);
					// Combine with rays that hit the AABB
					det_mask = _kand_mask16(det_mask, hit_aabb_mask);
					if (det_mask == 0)
						continue;

					__m512 inv_det = _mm512_div_ps(_mm512_set1_ps(1.0f), det);

					// tvec = ray.origin - v0
					__m512 tvec_x = _mm512_sub_ps(rays.originX, v0x);
					__m512 tvec_y = _mm512_sub_ps(rays.originY, v0y);
					__m512 tvec_z = _mm512_sub_ps(rays.originZ, v0z);

					// u = dot(tvec, pvec) * inv_det
					__m512 u =
						_mm512_mul_ps(inv_det,
													_mm512_add_ps(_mm512_mul_ps(tvec_x, pvec_x),
																				_mm512_add_ps(_mm512_mul_ps(tvec_y, pvec_y), _mm512_mul_ps(tvec_z, pvec_z))));

					// Check u bounds: 0 <= u <= 1
					// Corrected uMask logic:
					__mmask16 u_cond1_mask = _mm512_cmp_ps_mask(u, _mm512_set1_ps(0.0f), _CMP_GE_OQ); // u >= 0
					__mmask16 u_cond2_mask = _mm512_cmp_ps_mask(u, _mm512_set1_ps(1.0f), _CMP_LE_OQ); // u <= 1
					__mmask16 u_final_mask = _kand_mask16(u_cond1_mask, u_cond2_mask);
					u_final_mask = _kand_mask16(u_final_mask, det_mask); // Combine with previous valid mask

					if (u_final_mask == 0)
						continue;

					// qvec = cross(tvec, edge1)
					__m512 qvec_x = _mm512_sub_ps(_mm512_mul_ps(tvec_y, edge1z), _mm512_mul_ps(tvec_z, edge1y));
					__m512 qvec_y = _mm512_sub_ps(_mm512_mul_ps(tvec_z, edge1x), _mm512_mul_ps(tvec_x, edge1z));
					__m512 qvec_z = _mm512_sub_ps(_mm512_mul_ps(tvec_x, edge1y), _mm512_mul_ps(tvec_y, edge1x));

					// v = dot(ray.dir, qvec) * inv_det
					__m512 v = _mm512_mul_ps(
						inv_det,
						_mm512_add_ps(_mm512_mul_ps(rays.dirX, qvec_x),
													_mm512_add_ps(_mm512_mul_ps(rays.dirY, qvec_y), _mm512_mul_ps(rays.dirZ, qvec_z))));

					// Check v bounds: v >= 0 and u + v <= 1
					__mmask16 v_cond1_mask = _mm512_cmp_ps_mask(v, _mm512_set1_ps(0.0f), _CMP_GE_OQ); // v >= 0
					__mmask16 uv_sum_cond_mask =
						_mm512_cmp_ps_mask(_mm512_add_ps(u, v), _mm512_set1_ps(1.0f), _CMP_LE_OQ); // u + v <= 1
					__mmask16 v_final_mask = _kand_mask16(v_cond1_mask, uv_sum_cond_mask);
					v_final_mask = _kand_mask16(v_final_mask, u_final_mask); // Combine with previous valid mask

					if (v_final_mask == 0)
						continue;

					// t = dot(edge2, qvec) * inv_det
					__m512 t_intersect =
						_mm512_mul_ps(inv_det,
													_mm512_add_ps(_mm512_mul_ps(edge2x, qvec_x),
																				_mm512_add_ps(_mm512_mul_ps(edge2y, qvec_y), _mm512_mul_ps(edge2z, qvec_z))));

					// Check t bounds: t >= packet_tMin_overall (or a small epsilon) AND t < outHit.t (current closest hit)
					// Using a small epsilon for t_min to avoid self-intersection issues.
					__mmask16 t_min_cond_mask = _mm512_cmp_ps_mask(t_intersect, _mm512_set1_ps(1e-4f), _CMP_GT_OQ); // t > epsilon
					__mmask16 t_max_cond_mask = _mm512_cmp_ps_mask(t_intersect, outHit.t, _CMP_LT_OQ); // t < current closest hit

					__mmask16 t_final_mask = _kand_mask16(t_min_cond_mask, t_max_cond_mask);
					t_final_mask = _kand_mask16(t_final_mask, v_final_mask); // Combine with previous valid mask

					// If any rays passed all tests for this triangle:
					if (t_final_mask != 0)
					{
						// Update outHit for the rays that found a closer intersection
						outHit.t = _mm512_mask_blend_ps(t_final_mask, outHit.t, t_intersect);

						// Update triangleIndex using a standard blend intrinsic
						// This sets node.triangleIndex for lanes where t_final_mask is true, keeps old values otherwise.
						outHit.triangleIndex =
							_mm512_mask_blend_epi32(t_final_mask, outHit.triangleIndex, _mm512_set1_epi32(node.triangleIndex));

						// Update the overall hitMask for the packet
						outHit.hitMask = _kor_mask16(outHit.hitMask, t_final_mask);

						alignas(64) float u_lanes[RAY_PACKET_WIDTH];
						alignas(64) float v_lanes[RAY_PACKET_WIDTH];
						_mm512_store_ps(u_lanes, u);
						_mm512_store_ps(v_lanes, v);

						// Iterate through the lanes (rays) and update uv for those that hit
						for (int i = 0; i < RAY_PACKET_WIDTH; ++i)
						{
							// Check if the i-th bit in t_final_mask is set
							if ((t_final_mask >> i) & 1)
							{
								outHit.uv[i].x = u_lanes[i];
								outHit.uv[i].y = v_lanes[i];
							}
							outHit.data[i] = tri.data;
						}
					}
					// After processing a leaf, continue to the next node in the stack
					continue;
				}

				// If it's an internal node, push children onto the stack (if they exist)
				// A common optimization is to test the closer child first, based on ray direction,
				// but this basic version pushes them in fixed order.
				if (node.left >= 0)
				{
					// Optional: Add a check here to see if the left child's AABB could possibly contain a hit
					// closer than outHit.t before pushing. This is implicitly handled by passing outHit.t
					// to the AABB intersection test at the beginning of the loop for the child.
					if (stackPtr < 64)
					{ // Basic stack overflow check
						stack[stackPtr++] = {node.left};
					}
				}
				if (node.right >= 0)
				{
					if (stackPtr < 64)
					{ // Basic stack overflow check
						stack[stackPtr++] = {node.right};
					}
				}
			}

			return outHit.hitMask != 0; // Return true if any ray in the packet hit something
#else
			return intersect_packet_glm(rays, outHit);
#endif
		}

	private:
		std::vector<TriangleWithData> triangles;
		std::vector<BVHNode> nodes;
		bool built = false;
		bool changed = false;
		bool building = false;
		std::mutex build_mutex;
		std::condition_variable build_cv;

		int buildRecursive(std::vector<int>& indices)
		{
			AABB nodeBounds;
			auto triangles_data = triangles.data();
			for (int i : indices)
				nodeBounds.expand(triangles_data[i].bounds);

			int currentIndex = (int)nodes.size();
			nodes.emplace_back();
			BVHNode& node = nodes.back();
			node.bounds = nodeBounds;

			if (indices.size() == 1)
			{
				node.triangleIndex = indices[0];
				return currentIndex;
			}

			AABB centroidBounds;
			for (int i : indices)
				centroidBounds.expand(triangles_data[i].centroid);

			glm::vec3 extents = centroidBounds.max - centroidBounds.min;
			int axis = (extents.y > extents.x) ? 1 : 0;
			axis = (extents.z > extents[axis]) ? 2 : axis;

			std::sort(indices.begin(), indices.end(),
								[&](int a, int b) { return triangles_data[a].centroid[axis] < triangles_data[b].centroid[axis]; });

			size_t mid = indices.size() / 2;
			std::vector<int> left(indices.begin(), indices.begin() + mid);
			std::vector<int> right(indices.begin() + mid, indices.end());

			node.left = buildRecursive(left);
			node.right = buildRecursive(right);
			return currentIndex;
		}

		void fixNodeIndices(int index)
		{
			BVHNode& node = nodes[index];
			if (node.triangleIndex >= 0)
				return;
			if (node.left >= 0)
			{
				fixNodeIndices(node.left);
			}
			if (node.right >= 0)
			{
				fixNodeIndices(node.right);
			}
		}
	};
} // namespace zg::exp::raytracing
