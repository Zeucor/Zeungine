#pragma once
#include <anex/IScene.hpp>
#include "./lights/Lights.hpp"
#include "./lights/DirectionalLight.hpp"
#include "./lights/SpotLight.hpp"
namespace anex::modules::gl
{
	struct GLScene : anex::IScene
	{
		glm::vec3 cameraPosition;
		glm::vec3 cameraDirection;
    float fov;
		glm::mat4 view;
		glm::mat4 projection;
		std::vector<lights::PointLight> pointLights;
		std::vector<lights::DirectionalLight> directionalLights;
		std::vector<lights::DirectionalLightShadow> directionalLightShadows;
		std::vector<lights::SpotLightShadow> spotLightShadows;
		std::vector<lights::SpotLight> spotLights;
    GLScene(IWindow &window, const glm::vec3 &cameraPosition, const glm::vec3 &cameraDirection, const float &fov);
    void updateView();
    void preRender() override;
		void render() override;
		void entityPreRender(IEntity &entity) override;
  };
}