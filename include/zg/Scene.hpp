#pragma once
#include "./lights/Lights.hpp"
#include "./lights/PointLight.hpp"
#include "./lights/DirectionalLight.hpp"
#include "./lights/SpotLight.hpp"
#include "./vp/View.hpp"
#include "./vp/Projection.hpp"
#include "./textures/Framebuffer.hpp"
#include "./raytracing/BVH.hpp"
#include "./ComponentHolder.hpp"
#include "./interfaces/ISceneComponent.hpp"
namespace zg
{
	namespace entities
	{
		struct Plane;
	}
	struct Scene : ComponentHolder<interfaces::ISceneComponent>
	{
		bool drawColorToWindowPlane;
		Window &window;
		glm::vec4 clearColor = glm::vec4(0);
		std::shared_ptr<vp::Projection> projectionPointer;
		std::map<size_t, std::shared_ptr<Entity>> entities;
		size_t entitiesCount = 0;
		std::vector<lights::PointLight> pointLights;
		std::vector<lights::DirectionalLight> directionalLights;
		std::vector<lights::SpotLight> spotLights;
		std::vector<lights::SpotLightShadow> spotLightShadows;
		std::vector<lights::PointLightShadow> pointLightShadows;
		std::vector<lights::DirectionalLightShadow> directionalLightShadows;
		std::vector<std::shared_ptr<textures::Texture>> sceneTextures;
		std::shared_ptr<textures::Framebuffer> framebufferPointer;
		std::shared_ptr<entities::Plane> windowPlane;
		std::unique_ptr<raytracing::BVH> bvh;
		std::array<UniqueIdentifier, 7 - 0 + 1> mousePressIDs;
		UniqueIdentifier mouseMoveID;
		Entity *currentHoveredEntity = 0;
		std::shared_ptr<vp::View> viewPointer;
		bool useBVH;
		Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov, const std::shared_ptr<textures::Framebuffer>& framebufferPointer = {}, bool drawColorToWindowPlane = true, bool useBVH = true);
		Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize, const std::shared_ptr<textures::Framebuffer>& framebufferPointer = {}, bool drawColorToWindowPlane = true, bool useBVH = true);
		Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, float fov, const std::vector<textures::Framebuffer::AttachmentType>& attachments, bool drawColorToWindowPlane = true, bool useBVH = true);
		Scene(Window &_window, glm::vec3 cameraPosition, glm::vec3 cameraDirection, glm::vec2 orthoSize, const std::vector<textures::Framebuffer::AttachmentType>& attachments, bool drawColorToWindowPlane = true, bool _useBVH = true);
		std::vector<textures::Framebuffer::TextureAttachmentPair> generateTexturesFromAttachments(const std::vector<textures::Framebuffer::AttachmentType>& attachments);
		virtual ~Scene();
		size_t addEntity(const std::shared_ptr<Entity> &entity, bool callOnEntityAdded = true);
		void removeEntity(const size_t &id);
		virtual void preUpdate();
		void update();
		virtual void prePreRender();
		void preRender();
		void render();
		void renderEntities();
		void postRender();
		virtual void postPostRender();
		void entityPreRender(Entity &entity);
		void resize(glm::vec2 newSize);
		void postAddEntity(const std::shared_ptr<Entity> &entity, const std::vector<size_t> &entityIDs);
		void preRemoveEntity(const std::shared_ptr<Entity> &entity, const std::vector<size_t> &entityIDs);
		Entity *findEntityByPrimID(const size_t &primID);
		void hookMouseEvents();
		void unhookMouseEvents();
	};
}