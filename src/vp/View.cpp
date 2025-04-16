#include <zg/vp/View.hpp>
#include <zg/Serial.hpp>
using namespace zg;
using namespace zg::vp;
View::View(glm::vec3 _position, glm::vec3 _direction, glm::vec3 _up) : position(_position),
														direction(glm::normalize(_direction)),
														up(_up)
{
	phi = atan2(direction.z, direction.x);
	theta = acos(glm::clamp(direction.y, -1.0f, 1.0f));
	update();
}
View::View(glm::vec3 _position, glm::vec3 _direction, glm::vec3 _up, bool _lookAtSet, glm::vec3 _lookAt) : position(_position),
														direction(glm::normalize(_direction)),
														up(_up),
														lookAtSet(_lookAtSet),
														lookAt(_lookAt)
{
	phi = atan2(direction.z, direction.x);
	theta = acos(glm::clamp(direction.y, -1.0f, 1.0f));
	update();
}
void View::update()
{
	if (lookAtSet)
		matrix = glm::lookAt(position, lookAt, up);
	else
	{
		auto _direction_ = direction;
		_setMatrix:
		if (std::isnan(direction.x) || std::isnan(direction.y) || std::isnan(direction.z))
		{
			return;
		}
		matrix = glm::lookAt(position, position + _direction_, up);
		if (std::isnan(matrix[0][0]))
		{
			_direction_ += 0.000004;
			goto _setMatrix;
		}
	}
}
void View::addPhiTheta(float addPhi, float addTheta)
{
	phi += addPhi;
	theta += addTheta;
	theta = glm::clamp(theta, 0.001f, glm::pi<float>() - 0.001f);
	glm::dvec3 newDirection;
	newDirection.x = sin(theta) * cos(phi);
	newDirection.y = cos(theta);
	newDirection.z = sin(theta) * sin(phi);
	direction = newDirection;
	update();
}
UniqueIdentifier View::addResizeHandler(const ViewResizeHandler &callback)
{
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
}
void View::removeResizeHandler(UniqueIdentifier &id)
{
	auto &handlers = viewResizeHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
}
void View::callResizeHandler(glm::vec2 newSize)
{
	auto &handlersMap = viewResizeHandlers.second;
	std::vector<ViewResizeHandler> handlersCopy;
	for (const auto &pair : handlersMap)
		handlersCopy.push_back(pair.second);
	for (auto &handler : handlersCopy)
	{
		handler(newSize);
	}
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