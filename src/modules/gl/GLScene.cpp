#include <anex/IWindow.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/vaos/VAO.hpp>
using namespace anex::modules::gl;
GLScene::GLScene(IWindow &window, const glm::vec3 &cameraPosition, const glm::vec3 &cameraDirection, const float &fov, textures::Framebuffer *framebufferPointer):
	IScene(window),
	view(cameraPosition, cameraDirection),
	projection((GLWindow &)window, fov),
	framebufferPointer(framebufferPointer)
{};
GLScene::GLScene(IWindow &window, const glm::vec3 &cameraPosition, const glm::vec3 &cameraDirection, const glm::vec2 &orthoSize, textures::Framebuffer *framebufferPointer):
	IScene(window),
	view(cameraPosition, cameraDirection),
	projection((GLWindow &)window, orthoSize),
	framebufferPointer(framebufferPointer)
{};
void GLScene::preRender()
{
	for (auto &directionaLightShadow : directionalLightShadows)
	{
		directionaLightShadow.framebuffer.bind();
		directionaLightShadow.shader.bind();
		directionaLightShadow.shader.setBlock("LightSpaceMatrix", directionaLightShadow.lightSpaceMatrix, sizeof(glm::mat4));
		for (auto &entityPair : entities)
		{
			auto &entityPointer = entityPair.second;
			auto &vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto &glEntity = *std::dynamic_pointer_cast<gl::GLEntity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			const auto &model = glEntity.getModelMatrix();
			directionaLightShadow.shader.setBlock("Model", model);
			vbo.vaoDraw();
		}
		directionaLightShadow.shader.unbind();
		directionaLightShadow.framebuffer.unbind();
	}
	for (auto &spotLightShadow : spotLightShadows)
	{
		spotLightShadow.framebuffer.bind();
		spotLightShadow.shader.bind();
		spotLightShadow.shader.setBlock("LightSpaceMatrix", spotLightShadow.lightSpaceMatrix, sizeof(glm::mat4));
		for (auto &entityPair : entities)
		{
			auto &entityPointer = entityPair.second;
			auto &vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto &glEntity = *std::dynamic_pointer_cast<gl::GLEntity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			const auto &model = glEntity.getModelMatrix();
			spotLightShadow.shader.setBlock("Model", model);
			vbo.vaoDraw();
		}
		spotLightShadow.shader.unbind();
		spotLightShadow.framebuffer.unbind();
	}
	for (auto &pointLightShadow : pointLightShadows)
	{
		pointLightShadow.framebuffer.bind();
		pointLightShadow.shader.bind();
		pointLightShadow.shader.setBlock("PointLightSpaceMatrix", pointLightShadow.shadowTransforms, sizeof(glm::mat4) * 6);
		pointLightShadow.shader.setUniform("nearPlane", pointLightShadow.pointLight.nearPlane);
		pointLightShadow.shader.setUniform("farPlane", pointLightShadow.pointLight.farPlane);
		pointLightShadow.shader.setUniform("lightPos", pointLightShadow.pointLight.position);
		for (auto &entityPair : entities)
		{
			auto &entityPointer = entityPair.second;
			auto &vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto &glEntity = *std::dynamic_pointer_cast<gl::GLEntity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			const auto &model = glEntity.getModelMatrix();
			pointLightShadow.shader.setBlock("Model", model);
			vbo.vaoDraw();
		}
		pointLightShadow.shader.unbind();
		pointLightShadow.framebuffer.unbind();
	}
	auto &glWindow = (GLWindow &)window;
	if (framebufferPointer)
	{
		auto &framebuffer = *framebufferPointer;
		glWindow.glContext.Viewport(0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y);
	}
	else
		glWindow.glContext.Viewport(0, 0, window.windowWidth, window.windowHeight);
	GLcheck(glWindow, "glViewport");
	glWindow.glContext.ClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glWindow.glContext.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
};
void GLScene::render()
{
	preRender();
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->second->render();
	}
};
void GLScene::entityPreRender(IEntity &entity)
{
	GLEntity &glEntity = static_cast<GLEntity&>(entity);
	uint32_t index = 0;
	glm::mat4 directionalLightSpaceMatrices[4];
	for (auto &directionalLightShadow : directionalLightShadows)
	{
		directionalLightSpaceMatrices[index] = directionalLightShadow.lightSpaceMatrix;
		++index;
	}
	glEntity.shader.setBlock("DirectionalLightSpaceMatrices", directionalLightSpaceMatrices, sizeof(glm::mat4) * 4);
	glm::mat4 spotLightSpaceMatrices[4];
	index = 0;
	for (auto &spotLightShadow : spotLightShadows)
	{
		spotLightSpaceMatrices[index] = spotLightShadow.lightSpaceMatrix;
		++index;
	}
	glEntity.shader.setBlock("SpotLightSpaceMatrices", spotLightSpaceMatrices, sizeof(glm::mat4) * 4);
	int32_t unit = 0;
	index = 0;
	uint32_t unitRemaining = 4;
	for (auto &directionalLightShadow : directionalLightShadows)
	{
		glEntity.shader.setTexture("directionalLightSamplers[" + std::to_string(index) + "]", directionalLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto &spotLightShadow : spotLightShadows)
	{
		glEntity.shader.setTexture("spotLightSamplers[" + std::to_string(index) + "]", spotLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto &pointLightShadow : pointLightShadows)
	{
		glEntity.shader.setTexture("pointLightSamplers[" + std::to_string(index) + "]", pointLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
}
