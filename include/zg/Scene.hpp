#pragma once
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include "pureconstcharstreamcode.hpp"
#include "ComponentHolder.hpp"
#include "DataStorage.hpp"
#include "Entity.hpp"
#include "Mesh.hpp"
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
#include "FullscreenQuad.hpp"
#include "PostProcessingPipeline.hpp"
#include "InstancedDraw.hpp"
namespace zg
{
	enum class SceneDrawMode
	{
		Single,
		Instanced,
		MultiInstanced
	};
	struct SceneCreateInfo;
	struct Scene
			: DataStorage<Scene>,
				ComponentHolder<Scene, components::scenes::SceneComponent, components::scenes::SceneComponentCreateInfo>
	{
		SYS_CLOCK::time_point sceneFirstEncountered;
		SYS_CLOCK::time_point sceneIsAt;
		size_t ID = 0;
		size_t* INDEX = 0;
		std::vector<size_t*> INDEX_STACK;
		float z = 0.0f;
		IRenderer* iRenderer = 0;
		SceneDrawMode drawMode = SceneDrawMode::MultiInstanced;
		std::string name;
		glm::vec4 clearColor = glm::vec4(0);
		std::shared_ptr<vp::Projection> projectionPointer;
		KeyIDVector<std::string, Entity> entities;
		std::vector<lights::PointLight> pointLights;
		std::vector<lights::DirectionalLight> directionalLights;
		std::vector<lights::SpotLight> spotLights;
		std::vector<lights::SpotLightShadow> spotLightShadows;
		std::vector<lights::PointLightShadow> pointLightShadows;
		std::vector<lights::DirectionalLightShadow> directionalLightShadows;
		std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> keyedTextures;
		std::shared_ptr<textures::Framebuffer> framebuffer;
		std::shared_ptr<FullscreenQuad> fsq;
		PostProcessingPipeline postProcessingPipeline;
		std::shared_ptr<raytracing::BVH> bvh;
		std::array<UniqueIdentifier, MaxMouseButton> mousePressIDs;
		UniqueIdentifier mouseMoveID;
		size_t currentHoveredEntityID = 0;
		std::shared_ptr<vp::View> viewPointer;
		bool useBVH = true;
		long double updateTime = 0;
		long double transparentDrawListTime = 0;
		long double opaqueDrawListTime = 0;
		std::function<void(Scene&)> onAttachedFunction;
		std::function<void(Scene&)> onDetachedFunction;
		std::function<void(Scene&)> preUpdateFunction;
		std::function<void(Scene&)> prePreRenderFunction;
		std::function<void(Scene&)> postPostRenderFunction;
		InstancedDraw instancedDraw;
		std::vector<std::pair<Entity*, Mesh*>> transparentDrawList;
		std::vector<std::pair<Entity*, Mesh*>> opaqueDrawList;
		size_t oldOpaqueHash = 0;
		size_t oldTransparentHash = 0;
		int32_t unit = 0;
		int32_t unitRemaining = 0;
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
		bool removeEntity(size_t ID);
		void update();
		void preRender();
		void render();
		void renderEntities();
		void postRender();
		void meshPreRender(Mesh& entity);
		void resize(glm::vec2 newSize);
		void postAddEntity(Entity& entity);
		void preRemoveEntity(Entity& entity);
		std::pair<Entity&, Mesh&> findEntityAndMeshByPrimID(const size_t& primID);
		void hookMouseEvents();
		void unhookMouseEvents();
		zg::Entity& getEntityByName(const std::string& name);
		zg::Entity& getEntityByID(const size_t& id);
		size_t getTransparentDrawCount();
		size_t getOpaqueDrawCount();
		TransparentDrawList& getTransparentDrawList();
		OpaqueDrawList& getOpaqueDrawList();
		void resetRenderedThisPassAllEntities();
	private:
		std::unordered_map<std::string, std::function<void(Scene&, shaders::Shader&, Mesh&)>> shaderSets = {
			{"Lighting", [](auto& scene, auto& shader, auto& mesh) {
				uint32_t index = 0;
				glm::mat4 directionalLightSpaceMatrices[4];
				for (auto& directionalLightShadow : scene.directionalLightShadows)
				{
					directionalLightSpaceMatrices[index] = directionalLightShadow.lightSpaceMatrix;
					++index;
				}
				shader.setBlock("DirectionalLightSpaceMatrices", mesh, directionalLightSpaceMatrices, sizeof(glm::mat4) * 4);
				glm::mat4 spotLightSpaceMatrices[4];
				index = 0;
				for (auto& spotLightShadow : scene.spotLightShadows)
				{
					spotLightSpaceMatrices[index] = spotLightShadow.lightSpaceMatrix;
					++index;
				}
				shader.setBlock("SpotLightSpaceMatrices", mesh, spotLightSpaceMatrices, sizeof(glm::mat4) * 4);
				shader.setSSBO("DirectionalLights", mesh, scene.directionalLights.data(), scene.directionalLights.size() * sizeof(lights::DirectionalLight));
				shader.setSSBO("SpotLights", mesh, scene.spotLights.data(), scene.spotLights.size() * sizeof(lights::SpotLight));
				shader.setSSBO("PointLights", mesh, scene.pointLights.data(), scene.pointLights.size() * sizeof(lights::PointLight));
			}},
			{"DirectionalLightShadowMaps", [](auto& scene, auto& shader, auto& mesh) {
				scene.unit = 0;
				int32_t index = 0;
				scene.unitRemaining = 4;
				for (auto& directionalLightShadow : scene.directionalLightShadows)
				{
					shader.setTexture("directionalLightSamplers[" + std::to_string(index) + "]", mesh,
														*directionalLightShadow.texture, scene.unit);
					++scene.unit;
					--scene.unitRemaining;
				}
			}},
			{"SpotLightShadowMaps", [](auto& scene, auto& shader, auto& mesh) {
				int32_t index = 0;
				scene.unit += scene.unitRemaining;
				scene.unitRemaining = 4;
				for (auto& spotLightShadow : scene.spotLightShadows)
				{
					shader.setTexture("spotLightSamplers[" + std::to_string(index) + "]", mesh, *spotLightShadow.texture, scene.unit);
					++scene.unit;
					--scene.unitRemaining;
				}

			}},
			{"PointLightShadowMaps", [](auto& scene, auto& shader, auto& mesh) {
				int32_t index = 0;
				scene.unit += scene.unitRemaining;
				scene.unitRemaining = 4;
				for (auto& pointLightShadow : scene.pointLightShadows)
				{
					shader.setTexture("pointLightSamplers[" + std::to_string(index) + "]", mesh, *pointLightShadow.texture, scene.unit);
					++scene.unit;
					--scene.unitRemaining;
				}
			}}
		};
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
			0; // 0 = creates standard color/depth framebuffer, 1 = use framebuffer, 2 = create from framebufferAttachments
		std::shared_ptr<textures::Framebuffer> framebuffer = {};
		std::vector<textures::Framebuffer::AttachmentType> frameBufferAttachments = {};
		std::function<void(Scene&)> onAttachedFunction = {};
		std::function<void(Scene&)> onDetachedFunction = {};
		std::function<void(Scene&)> preUpdateFunction = {};
		std::function<void(Scene&)> prePreRenderFunction = {};
		std::function<void(Scene&)> postPostRenderFunction = {};
		bool drawColorToWindowPlane = true;
		bool useBVH = true;
		DataStorage<Scene>::DataMap dataMap;
		DataStorage<Scene>::GetDataFunctionMap getDataFunctionMap;
		DataStorage<Scene>::SetDataFunctionMap setDataFunctionMap;
		size_t ID;
		size_t* INDEX;
		std::vector<size_t*> INDEX_STACK;
	};
} // namespace zg
