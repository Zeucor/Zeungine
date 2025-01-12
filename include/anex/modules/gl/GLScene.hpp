#pragma once
#include <anex/IScene.hpp>
#include "./lights/Lights.hpp"
#include "./lights/PointLight.hpp"
#include "./lights/DirectionalLight.hpp"
#include "./lights/SpotLight.hpp"
#include "./vp/View.hpp"
namespace anex::modules::gl
{
	struct GLScene : anex::IScene
	{
		vp::View view;
    float fov;
		glm::mat4 projection;
		std::vector<lights::PointLight> pointLights;
		std::vector<lights::DirectionalLight> directionalLights;
		std::vector<lights::SpotLight> spotLights;
		std::vector<lights::PointLightShadow> pointLightShadows;
		std::vector<lights::DirectionalLightShadow> directionalLightShadows;
		std::vector<lights::SpotLightShadow> spotLightShadows;
    GLScene(IWindow &window, const glm::vec3 &cameraPosition, const glm::vec3 &cameraDirection, const float &fov);
    void preRender() override;
		void render() override;
		void entityPreRender(IEntity &entity) override;
  };
}