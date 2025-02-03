#include <zg/interfaces/IWindow.hpp>
#include <zg/crypto/vector.hpp>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAO.hpp>
#include <iostream>
using namespace zg;
Scene::Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov, textures::Framebuffer *_framebufferPointer, bool _useBVH):
	IScene(_window),
	view(cameraPosition, cameraDirection),
	projection((Window &)window, fov),
	framebufferPointer(_framebufferPointer),
	useBVH(_useBVH)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
};
Scene::Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize, textures::Framebuffer *_framebufferPointer, bool _useBVH):
	IScene(_window),
	view(cameraPosition, cameraDirection),
	projection((Window &)window, orthoSize),
	framebufferPointer(_framebufferPointer),
	useBVH(_useBVH)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
};
Scene::~Scene()
{
	unhookMouseEvents();
};
void Scene::update()
{
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->second->update();
	}
}
void Scene::preRender()
{
	update();
	auto &iRenderer = *window.iRenderer;
	for (auto &directionaLightShadow : directionalLightShadows)
	{
		directionaLightShadow.framebuffer.bind();
		iRenderer.clear();
		directionaLightShadow.shader.bind();
		for (auto &entityPair : entities)
		{
			auto &entityPointer = entityPair.second;
			auto &vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto &glEntity = *std::dynamic_pointer_cast<Entity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			directionaLightShadow.shader.setBlock("LightSpaceMatrix", glEntity, directionaLightShadow.lightSpaceMatrix, sizeof(glm::mat4));
			const auto &model = glEntity.getModelMatrix();
			directionaLightShadow.shader.setBlock("Model", glEntity, model);
			vbo.drawVAO();
		}
		directionaLightShadow.shader.unbind();
		directionaLightShadow.framebuffer.unbind();
	}
	for (auto &spotLightShadow : spotLightShadows)
	{
		spotLightShadow.framebuffer.bind();
		iRenderer.clear();
		spotLightShadow.shader.bind();
		for (auto &entityPair : entities)
		{
			auto &entityPointer = entityPair.second;
			auto &vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto &glEntity = *std::dynamic_pointer_cast<Entity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			spotLightShadow.shader.setBlock("LightSpaceMatrix", glEntity, spotLightShadow.lightSpaceMatrix, sizeof(glm::mat4));
			const auto &model = glEntity.getModelMatrix();
			spotLightShadow.shader.setBlock("Model", glEntity, model);
			vbo.drawVAO();
		}
		spotLightShadow.shader.unbind();
		spotLightShadow.framebuffer.unbind();
	}
	for (auto &pointLightShadow : pointLightShadows)
	{
		pointLightShadow.framebuffer.bind();
		iRenderer.clear();
		pointLightShadow.shader.bind();
		for (auto &entityPair : entities)
		{
			auto &entityPointer = entityPair.second;
			auto &vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto &glEntity = *std::dynamic_pointer_cast<Entity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			pointLightShadow.shader.setBlock("PointLightSpaceMatrix", glEntity, pointLightShadow.shadowTransforms, sizeof(glm::mat4) * 6);
			pointLightShadow.shader.setUniform("nearPlane", glEntity, pointLightShadow.pointLight.nearPlane);
			pointLightShadow.shader.setUniform("farPlane", glEntity, pointLightShadow.pointLight.farPlane);
			pointLightShadow.shader.setUniform("lightPos", glEntity, pointLightShadow.pointLight.position);
			const auto &model = glEntity.getModelMatrix();
			pointLightShadow.shader.setBlock("Model", glEntity, model);
			vbo.drawVAO();
		}
		pointLightShadow.shader.unbind();
		pointLightShadow.framebuffer.unbind();
	}
#if defined(USE_GL) || defined(USE_EGL)
	if (framebufferPointer != 0)
	{
		auto &framebuffer = *framebufferPointer;	
		iRenderer.viewport({0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y});
	}
	else
		iRenderer.viewport({0, 0, window.windowWidth, window.windowHeight});
	iRenderer.clearColor(clearColor);
	iRenderer.clear();
#endif
};
void Scene::render()
{
	if (useBVH)
	{
		if (bvh->changed)
		{
			bvh->buildBVH();
		}
	}
	preRender();
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->second->render();
	}
};
void Scene::entityPreRender(IEntity &entity)
{
	Entity &glEntity = static_cast<Entity&>(entity);
	uint32_t index = 0;
	glm::mat4 directionalLightSpaceMatrices[4];
	for (auto &directionalLightShadow : directionalLightShadows)
	{
		directionalLightSpaceMatrices[index] = directionalLightShadow.lightSpaceMatrix;
		++index;
	}
	glEntity.shader.setBlock("DirectionalLightSpaceMatrices", glEntity, directionalLightSpaceMatrices, sizeof(glm::mat4) * 4);
	glm::mat4 spotLightSpaceMatrices[4];
	index = 0;
	for (auto &spotLightShadow : spotLightShadows)
	{
		spotLightSpaceMatrices[index] = spotLightShadow.lightSpaceMatrix;
		++index;
	}
	glEntity.shader.setBlock("SpotLightSpaceMatrices", glEntity, spotLightSpaceMatrices, sizeof(glm::mat4) * 4);
	int32_t unit = 0;
	index = 0;
	uint32_t unitRemaining = 4;
	for (auto &directionalLightShadow : directionalLightShadows)
	{
		glEntity.shader.setTexture("directionalLightSamplers[" + std::to_string(index) + "]", glEntity, directionalLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto &spotLightShadow : spotLightShadows)
	{
		glEntity.shader.setTexture("spotLightSamplers[" + std::to_string(index) + "]", glEntity, spotLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto &pointLightShadow : pointLightShadows)
	{
		glEntity.shader.setTexture("pointLightSamplers[" + std::to_string(index) + "]", glEntity, pointLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
};
void Scene::resize(glm::vec2 newSize)
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
void Scene::postAddEntity(const std::shared_ptr<IEntity>& entity, const std::vector<size_t> &entityIDs)
{
	auto &glEntity = (Entity &)*entity;
	if (useBVH && glEntity.addToBVH)
	{
		bvh->addEntity(glEntity);
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
void Scene::preRemoveEntity(const std::shared_ptr<IEntity> &entity, const std::vector<size_t> &entityIDs)
{
	auto &glEntity = (Entity &)*entity;
	if (useBVH && glEntity.addToBVH)
	{
		bvh->removeEntity(*this, glEntity);
	}
};
Entity *Scene::findEntityByPrimID(const size_t &primID)
{
	if (!useBVH)
		return 0;
	auto& _bvh = *bvh;
	auto &tri = _bvh.triangles[_bvh.bvh.prim_ids[primID]];
	auto &userData = tri.userData;
	if (!userData)
		return 0;
	return (Entity *&)userData;
}
void Scene::hookMouseEvents()
{
	for (unsigned int button = Window::MinMouseButton; button <= Window::MaxMouseButton; ++button)
	{
		mousePressIDs[button] = window.addMousePressHandler(button, [&, button](auto pressed)
		{
			if (!useBVH)
				return;
			auto& _bvh = *bvh;
			auto &screenCoord = ((Window &)window).mouseCoords;
			auto ray = _bvh.mouseCoordToRay(window.windowHeight, screenCoord, {0, 0, window.windowWidth, window.windowHeight}, projection.matrix, view.matrix, projection.nearPlane, projection.farPlane);
			auto primID = _bvh.trace(ray);
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
		if (!useBVH)
			return;
		auto& _bvh = *bvh;
		auto ray = _bvh.mouseCoordToRay(window.windowHeight, coords, {0, 0, window.windowWidth, window.windowHeight}, projection.matrix, view.matrix, projection.nearPlane, projection.farPlane);
		auto primID = _bvh.trace(ray);
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
void Scene::unhookMouseEvents()
{
	for (unsigned int button = Window::MinMouseButton; button <= Window::MaxMouseButton; ++button)
	{
		window.removeMousePressHandler(button, mousePressIDs[button]);
	}
	window.removeMouseMoveHandler(mouseMoveID);
};