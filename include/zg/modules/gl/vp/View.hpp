#pragma once
#include <zg/glm.hpp>
#include <zg/IWindow.hpp>
namespace zg::modules::gl::vp
{
	struct View
  {
    glm::vec3 position;
    glm::vec3 direction;
    glm::mat4 matrix;
    float phi;
    float theta;
		std::pair<IWindow::EventIdentifier, std::map<IWindow::EventIdentifier, IWindow::ViewResizeHandler>> viewResizeHandlers;
    View(glm::vec3 position, glm::vec3 direction);
    void update();
		void addPhiTheta(float addPhi, float addTheta);
		IWindow::EventIdentifier addResizeHandler(const IWindow::ViewResizeHandler &callback);
		void removeResizeHandler(IWindow::EventIdentifier &id);
		void callResizeHandler(glm::vec2 newSize);
  };
}