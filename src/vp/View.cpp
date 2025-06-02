#include <zg/vp/View.hpp>
#include <zg/Serial.hpp>
using namespace zg;
using namespace zg::vp;
View::View(glm::vec3 _position, glm::vec3 _direction, glm::vec3 _up) : position(_position),
														direction(glm::normalize(_direction)),
														up(_up),
														phi(atan2(this->direction.z, this->direction.x)),
														theta(acos(glm::clamp(this->direction.y, -1.0f, 1.0f)))
{
	update();
}
View::View(glm::vec3 _position, glm::vec3 _direction, glm::vec3 _up, bool _lookAtSet, glm::vec3 _lookAt) : position(_position),
														direction(glm::normalize(_direction)),
														up(_up),
														phi(atan2(this->direction.z, this->direction.x)),
														theta(acos(glm::clamp(this->direction.y, -1.0f, 1.0f))),
														lookAtSet(_lookAtSet),
														lookAt(_lookAt)
{
	update();
}
void View::update()
{
	glm::mat4 newMatrix;
	if (lookAtSet)
		newMatrix = glm::lookAt(position, lookAt, up);
	else
	{
		auto _direction_ = direction;
		newMatrix = glm::lookAt(position, position + _direction_, up);
	}
	{
		matrix = newMatrix;
	}
}
void View::addPhiTheta(float addPhi, float addTheta)
{
	static auto _pi_ = glm::pi<float>();
	addPhi = glm::clamp(addPhi, -_pi_, _pi_ * 2);
	phi += addPhi;
	theta += addTheta;
	theta = glm::clamp(theta, 0.001f, _pi_ - 0.001f);
	glm::vec3 newDirection;
	newDirection.x = sin(theta) * cos(phi);
	newDirection.y = cos(theta);
	newDirection.z = sin(theta) * sin(phi);
	direction = newDirection;
	update();
}
template<>
Serial& serialize(Serial& serial, const std::shared_ptr<zg::vp::View>& viewPointer)
{
	auto& view = *viewPointer;
	serial << true << view.position << view.direction << view.up << view.lookAtSet << view.lookAt;
	return serial;
}
template<>
Serial& deserialize(Serial& serial, std::shared_ptr<zg::vp::View>& viewPointer)
{
	bool wroteBit = false;
	serial >> wroteBit;
	if (!wroteBit)
		return serial;
	glm::vec3 position{0};
	glm::vec3 direction{0};
	glm::vec3 up{0};
	bool lookAtSet = false;
	glm::vec3 lookAt{0};
	serial >> position >> direction >> up >> lookAtSet >> lookAt;
	viewPointer = std::make_shared<zg::vp::View>(position, direction, up, lookAtSet, lookAt);
	return serial;
}
void View::setDirty()
{
	phi = atan2(direction.z, direction.x);
	theta = acos(glm::clamp(direction.y, -1.0f, 1.0f));
	update();
}