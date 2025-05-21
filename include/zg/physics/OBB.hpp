#pragma once
#include <zg/glm.hpp>
namespace zg::physics
{
	struct OBB
	{
		glm::vec3 center; ///< World space center of the OBB.
		glm::vec3 halfExtents; ///< Local half-extents along the OBB's axes.
		glm::quat orientation; ///< World space orientation of the OBB.
		glm::vec3 axes[3]; ///< World space axes of the OBB (derived from orientation).

		/**
		 * @brief Constructs an OBB from a world position, orientation, and calculated half-extents.
		 * @param position The world space position (center) of the OBB.
		 * @param obbOrientation The world space orientation (rotation) of the OBB.
		 * @param calculatedHalfExtents The half-dimensions (radii) along the OBB's local axes.
		 */
		OBB(const glm::vec3& c, const glm::quat& o, const glm::vec3& he) : center(c), orientation(o), halfExtents(he)
		{
			glm::mat3 rotMat = glm::mat3_cast(orientation);
			axes[0] = rotMat[0]; // Right
			axes[1] = rotMat[1]; // Up
			axes[2] = rotMat[2]; // Forward
		}

		// Projects the OBB onto an axis and returns the radius (half-extent) of the projection.
		float projectedRadius(const glm::vec3& axis) const
		{
			return std::abs(glm::dot(axis, axes[0])) * halfExtents.x + std::abs(glm::dot(axis, axes[1])) * halfExtents.y +
				std::abs(glm::dot(axis, axes[2])) * halfExtents.z;
		}

		// Helper to get the 8 vertices of the OBB
		std::vector<glm::vec3> getVertices() const
		{
			std::vector<glm::vec3> vertices(8);
			glm::vec3 extX = axes[0] * halfExtents.x;
			glm::vec3 extY = axes[1] * halfExtents.y;
			glm::vec3 extZ = axes[2] * halfExtents.z;
			vertices[0] = center - extX - extY - extZ;
			vertices[1] = center + extX - extY - extZ;
			vertices[2] = center + extX + extY - extZ;
			vertices[3] = center - extX + extY - extZ;
			vertices[4] = center - extX - extY + extZ;
			vertices[5] = center + extX - extY + extZ;
			vertices[6] = center + extX + extY + extZ;
			vertices[7] = center - extX + extY + extZ;
			return vertices;
		}

		// Helper to get the 4 vertices of a specific face
		std::vector<glm::vec3> getFaceVertices(int axisIndex, bool positiveFace) const
		{
			std::vector<glm::vec3> faceVertices(4);
			glm::vec3 faceNormal = axes[axisIndex] * (positiveFace ? 1.0f : -1.0f);
			glm::vec3 faceCenter = center + faceNormal * halfExtents[axisIndex];

			int axis1 = (axisIndex + 1) % 3;
			int axis2 = (axisIndex + 2) % 3;

			glm::vec3 ext1 = axes[axis1] * halfExtents[axis1];
			glm::vec3 ext2 = axes[axis2] * halfExtents[axis2];

			// Ensure consistent winding order (e.g., counter-clockwise when looking against the normal)
			faceVertices[0] = faceCenter - ext1 - ext2;
			faceVertices[1] = faceCenter + ext1 - ext2;
			faceVertices[2] = faceCenter + ext1 + ext2;
			faceVertices[3] = faceCenter - ext1 + ext2;

			return faceVertices;
		}

		// Finds the face most aligned with a given direction
		void findSupportFace(const glm::vec3& direction, int& bestAxisIndex, bool& positiveFace) const
		{
			float maxDot = -(std::numeric_limits<float>::max)();
			bestAxisIndex = -1;

			for (int i = 0; i < 3; ++i)
			{
				float currentDotPos = glm::dot(direction, axes[i]);
				if (currentDotPos > maxDot)
				{
					maxDot = currentDotPos;
					bestAxisIndex = i;
					positiveFace = true;
				}
				// Check negative axis direction as well
				float currentDotNeg = glm::dot(direction, -axes[i]);
				if (currentDotNeg > maxDot)
				{
					maxDot = currentDotNeg;
					bestAxisIndex = i;
					positiveFace = false;
				}
			}
		}

		// Helper to find the two vertices defining an edge given the axes indices
		// axisParallel: Index of the axis the edge is parallel to
		// axisPerp1/2: Indices of the perpendicular axes defining the edge's position
		// sign1/2: +/- 1 indicating which side of the perpendicular axes the edge lies on
		std::pair<glm::vec3, glm::vec3> getEdgeVertices(int axisParallel, int axisPerp1, int axisPerp2, float sign1,
																										float sign2) const
		{
			glm::vec3 p1 = center + axes[axisPerp1] * halfExtents[axisPerp1] * sign1 +
				axes[axisPerp2] * halfExtents[axisPerp2] * sign2 - axes[axisParallel] * halfExtents[axisParallel];
			glm::vec3 p2 = center + axes[axisPerp1] * halfExtents[axisPerp1] * sign1 +
				axes[axisPerp2] * halfExtents[axisPerp2] * sign2 + axes[axisParallel] * halfExtents[axisParallel];
			return {p1, p2};
		}

		bool isPointInside(glm::vec3 point) const
		{
			auto corners = getCorners();
			// check plane srcbot-ttop
			return (point.x <= corners[0].x && point.y <= corners[0].y && point.z <= corners[0].z &&
							point.x >= corners[7].x && point.y >= corners[7].y && point.z >= corners[7].z) ||
				// check plane rightbot-lefttop
				(point.x <= corners[4].x && point.y <= corners[4].y && point.z <= corners[4].z && point.x >= corners[3].x &&
				 point.y >= corners[3].y && point.z >= corners[3].z) ||
				// check plane tbot-srctop
				(point.x <= corners[5].x && point.y <= corners[5].y && point.z <= corners[5].z && point.x >= corners[2].x &&
				 point.y >= corners[2].y && point.z >= corners[2].z) ||
				// check plane leftbot-righttop
				(point.x <= corners[1].x && point.y <= corners[1].y && point.z <= corners[1].z && point.x >= corners[6].x &&
				 point.y >= corners[6].y && point.z >= corners[6].z);
		}

		/**
		 * @brief Gets the 8 corners of the OBB in world space. Useful for visualization or precise tests.
		 * @return A vector containing the 8 corner points in world space.
		 */
		std::vector<glm::vec3> getCorners() const
		{
			std::vector<glm::vec3> corners(8);
			// Calculate the scaled axes vectors based on half-extents
			glm::vec3 scaledAxisX = axes[0] * halfExtents.x;
			glm::vec3 scaledAxisY = axes[1] * halfExtents.y;
			glm::vec3 scaledAxisZ = axes[2] * halfExtents.z;

			// Compute corners by adding/subtracting scaled axes from the center
			corners[0] = center + scaledAxisX + scaledAxisY + scaledAxisZ; // + + +
			corners[1] = center + scaledAxisX + scaledAxisY - scaledAxisZ; // + + -
			corners[2] = center + scaledAxisX - scaledAxisY + scaledAxisZ; // + - +
			corners[3] = center + scaledAxisX - scaledAxisY - scaledAxisZ; // + - -
			corners[4] = center - scaledAxisX + scaledAxisY + scaledAxisZ; // - + +
			corners[5] = center - scaledAxisX + scaledAxisY - scaledAxisZ; // - + -
			corners[6] = center - scaledAxisX - scaledAxisY + scaledAxisZ; // - - +
			corners[7] = center - scaledAxisX - scaledAxisY - scaledAxisZ; // - - -

			return corners;
		}
	};
} // namespace zg::physics
