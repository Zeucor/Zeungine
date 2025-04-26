#include <iostream>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/crypto/vector.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAO.hpp>
#include <zg/Registry.hpp>
using namespace zg;
std::unordered_map<std::string, size_t> entityKeyCounts;
Scene::Scene(const SceneCreateInfo& info) :
		name(info.name), drawColorToWindowPlane(info.drawColorToWindowPlane), window(*info.windowPointer),
		viewPointer(std::make_shared<vp::View>(info.cameraPosition, info.cameraDirection, info.cameraUp)),
		useBVH(info.useBVH),
		entities(
			[](const auto& entity) { return entity.name; },
			[](const std::string& key) {
				auto _key = key;
				auto keySize = _key.size(); 
				if (!keySize)
				{
					_key = std::string("Unknown");
				}
				auto entityKeyCountsIter = entityKeyCounts.find(_key);
				if (entityKeyCountsIter == entityKeyCounts.end())
				{
					entityKeyCounts[_key] = 1;
					entityKeyCountsIter = entityKeyCounts.find(_key);
				}
				return _key + " " + std::to_string(++entityKeyCountsIter->second);
			}
		),
		onAttachedFunction(info.onAttachedFunction),
		onDetachedFunction(info.onDetachedFunction),
		preUpdateFunction(info.preUpdateFunction),
		prePreRenderFunction(info.prePreRenderFunction),
		postPostRenderFunction(info.postPostRenderFunction)
{
	switch (info.projectionType)
	{
	case vp::Projection::TYPE::Perspective:
		projectionPointer = std::make_shared<vp::Projection>(window, info.fov);
		break;
	case vp::Projection::TYPE::Orthographic:
		projectionPointer = std::make_shared<vp::Projection>(window, info.orthoSize);
		break;
	}
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	switch (info.framebufferCreateInt)
	{
	case 0:
		break;
	case 1:
		framebufferPointer = info.framebufferPointer;
		break;
	case 2:
		framebufferPointer =
			std::make_shared<textures::Framebuffer>(window.iRenderer, generateTexturesFromAttachments(info.frameBufferAttachments));
		break;
	}
	hookMouseEvents();
}
Scene::Scene(const Scene& other) :
		name(other.name), drawColorToWindowPlane(other.drawColorToWindowPlane), window(other.window),
		viewPointer(other.viewPointer), projectionPointer(other.projectionPointer), useBVH(other.useBVH),
		entities(other.entities),
		onAttachedFunction(other.onAttachedFunction),
		onDetachedFunction(other.onDetachedFunction),
		preUpdateFunction(other.preUpdateFunction),
		prePreRenderFunction(other.prePreRenderFunction),
		postPostRenderFunction(other.postPostRenderFunction)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
}
Scene& Scene::operator=(const Scene& other)
{
	ID = other.ID;
	name = other.name;
	drawColorToWindowPlane = other.drawColorToWindowPlane;
	window = other.window;
	clearColor = other.clearColor;
	projectionPointer = other.projectionPointer;
	entities = other.entities;
	pointLights = other.pointLights;
	directionalLights = other.directionalLights;
	spotLights = other.spotLights;
	spotLightShadows = other.spotLightShadows;
	pointLightShadows = other.pointLightShadows;
	directionalLightShadows = other.directionalLightShadows;
	sceneTextures = other.sceneTextures;
	framebufferPointer = other.framebufferPointer;
	windowPlane = other.windowPlane;
	mousePressIDs = other.mousePressIDs;
	mouseMoveID = other.mouseMoveID;
	currentHoveredEntity = other.currentHoveredEntity;
	viewPointer = other.viewPointer;
	useBVH = other.useBVH;
	updateNonce = other.updateNonce;
	onAttachedFunction = other.onAttachedFunction;
	onDetachedFunction = other.onDetachedFunction;
	preUpdateFunction = other.preUpdateFunction;
	prePreRenderFunction = other.prePreRenderFunction;
	postPostRenderFunction = other.postPostRenderFunction;
	return *this;
};
Scene::~Scene()
{
	unhookMouseEvents();
}
std::vector<textures::Framebuffer::TextureAttachmentPair>
Scene::generateTexturesFromAttachments(const std::vector<textures::Framebuffer::AttachmentType>& attachments)
{
	std::vector<textures::Framebuffer::TextureAttachmentPair> textureAttachmentPairs;
	for (auto& attachment : attachments)
	{
		auto isDepthStencil = attachment == textures::Framebuffer::AttachmentType::DepthStencil;
		auto isDepth = attachment == textures::Framebuffer::AttachmentType::Depth;
		auto isColor = attachment == textures::Framebuffer::AttachmentType::Color;
		auto isStencil = attachment == textures::Framebuffer::AttachmentType::Stencil;
		textures::Texture::Format format;
		textures::Texture::Type type;
		if (isDepthStencil)
		{
			format = textures::Texture::Format::DepthStencil;
			type = textures::Texture::Type::Float;
		}
		else if (isDepth)
		{
			format = textures::Texture::Format::Depth;
			type = textures::Texture::Type::Float;
		}
		else if (isColor)
		{
			format = textures::Texture::Format::RGBA8;
			type = textures::Texture::Type::UnsignedByte;
		}
		else if (isStencil)
		{
			format = textures::Texture::Format::Stencil;
			type = textures::Texture::Type::UnsignedByte;
		}
		sceneTextures.push_back(
			std::make_shared<textures::Texture>(window.iRenderer, glm::ivec4(window.windowWidth, window.windowHeight, 0, 0),
																					(const void*)0, format, type, textures::Texture::FilterType::Linear));
		textureAttachmentPairs.push_back({sceneTextures[sceneTextures.size() - 1].get(), attachment});
	}
	return textureAttachmentPairs;
}
KeyIDVector<std::string, Entity>::EmplaceBackTuple Scene::addEntity(const EntityCreateInfo& info, bool callOnEntityAdded)
{
	auto usingInfo{info};
	usingInfo.scenePointer = this;
	auto entity_tuple = entities.emplace_back(usingInfo);
	auto& entity = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(entity_tuple);
	entity.ID = std::get<KEY_ID_VECTOR_ID_INDEX>(entity_tuple);
	entity.INDEX = std::get<KEY_ID_VECTOR_INDEX_INDEX>(entity_tuple);
	entity.INDEX_STACK = {INDEX_STACK[0], INDEX_STACK[1], entity.INDEX};
	(*Registry::idEntities)[entity.ID] = entity.INDEX_STACK;
	postAddEntity(entity, {entity.ID});
	if (entity.onAddedToSceneFunction)
		entity.onAddedToSceneFunction(entity);
	if (callOnEntityAdded && window.onEntityAdded)
		window.onEntityAdded(entity);
	return entity_tuple;
}
bool Scene::removeEntity(size_t& ID)
{
	auto entityIter = entities.find_id(ID);
	if (entityIter == entities.end())
		return false;
	auto& entity = *entityIter;
	if (entity.onRemovedFromSceneFunction)
		entity.onRemovedFromSceneFunction(entity);
	preRemoveEntity(entity, {ID});
	entity.ID = 0;
	entities.erase(entityIter);
	auto& idEntitiesRef = *Registry::idEntities;
	auto idIter = idEntitiesRef.find(ID);
	if (idIter != idEntitiesRef.end())
	{
		idEntitiesRef.erase(idIter);
	}
	ID = 0;
	return true;
}
void Scene::update()
{
	++updateNonce;
	if (preUpdateFunction)
		preUpdateFunction(*this);
	auto componentsData = m_components.data();
	auto componentsSize = m_components.size();
	for (size_t index = 0; index < componentsSize; ++index)
		componentsData[index].onUpdate();
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	for (size_t index = 0; index < entitiesSize; ++index)
	{
		entitiesData[index].update();
	}
	if (useBVH)
	{
		if (bvh->changed)
		{
			bvh->buildBVH();
		}
	}
}
void Scene::preRender()
{
	if (prePreRenderFunction)
		prePreRenderFunction(*this);
	update();
	auto& iRenderer = *window.iRenderer;
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	for (auto& directionaLightShadow : directionalLightShadows)
	{
		directionaLightShadow.framebuffer.bind();
		directionaLightShadow.addShader();
		iRenderer.clear();
		for (size_t index = 0; index < entitiesSize; ++index)
		{
			auto& entity = entitiesData[index];
			if (!entity.affectedByShadows)
				continue;
			directionaLightShadow.shader->bind(entity);
			directionaLightShadow.shader->setBlock("LightSpaceMatrix", entity, directionaLightShadow.lightSpaceMatrix,
																						 sizeof(glm::mat4));
			const auto& model = entity.getModelMatrix();
			directionaLightShadow.shader->setBlock("Model", entity, model);
			iRenderer.bindShader(*entity.addShader(directionaLightShadow.shader), entity);
			entity.drawVAO();
			directionaLightShadow.shader->unbind();
		}
		directionaLightShadow.framebuffer.unbind();
	}
	for (auto& spotLightShadow : spotLightShadows)
	{
		spotLightShadow.framebuffer.bind();
		iRenderer.clear();
		for (size_t index = 0; index < entitiesSize; ++index)
		{
			auto& entity = entitiesData[index];
			if (!entity.affectedByShadows)
				continue;
			spotLightShadow.shader->bind(entity);
			spotLightShadow.shader->setBlock("LightSpaceMatrix", entity, spotLightShadow.lightSpaceMatrix,
																			 sizeof(glm::mat4));
			const auto& model = entity.getModelMatrix();
			spotLightShadow.shader->setBlock("Model", entity, model);
			entity.drawVAO();
			spotLightShadow.shader->unbind();
		}
		spotLightShadow.framebuffer.unbind();
	}
	for (auto& pointLightShadow : pointLightShadows)
	{
		pointLightShadow.framebuffer.bind();
		iRenderer.clear();
		for (size_t index = 0; index < entitiesSize; ++index)
		{
			auto& entity = entitiesData[index];
			if (!entity.affectedByShadows)
				continue;
			pointLightShadow.shader->bind(entity);
			pointLightShadow.shader->setBlock("PointLightSpaceMatrix", entity, pointLightShadow.shadowTransforms,
																				sizeof(glm::mat4) * 6);
			pointLightShadow.shader->setUniform("nearPlane", entity, pointLightShadow.pointLight.nearPlane);
			pointLightShadow.shader->setUniform("farPlane", entity, pointLightShadow.pointLight.farPlane);
			pointLightShadow.shader->setUniform("lightPos", entity, pointLightShadow.pointLight.position);
			const auto& model = entity.getModelMatrix();
			pointLightShadow.shader->setBlock("Model", entity, model);
			entity.drawVAO();
			pointLightShadow.shader->unbind();
		}
		pointLightShadow.framebuffer.unbind();
	}
#if defined(USE_GL) || defined(USE_EGL)
	// if (framebufferPointer != 0)
	// {
	// 	auto &framebuffer = *framebufferPointer;
	// 	iRenderer.viewPointerport({0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y});
	// }
	// else
	// 	iRenderer.viewPointerport({0, 0, window.windowWidth, window.windowHeight});
	// iRenderer.clearColor(clearColor);
	// iRenderer.clear();
#endif
	if (framebufferPointer)
		renderEntities();
}
void Scene::render()
{
	if (!framebufferPointer)
		renderEntities();
	else if (drawColorToWindowPlane)
		windowPlane->render();
}
void Scene::renderEntities()
{
	if (framebufferPointer)
		framebufferPointer->bind();
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	for (size_t index = 0; index < entitiesSize; ++index)
	{
		entitiesData[index].render();
	}
	if (framebufferPointer)
		framebufferPointer->unbind();
}
void Scene::postRender()
{
	if (postPostRenderFunction)
		postPostRenderFunction(*this);
}
void Scene::entityPreRender(Entity& entity)
{
	auto data = entity.getShaderData(window.iRenderer);
	auto shader = entity.shaders[data];
	uint32_t index = 0;
	glm::mat4 directionalLightSpaceMatrices[4];
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		directionalLightSpaceMatrices[index] = directionalLightShadow.lightSpaceMatrix;
		++index;
	}
	shader->setBlock("DirectionalLightSpaceMatrices", entity, directionalLightSpaceMatrices, sizeof(glm::mat4) * 4);
	glm::mat4 spotLightSpaceMatrices[4];
	index = 0;
	for (auto& spotLightShadow : spotLightShadows)
	{
		spotLightSpaceMatrices[index] = spotLightShadow.lightSpaceMatrix;
		++index;
	}
	shader->setBlock("SpotLightSpaceMatrices", entity, spotLightSpaceMatrices, sizeof(glm::mat4) * 4);
	int32_t unit = 0;
	index = 0;
	uint32_t unitRemaining = 4;
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		shader->setTexture("directionalLightSamplers[" + std::to_string(index) + "]", entity,
											 directionalLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("DirectionalLights", entity, directionalLights.data(),
									directionalLights.size() * sizeof(lights::DirectionalLight));
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto& spotLightShadow : spotLightShadows)
	{
		shader->setTexture("spotLightSamplers[" + std::to_string(index) + "]", entity, spotLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("SpotLights", entity, spotLights.data(), spotLights.size() * sizeof(lights::SpotLight));
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto& pointLightShadow : pointLightShadows)
	{
		shader->setTexture("pointLightSamplers[" + std::to_string(index) + "]", entity, pointLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("PointLights", entity, pointLights.data(), pointLights.size() * sizeof(lights::PointLight));
}
void Scene::resize(glm::vec2 newSize)
{
	viewPointer->callResizeHandler(newSize);
	// projectionPointer->orthoSize = newSize;
	// projectionPointer->update();
	if (framebufferPointer)
	{
		for (auto& pair : framebufferPointer->textureAttachmentPairs)
		{
			auto& texture = *pair.first;
			texture.size = {newSize.x, newSize.y, 1, 0};
			textures::FramebufferFactory::destroyFramebuffer(*framebufferPointer);
			textures::TextureFactory::destroyTexture(texture);
			textures::TextureFactory::initTexture(texture, 0);
			textures::FramebufferFactory::initFramebuffer(*framebufferPointer);
		}
	}
}
void Scene::postAddEntity(Entity& entity, const std::vector<size_t>& entityIDs)
{
	if (useBVH && entity.addToBVH)
	{
		bvh->addEntity(entity);
		// for (auto &triangleID : triangleIDs)
		// {
		// 	triangleIDsToEntityIDsMap[triangleID] = entityIDs;
		// }
	}
	auto entityChildrenData = entity.children.data();
	auto entityChildrenSize = entity.children.size();
	for (size_t index = 0; index < entityChildrenSize; ++index)
	{
		auto& childEntity = entityChildrenData[index];
		auto& childEntityID = childEntity.ID;
		auto entityIDsWithSubID = entityIDs;
		entityIDsWithSubID.push_back(childEntityID);
		postAddEntity(childEntity, entityIDsWithSubID);
	}
}
void Scene::preRemoveEntity(Entity& entity, const std::vector<size_t>& entityIDs)
{
	if (useBVH && entity.addToBVH)
	{
		bvh->removeEntity(*this, entity);
	}
}
Entity* Scene::findEntityByPrimID(const size_t& primID)
{
	if (!useBVH)
		return 0;
	auto& _bvh = *bvh;
	auto& tri = _bvh.triangles[_bvh.bvh.prim_ids[primID]];
	auto& entityID = tri.userData;
	if (!entityID)
		return 0;
	auto iter = entities.find_id(entityID);
	if (iter == entities.end())
		return 0;
	return &*iter;
}
void Scene::hookMouseEvents()
{
	for (unsigned int button = MinMouseButton; button <= MaxMouseButton; ++button)
	{
		mousePressIDs[button] = window.addMousePressHandler(
			button,
			[&, button](auto pressed)
			{
				if (!useBVH)
					return;
				auto& _bvh = *bvh;
				auto& screenCoord = (window).mouseCoords;
				auto ray = _bvh.mouseCoordToRay(window.windowHeight, screenCoord,
																				{0, 0, window.windowWidth, window.windowHeight}, projectionPointer->matrix,
																				viewPointer->matrix, projectionPointer->nearPlane, projectionPointer->farPlane);
				auto primID = _bvh.trace(ray);
				if (primID == raytracing::invalidID)
				{
					return;
				}
				auto foundEntity = findEntityByPrimID(primID);
				foundEntity->callMousePressHandler(button, pressed);
			});
	}
	mouseMoveID = window.addMouseMoveHandler(
		[&](auto coords)
		{
			if (!useBVH)
				return;
			auto& _bvh = *bvh;
			auto ray = _bvh.mouseCoordToRay(window.windowHeight, coords, {0, 0, window.windowWidth, window.windowHeight},
																			projectionPointer->matrix, viewPointer->matrix, projectionPointer->nearPlane,
																			projectionPointer->farPlane);
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
	if (drawColorToWindowPlane && framebufferPointer)
	{
		// also, side quest to create windowPlane
		auto colorAttachmentIter =
			std::find_if(framebufferPointer->textureAttachmentPairs.begin(), framebufferPointer->textureAttachmentPairs.end(),
									 [](const auto& pair) { return pair.second == textures::Framebuffer::AttachmentType::Color; });
		if (colorAttachmentIter != framebufferPointer->textureAttachmentPairs.end())
		{
			auto xRatio = window.windowWidth / window.windowHeight;
			auto yRatio = window.windowHeight / window.windowWidth;
			auto xRatio2 = 1 / yRatio;
			auto x = xRatio * 2;
			auto x2 = yRatio * x;
			// windowPlane = std::make_shared<entities::Plane>(window, *this, glm::vec3(0), glm::vec3(0, 180, 0),
			// glm::vec3(1), 																								glm::vec2(2, 2), *colorAttachmentIter->first);
			// auto& windowPlaneRef = *windowPlane; windowPlaneRef.projectionPointer =
			// std::make_shared<vp::Projection>(window, glm::vec2(2, 2)); windowPlaneRef.viewPointer =
			// 	std::make_shared<vp::View>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		}
	}
}
void Scene::unhookMouseEvents()
{
	for (unsigned int button = MinMouseButton; button <= MaxMouseButton; ++button)
	{
		window.removeMousePressHandler(button, mousePressIDs[button]);
	}
	window.removeMouseMoveHandler(mouseMoveID);
}
zg::Entity& Scene::getEntityByName(const std::string& name)
{
	auto iter = entities.find_key(name);
	if (iter == entities.end())
		throw std::runtime_error("Entity not found with name");
	return *iter;
}
zg::Entity& Scene::getEntityByID(const size_t& id)
{
	auto iter = entities.find_id(id);
	if (iter == entities.end())
		throw std::runtime_error("Entity not found with name");
	return *iter;
}
template <>
Serial& serialize(Serial& serial, const Scene& scene)
{
	serial << true << scene.drawColorToWindowPlane << scene.clearColor << scene.projectionPointer;
	auto entitiesSize = scene.entities.size();
	serial << entitiesSize;
	auto entitiesData = scene.entities.data();
	for (size_t index = 0; index < entitiesSize; ++index)
	{
		const auto& entity = entitiesData[index];
		auto& ID = entity.ID;
		auto& entity_typeName = entity.typeName;
		serial << true << ID << entity_typeName;
		auto serializeFunction = Entity::getSerialize(entity_typeName);
		serializeFunction(serial, entity);
	}

	auto pointLightsSize = scene.pointLights.size();
	serial << pointLightsSize;
	for (auto pointLight : scene.pointLights)
	{
		serial << pointLight;
	}
	auto directionalLightsSize = scene.directionalLights.size();
	serial << directionalLightsSize;
	for (auto directionalLight : scene.directionalLights)
	{
		serial << directionalLight;
	}
	auto spotLightsSize = scene.spotLights.size();
	serial << spotLightsSize;
	for (auto spotLight : scene.spotLights)
	{
		serial << spotLight;
	}
	auto texturesSize = scene.sceneTextures.size();
	serial << texturesSize;
	for (auto& texturePointer : scene.sceneTextures)
	{
		serial << texturePointer;
	}
	bool framebuffer = !!scene.framebufferPointer;
	serial << framebuffer;
	if (framebuffer)
	{
		auto textureAttachmentSize = scene.framebufferPointer->textureAttachmentPairs.size();
		serial << textureAttachmentSize;
		for (auto& textureAttachment : scene.framebufferPointer->textureAttachmentPairs)
		{
			serial << textureAttachment.second;
		}
	}
	serial << scene.windowPlane << scene.viewPointer;
	return serial;
}
template <>
Serial& deserialize(Serial& serial, Scene& scene)
{
	bool wroteBit = false;
	serial >> wroteBit;
	if (!wroteBit)
		return serial;
	serial >> scene.drawColorToWindowPlane >> scene.clearColor >> scene.projectionPointer;
	auto entitiesSize = scene.entities.size();
	serial >> entitiesSize;
	for (size_t i = 0; i < entitiesSize; i++)
	{
		bool readBit = false;
		serial >> readBit;
		if (!readBit)
			continue;
		size_t ID = 0;
		std::string entity_typeName;
		serial >> ID >> entity_typeName;
		auto deserializeFunction = Entity::getDeserialize(entity_typeName);
		zg::EntityCreateInfo entityCreateInfo{.scenePointer = &scene};
		deserializeFunction(serial, entityCreateInfo);
		auto entity_tuple = scene.entities.emplace_back(entityCreateInfo);
		auto& entity = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(entity_tuple);
		entity.ID = ID;
		scene.postAddEntity(entity, {ID});
		if (scene.window.onEntityAdded)
			scene.window.onEntityAdded(entity);
	}
	auto pointLightsSize = scene.pointLights.size();
	serial >> pointLightsSize;
	scene.pointLights.resize(pointLightsSize);
	for (auto& pointLight : scene.pointLights)
	{
		serial >> pointLight;
	}
	for (auto& pointLight : scene.pointLights)
	{
		scene.pointLightShadows.emplace_back(scene.window, pointLight);
	}
	auto directionalLightsSize = scene.directionalLights.size();
	serial >> directionalLightsSize;
	scene.directionalLights.resize(directionalLightsSize);
	for (auto& directionalLight : scene.directionalLights)
	{
		serial >> directionalLight;
	}
	for (auto& directionalLight : scene.directionalLights)
	{
		scene.directionalLightShadows.emplace_back(scene.window, directionalLight);
	}
	auto spotLightsSize = scene.spotLights.size();
	serial >> spotLightsSize;
	scene.spotLights.resize(spotLightsSize);
	for (auto& spotLight : scene.spotLights)
	{
		serial >> spotLight;
	}
	for (auto& spotLight : scene.spotLights)
	{
		scene.spotLightShadows.emplace_back(scene.window, spotLight);
	}
	auto texturesSize = scene.sceneTextures.size();
	serial >> texturesSize;
	scene.sceneTextures.resize(texturesSize);
	for (auto& texturePointer : scene.sceneTextures)
	{
		serial >> texturePointer;
	}
	bool framebuffer = false;
	serial >> framebuffer;
	if (framebuffer)
	{
		std::vector<zg::textures::Framebuffer::TextureAttachmentPair> textureAttachmentPairs;
		size_t textureAttachmentSize = textureAttachmentPairs.size();
		serial >> textureAttachmentSize;
		for (size_t i = 0; i < textureAttachmentSize; ++i)
		{
			zg::textures::Framebuffer::AttachmentType attachmentType;
			serial >> attachmentType;
			textureAttachmentPairs.emplace_back((zg::textures::Texture*)scene.sceneTextures[i].get(), attachmentType);
		}
		scene.framebufferPointer = std::make_shared<zg::textures::Framebuffer>(scene.window.iRenderer, textureAttachmentPairs);
	}
	serial >> scene.windowPlane >> scene.viewPointer;
	return serial;
}
