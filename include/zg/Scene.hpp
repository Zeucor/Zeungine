#pragma once
#include "./lights/Lights.hpp"
#include "./lights/PointLight.hpp"
#include "./lights/DirectionalLight.hpp"
#include "./lights/SpotLight.hpp"
#include "./vp/View.hpp"
#include "./vp/Projection.hpp"
#include "./textures/Framebuffer.hpp"
#include "./raytracing/BVH.hpp"
namespace zg
{
	struct Scene
	{
		Window &window;
		glm::vec4 clearColor = glm::vec4(0);
		vp::Projection projection;
		std::map<size_t, std::shared_ptr<Entity>> entities;
		size_t entitiesCount = 0;
		std::vector<lights::PointLight> pointLights;
		std::vector<lights::DirectionalLight> directionalLights;
		std::vector<lights::SpotLight> spotLights;
		std::vector<lights::SpotLightShadow> spotLightShadows;
		std::vector<lights::PointLightShadow> pointLightShadows;
		std::vector<lights::DirectionalLightShadow> directionalLightShadows;
		textures::Framebuffer *framebufferPointer = 0;
		std::unique_ptr<raytracing::BVH> bvh;
		std::array<EventIdentifier, 7 - 0 + 1> mousePressIDs;
		EventIdentifier mouseMoveID;
		Entity *currentHoveredEntity = 0;
		vp::View view;
		bool useBVH;
		Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov, textures::Framebuffer *framebufferPointer = 0, bool useBVH = true);
		Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize, textures::Framebuffer *framebufferPointer = 0, bool useBVH = true);
		virtual ~Scene();
		size_t addEntity(const std::shared_ptr<Entity> &entity, bool callOnEntityAdded = true);
		void removeEntity(const size_t &id);
		virtual void update();
		void preRender();
		void render();
		void entityPreRender(Entity &entity);
		void resize(glm::vec2 newSize);
		void postAddEntity(const std::shared_ptr<Entity> &entity, const std::vector<size_t> &entityIDs);
		void preRemoveEntity(const std::shared_ptr<Entity> &entity, const std::vector<size_t> &entityIDs);
		Entity *findEntityByPrimID(const size_t &primID);
		void hookMouseEvents();
		void unhookMouseEvents();
	};
}