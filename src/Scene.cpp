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
	ID(info.ID),
	INDEX(info.INDEX),
	INDEX_STACK(info.INDEX_STACK),
	iRenderer(Registry::getWindow(INDEX_STACK).iRenderer),
	name(info.name),
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
	postProcessingPipeline(INDEX_STACK),
	onAttachedFunction(info.onAttachedFunction),
	onDetachedFunction(info.onDetachedFunction),
	preUpdateFunction(info.preUpdateFunction),
	prePreRenderFunction(info.prePreRenderFunction),
	postPostRenderFunction(info.postPostRenderFunction)
{
	auto& window = Registry::getWindow(INDEX_STACK);
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
		keyedTextures = {
			{"ColorTexture", std::make_shared<textures::Texture>(window.iRenderer, glm::ivec4(window.windowWidth, window.windowHeight, 1, 0), (const void*)0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Linear, true)},
			{"DepthTexture", std::make_shared<textures::Texture>(window.iRenderer, glm::ivec4(window.windowWidth, window.windowHeight, 1, 0), (const void*)0, textures::Texture::Format::Depth, textures::Texture::Type::Float, textures::Texture::FilterType::Linear, true)}
		};
		{
			std::vector<textures::Framebuffer::TextureAttachmentPair> attachments{
				{keyedTextures[0].second, textures::Framebuffer::AttachmentType::Color},
				{keyedTextures[1].second, textures::Framebuffer::AttachmentType::Depth}
			};
			framebuffer = std::make_shared<textures::Framebuffer>(window.iRenderer, attachments);
		}
		break;
	case 1:
		framebuffer = info.framebuffer;
		for (auto& pair : framebuffer->textureAttachmentPairs)
		{
			std::string key;
			switch (pair.first->format)
			{
			case textures::Texture::Format::Depth:
				key = "DepthTexture";
				break;
			case textures::Texture::Format::RGB8:
			case textures::Texture::Format::RGBA8:
			case textures::Texture::Format::RGBA32F:
			case textures::Texture::Format::Integer32:
				key = "ColorTexture";
				break;
			case textures::Texture::Format::DepthStencil:
				key = "DepthStencilTexture";
				break;
			case textures::Texture::Format::Stencil:
				key = "StencilTexture";
				break;
			}
			keyedTextures.push_back({key, pair.first});
		}
		break;
	case 2:
		framebuffer =
			std::make_shared<textures::Framebuffer>(iRenderer, generateTexturesFromAttachments(info.frameBufferAttachments));
		break;
	}
	framebuffer->sceneID = ID;
	shaders::RuntimeConstants fsqconstants;
	for (auto& pair : keyedTextures)
	{
		fsqconstants.push_back(pair.first);
		postProcessingPipeline.textureRegistry.registerOutput((std::numeric_limits<float>::lowest)(), pair.first, pair.second);
	}
	fsq = std::make_shared<FullscreenQuad>(INDEX_STACK, fsqconstants);
	hookMouseEvents();
}
Scene::Scene(const Scene& other) :
	DataStorage<Scene>(other),
	ComponentHolder<Scene, components::scenes::SceneComponent, components::scenes::SceneComponentCreateInfo>(other),
	ID(other.ID),
	INDEX(other.INDEX),
	INDEX_STACK(other.INDEX_STACK),
	iRenderer(other.iRenderer),
	name(other.name),
	clearColor(other.clearColor),
	projectionPointer(other.projectionPointer), entities(other.entities),
	pointLights(other.pointLights), directionalLights(other.directionalLights),
	spotLights(other.spotLights), spotLightShadows(other.spotLightShadows),
	pointLightShadows(other.pointLightShadows), directionalLightShadows(other.directionalLightShadows),
	keyedTextures(other.keyedTextures),
	framebuffer(other.framebuffer),
	fsq(other.fsq),
	postProcessingPipeline(other.postProcessingPipeline),
	// bvh(other.bvh)
	mousePressIDs(other.mousePressIDs),
	mouseMoveID(other.mouseMoveID),
	viewPointer(other.viewPointer),
	useBVH(other.useBVH),
	updateNonce(other.updateNonce),
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
	((DataStorage<Scene>&)*this) = other;
	((ComponentHolder<Scene, components::scenes::SceneComponent, components::scenes::SceneComponentCreateInfo>&)*this) = other;
	ID = other.ID;
	INDEX = other.INDEX;
	INDEX_STACK = other.INDEX_STACK;
	iRenderer = other.iRenderer;
	name = other.name;
	clearColor = other.clearColor;
	projectionPointer = other.projectionPointer;
	entities = other.entities;
	pointLights = other.pointLights;
	directionalLights = other.directionalLights;
	spotLights = other.spotLights;
	spotLightShadows = other.spotLightShadows;
	pointLightShadows = other.pointLightShadows;
	directionalLightShadows = other.directionalLightShadows;
	keyedTextures = other.keyedTextures;
	framebuffer = other.framebuffer;
	fsq = other.fsq;
	postProcessingPipeline = other.postProcessingPipeline;
	mousePressIDs = other.mousePressIDs;
	mouseMoveID = other.mouseMoveID;
	currentHoveredEntityID = other.currentHoveredEntityID;
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
	for (auto& entity : entities)
		if (entity.onRemovedFunction)
			entity.onRemovedFunction(entity);
	entities.clear();
	postProcessingPipeline.cleanup();
	unhookMouseEvents();
}
std::vector<textures::Framebuffer::TextureAttachmentPair>
Scene::generateTexturesFromAttachments(const std::vector<textures::Framebuffer::AttachmentType>& attachments)
{
	auto& window = Registry::getWindow(INDEX_STACK);
	std::vector<textures::Framebuffer::TextureAttachmentPair> textureAttachmentPairs;
	for (auto& attachment : attachments)
	{
		auto isDepthStencil = attachment == textures::Framebuffer::AttachmentType::DepthStencil;
		auto isDepth = attachment == textures::Framebuffer::AttachmentType::Depth;
		auto isColor = attachment == textures::Framebuffer::AttachmentType::Color;
		auto isStencil = attachment == textures::Framebuffer::AttachmentType::Stencil;
		textures::Texture::Format format;
		textures::Texture::Type type;
		std::string key;
		if (isDepthStencil)
		{
			key = "DepthStencilTexture";
			format = textures::Texture::Format::DepthStencil;
			type = textures::Texture::Type::Float;
		}
		else if (isDepth)
		{
			key = "DepthTexture";
			format = textures::Texture::Format::Depth;
			type = textures::Texture::Type::Float;
		}
		else if (isColor)
		{
			key = "ColorTexture";
			format = textures::Texture::Format::RGBA8;
			type = textures::Texture::Type::UnsignedByte;
		}
		else if (isStencil)
		{
			key = "StencilTexture";
			format = textures::Texture::Format::Stencil;
			type = textures::Texture::Type::UnsignedByte;
		}
		keyedTextures.push_back({key,
			std::make_shared<textures::Texture>(iRenderer, glm::ivec4(window.windowWidth, window.windowHeight, 1, 0),
																					(const void*)0, format, type, textures::Texture::FilterType::Linear)});
		textureAttachmentPairs.push_back({keyedTextures[keyedTextures.size() - 1].second, attachment});
	}
	return textureAttachmentPairs;
}
KeyIDVector<std::string, Entity>::EmplaceBackTuple Scene::addEntity(const EntityCreateInfo& info, bool callOnEntityAdded)
{
	auto usingInfo{info};
	auto transaction = entities.startTransaction();
	usingInfo.INDEX_STACK = {INDEX_STACK.begin(), INDEX_STACK.end()};
	usingInfo.INDEX_STACK.push_back(transaction.index);
	usingInfo.ID = transaction.id;
	usingInfo.INDEX = transaction.index;
	auto& entity = entities.commitTransaction(transaction, usingInfo);
	(*Registry::idEntities)[entity.ID] = entity.INDEX_STACK;
	postAddEntity(entity, {entity.ID});
	if (entity.onAddedFunction)
		entity.onAddedFunction(entity);
	auto& window = Registry::getWindow(INDEX_STACK);
	if (callOnEntityAdded && window.onEntityAdded)
		window.onEntityAdded(entity);
	return {transaction.key, transaction.id, transaction.index, &entity};
}
bool Scene::removeEntity(size_t& ID)
{
	auto entityIter = entities.find_id(ID);
	if (entityIter == entities.end())
		return false;
	auto& entity = *entityIter;
	if (entity.onRemovedFunction)
		entity.onRemovedFunction(entity);
	preRemoveEntity(entity, {ID});
	entity.ID = 0;
	entities.erase(entityIter);
	auto& idEntitiesRef = *Registry::idEntities;
	auto idIter = idEntitiesRef.find(ID);
	if (idIter != idEntitiesRef.end())
		idEntitiesRef.erase(idIter);
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
		entitiesData[index].update();
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
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	for (auto& directionaLightShadow : directionalLightShadows)
	{
		directionaLightShadow.framebuffer->bind();
		directionaLightShadow.addShader();
		iRenderer->clear();
		for (size_t index = 0; index < entitiesSize; ++index)
		{
			auto& entity = entitiesData[index];
			if (!entity.affectedByShadows)
				continue;
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = Registry::getMesh(meshID);
				directionaLightShadow.shader->bind(mesh);
				directionaLightShadow.shader->setBlock("LightSpaceMatrix", mesh, directionaLightShadow.lightSpaceMatrix,
																							 sizeof(glm::mat4));
				// const auto& model = mesh.getModelMatrix();
				directionaLightShadow.shader->setBlock("Model", mesh, entity.getModelMatrix());
				iRenderer->bindShader(*mesh.addShader(directionaLightShadow.shader), mesh);
				mesh.drawVAO();
				directionaLightShadow.shader->unbind();
			}
		}
		directionaLightShadow.framebuffer->unbind();
	}
	for (auto& spotLightShadow : spotLightShadows)
	{
		spotLightShadow.framebuffer->bind();
		iRenderer->clear();
		for (size_t index = 0; index < entitiesSize; ++index)
		{
			auto& entity = entitiesData[index];
			if (!entity.affectedByShadows)
				continue;
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = Registry::getMesh(meshID);
				spotLightShadow.shader->bind(mesh);
				spotLightShadow.shader->setBlock("LightSpaceMatrix", mesh, spotLightShadow.lightSpaceMatrix,
																				 sizeof(glm::mat4));
				// const auto& model = mesh.getModelMatrix();
				spotLightShadow.shader->setBlock("Model", mesh, entity.getModelMatrix());
				mesh.drawVAO();
				spotLightShadow.shader->unbind();
			}
		}
		spotLightShadow.framebuffer->unbind();
	}
	for (auto& pointLightShadow : pointLightShadows)
	{
		pointLightShadow.framebuffer->bind();
		iRenderer->clear();
		for (size_t index = 0; index < entitiesSize; ++index)
		{
			auto& entity = entitiesData[index];
			if (!entity.affectedByShadows)
				continue;
			for (auto& meshID : entity.meshIDs)
			{
				auto& mesh = Registry::getMesh(meshID);
				pointLightShadow.shader->bind(mesh);
				pointLightShadow.shader->setBlock("PointLightSpaceMatrix", mesh, pointLightShadow.shadowTransforms,
																					sizeof(glm::mat4) * 6);
				pointLightShadow.shader->setUniform("nearPlane", mesh, pointLightShadow.pointLight.nearPlane);
				pointLightShadow.shader->setUniform("farPlane", mesh, pointLightShadow.pointLight.farPlane);
				pointLightShadow.shader->setUniform("lightPos", mesh, pointLightShadow.pointLight.position);
				// const auto& model = mesh.getModelMatrix();
				pointLightShadow.shader->setBlock("Model", mesh, entity.getModelMatrix());
				mesh.drawVAO();
				pointLightShadow.shader->unbind();
			}
		}
		pointLightShadow.framebuffer->unbind();
	}
#if defined(USE_GL) || defined(USE_EGL)
	// if (framebuffer != 0)
	// {
	// 	auto &framebuffer = *framebuffer;
	// 	iRenderer->viewPointerport({0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y});
	// }
	// else
	// 	iRenderer->viewPointerport({0, 0, window.windowWidth, window.windowHeight});
	// iRenderer->clearColor(clearColor);
	// iRenderer->clear();
#endif
}
void Scene::render()
{
	renderEntities();
}
void Scene::renderEntities()
{
	auto& framebufferRef = *framebuffer;
	framebufferRef.bind();
	auto transparentDrawList = getTransparentDrawList();
	auto opaqueDrawList = getOpaqueDrawList();
	auto cameraPosition = viewPointer->position;
	std::sort(transparentDrawList.begin(), transparentDrawList.end(), [&](auto& a, auto& b)
	{
		auto distA = glm::distance(a.first->position, cameraPosition);
		auto distB = glm::distance(b.first->position, cameraPosition);
		return distA > distB;
	});
	for (auto& ep : opaqueDrawList)
	{
		ep.second->uid = ep.first->ID;
		ep.second->render(*ep.first);
	}
	for (auto& ep : transparentDrawList)
	{
		ep.second->uid = ep.first->ID;
		ep.second->render(*ep.first);
	}
	framebufferRef.unbind();
}
void Scene::postRender()
{
	if (postPostRenderFunction)
		postPostRenderFunction(*this);
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	for (size_t index = 0; index < entitiesSize; ++index)
		entitiesData[index].postRender();
}
void Scene::meshPreRender(Mesh& mesh)
{
	auto shader = mesh.addShader();
	uint32_t index = 0;
	glm::mat4 directionalLightSpaceMatrices[4];
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		directionalLightSpaceMatrices[index] = directionalLightShadow.lightSpaceMatrix;
		++index;
	}
	shader->setBlock("DirectionalLightSpaceMatrices", mesh, directionalLightSpaceMatrices, sizeof(glm::mat4) * 4);
	glm::mat4 spotLightSpaceMatrices[4];
	index = 0;
	for (auto& spotLightShadow : spotLightShadows)
	{
		spotLightSpaceMatrices[index] = spotLightShadow.lightSpaceMatrix;
		++index;
	}
	shader->setBlock("SpotLightSpaceMatrices", mesh, spotLightSpaceMatrices, sizeof(glm::mat4) * 4);
	int32_t unit = 0;
	index = 0;
	uint32_t unitRemaining = 4;
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		shader->setTexture("directionalLightSamplers[" + std::to_string(index) + "]", mesh,
											 *directionalLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("DirectionalLights", mesh, directionalLights.data(),
									directionalLights.size() * sizeof(lights::DirectionalLight));
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto& spotLightShadow : spotLightShadows)
	{
		shader->setTexture("spotLightSamplers[" + std::to_string(index) + "]", mesh, *spotLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("SpotLights", mesh, spotLights.data(), spotLights.size() * sizeof(lights::SpotLight));
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto& pointLightShadow : pointLightShadows)
	{
		shader->setTexture("pointLightSamplers[" + std::to_string(index) + "]", mesh, *pointLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("PointLights", mesh, pointLights.data(), pointLights.size() * sizeof(lights::PointLight));
}
void Scene::resize(glm::vec2 newSize)
{
	viewPointer->callResizeHandler(newSize);
	// projectionPointer->orthoSize = newSize;
	// projectionPointer->update();
	if (framebuffer)
	{
		for (auto& pair : framebuffer->textureAttachmentPairs)
		{
			auto& texture = *pair.first;
			texture.size = {newSize.x, newSize.y, 1, 0};
			textures::FramebufferFactory::destroyFramebuffer(*framebuffer);
			textures::TextureFactory::destroyTexture(texture);
			textures::TextureFactory::initTexture(texture, 0);
			textures::FramebufferFactory::initFramebuffer(*framebuffer);
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
std::pair<Entity&, Mesh&> Scene::findEntityAndMeshByPrimID(const size_t& primID)
{
	if (!useBVH)
		throw std::runtime_error("Scene is not using a BVH");
	auto& _bvh = *bvh;
	auto& tri = _bvh.triangles[_bvh.bvh.prim_ids[primID]];
	auto& entityMeshID = tri.userData;
	if (!entityMeshID.first || !entityMeshID.second)
	{
		throw std::runtime_error("Ohmy now, we always set tri IDs, so this should logically never happen");
	}
	auto iter = entities.find_id(entityMeshID.first);
	if (iter == entities.end())
		throw std::runtime_error("We should always find an entity due to the add/update/remove structure");
	return {*iter, Registry::getMesh(entityMeshID.second)};
}
void Scene::hookMouseEvents()
{
	auto& window = Registry::getWindow(INDEX_STACK);
	for (unsigned int button = MinMouseButtonIndex; button < MaxMouseButton; ++button)
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
				auto foundEntityMesh = findEntityAndMeshByPrimID(primID);
				foundEntityMesh.first.callMousePressHandler(button, pressed);
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
				if (currentHoveredEntityID)
				{
					auto& currentHoveredEntity = Registry::getEntity(currentHoveredEntityID);
					currentHoveredEntity.callMouseHoverHandler(false);
					currentHoveredEntityID = 0;
				}
				return;
			}
			auto foundEntityMesh = findEntityAndMeshByPrimID(primID);
			if (currentHoveredEntityID != foundEntityMesh.first.ID)
			{
				if (currentHoveredEntityID)
				{
					auto& currentHoveredEntity = Registry::getEntity(currentHoveredEntityID);
					currentHoveredEntity.callMouseHoverHandler(false);
				}
				currentHoveredEntityID = foundEntityMesh.first.ID;
				foundEntityMesh.first.callMouseHoverHandler(true);
			}
			foundEntityMesh.first.callMouseMoveHandler(coords);
		});
}
void Scene::unhookMouseEvents()
{
	auto& window = Registry::getWindow(INDEX_STACK);
	for (unsigned int button = MinMouseButtonIndex; button <= MaxMouseButtonIndex; ++button)
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
	serial << true << scene.clearColor << scene.projectionPointer;
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
	auto texturesSize = scene.keyedTextures.size();
	serial << texturesSize;
	for (auto& texturePointer : scene.keyedTextures)
	{
		serial << texturePointer;
	}
	bool framebuffer = !!scene.framebuffer;
	serial << framebuffer;
	if (framebuffer)
	{
		auto textureAttachmentSize = scene.framebuffer->textureAttachmentPairs.size();
		serial << textureAttachmentSize;
		for (auto& textureAttachment : scene.framebuffer->textureAttachmentPairs)
		{
			serial << textureAttachment.second;
		}
	}
	serial << scene.fsq << scene.viewPointer;
	return serial;
}
template <>
Serial& deserialize(Serial& serial, Scene& scene)
{
	auto& window = Registry::getWindow(scene.INDEX_STACK);
	bool wroteBit = false;
	serial >> wroteBit;
	if (!wroteBit)
		return serial;
	serial >> scene.clearColor >> scene.projectionPointer;
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
		zg::EntityCreateInfo entityCreateInfo{};
		deserializeFunction(serial, entityCreateInfo);
		auto transaction = scene.entities.startTransaction();
		entityCreateInfo.INDEX_STACK = {scene.INDEX_STACK.begin(), scene.INDEX_STACK.end()};
		entityCreateInfo.INDEX_STACK.push_back(transaction.index);
		auto& entity = scene.entities.commitTransaction(transaction, entityCreateInfo);
		entity.ID = ID;
		scene.postAddEntity(entity, {ID});
		if (window.onEntityAdded)
			window.onEntityAdded(entity);
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
		scene.pointLightShadows.emplace_back(window, pointLight);
	}
	auto directionalLightsSize = scene.directionalLights.size();
	serial >> directionalLightsSize;
	scene.directionalLights.resize(directionalLightsSize);
	for (auto& directionalLight : scene.directionalLights)
	{
		serial >> directionalLight;
	}
	auto index = 0;
	for (auto& directionalLight : scene.directionalLights)
	{
		scene.directionalLightShadows.emplace_back(scene.INDEX_STACK, index++);
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
		scene.spotLightShadows.emplace_back(window, spotLight);
	}
	auto texturesSize = scene.keyedTextures.size();
	serial >> texturesSize;
	scene.keyedTextures.resize(texturesSize);
	for (auto& texturePointer : scene.keyedTextures)
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
			textureAttachmentPairs.emplace_back(scene.keyedTextures[i].second, attachmentType);
		}
		scene.framebuffer = std::make_shared<zg::textures::Framebuffer>(scene.iRenderer, textureAttachmentPairs);
	}
	serial >> scene.fsq >> scene.viewPointer;
	return serial;
}
size_t Scene::getTransparentDrawCount()
{
	size_t c = 0;
	auto entitiesSize = entities.size();
	auto entitiesData = entities.data();
	std::function<void(Entity&)> countEntity;
	countEntity = [&](auto& entity)
	{
		for (auto& meshID : entity.meshIDs)
		{
			bool addMesh = false;
			auto& mesh = Registry::getMesh(meshID);
			for (auto& keyedPair : mesh.keyedTextures)
			{
				if (keyedPair.second->isTransparent)
				{
					addMesh = true;
					break;
				}
			}
			if (addMesh)
				c++;
		}
		for (auto& child : entity.children)
			countEntity(child);
	};
	for (size_t i = 0; i < entitiesSize; ++i)
	{
		auto& entity = entitiesData[i];
		countEntity(entity);
	}
	return c;
}
size_t Scene::getOpaqueDrawCount()
{
	size_t c = 0;
	auto entitiesSize = entities.size();
	auto entitiesData = entities.data();
	std::function<void(Entity&)> countEntity;
	countEntity = [&](auto& entity)
	{
		bool addMesh = true;
		for (auto& meshID : entity.meshIDs)
		{
			auto& mesh = Registry::getMesh(meshID);
			for (auto& keyedPair : mesh.keyedTextures)
			{
				if (keyedPair.second->isTransparent)
				{
					addMesh = false;
					break;
				}
			}
			if (addMesh)
				c++;
		}
		for (auto& child : entity.children)
			countEntity(child);
	};
	for (size_t i = 0; i < entitiesSize; ++i)
	{
		auto& entity = entitiesData[i];
		countEntity(entity);
	}
	return c;
}
std::vector<std::pair<Entity*, Mesh*>> Scene::getTransparentDrawList()
{
	auto size = getTransparentDrawCount();
	std::vector<std::pair<Entity*, Mesh*>> vec;
	vec.reserve(size);
	auto entitiesSize = entities.size();
	auto entitiesData = entities.data();
	std::function<void(Entity&)> addEntity;
	addEntity = [&](auto& entity)
	{
		for (auto& meshID : entity.meshIDs)
		{
			bool addMesh = false;
			auto& mesh = Registry::getMesh(meshID);
			for (auto& keyedPair : mesh.keyedTextures)
			{
				if (keyedPair.second->isTransparent)
				{
					addMesh = true;
					break;
				}
			}
			if (addMesh)
			{
				vec.push_back({&entity, &mesh});
			}
		}
		for (auto& child : entity.children)
			addEntity(child);
	};
	for (size_t i = 0; i < entitiesSize; ++i)
	{
		auto& entity = entitiesData[i];
		addEntity(entity);
	}
	return vec;
}
std::vector<std::pair<Entity*, Mesh*>> Scene::getOpaqueDrawList()
{
	auto size = getOpaqueDrawCount();
	std::vector<std::pair<Entity*, Mesh*>> vec;
	vec.reserve(size);
	auto entitiesSize = entities.size();
	auto entitiesData = entities.data();
	std::function<void(Entity&)> addEntity;
	addEntity = [&](auto& entity)
	{
		for (auto& meshID : entity.meshIDs)
		{
			bool addMesh = true;
			auto& mesh = Registry::getMesh(meshID);
			for (auto& keyedPair : mesh.keyedTextures)
			{
				if (keyedPair.second->isTransparent)
				{
					addMesh = false;
					break;
				}
			}
			if (addMesh)
				vec.push_back({&entity, &mesh});
		}
		for (auto& child : entity.children)
			addEntity(child);
	};
	for (size_t i = 0; i < entitiesSize; ++i)
	{
		auto& entity = entitiesData[i];
		addEntity(entity);
	}
	return vec;
}