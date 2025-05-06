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
namespace zg
{
	struct Scene;
	inline static std::mutex EntityTypeMutex = std::mutex();
	inline static zg::UniqueIdentifier EntityType = 0;
	struct EntityCreateInfo;
	struct Window;
	struct Entity :
		ComponentHolder<Entity, components::entities::EntityComponent, components::entities::EntityComponentCreateInfo>,
		DataStorage<Entity>
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
		size_t VALUE = 0;
		std::string typeName;
		std::string name;
		// transform
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		glm::mat4 model;
		// view/projection overrides
		std::shared_ptr<vp::Projection> projectionPointer;
		std::shared_ptr<vp::View> viewPointer;
		// some flags
		bool affectedByShadows = true;
		bool addToBVH = true;
		// event handlers
		std::unordered_map<Button, bool> buttons;
		std::unordered_map<
			Button,
			std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MousePressHandler>>
		> mousePressHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseMoveHandler>> mouseMoveHandlers;
		using MouseHoverHandler = std::function<void(bool)>;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseHoverHandler>> mouseHoverHandlers;
		size_t updateNonce;
		// settable functions
		std::function<void(Entity&)> preUpdateFunction;
		std::function<bool(Entity&)> preRenderFunction;
		std::function<void(Entity&)> postRenderFunction;
		std::function<void(Entity&)> onAddedFunction;
		std::function<void(Entity&)> onRemovedFunction;
		std::recursive_mutex handlersMutex;
		// meshes and children
		std::vector<size_t> meshIDs;
		std::vector<MeshCreateInfo> meshInfos;
		KeyIDVector<std::string, Entity> children;
		bool isTransparent = false;
	public:
		Entity(const EntityCreateInfo& info);
		Entity(const Entity& other);
		~Entity();
		Entity& operator=(const Entity& other);
		void refreshMeshes();
		void update();
		void render();
		void postRender();
		glm::mat4& getModelMatrix();
		KeyIDVector<std::string, Entity>::EmplaceBackTuple addChild(const EntityCreateInfo& childInfo);
		void removeChild(size_t ID);
		UniqueIdentifier addMousePressHandler(const Button& button, const MousePressHandler& callback);
		void removeMousePressHandler(const Button& button, UniqueIdentifier& id);
		UniqueIdentifier addMouseMoveHandler(const MouseMoveHandler& callback);
		void removeMouseMoveHandler(UniqueIdentifier& id);
		UniqueIdentifier addMouseHoverHandler(const MouseHoverHandler& callback);
		void removeMouseHoverHandler(UniqueIdentifier& id);
		void callMousePressHandler(const Button& button, bool pressed);
		void callMouseMoveHandler(glm::vec2 coords);
		void callMouseHoverHandler(bool hovered);
		void setPosition(glm::vec3 newPosition);
		void setOrientation(glm::quat newOrientation);
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
		std::function<void(Entity&)> onRemovedFunction;
		DataStorage<Entity>::DataMap dataMap;
		DataStorage<Entity>::GetDataFunctionMap getDataFunctionMap;
		DataStorage<Entity>::SetDataFunctionMap setDataFunctionMap;
		std::vector<MeshCreateInfo> meshInfos;
		std::vector<EntityCreateInfo> childrenInfos;
		size_t ID;
		size_t* INDEX;
		std::vector<size_t*> INDEX_STACK;
	};
} // namespace zg
