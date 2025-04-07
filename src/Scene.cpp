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
using namespace zg;
Scene::Scene(Window& _window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov,
						 const std::shared_ptr<textures::Framebuffer>& _framebufferPointer, bool drawColorToWindowPlane,
						 bool _useBVH) :
		drawColorToWindowPlane(drawColorToWindowPlane), window(_window),
		viewPointer(std::make_shared<vp::View>(cameraPosition, cameraDirection, glm::vec3(0, 1, 0))),
		projectionPointer(std::make_shared<vp::Projection>(window, fov)), framebufferPointer(_framebufferPointer),
		useBVH(_useBVH)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
}
Scene::Scene(Window& _window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize,
						 const std::shared_ptr<textures::Framebuffer>& _framebufferPointer, bool drawColorToWindowPlane,
						 bool _useBVH) :
		drawColorToWindowPlane(drawColorToWindowPlane), window(_window),
		viewPointer(std::make_shared<vp::View>(cameraPosition, cameraDirection, glm::vec3(0, 1, 0))),
		projectionPointer(std::make_shared<vp::Projection>(window, orthoSize)), framebufferPointer(_framebufferPointer),
		useBVH(_useBVH)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
}
Scene::Scene(Window& _window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov,
						 const std::vector<textures::Framebuffer::AttachmentType>& attachments, bool drawColorToWindowPlane,
						 bool _useBVH) :
		drawColorToWindowPlane(drawColorToWindowPlane), window(_window),
		viewPointer(std::make_shared<vp::View>(cameraPosition, cameraDirection, glm::vec3(0, 1, 0))),
		projectionPointer(std::make_shared<vp::Projection>(window, fov)),
		framebufferPointer(std::make_shared<textures::Framebuffer>(_window, generateTexturesFromAttachments(attachments))),
		useBVH(_useBVH)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
}
Scene::Scene(Window& _window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize,
						 const std::vector<textures::Framebuffer::AttachmentType>& attachments, bool drawColorToWindowPlane,
						 bool _useBVH) :
		drawColorToWindowPlane(drawColorToWindowPlane), window(_window),
		viewPointer(std::make_shared<vp::View>(cameraPosition, cameraDirection, glm::vec3(0, 1, 0))),
		projectionPointer(std::make_shared<vp::Projection>(window, orthoSize)),
		framebufferPointer(std::make_shared<textures::Framebuffer>(_window, generateTexturesFromAttachments(attachments))),
		useBVH(_useBVH)
{
	if (useBVH)
	{
		bvh = std::make_unique<raytracing::BVH>();
	}
	hookMouseEvents();
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
			std::make_shared<textures::Texture>(window, glm::ivec4(window.windowWidth, window.windowHeight, 0, 0),
																					(const void*)0, format, type, textures::Texture::FilterType::Nearest));
		textureAttachmentPairs.push_back({sceneTextures[sceneTextures.size() - 1].get(), attachment});
	}
	return textureAttachmentPairs;
}
Scene::~Scene() { unhookMouseEvents(); }
size_t Scene::addEntity(const std::shared_ptr<Entity>& entity, bool callOnEntityAdded)
{
	auto id = ++entitiesCount;
	entity->ID = id;
	entities.insert({id, entity->name, entity});
	postAddEntity(entity, {id});
	if (callOnEntityAdded && window.onEntityAdded)
		window.onEntityAdded(entity);
	return id;
}
void Scene::removeEntity(const size_t& id)
{
	auto entityIter = entities.find(id);
	if (entityIter != entities.end())
	{
		preRemoveEntity(entityIter->ENTITY, {id});
		entityIter->ENTITY->ID = 0;
		entities.erase(entityIter);
	}
}
void Scene::preUpdate() {}
void Scene::update()
{
	preUpdate();
	for (auto& component : std::get<1>(m_components))
	{
		component.second->onUpdate();
	}
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->ENTITY->update();
	}
	if (useBVH)
	{
		if (bvh->changed)
		{
			bvh->buildBVH();
		}
	}
}
void Scene::prePreRender() {}
void Scene::preRender()
{
	prePreRender();
	update();
	auto& iRenderer = *window.iRenderer;
	for (auto& directionaLightShadow : directionalLightShadows)
	{
		directionaLightShadow.framebuffer.bind();
		directionaLightShadow.addShader();
		iRenderer.clear();
		for (auto& entityPair : entities)
		{
			auto& entityPointer = entityPair.ENTITY;
			auto& vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto& glEntity = *std::dynamic_pointer_cast<Entity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			directionaLightShadow.shader->bind(glEntity);
			directionaLightShadow.shader->setBlock("LightSpaceMatrix", glEntity, directionaLightShadow.lightSpaceMatrix,
																						 sizeof(glm::mat4));
			const auto& model = glEntity.getModelMatrix();
			directionaLightShadow.shader->setBlock("Model", glEntity, model);
			iRenderer.bindShader(*glEntity.addShader(directionaLightShadow.shader), glEntity);
			vbo.drawVAO();
			directionaLightShadow.shader->unbind();
		}
		directionaLightShadow.framebuffer.unbind();
	}
	for (auto& spotLightShadow : spotLightShadows)
	{
		spotLightShadow.framebuffer.bind();
		iRenderer.clear();
		for (auto& entityPair : entities)
		{
			auto& entityPointer = entityPair.ENTITY;
			auto& vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto& glEntity = *std::dynamic_pointer_cast<Entity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			spotLightShadow.shader->bind(glEntity);
			spotLightShadow.shader->setBlock("LightSpaceMatrix", glEntity, spotLightShadow.lightSpaceMatrix,
																			 sizeof(glm::mat4));
			const auto& model = glEntity.getModelMatrix();
			spotLightShadow.shader->setBlock("Model", glEntity, model);
			vbo.drawVAO();
			spotLightShadow.shader->unbind();
		}
		spotLightShadow.framebuffer.unbind();
	}
	for (auto& pointLightShadow : pointLightShadows)
	{
		pointLightShadow.framebuffer.bind();
		iRenderer.clear();
		for (auto& entityPair : entities)
		{
			auto& entityPointer = entityPair.ENTITY;
			auto& vbo = *std::dynamic_pointer_cast<vaos::VAO>(entityPointer);
			auto& glEntity = *std::dynamic_pointer_cast<Entity>(entityPointer);
			if (!glEntity.affectedByShadows)
			{
				continue;
			}
			pointLightShadow.shader->bind(glEntity);
			pointLightShadow.shader->setBlock("PointLightSpaceMatrix", glEntity, pointLightShadow.shadowTransforms,
																				sizeof(glm::mat4) * 6);
			pointLightShadow.shader->setUniform("nearPlane", glEntity, pointLightShadow.pointLight.nearPlane);
			pointLightShadow.shader->setUniform("farPlane", glEntity, pointLightShadow.pointLight.farPlane);
			pointLightShadow.shader->setUniform("lightPos", glEntity, pointLightShadow.pointLight.position);
			const auto& model = glEntity.getModelMatrix();
			pointLightShadow.shader->setBlock("Model", glEntity, model);
			vbo.drawVAO();
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
	auto it = entities.begin();
	auto end = entities.end();
	for (; it != end; it++)
	{
		it->ENTITY->render();
	}
	if (framebufferPointer)
		framebufferPointer->unbind();
}
void Scene::postRender() { postPostRender(); }
void Scene::postPostRender() {}
void Scene::entityPreRender(Entity& entity)
{
	Entity& glEntity = static_cast<Entity&>(entity);
	auto data = glEntity.getShaderData(window);
	auto shader = glEntity.shaders[data];
	uint32_t index = 0;
	glm::mat4 directionalLightSpaceMatrices[4];
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		directionalLightSpaceMatrices[index] = directionalLightShadow.lightSpaceMatrix;
		++index;
	}
	shader->setBlock("DirectionalLightSpaceMatrices", glEntity, directionalLightSpaceMatrices, sizeof(glm::mat4) * 4);
	glm::mat4 spotLightSpaceMatrices[4];
	index = 0;
	for (auto& spotLightShadow : spotLightShadows)
	{
		spotLightSpaceMatrices[index] = spotLightShadow.lightSpaceMatrix;
		++index;
	}
	shader->setBlock("SpotLightSpaceMatrices", glEntity, spotLightSpaceMatrices, sizeof(glm::mat4) * 4);
	int32_t unit = 0;
	index = 0;
	uint32_t unitRemaining = 4;
	for (auto& directionalLightShadow : directionalLightShadows)
	{
		shader->setTexture("directionalLightSamplers[" + std::to_string(index) + "]", glEntity,
											 directionalLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("DirectionalLights", glEntity, directionalLights.data(),
									directionalLights.size() * sizeof(lights::DirectionalLight));
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto& spotLightShadow : spotLightShadows)
	{
		shader->setTexture("spotLightSamplers[" + std::to_string(index) + "]", glEntity, spotLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("SpotLights", glEntity, spotLights.data(), spotLights.size() * sizeof(lights::SpotLight));
	index = 0;
	unit += unitRemaining;
	unitRemaining = 4;
	for (auto& pointLightShadow : pointLightShadows)
	{
		shader->setTexture("pointLightSamplers[" + std::to_string(index) + "]", glEntity, pointLightShadow.texture, unit);
		++unit;
		--unitRemaining;
	}
	shader->setSSBO("PointLights", glEntity, pointLights.data(), pointLights.size() * sizeof(lights::PointLight));
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
void Scene::postAddEntity(const std::shared_ptr<Entity>& entity, const std::vector<size_t>& entityIDs)
{
	auto& glEntity = (Entity&)*entity;
	if (useBVH && glEntity.addToBVH)
	{
		bvh->addEntity(glEntity);
		// for (auto &triangleID : triangleIDs)
		// {
		// 	triangleIDsToEntityIDsMap[triangleID] = entityIDs;
		// }
	}
	auto glEntityChildrenSize = glEntity.children.size();
	for (auto& pair : glEntity.children)
	{
		auto childEntityID = pair.first;
		auto entityIDsWithSubID = entityIDs;
		entityIDsWithSubID.push_back(childEntityID);
		postAddEntity(pair.second, entityIDsWithSubID);
	}
}
void Scene::preRemoveEntity(const std::shared_ptr<Entity>& entity, const std::vector<size_t>& entityIDs)
{
	auto& glEntity = (Entity&)*entity;
	if (useBVH && glEntity.addToBVH)
	{
		bvh->removeEntity(*this, glEntity);
	}
}
Entity* Scene::findEntityByPrimID(const size_t& primID)
{
	if (!useBVH)
		return 0;
	auto& _bvh = *bvh;
	auto& tri = _bvh.triangles[_bvh.bvh.prim_ids[primID]];
	auto& userData = tri.userData;
	if (!userData)
		return 0;
	return (Entity*&)userData;
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
			windowPlane = std::make_shared<entities::Plane>(window, *this, glm::vec3(0), glm::vec3(0, 180, 0),
																											glm::vec3(1), glm::vec2(2, 2), *colorAttachmentIter->first);
			auto& windowPlaneRef = *windowPlane;
			windowPlaneRef.projectionPointer = std::make_shared<vp::Projection>(window, glm::vec2(2, 2));
			windowPlaneRef.viewPointer = std::make_shared<vp::View>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
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
std::shared_ptr<zg::Entity> Scene::getEntityByName(const std::string& name)
{
	auto& entities_name_index = entities.get<entity_by_name>();
	auto it_name = entities_name_index.find(name);
	if (it_name != entities_name_index.end())
	{
		return it_name->ENTITY;
	}
	return {};
}
std::shared_ptr<zg::Entity> Scene::getEntityByID(const size_t& id)
{
	auto& entities_id_index = entities.get<entity_by_id>();
	auto it_id = entities_id_index.find(id);
	if (it_id != entities_id_index.end())
	{
		return it_id->ENTITY;
	}
	return {};
}
template<>
Serial& serialize(Serial& serial, const Scene& scene)
{
	serial << true << scene.drawColorToWindowPlane << scene.clearColor << scene.projectionPointer;
	auto entitiesSize = scene.entities.size();
	serial << entitiesSize;
	for (auto& entityPair : scene.entities)
	{
		auto& ID = entityPair.ID;
		auto& entityPointer = entityPair.ENTITY;
		if (!entityPointer)
		{
			serial << false;
			continue;
		}
		auto ENTITY_TYPE_ID = entityPointer->getTypeID();
		serial << true << ID << ENTITY_TYPE_ID;
		auto serializeFunction = Entity::getSerialize(ENTITY_TYPE_ID);
		serializeFunction(serial, entityPointer);
	}
	serial << scene.entitiesCount;

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
template<>
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
		size_t ID = 0, ENTITY_TYPE_ID = 0;
		serial >> ID >> ENTITY_TYPE_ID;
		auto deserializeFunction = Entity::getDeserialize(ENTITY_TYPE_ID);
		std::shared_ptr<zg::Entity> entityPointer;
		deserializeFunction(serial, entityPointer);
		scene.entities.insert({ID, entityPointer->name, entityPointer});
		scene.postAddEntity(entityPointer, {ID});
		if (scene.window.onEntityAdded)
			scene.window.onEntityAdded(entityPointer);
	}
	serial >> scene.entitiesCount;
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
		scene.framebufferPointer = std::make_shared<zg::textures::Framebuffer>(scene.window, textureAttachmentPairs);
	}
	serial >> scene.windowPlane >> scene.viewPointer;
	return serial;
}