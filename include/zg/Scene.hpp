#pragma once
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include "ComponentHolder.hpp"
#include "DataStorage.hpp"
#include "Entity.hpp"
#include "KeyIDVector.hpp"
#include "components/scenes/SceneComponent.hpp"
#include "lights/DirectionalLight.hpp"
#include "lights/Lights.hpp"
#include "lights/PointLight.hpp"
#include "lights/SpotLight.hpp"
#include "raytracing/BVH.hpp"
#include "textures/Framebuffer.hpp"
#include "vp/Projection.hpp"
#include "vp/View.hpp"
namespace zg
{
	struct SceneCreateInfo;
	struct Scene
			: DataStorage<Scene>,
				ComponentHolder<Scene, components::scenes::SceneComponent, components::scenes::SceneComponentCreateInfo>
	{
		size_t ID = 0;
		size_t* INDEX = 0;
		std::string name;
		bool drawColorToWindowPlane;
		Window& window;
		glm::vec4 clearColor = glm::vec4(0);
		std::shared_ptr<vp::Projection> projectionPointer;
		KeyIDVector<std::string, Entity> entities;
		std::vector<lights::PointLight> pointLights;
		std::vector<lights::DirectionalLight> directionalLights;
		std::vector<lights::SpotLight> spotLights;
		std::vector<lights::SpotLightShadow> spotLightShadows;
		std::vector<lights::PointLightShadow> pointLightShadows;
		std::vector<lights::DirectionalLightShadow> directionalLightShadows;
		std::vector<std::shared_ptr<textures::Texture>> sceneTextures;
		std::shared_ptr<textures::Framebuffer> framebufferPointer;
		Entity* windowPlane;
		std::unique_ptr<raytracing::BVH> bvh;
		std::array<UniqueIdentifier, 7 - 0 + 1> mousePressIDs;
		UniqueIdentifier mouseMoveID;
		Entity* currentHoveredEntity = 0;
		std::shared_ptr<vp::View> viewPointer;
		bool useBVH = true;
		size_t updateNonce = 0;
		std::function<void(Scene&)> onAttachedFunction;
		std::function<void(Scene&)> onDetachedFunction;
		std::function<void(Scene&)> preUpdateFunction;
		std::function<void(Scene&)> prePreRenderFunction;
		std::function<void(Scene&)> postPostRenderFunction;
		// when adding new members remember to add to operator=

	public:
		//
		Scene(const SceneCreateInfo& info);
		Scene(const Scene& other);
		Scene& operator=(const Scene& other);
		~Scene();
		std::vector<textures::Framebuffer::TextureAttachmentPair>
		generateTexturesFromAttachments(const std::vector<textures::Framebuffer::AttachmentType>& attachments);
		KeyIDVector<std::string, Entity>::EmplaceBackTuple addEntity(const EntityCreateInfo& info,
																																 bool callOnEntityAdded = true);
		template <typename... Args>
		std::array<KeyIDVector<std::string, Entity>::EmplaceBackTuple, sizeof...(Args) + 1>
		addEntities(bool callOnEntityAdded, const EntityCreateInfo& info, const Args&... args)
		{
			std::array<KeyIDVector<std::string, Entity>::EmplaceBackTuple, sizeof...(Args) + 1> arr;
			entities.reserve(entities.size() + sizeof...(Args) + 1);
			size_t index = 0;
			addEntitiesHelper<sizeof...(Args) + 1>(callOnEntityAdded, index, arr, info, args...);
		};
		template <size_t N, typename... Args>
		void addEntities(bool callOnEntityAdded, size_t& index,
										 std::array<KeyIDVector<std::string, Entity>::EmplaceBackTuple, N>& arr,
										 const EntityCreateInfo& info, const Args&... args)
		{
			arr[index] = addEntity(info, callOnEntityAdded);
			addEntitiesHelper<N>(callOnEntityAdded, index, arr, args...);
		};
		template <size_t N, typename... Args>
		void addEntities(bool callOnEntityAdded, size_t& index,
										 std::array<KeyIDVector<std::string, Entity>::EmplaceBackTuple, N>& arr) {};
		void removeEntity(const size_t& id);
		void update();
		void preRender();
		void render();
		void renderEntities();
		void postRender();
		void entityPreRender(Entity& entity);
		void resize(glm::vec2 newSize);
		void postAddEntity(Entity& entity, const std::vector<size_t>& entityIDs);
		void preRemoveEntity(Entity& entity, const std::vector<size_t>& entityIDs);
		Entity* findEntityByPrimID(const size_t& primID);
		void hookMouseEvents();
		void unhookMouseEvents();
		zg::Entity& getEntityByName(const std::string& name);
		zg::Entity& getEntityByID(const size_t& id);
	};
	struct SceneCreateInfo
	{
		std::string name = "Default Scene Name";
		glm::vec3 cameraPosition = glm::vec3(-5, 5, 0);
		glm::vec3 cameraDirection = glm::normalize(glm::vec3(0, -1, 1));
		glm::vec3 cameraUp = glm::vec3(0, 1, 0);
		vp::Projection::TYPE projectionType = vp::Projection::TYPE::Perspective;
		glm::vec2 orthoSize = glm::vec2(2, 2);
		float fov = 81.f;
		int framebufferCreateInt =
			0; // 0 = don't use framebuffer, 1 = use framebufferPointer, 2 = create from framebufferAttachments
		std::shared_ptr<textures::Framebuffer> framebufferPointer = {};
		std::vector<textures::Framebuffer::AttachmentType> frameBufferAttachments = {};
		std::function<void(Scene&)> onAttachedFunction = {};
		std::function<void(Scene&)> onDetachedFunction = {};
		std::function<void(Scene&)> preUpdateFunction = {};
		std::function<void(Scene&)> prePreRenderFunction = {};
		std::function<void(Scene&)> postPostRenderFunction = {};
		bool drawColorToWindowPlane = true;
		bool useBVH = true;
		Window* windowPointer = 0;
	};
} // namespace zg
