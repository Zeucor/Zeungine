#include <zg/math/Rotations.hpp>
glm::vec3 zg::math::Rotations::Vec3AroundVec3(glm::vec3 pointToRotate, glm::vec3 rotationCenter, glm::vec3 angleDegrees)
{
	glm::vec3 translatedPoint = pointToRotate - rotationCenter;
    glm::mat4 model(1.0f);
	model = glm::rotate(model, glm::radians(angleDegrees.x), {1, 0, 0});
	model = glm::rotate(model, glm::radians(angleDegrees.y), {0, 1, 0});
	model = glm::rotate(model, glm::radians(angleDegrees.z), {0, 0, 1});
	glm::vec4 rotatedPoint4D = model * glm::vec4(translatedPoint, 1.0f);
	glm::vec3 rotatedPoint = glm::vec3(rotatedPoint4D);
	glm::vec3 finalRotatedPoint = rotatedPoint + rotationCenter;
	return finalRotatedPoint;
}
