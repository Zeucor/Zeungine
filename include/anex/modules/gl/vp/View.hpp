#pragma once
#include <anex/glm.hpp>
#include <anex/IWindow.hpp>
namespace anex::modules::gl::vp
{
	struct View
  {
    glm::vec3 position;
    glm::vec3 direction;
    glm::mat4 matrix;
    float phi;
    float theta;
		using ViewResizeHandler = std::function<void(const glm::vec2&)>;
		std::pair<IWindow::EventIdentifier, std::map<IWindow::EventIdentifier, ViewResizeHandler>> viewResizeHandlers;
    View(const glm::vec3& position, const glm::vec3& direction);
    void update();
		void addPhiTheta(const float &addPhi, const float &addTheta);
		IWindow::EventIdentifier addResizeHandler(const ViewResizeHandler &callback);
		void removeResizeHandler(IWindow::EventIdentifier &id);
		void callResizeHandler(const glm::vec2 &newSize);
  };
}