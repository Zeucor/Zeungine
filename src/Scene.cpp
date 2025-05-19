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
	DataStorage<Scene>(info.getDataFunctionMap, info.setDataFunctionMap, info.dataMap),
	sceneFirstEncountered(SYS_CLOCK::now()),
	ID(info.ID),
	INDEX(info.INDEX),
	INDEX_STACK(info.INDEX_STACK),
	iRenderer(Registry::GetSingleton().getWindow(INDEX_STACK).iRenderer),
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
	ZGZoneScopedN("Scene::constructor");
	auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
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
		bvh = std::make_shared<raytracing::BVH>();
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
	updateTime(other.updateTime),
	onAttachedFunction(other.onAttachedFunction),
	onDetachedFunction(other.onDetachedFunction),
	preUpdateFunction(other.preUpdateFunction),
	prePreRenderFunction(other.prePreRenderFunction),
	postPostRenderFunction(other.postPostRenderFunction)
{
	ZGZoneScopedN("Scene::constructor");
	if (useBVH)
	{
		bvh = std::make_shared<raytracing::BVH>();
	}
	hookMouseEvents();
}
Scene& Scene::operator=(const Scene& other)
{
	ZGZoneScopedN("Scene::operator=");
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
	unhookMouseEvents();
	hookMouseEvents();
	currentHoveredEntityID = other.currentHoveredEntityID;
	viewPointer = other.viewPointer;
	useBVH = other.useBVH;
	bvh = other.bvh;
	updateTime = other.updateTime;
	onAttachedFunction = other.onAttachedFunction;
	onDetachedFunction = other.onDetachedFunction;
	preUpdateFunction = other.preUpdateFunction;
	prePreRenderFunction = other.prePreRenderFunction;
	postPostRenderFunction = other.postPostRenderFunction;
	return *this;
};
Scene::~Scene()
{
	ZGZoneScopedN("Scene::destructor");
	detachAllComponents();
	for (auto& entity : entities)
		if (entity.onRemovedFunction)
			entity.onRemovedFunction(entity);
	dataMap.clear();
	getDataFunctionMap.clear();
	setDataFunctionMap.clear();
	entities.clear();
	postProcessingPipeline.cleanup();
	unhookMouseEvents();
}
std::vector<textures::Framebuffer::TextureAttachmentPair>
Scene::generateTexturesFromAttachments(const std::vector<textures::Framebuffer::AttachmentType>& attachments)
{
	ZGZoneScoped;
	auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
	std::vector<textures::Framebuffer::TextureAttachmentPair> textureAttachmentPairs;
	for (auto& attachment : attachments)
	{
		ZGZoneScoped;
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
	ZGZoneScoped;
	auto usingInfo{info};
	auto transaction = entities.startTransaction();
	usingInfo.INDEX_STACK = {INDEX_STACK.begin(), INDEX_STACK.end()};
	usingInfo.INDEX_STACK.push_back(transaction.index);
	usingInfo.ID = transaction.id;
	usingInfo.INDEX = transaction.index;
	auto& entity = entities.commitTransaction(transaction, usingInfo);
	Registry::GetSingleton().idEntities[entity.ID] = entity.INDEX_STACK;
	postAddEntity(entity);
	if (entity.onAddedFunction)
		entity.onAddedFunction(entity);
	auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
	if (callOnEntityAdded && window.onEntityAdded)
		window.onEntityAdded(entity);
	return {transaction.key, transaction.id, transaction.index, &entity};
}
bool Scene::removeEntity(size_t ID)
{
	ZGZoneScoped;
	auto entityIter = entities.find_id(ID);
	if (entityIter == entities.end())
		return false;
	auto& entity = *entityIter;
	instancedDraw.removeEntity(entity);
	if (entity.onRemovedFunction)
		entity.onRemovedFunction(entity);
	preRemoveEntity(entity);
	entities.erase(entityIter);
	auto& idEntities = Registry::GetSingleton().idEntities;
	auto idIter = idEntities.find(ID);
	if (idIter != idEntities.end())
		idEntities.erase(idIter);
	return true;
}
void Scene::update()
{
	ZGZoneScoped;
	sceneIsAt = SYS_CLOCK::now();
	updateTime = (sceneIsAt - sceneFirstEncountered).count() / 10'000'000.0;
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
			ZGZoneScoped;
			bvh->buildBVH();
		}
	}
}
void Scene::preRender()
{
	ZGZoneScoped;
	if (prePreRenderFunction)
		prePreRenderFunction(*this);
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	std::function<void(Entity&, shaders::Shader&, const std::function<void(Mesh&, shaders::Shader&)>)> drawEntity;
	drawEntity = [&](auto& entity, auto& shader, auto setShader)
	{
		ZGZoneScoped;
		if (!entity.affectedByShadows)
			return;
		for (auto& meshID : entity.meshIDs)
		{
			ZGZoneScoped;
			auto& mesh = Registry::GetSingleton().getMesh(meshID);
			mesh.uid = entity.ID;
			shader.bind(mesh);
			setShader(mesh, shader);
			shader.setSSBO("InstanceModels", mesh, &entity.getModelMatrix(), sizeof(glm::mat4));
			mesh.drawVAO(&shader);
			shader.unbind();
		}
		for (auto& child : entity.children)
			drawEntity(child, shader, setShader);
	};
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		ZGZoneScoped;
		directionalLightShadow.framebuffer->bind();
		auto shaderPointer = directionalLightShadow.addShader();
		iRenderer->clear();
		auto transparentDrawList = getTransparentDrawList();
		auto opaqueDrawList = getOpaqueDrawList();
		instancedDraw.drawMulti(SHADER_BATCH_DIRECTIONAL_LIGHT, *this, opaqueDrawList, transparentDrawList, oldOpaqueHash, oldTransparentHash, shaderPointer, {
			{"InverseInstanceProjections", directionalLightShadow.inverseProjection},
			{"Projection", directionalLightShadow.projection},
			{"InverseInstanceViews", directionalLightShadow.inverseView},
			{"View", directionalLightShadow.view}
		});
		directionalLightShadow.framebuffer->unbind();
	}
	// for (auto& spotLightShadow : spotLightShadows)
	// {
	// 	ZGZoneScoped;
	// 	spotLightShadow.framebuffer->bind();
	// 	auto shaderPointer = spotLightShadow.addShader();
	// 	iRenderer->clear();
	// 	for (size_t index = 0; index < entitiesSize; ++index)
	// 	{
	// 		ZGZoneScoped;
	// 		auto& entity = entitiesData[index];
	// 		drawEntity(entity, *spotLightShadow.shader, [&](auto& mesh, auto& shader) {
	// 			shader.setBlock(
	// 				"LightSpaceMatrix",
	// 				mesh,
	// 				spotLightShadow.lightSpaceMatrix,
	// 				sizeof(glm::mat4)
	// 			);
	// 		});
	// 	}
	// 	spotLightShadow.framebuffer->unbind();
	// }
	// for (auto& pointLightShadow : pointLightShadows)
	// {
	// 	ZGZoneScoped;
	// 	pointLightShadow.framebuffer->bind();
	// 	iRenderer->clear();
	// 	for (size_t index = 0; index < entitiesSize; ++index)
	// 	{
	// 		ZGZoneScoped;
	// 		auto& entity = entitiesData[index];
	// 		drawEntity(entity, *pointLightShadow.shader, [&](auto& mesh, auto& shader) {
	// 			shader.setBlock(
	// 				"PointLightSpaceMatrix",
	// 				mesh,
	// 				pointLightShadow.shadowTransforms,
	// 				sizeof(glm::mat4) * 6
	// 			);
	// 			shader.setUniform("nearPlane", mesh, pointLightShadow.pointLight.nearPlane);
	// 			shader.setUniform("farPlane", mesh, pointLightShadow.pointLight.farPlane);
	// 			shader.setUniform("lightPos", mesh, pointLightShadow.pointLight.position);
	// 		});
	// 	}
	// 	pointLightShadow.framebuffer->unbind();
	// }
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
	ZGZoneScoped;
	auto& framebufferRef = *framebuffer;
	framebufferRef.bind();
	auto transparentDrawList = getTransparentDrawList();
	auto opaqueDrawList = getOpaqueDrawList();
	instancedDraw.drawMulti(
		SHADER_BATCH_MAIN,
		*this,
		opaqueDrawList,
		transparentDrawList,
		oldOpaqueHash,
		oldTransparentHash,
		0,
		{},
		shaderSets
	);
	// for (auto& ep : transparentDrawList)
	// {
	// 	ep.second->uid = ep.first->ID;
	// 	ep.second->render(*ep.first);
	// }
	framebufferRef.unbind();
}
void Scene::postRender()
{
	ZGZoneScoped;
	if (postPostRenderFunction)
		postPostRenderFunction(*this);
	auto entitiesData = entities.data();
	auto entitiesSize = entities.size();
	for (size_t index = 0; index < entitiesSize; ++index)
		entitiesData[index].postRender();
}
void Scene::meshPreRender(Mesh& mesh)
{
	ZGZoneScoped;
}
void Scene::resize(glm::vec2 newSize)
{
	ZGZoneScoped;
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
void Scene::postAddEntity(Entity& entity)
{
	ZGZoneScoped;
	if (useBVH)
		bvh->addEntity(entity);
}
void Scene::preRemoveEntity(Entity& entity)
{
	ZGZoneScoped;
	if (useBVH)
		bvh->removeEntity(*this, entity);
}
std::pair<Entity&, Mesh&> Scene::findEntityAndMeshByPrimID(const size_t& primID)
{
	ZGZoneScoped;
	if (!useBVH)
		throw std::runtime_error("Scene is not using a BVH");
	auto& _bvh = *bvh;
	auto& tri = _bvh.triangles[_bvh.bvh.prim_ids[primID]];
	auto& entityMeshID = tri.userData;
	if (!entityMeshID.first || !entityMeshID.second)
	{
		throw std::runtime_error("Ohmy now, we always set tri IDs, so this should logically never happen");
	}
	auto& entity = Registry::GetSingleton().getEntity(entityMeshID.first);
	return {entity, Registry::GetSingleton().getMesh(entityMeshID.second)};
}
void Scene::hookMouseEvents()
{
	ZGZoneScoped;
	auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
	for (unsigned int button = MinMouseButtonIndex; button < MaxMouseButton; ++button)
	{
		mousePressIDs[button] = window.addMousePressHandler(
			button,
			[&, button](auto pressed)
			{
				ZGZoneScoped;
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
			ZGZoneScoped;
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
					auto& currentHoveredEntity = Registry::GetSingleton().getEntity(currentHoveredEntityID);
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
					auto& currentHoveredEntity = Registry::GetSingleton().getEntity(currentHoveredEntityID);
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
	ZGZoneScoped;
	auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
	for (unsigned int button = MinMouseButtonIndex; button <= MaxMouseButtonIndex; ++button)
	{
		window.removeMousePressHandler(button, mousePressIDs[button]);
	}
	window.removeMouseMoveHandler(mouseMoveID);
}
zg::Entity& Scene::getEntityByName(const std::string& name)
{
	ZGZoneScoped;
	auto iter = entities.find_key(name);
	if (iter == entities.end())
		throw std::runtime_error("Entity not found with name");
	return *iter;
}
zg::Entity& Scene::getEntityByID(const size_t& id)
{
	ZGZoneScoped;
	auto iter = entities.find_id(id);
	if (iter == entities.end())
		throw std::runtime_error("Entity not found with name");
	return *iter;
}
template <>
Serial& serialize(Serial& serial, const Scene& scene)
{
	ZGZoneScoped;
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
	ZGZoneScoped;
	auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
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
		scene.postAddEntity(entity);
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
			if (entity.isTransparent)
			{
				addMesh = true;
			}
			else
			{
				auto& mesh = Registry::GetSingleton().getMesh(meshID);
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						addMesh = true;
						break;
					}
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
			if (entity.isTransparent)
			{
				addMesh = false;
				break;
			}
			else
			{
				auto& mesh = Registry::GetSingleton().getMesh(meshID);
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						addMesh = false;
						break;
					}
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
std::vector<std::pair<Entity*, Mesh*>>& Scene::getTransparentDrawList()
{
	if (updateTime == transparentDrawListTime)
		return transparentDrawList;
	transparentDrawListTime = updateTime;
	auto size = getTransparentDrawCount();
	transparentDrawList.resize(size);
	auto transparentDrawListData = transparentDrawList.data();
	auto entitiesSize = entities.size();
	auto entitiesData = entities.data();
	std::function<void(Entity&)> addEntity;
	size_t index = 0;
	addEntity = [&](auto& entity)
	{
		for (auto& meshID : entity.meshIDs)
		{
			auto& mesh = Registry::GetSingleton().getMesh(meshID);
			bool addMesh = false;
			if (entity.isTransparent)
			{
				addMesh = true;
			}
			else
			{
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						addMesh = true;
						break;
					}
				}
			}
			if (addMesh)
			{
				transparentDrawListData[index++] = {&entity, &mesh};
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
	auto cameraPosition = viewPointer->position;
	{
		ZGZoneScoped;
		std::sort(transparentDrawList.begin(), transparentDrawList.end(), [&](auto& a, auto& b)
		{
			auto distA = glm::distance(a.first->position, cameraPosition);
			auto distB = glm::distance(b.first->position, cameraPosition);
			return distA > distB;
		});
	}
	return transparentDrawList;
}
std::vector<std::pair<Entity*, Mesh*>>& Scene::getOpaqueDrawList()
{
	if (updateTime == opaqueDrawListTime)
		return opaqueDrawList;
	opaqueDrawListTime = updateTime;
	auto size = getOpaqueDrawCount();
	opaqueDrawList.resize(size);
	auto opaqueDrawListData = opaqueDrawList.data();
	auto entitiesSize = entities.size();
	auto entitiesData = entities.data();
	std::function<void(Entity&)> addEntity;
	size_t index = 0;
	addEntity = [&](auto& entity)
	{
		for (auto& meshID : entity.meshIDs)
		{
			auto& mesh = Registry::GetSingleton().getMesh(meshID);
			bool addMesh = true;
			if (entity.isTransparent)
			{
				addMesh = false;
				break;
			}
			else
			{
				for (auto& keyedPair : mesh.info.keyedTextures)
				{
					if (keyedPair.second->isTransparent)
					{
						addMesh = false;
						break;
					}
				}
			}
			if (addMesh)
				opaqueDrawListData[index++] = {&entity, &mesh};
		}
		for (auto& child : entity.children)
			addEntity(child);
	};
	for (size_t i = 0; i < entitiesSize; ++i)
	{
		auto& entity = entitiesData[i];
		addEntity(entity);
	}
	return opaqueDrawList;
}