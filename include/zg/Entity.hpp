#pragma once
#include "ComponentHolder.hpp"
#include "DataStorage.hpp"
#include "KeyIDVector.hpp"
#include "Serial.hpp"
#include "components/entities/EntityComponent.hpp"
#include "entities/TypeID.hpp"
#include "renderers/GLRenderer.hpp"
#include "vaos/VAO.hpp"
#include "vp/Projection.hpp"
#include "vp/View.hpp"
#include "Mesh.hpp"
#include "observable_ptr.hpp"
#include "EventExecutor.hpp"
namespace zg
{
	struct Scene;
	struct EntityCreateInfo;
	struct Window;
	using ValueSetterPair = std::pair<zg::observable_ptr<std::any>, std::function<void(Mesh&, shaders::Shader&, const std::any&)>>;
	struct Entity :
		ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>,
		DataStorage<Entity>,
		EventExecutor
	{
		friend Scene;
		friend Window;

	public:
		using SerializeFunction = std::function<Serial&(Serial&, const Entity&)>;
		using DeserializeFunction = std::function<Serial&(Serial&, EntityCreateInfo&)>;
		using SerializeMap = std::unordered_map<std::string, SerializeFunction>;
		using DeserializeMap = std::unordered_map<std::string, DeserializeFunction>;
		static void registerSerialize(const std::string& typeName, const SerializeFunction& function);
		static void registerDeserialize(const std::string& typeName, const DeserializeFunction& function);
		static SerializeFunction getSerialize(const std::string& typeName);
		static DeserializeFunction getDeserialize(const std::string& typeName);

	protected:
		static void cleanupSerialize();

	public:
		size_t ID = 0;
		size_t* INDEX = 0;
		std::vector<size_t*> INDEX_STACK;
		std::any VALUE;
		int32_t meta_int;
		float meta_float;
		glm::vec4 meta_vec4;
		template<typename T>
		T& getValue()
		{
			return std::any_cast<T&>(VALUE);
		}
		template <typename T>
		const T& getValue() const
		{
			return std::any_cast<const T&>(VALUE);
		}
		std::string typeName;
		std::string name;
		size_t mesh_hash = 0;
		// transform
		glm::vec3 position;
		glm::quat rotation;
		observable_ptr<glm::vec3> scale;
		observable_ptr<bool> isDirty =  observable_ptr<bool>(true, true);
		glm::mat4 model;
		// view/projection overrides
		std::shared_ptr<vp::Projection> projectionPointer;
		std::shared_ptr<vp::View> viewPointer;
		// some flags
		bool affectedByShadows = true;
		bool addToBVH = true;
		bool skipRender = false;
		bool renderOncePerPass = true;
		bool renderedThisPass = false;
		long double updateTime;
		// settable functions
		std::function<void(Entity&)> preUpdateFunction;
		std::function<bool(Entity&)> preRenderFunction;
		std::function<void(Entity&)> postRenderFunction;
		std::function<void(Entity&)> onAddedFunction;
		std::unordered_map<std::string, std::function<void(Entity&)>> onRemoveFunctionMap;
		std::recursive_mutex handlersMutex;
		// meshes and children
		std::vector<size_t> meshIDs;
		std::vector<MeshCreateInfo> meshInfos;
		KeyIDVector<std::string, Entity> children;
		bool isTransparent = false;
		// runtime constant value shader setters
		std::unordered_map<std::string, ValueSetterPair> runtimeConstantValueShaderSetters;
	public:
		Entity(const EntityCreateInfo& info);
		Entity(const Entity& other);
		~Entity();
		Entity& operator=(const Entity& other);
		void refreshMeshes();
		Material& meshMaterial(size_t meshID);
		float operator()(glm::vec3 p) const;
		// K::Sphere_3 getSuggestedBoundingSphere(float multiplier = 1.5f) const;
		glm::vec3 getBoundingSize() const;
		
	private:
		void reMeshhash();

	public:
		void update();
		void render();
		size_t getOpaqueChildDrawCount();
		std::vector<std::pair<Entity*, Mesh*>> getOpaqueChildDrawList();
		size_t getTransparentChildDrawCount();
		std::vector<std::pair<Entity*, Mesh*>> getTransparentChildDrawList();
		void postRender();
		glm::mat4& getModelMatrix();
		std::tuple<glm::vec3, glm::quat, glm::vec3, glm::vec3, glm::vec4> decomposeModel();
		glm::vec3 getModelPosition();
		glm::quat getModelRotation();
		glm::vec3 getModelScale();
		KeyIDVector<std::string, Entity>::EmplaceBackTuple addEntity(const EntityCreateInfo& childInfo);
		void removeEntity(size_t ID);
		void setPosition(glm::vec3 newPosition);
		void setOrientation(glm::quat newOrientation);
		void setTexture(size_t meshIndex, size_t textureIndex, const std::shared_ptr<textures::Texture>& new_texture, bool refresh_meshes = true);
	protected:
		void onRemove();
	};
	struct EntityCreateInfo
	{
		std::string typeName;
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		std::string name;
		std::function<void(Entity&)> preUpdateFunction;
		std::function<bool(Entity&)> preRenderFunction;
		std::function<void(Entity&)> postRenderFunction;
		std::function<void(Entity&)> onAddedFunction;
		std::unordered_map<std::string, std::function<void(Entity&)>> onRemoveFunctionMap;
		DataStorage<Entity>::DataMap dataMap;
		DataStorage<Entity>::GetDataFunctionMap getDataFunctionMap;
		DataStorage<Entity>::SetDataFunctionMap setDataFunctionMap;
		std::vector<MeshCreateInfo> meshInfos;
		std::vector<EntityCreateInfo> childrenInfos;
		bool addToBVH = true;
		size_t ID;
		size_t* INDEX;
		std::vector<size_t*> INDEX_STACK;
		int32_t meta_int = -1;
		float meta_float = 0.f;
		glm::vec4 meta_vec4 = glm::vec4(0);
		std::unordered_map<std::string, ValueSetterPair> runtimeConstantValueShaderSetters;
	};
} // namespace zg
