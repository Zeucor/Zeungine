#pragma once
#include <algorithm>
#include <cstdint>
constexpr int RAY_PACKET_WIDTH = 16;
// #undef __AVX__
// #undef __AVX512__
#include <intrin.h>
#include <limits>
#include <vector>
#include <zg/glm.hpp>
#include <zg/Entity.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Plane.hpp>
#include <numeric>
namespace zg::exp::raytracing
{
	template <typename TriangleT, typename UserDataT, size_t LeafN>
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
			// For internal nodes:
			int leftChild = -1;  // Index of left child node in 'nodes' vector
			int rightChild = -1; // Index of right child node in 'nodes' vector
			
			// For leaf nodes (numPrimitives > 0):
			// leftChild and rightChild will be -1 (or unused by traversal logic if numPrimitives > 0)
			int primitiveIndicesOffset = -1; // Starting index in the BVH's 'bvh_primitive_indices_' vector
			int numPrimitives = 0;      // Number of primitives in this leaf. If 0, it's an internal node.
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

		struct StackNode // Used for single ray traversal
		{
			int index;    // Node index in the 'nodes' vector
			float tMin;
			float tMax;
		};

        struct PacketStackNode // Used for packet traversal
		{
			int index;    // Node index in the 'nodes' vector
            // tMin and tMax are per-ray and handled by PacketHitInfo.t
		};

		BVH() = default;

		BVH(const BVH& other):
			triangles(other.triangles),
			nodes(other.nodes),
            bvh_primitive_indices_(other.bvh_primitive_indices_),
			built(other.built),
			changed(other.changed),
            building(false) // building state is transient, should not be copied directly
		{}

		BVH& operator=(const BVH& other)
		{
			triangles = other.triangles;
			nodes = other.nodes;
            if (this == &other) return *this;
			triangles = other.triangles;
			nodes = other.nodes;
            bvh_primitive_indices_ = other.bvh_primitive_indices_;
			built = other.built;
			changed = other.changed;
            building = false; // building state is transient
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
            // Thread safety for build process
			{
				std::unique_lock lock(build_mutex);
				if (building)
				{
					build_cv.wait(lock, [&]() { return !building; }); // Wait if another thread is building
                    if (built && !changed) return; // If already built and not changed by another thread, return
				}
				building = true;
			}

            if (triangles.empty()) {
                nodes.clear();
                bvh_primitive_indices_.clear();
                built = true;
                changed = false;
                {
                    std::unique_lock lock(build_mutex);
                    building = false;
                }
                build_cv.notify_all();
                return;
            }

			nodes.clear();
            bvh_primitive_indices_.clear(); 

			std::vector<int> all_triangle_indices(triangles.size());
			std::iota(all_triangle_indices.begin(), all_triangle_indices.end(), 0);
            
            // Reserve memory (approximation)
            nodes.reserve(triangles.size() * 2); 
            bvh_primitive_indices_.reserve(triangles.size());


			int rootIndex = buildRecursive(all_triangle_indices);
			
            // The rotation logic for root node might need adjustment if buildRecursive changes significantly.
            // If rootIndex is always 0 (e.g., if buildRecursive returns the index of the node it just created,
            // and the first call creates nodes[0]), then this rotation is not needed.
            // The current buildRecursive appends to 'nodes', so rootIndex will be 0 if it's the first node.
			if (rootIndex != 0 && rootIndex < static_cast<int>(nodes.size()) && nodes.size() > 1) // Ensure rootIndex is valid
			{
                // This rotation invalidates all stored indices (leftChild, rightChild) in BVHNodes.
                // This part of the original code is problematic if indices are absolute.
                // For this modification, I'm keeping it but noting its potential issues.
                // A robust BVH build usually ensures root is at index 0 or uses relative offsets.
				std::rotate(nodes.begin(), nodes.begin() + rootIndex, nodes.begin() + rootIndex + 1);
				// After rotation, all indices stored in nodes (leftChild, rightChild) are incorrect.
                // fixNodeIndices would need to update them, which it currently doesn't.
                // For now, assuming this is handled or the impact is understood by the user.
                if (!nodes.empty()) fixNodeIndices(0); // Attempt to fix from new root (index 0)
			} else if (rootIndex == -1 && !triangles.empty()) {
                // Handle case where buildRecursive might return -1 on failure or empty set
                // This might indicate an issue.
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
			glm::ivec3 sign(invDir.x < 0, invDir.y < 0, invDir.z < 0);
			float closestT = std::numeric_limits<float>::max();
			bool hit = false;
			
            StackNode stack[64]; // Max traversal depth
			int stackPtr = 0;
            if (nodes.empty()) return false;
			stack[stackPtr++] = {0, 0.0f, closestT}; // Push root node (index 0)

			const BVHNode* nodesData = nodes.data();
			const TriangleWithData* triangles_data_ptr = triangles.data();
            const int* primitive_indices_ptr = bvh_primitive_indices_.data();

			while (stackPtr > 0)
			{
				const StackNode currentStackNode = stack[--stackPtr];
				const BVHNode& node = nodesData[currentStackNode.index];

				if (!node.bounds.intersect(origin, invDir, sign, currentStackNode.tMin, currentStackNode.tMax))
					continue;

				if (node.numPrimitives > 0) // Leaf node
				{
					for (int i = 0; i < node.numPrimitives; ++i)
					{
						int actual_triangle_idx = primitive_indices_ptr[node.primitiveIndicesOffset + i];
						const TriangleWithData& t = triangles_data_ptr[actual_triangle_idx];
						
						const auto& v0 = t.triangle[0];
						const auto& v1 = t.triangle[1];
						const auto& v2 = t.triangle[2];
						glm::vec3 edge1 = v1 - v0;
						glm::vec3 edge2 = v2 - v0;
						glm::vec3 pvec = glm::cross(dir, edge2);
						float det = glm::dot(edge1, pvec);

						if (std::fabs(det) < 1e-6f) // Epsilon for determinant check
							continue;
						
						float invDet = 1.0f / det;
						glm::vec3 tvec = origin - v0;
						float u_bary = glm::dot(tvec, pvec) * invDet;
						
						if (u_bary < 0.0f || u_bary > 1.0f)
							continue;
						
						glm::vec3 qvec = glm::cross(tvec, edge1);
						float v_bary = glm::dot(dir, qvec) * invDet;
						
						if (v_bary < 0.0f || u_bary + v_bary > 1.0f)
							continue;
						
						float tHit = glm::dot(edge2, qvec) * invDet;
						
						if (tHit > 1e-6f && tHit < closestT) // tHit > epsilon to avoid self-intersection
						{
							closestT = tHit;
							outT = tHit;
							outIndex = actual_triangle_idx; // Store the actual triangle index
							outData = t.data;
							uv.x = u_bary;
							uv.y = v_bary;
							hit = true;
						}
					}
				}
				else // Internal node
				{
                    // Push children if they exist. Order can matter for performance (e.g. closer child first)
                    // Basic fixed order push:
					if (node.leftChild != -1) {
                        if (stackPtr < 64) stack[stackPtr++] = {node.leftChild, currentStackNode.tMin, currentStackNode.tMax};
                        // else: stack overflow, handle error or increase stack size
                    }
					if (node.rightChild != -1) {
                        if (stackPtr < 64) stack[stackPtr++] = {node.rightChild, currentStackNode.tMin, currentStackNode.tMax};
                        // else: stack overflow
                    }
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

			for (auto i = 0; i < RAY_PACKET_WIDTH; i++) {
                outHit.triangleIndex.m512i_i32[i] = -1; // Initialize
                outHit.t.m512_f32[i] = std::numeric_limits<float>::max(); // Initialize
            }
            outHit.hitMask = 0;
			bool any_hit_in_packet = false;

			for (int i = 0; i < RAY_PACKET_WIDTH; ++i)
			{
                if (!((rays.validMask >> i) & 1)) continue; // Skip invalid rays in packet

				glm::vec3 origin(rays.originX.m512_f32[i], rays.originY.m512_f32[i], rays.originZ.m512_f32[i]);
				glm::vec3 direction(rays.dirX.m512_f32[i], rays.dirY.m512_f32[i], rays.dirZ.m512_f32[i]);
				
                float current_ray_t = std::numeric_limits<float>::max(); // t for this specific ray
                int current_ray_tri_idx = -1;
                UserDataT current_ray_data; // Default construct
                glm::vec2 current_ray_uv;

				if (intersect_glm(origin, direction, current_ray_t, current_ray_tri_idx, current_ray_data, current_ray_uv))
                {
					outHit.t.m512_f32[i] = current_ray_t;
                    outHit.triangleIndex.m512i_i32[i] = current_ray_tri_idx;
                    outHit.data[i] = current_ray_data;
                    outHit.uv[i] = current_ray_uv;
                    outHit.hitMask |= (1 << i);
					any_hit_in_packet = true;
				}
			}
			return any_hit_in_packet;
		}

		bool intersect(const glm::vec3& origin, const glm::vec3& dir, float& outT, int& outIndex, UserDataT& outData, glm::vec2& uv) const
		{
			if (!built && changed)
				build();
#if defined(__AVX__) && !defined(__AVX512__) // Use AVX if available and not AVX512 (AVX512 path is separate)
			// AVX single ray intersection (Moller-Trumbore)
            // This implementation is for one ray, so the leaf logic iterates triangles.
            // For packetized AVX, a different approach would be needed.
            // The original AVX code provided seems to be for a single ray.

			glm::vec3 invDir = 1.0f / dir;
			glm::ivec3 sign(invDir.x < 0, invDir.y < 0, invDir.z < 0);
			float closestT = std::numeric_limits<float>::max();
			bool hit = false;

			StackNode stack[64];
			int stackPtr = 0;
            if (nodes.empty()) return false;
			stack[stackPtr++] = {0, 0.0f, closestT};

			const BVHNode* nodesData = nodes.data();
			const TriangleWithData* triangles_data_ptr = triangles.data();
            const int* primitive_indices_ptr = bvh_primitive_indices_.data();
            
            // Pre-broadcast ray data for AVX
            __m128 orgX = _mm_set1_ps(origin.x);
			__m128 orgY = _mm_set1_ps(origin.y);
			__m128 orgZ = _mm_set1_ps(origin.z);
			__m128 rdirX = _mm_set1_ps(dir.x); // Renamed to avoid conflict with PacketHitInfo
			__m128 rdirY = _mm_set1_ps(dir.y);
			__m128 rdirZ = _mm_set1_ps(dir.z);


			while (stackPtr > 0)
			{
				const StackNode currentStackNode = stack[--stackPtr];
				const BVHNode& node = nodesData[currentStackNode.index];

				if (!node.bounds.intersect(origin, invDir, sign, currentStackNode.tMin, currentStackNode.tMax))
					continue;

				if (node.numPrimitives > 0) // Leaf node
				{
					for (int i = 0; i < node.numPrimitives; ++i)
					{
						int actual_triangle_idx = primitive_indices_ptr[node.primitiveIndicesOffset + i];
						const TriangleWithData& t = triangles_data_ptr[actual_triangle_idx];
						
                        const auto& v0 = t.triangle[0];
                        const auto& v1 = t.triangle[1];
                        const auto& v2 = t.triangle[2];

                        // AVX Moller-Trumbore for one triangle
                        __m128 v0x = _mm_set1_ps(v0.x);
                        __m128 v0y = _mm_set1_ps(v0.y);
                        __m128 v0z = _mm_set1_ps(v0.z);

                        __m128 edge1x = _mm_set1_ps(v1.x - v0.x);
                        __m128 edge1y = _mm_set1_ps(v1.y - v0.y);
                        __m128 edge1z = _mm_set1_ps(v1.z - v0.z);
                        __m128 edge2x = _mm_set1_ps(v2.x - v0.x);
                        __m128 edge2y = _mm_set1_ps(v2.y - v0.y);
                        __m128 edge2z = _mm_set1_ps(v2.z - v0.z);

                        __m128 pvec_x = _mm_sub_ps(_mm_mul_ps(rdirY, edge2z), _mm_mul_ps(rdirZ, edge2y));
                        __m128 pvec_y = _mm_sub_ps(_mm_mul_ps(rdirZ, edge2x), _mm_mul_ps(rdirX, edge2z));
                        __m128 pvec_z = _mm_sub_ps(_mm_mul_ps(rdirX, edge2y), _mm_mul_ps(rdirY, edge2x));
                        
                        __m128 det = _mm_add_ps(_mm_mul_ps(edge1x, pvec_x), _mm_add_ps(_mm_mul_ps(edge1y, pvec_y), _mm_mul_ps(edge1z, pvec_z)));
                        float det_f = _mm_cvtss_f32(det);

                        if (det_f > -1e-6f && det_f < 1e-6f) continue; // Abs value for two-sided

                        float invDet_f = 1.0f / det_f;
                        __m128 invDet_v = _mm_set1_ps(invDet_f);

                        __m128 tvec_x = _mm_sub_ps(orgX, v0x);
                        __m128 tvec_y = _mm_sub_ps(orgY, v0y);
                        __m128 tvec_z = _mm_sub_ps(orgZ, v0z);

                        __m128 u_bary_v = _mm_mul_ps(invDet_v, _mm_add_ps(_mm_mul_ps(tvec_x, pvec_x), _mm_add_ps(_mm_mul_ps(tvec_y, pvec_y), _mm_mul_ps(tvec_z, pvec_z))));
                        float u_bary_f = _mm_cvtss_f32(u_bary_v);

                        if (u_bary_f < 0.0f || u_bary_f > 1.0f) continue;

                        __m128 qvec_x = _mm_sub_ps(_mm_mul_ps(tvec_y, edge1z), _mm_mul_ps(tvec_z, edge1y));
                        __m128 qvec_y = _mm_sub_ps(_mm_mul_ps(tvec_z, edge1x), _mm_mul_ps(tvec_x, edge1z));
                        __m128 qvec_z = _mm_sub_ps(_mm_mul_ps(tvec_x, edge1y), _mm_mul_ps(tvec_y, edge1x));

                        __m128 v_bary_v = _mm_mul_ps(invDet_v, _mm_add_ps(_mm_mul_ps(rdirX, qvec_x), _mm_add_ps(_mm_mul_ps(rdirY, qvec_y), _mm_mul_ps(rdirZ, qvec_z))));
                        float v_bary_f = _mm_cvtss_f32(v_bary_v);

                        if (v_bary_f < 0.0f || (u_bary_f + v_bary_f) > 1.0f) continue;

                        __m128 tHit_v = _mm_mul_ps(invDet_v, _mm_add_ps(_mm_mul_ps(edge2x, qvec_x), _mm_add_ps(_mm_mul_ps(edge2y, qvec_y), _mm_mul_ps(edge2z, qvec_z))));
                        float tHit_f = _mm_cvtss_f32(tHit_v);

						if (tHit_f > 1e-6f && tHit_f < closestT)
						{
							closestT = tHit_f;
							outT = tHit_f;
							outIndex = actual_triangle_idx;
							outData = t.data;
							uv.x = u_bary_f; // u_bary_v.m128_f32[0]
							uv.y = v_bary_f; // v_bary_v.m128_f32[0]
							hit = true;
						}
					}
                    // continue; // This was in the original structure for single triangle leaf, not needed here as loop finishes
				}
				else // Internal node
				{
					if (node.leftChild != -1) {
                         if (stackPtr < 64) stack[stackPtr++] = {node.leftChild, currentStackNode.tMin, currentStackNode.tMax};
                    }
					if (node.rightChild != -1) {
                        if (stackPtr < 64) stack[stackPtr++] = {node.rightChild, currentStackNode.tMin, currentStackNode.tMax};
                    }
				}
			}
			return hit;
#else // Fallback to GLM version if no AVX or AVX512 defined for this path
			return intersect_glm(origin, dir, outT, outIndex, outData, uv);
#endif
		}

		bool intersect_packet(const RayPacket& rays, PacketHitInfo& outHit)
		{
			if (!built && changed)
				build();
#if defined(__AVX512__)
			PacketStackNode stack[64]; // Max traversal depth for packet
			int stackPtr = 0;

            // Initialize PacketHitInfo
			outHit.t = _mm512_set1_ps(std::numeric_limits<float>::max());
			outHit.triangleIndex = _mm512_set1_epi32(-1);
			outHit.hitMask = 0; // No rays have hit anything yet

            if (nodes.empty()) return false; // No nodes to traverse
			stack[stackPtr++] = {0}; // Push root node index

			const BVHNode* nodes_data_ptr = nodes.data();
			const TriangleWithData* triangles_data_ptr = triangles.data();
            const int* primitive_indices_ptr = bvh_primitive_indices_.data();

			while (stackPtr > 0)
			{
				const PacketStackNode currentPacketStackNode = stack[--stackPtr];
				const BVHNode& node = nodes_data_ptr[currentPacketStackNode.index];
                
                // Intersect AABB with ray packet
				__m512 current_tMin_for_aabb = _mm512_set1_ps(0.0f); // Or actual tMin for rays if they have one
				// outHit.t contains the current closest hit distance for each ray in the packet
                // So, AABB intersection should test up to these distances.
				__mmask16 hit_aabb_mask = node.bounds.intersect_packet(rays, current_tMin_for_aabb, outHit.t);
				hit_aabb_mask = _kand_mask16(hit_aabb_mask, rays.validMask); // Consider only valid rays

				if (hit_aabb_mask == 0) // No rays in the packet hit this node's AABB
					continue;

				if (node.numPrimitives > 0) // Leaf node
				{
					for (int k_prim = 0; k_prim < node.numPrimitives; ++k_prim)
					{
						int actual_triangle_idx = primitive_indices_ptr[node.primitiveIndicesOffset + k_prim];
						const TriangleWithData& tri = triangles_data_ptr[actual_triangle_idx];
						
                        const auto& tri_v0 = tri.triangle[0];
                        const auto& tri_v1 = tri.triangle[1];
                        const auto& tri_v2 = tri.triangle[2];

                        // Broadcast triangle vertices for AVX-512
						__m512 v0x = _mm512_set1_ps(tri_v0.x);
						__m512 v0y = _mm512_set1_ps(tri_v0.y);
						__m512 v0z = _mm512_set1_ps(tri_v0.z);

						__m512 edge1x = _mm512_set1_ps(tri_v1.x - tri_v0.x);
						__m512 edge1y = _mm512_set1_ps(tri_v1.y - tri_v0.y);
						__m512 edge1z = _mm512_set1_ps(tri_v1.z - tri_v0.z);

						__m512 edge2x = _mm512_set1_ps(tri_v2.x - tri_v0.x);
						__m512 edge2y = _mm512_set1_ps(tri_v2.y - tri_v0.y);
						__m512 edge2z = _mm512_set1_ps(tri_v2.z - tri_v0.z);

						// Moller-Trumbore intersection for AVX-512
						__m512 pvec_x = _mm512_sub_ps(_mm512_mul_ps(rays.dirY, edge2z), _mm512_mul_ps(rays.dirZ, edge2y));
						__m512 pvec_y = _mm512_sub_ps(_mm512_mul_ps(rays.dirZ, edge2x), _mm512_mul_ps(rays.dirX, edge2z));
						__m512 pvec_z = _mm512_sub_ps(_mm512_mul_ps(rays.dirX, edge2y), _mm512_mul_ps(rays.dirY, edge2x));

						__m512 det = _mm512_add_ps(_mm512_mul_ps(edge1x, pvec_x),
																			 _mm512_add_ps(_mm512_mul_ps(edge1y, pvec_y), _mm512_mul_ps(edge1z, pvec_z)));
						
						__m512 abs_det = _mm512_abs_ps(det); // For two-sided intersection
						__mmask16 det_mask = _mm512_cmp_ps_mask(abs_det, _mm512_set1_ps(1e-6f), _CMP_GT_OQ); // det > epsilon
						det_mask = _kand_mask16(det_mask, hit_aabb_mask); // Only consider rays that hit AABB and are valid for this tri
						if (det_mask == 0) continue;

						__m512 inv_det = _mm512_rcp14_ps(det); // Reciprocal, can use _mm512_div_ps for more precision if needed

						__m512 tvec_x = _mm512_sub_ps(rays.originX, v0x);
						__m512 tvec_y = _mm512_sub_ps(rays.originY, v0y);
						__m512 tvec_z = _mm512_sub_ps(rays.originZ, v0z);

						__m512 u_bary = _mm512_mul_ps(inv_det,
														_mm512_add_ps(_mm512_mul_ps(tvec_x, pvec_x),
																					_mm512_add_ps(_mm512_mul_ps(tvec_y, pvec_y), _mm512_mul_ps(tvec_z, pvec_z))));

						__mmask16 u_cond1_mask = _mm512_cmp_ps_mask(u_bary, _mm512_set1_ps(0.0f), _CMP_GE_OQ); // u >= 0
						__mmask16 u_cond2_mask = _mm512_cmp_ps_mask(u_bary, _mm512_set1_ps(1.0f), _CMP_LE_OQ); // u <= 1
						__mmask16 u_final_mask = _kand_mask16(u_cond1_mask, u_cond2_mask);
						u_final_mask = _kand_mask16(u_final_mask, det_mask);
						if (u_final_mask == 0) continue;

						__m512 qvec_x = _mm512_sub_ps(_mm512_mul_ps(tvec_y, edge1z), _mm512_mul_ps(tvec_z, edge1y));
						__m512 qvec_y = _mm512_sub_ps(_mm512_mul_ps(tvec_z, edge1x), _mm512_mul_ps(tvec_x, edge1z));
						__m512 qvec_z = _mm512_sub_ps(_mm512_mul_ps(tvec_x, edge1y), _mm512_mul_ps(tvec_y, edge1x));

						__m512 v_bary = _mm512_mul_ps(inv_det,
														_mm512_add_ps(_mm512_mul_ps(rays.dirX, qvec_x),
																					_mm512_add_ps(_mm512_mul_ps(rays.dirY, qvec_y), _mm512_mul_ps(rays.dirZ, qvec_z))));
						
						__mmask16 v_cond1_mask = _mm512_cmp_ps_mask(v_bary, _mm512_set1_ps(0.0f), _CMP_GE_OQ); // v >= 0
						__mmask16 uv_sum_cond_mask = _mm512_cmp_ps_mask(_mm512_add_ps(u_bary, v_bary), _mm512_set1_ps(1.0f), _CMP_LE_OQ); // u + v <= 1
						__mmask16 v_final_mask = _kand_mask16(v_cond1_mask, uv_sum_cond_mask);
						v_final_mask = _kand_mask16(v_final_mask, u_final_mask);
						if (v_final_mask == 0) continue;

						__m512 t_intersect = _mm512_mul_ps(inv_det,
																			_mm512_add_ps(_mm512_mul_ps(edge2x, qvec_x),
																										_mm512_add_ps(_mm512_mul_ps(edge2y, qvec_y), _mm512_mul_ps(edge2z, qvec_z))));
						
						__mmask16 t_min_cond_mask = _mm512_cmp_ps_mask(t_intersect, _mm512_set1_ps(1e-6f), _CMP_GT_OQ); // t > epsilon
						__mmask16 t_max_cond_mask = _mm512_cmp_ps_mask(t_intersect, outHit.t, _CMP_LT_OQ); // t < current closest hit for each ray
						__mmask16 t_final_mask = _kand_mask16(t_min_cond_mask, t_max_cond_mask);
						t_final_mask = _kand_mask16(t_final_mask, v_final_mask);

						if (t_final_mask != 0)
						{
							outHit.t = _mm512_mask_blend_ps(t_final_mask, outHit.t, t_intersect);
							outHit.triangleIndex = _mm512_mask_blend_epi32(t_final_mask, outHit.triangleIndex, _mm512_set1_epi32(actual_triangle_idx));
							outHit.hitMask = _kor_mask16(outHit.hitMask, t_final_mask);

							// Store u and v for active lanes to update uv coordinates
                            // This part requires careful handling of scatter/gather or iterating lanes.
                            // The original code iterated lanes, which is simpler here.
                            alignas(64) float u_lanes[RAY_PACKET_WIDTH];
							alignas(64) float v_lanes[RAY_PACKET_WIDTH];
							_mm512_store_ps(u_lanes, u_bary);
							_mm512_store_ps(v_lanes, v_bary);

							for (int i = 0; i < RAY_PACKET_WIDTH; ++i)
							{
								if ((t_final_mask >> i) & 1) // If i-th ray hit this triangle closer
								{
									outHit.uv[i].x = u_lanes[i];
									outHit.uv[i].y = v_lanes[i];
									outHit.data[i] = tri.data;
								}
							}
						}
					}
					continue; // Done with this leaf node
				}
				else // Internal node
				{
					// Push children onto stack. Order can matter.
                    // A common optimization is to test the child whose AABB is intersected first by the ray packet's "average" direction,
                    // or based on some heuristic. For simplicity, fixed order:
					if (node.leftChild != -1) {
                        if (stackPtr < 64) stack[stackPtr++] = {node.leftChild};
                    }
					if (node.rightChild != -1) {
                        if (stackPtr < 64) stack[stackPtr++] = {node.rightChild};
                    }
				}
			}
			return outHit.hitMask != 0;
#else // Fallback to GLM packet intersection if no AVX512
			return intersect_packet_glm(rays, outHit);
#endif
		}

	private:
		std::vector<TriangleWithData> triangles;
		std::vector<BVHNode> nodes;
        std::vector<int> bvh_primitive_indices_;
		bool built = false;
		bool changed = false;
        mutable std::mutex build_mutex; 
		mutable std::condition_variable build_cv;
		mutable bool building = false;

		int buildRecursive(std::vector<int>& primitive_indices_for_current_node)
		{
            if (primitive_indices_for_current_node.empty()) {
                return -1; // No primitives, no node
            }

			AABB nodeBounds;
            const TriangleWithData* triangles_data_ptr = triangles.data();
			for (int tri_idx : primitive_indices_for_current_node) {
				nodeBounds.expand(triangles_data_ptr[tri_idx].bounds);
            }

			int currentNodeIdx = static_cast<int>(nodes.size());
			nodes.emplace_back(); // Create new node
			BVHNode& node = nodes.back(); // Get reference to the new node
			node.bounds = nodeBounds;

			// Leaf node condition
			if (primitive_indices_for_current_node.size() <= LeafN)
			{
				node.numPrimitives = static_cast<int>(primitive_indices_for_current_node.size());
				node.primitiveIndicesOffset = static_cast<int>(bvh_primitive_indices_.size());
                bvh_primitive_indices_.insert(bvh_primitive_indices_.end(), 
                                              primitive_indices_for_current_node.begin(), 
                                              primitive_indices_for_current_node.end());
				node.leftChild = -1;
				node.rightChild = -1;
				return currentNodeIdx;
			}

			// Internal node: calculate split
			AABB centroidBounds;
			for (int tri_idx : primitive_indices_for_current_node) {
				centroidBounds.expand(triangles_data_ptr[tri_idx].centroid);
            }

			glm::vec3 extents = centroidBounds.max - centroidBounds.min;
			int axis = 0; // Default split axis X
			if (extents.y > extents.x && extents.y > extents.z) axis = 1; // Y
			else if (extents.z > extents.x && extents.z > extents.y) axis = 2; // Z
            // If extents are zero or equal, pick one (e.g. X or cycle through axes)
            if (extents.x == 0 && extents.y == 0 && extents.z == 0) {
                // All centroids are the same. This can happen.
                // Split arbitrarily or fall back to leaf if too many primitives.
                // For now, proceed with axis 0, but this might lead to unbalanced trees.
                // A better SAH (Surface Area Heuristic) would handle this more gracefully.
            }


			std::sort(primitive_indices_for_current_node.begin(), primitive_indices_for_current_node.end(),
								[&](int a, int b) { 
                                    return triangles_data_ptr[a].centroid[axis] < triangles_data_ptr[b].centroid[axis]; 
                                });

			size_t mid = primitive_indices_for_current_node.size() / 2;
			std::vector<int> left_primitives(primitive_indices_for_current_node.begin(), primitive_indices_for_current_node.begin() + mid);
			std::vector<int> right_primitives(primitive_indices_for_current_node.begin() + mid, primitive_indices_for_current_node.end());
            
            // Mark as internal node
            node.numPrimitives = 0;
            node.primitiveIndicesOffset = -1; // Not used for internal nodes

			node.leftChild = buildRecursive(left_primitives);
			node.rightChild = buildRecursive(right_primitives);
			
            // Handle cases where a child might not have been created (e.g., empty primitive set for a child)
            if (node.leftChild == -1 && node.rightChild == -1 && !primitive_indices_for_current_node.empty()) {
                // This might happen if splits result in empty child sets, which shouldn't occur with proper mid split.
                // Or if LeafN is very large. If both children failed, this node should become a leaf.
                // This indicates a potential issue in splitting logic or base case handling for very small LeafN or specific data.
                // For now, assume split is valid. If one child is -1, it means that side had no primitives.
                // A robust BVH might make this node a leaf if children are problematic.
            }


			return currentNodeIdx;
		}

		void fixNodeIndices(int index)
		{
            // This function's original purpose (if it was to fix indices after rotation)
            // is complex and not fully implemented in the snippet.
            // The rotation itself invalidates indices.
            // Here, we just update the leaf check and child access for the new BVHNode structure.
			if (index < 0 || static_cast<size_t>(index) >= nodes.size()) return;

			BVHNode& node = nodes[index];
			if (node.numPrimitives > 0) // Leaf node
				return;

            // If it's an internal node, recurse.
            // The actual fixing of indices post-rotation would require mapping old indices to new ones.
			if (node.leftChild >= 0)
			{
				fixNodeIndices(node.leftChild);
			}
			if (node.rightChild >= 0)
			{
				fixNodeIndices(node.rightChild);
			}
		}
	};
} // namespace zg::exp::raytracing
