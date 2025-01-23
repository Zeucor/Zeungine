#include <iostream>
#include <anex/IWindow.hpp>
#include <anex/crypto/vector.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/textures/TextureFactory.hpp>
#include <anex/modules/gl/textures/FramebufferFactory.hpp>
#include <anex/modules/gl/vaos/VAO.hpp>
using namespace anex::modules::gl;
GLScene::GLScene(IWindow &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov, textures::Framebuffer *framebufferPointer):
	IScene(_window),
	view(cameraPosition, cameraDirection),
	projection((GLWindow &)window, fov),
	framebufferPointer(framebufferPointer)
{
	hookMouseEvents();
};
GLScene::GLScene(IWindow &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize, textures::Framebuffer *framebufferPointer):
	IScene(_window),
	view(cameraPosition, cameraDirection),
	projection((GLWindow &)window, orthoSize),
	framebufferPointer(framebufferPointer)
{
	hookMouseEvents();
};
GLScene::~GLScene()
{
	unhookMouseEvents();
};
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
		glWindow.glContext->Viewport(0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y);
	}
	else
		glWindow.glContext->Viewport(0, 0, window.windowWidth, window.windowHeight);
	GLcheck(glWindow, "glViewport");
	glWindow.glContext->ClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glWindow.glContext->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
};
void GLScene::render()
{
	if (bvh.changed)
	{
		bvh.buildBVH();
	}
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
};
void GLScene::resize(glm::vec2 newSize)
{
	view.callResizeHandler(newSize);
	// projection.orthoSize = newSize;
	// projection.update();
	if (framebufferPointer)
	{
		framebufferPointer->texture.size = {newSize.x, newSize.y, 1, 0};
		textures::FramebufferFactory::destroyFramebuffer(*framebufferPointer);
		textures::TextureFactory::destroyTexture(framebufferPointer->texture);
		textures::TextureFactory::initTexture(framebufferPointer->texture, 0);
		textures::FramebufferFactory::initFramebuffer(*framebufferPointer);
	}
};
void GLScene::postAddEntity(const std::shared_ptr<IEntity>& entity, const std::vector<size_t> &entityIDs)
{
	auto &glEntity = (GLEntity &)*entity;
	if (glEntity.addToBVH)
	{
		bvh.addEntity(glEntity);
		// for (auto &triangleID : triangleIDs)
		// {
		// 	triangleIDsToEntityIDsMap[triangleID] = entityIDs;
		// }
	}
	auto glEntityChildrenSize = glEntity.children.size();
	for (auto &pair : glEntity.children)
	{
		auto childEntityID = pair.first;
		auto entityIDsWithSubID = entityIDs;
		entityIDsWithSubID.push_back(childEntityID);
		postAddEntity(pair.second, entityIDsWithSubID);
	}
};
void GLScene::preRemoveEntity(const std::shared_ptr<IEntity> &entity, const std::vector<size_t> &entityIDs)
{
	auto &glEntity = (GLEntity &)*entity;
	if (glEntity.addToBVH)
	{
		bvh.removeEntity(*this, glEntity);
	}
};
GLEntity *GLScene::findEntityByPrimID(const size_t &primID)
{
	auto &tri = bvh.triangles[bvh.bvh.prim_ids[primID]];
	auto &userData = tri.userData;
	if (!userData)
		return 0;
	return (GLEntity *&)userData;
}
void GLScene::hookMouseEvents()
{
	for (unsigned int button = GLWindow::MinMouseButton; button <= GLWindow::MaxMouseButton; ++button)
	{
		mousePressIDs[button] = window.addMousePressHandler(button, [&, button](auto pressed)
		{
			auto &screenCoord = ((GLWindow &)window).mouseCoords;
			auto ray = bvh.mouseCoordToRay(window.windowHeight, screenCoord, {0, 0, window.windowWidth, window.windowHeight}, projection.matrix, view.matrix, projection.nearPlane, projection.farPlane);
			auto primID = bvh.trace(ray);
			if (primID == raytracing::invalidID)
			{
				return;
			}
			auto foundEntity = findEntityByPrimID(primID);
			foundEntity->callMousePressHandler(button, pressed);
		});
	}
	mouseMoveID = window.addMouseMoveHandler([&](auto coords)
	{
		auto ray = bvh.mouseCoordToRay(window.windowHeight, coords, {0, 0, window.windowWidth, window.windowHeight}, projection.matrix, view.matrix, projection.nearPlane, projection.farPlane);
		auto primID = bvh.trace(ray);
		if (primID == raytracing::invalidID)
		{
			if (currentHoveredEntity)
			{
				currentHoveredEntity->callMouseHoverHandler(false);
				currentHoveredEntity = 0;
			}
			return;
		}
		auto foundEntity = findEntityByPrimID(primID);
		if (currentHoveredEntity != foundEntity)
		{
			if (currentHoveredEntity)
				currentHoveredEntity->callMouseHoverHandler(false);
			currentHoveredEntity = foundEntity;
			foundEntity->callMouseHoverHandler(true);
		}
		foundEntity->callMouseMoveHandler(coords);
	});
};
void GLScene::unhookMouseEvents()
{
	for (unsigned int button = GLWindow::MinMouseButton; button <= GLWindow::MaxMouseButton; ++button)
	{
		window.removeMousePressHandler(button, mousePressIDs[button]);
	}
	window.removeMouseMoveHandler(mouseMoveID);
};