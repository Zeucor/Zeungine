#pragma once
#include <anex/IEntity.hpp>
#include <anex/modules/gl/vaos/VAO.hpp>
#include <anex/modules/gl/shaders/Shader.hpp>
namespace anex::modules::gl
{
	struct GLEntity : anex::IEntity, vaos::VAO
  {
    glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 model;
		shaders::Shader &shader;
		bool affectedByShadows = true;
    GLEntity(anex::IWindow &window, const shaders::RuntimeConstants &constants, const uint32_t &indiceCount, const uint32_t &elementCount, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);
		virtual void update();
		virtual void preRender();
    void render() override;
		virtual void postRender();
		const glm::mat4 &getModelMatrix();
  };
}