#pragma once
#include "./ComponentHolder.hpp"
#include "DataStorage.hpp"
#include "components/entities/EntityComponent.hpp"
#include "entities/TypeID.hpp"
#include "renderers/GLRenderer.hpp"
#include "shaders/Shader.hpp"
#include "vaos/VAO.hpp"
#include "vp/Projection.hpp"
#include "vp/View.hpp"
#include "KeyIDVector.hpp"
#include "Serial.hpp"
namespace zg
{
	struct Scene;
	inline static std::mutex EntityTypeMutex = std::mutex();
	inline static zg::UniqueIdentifier EntityType = 0;
	struct EntityCreateInfo;
	struct Window;
	struct Entity
			: vaos::VAO,
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
		Window& window;
		Scene& scene;
		size_t ID = 0;
		size_t VALUE = 0;
		std::string typeName;
		std::string name;
		Entity* parentEntity = 0;
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> uv2s;
		std::vector<glm::vec3> uv3s;
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		glm::mat4 model;
		std::shared_ptr<vp::Projection> projectionPointer;
		std::shared_ptr<vp::View> viewPointer;
		std::unordered_map<void*, shaders::Shader*> shaders;
		std::unordered_map<void*, bool> ensuredBools;
		bool affectedByShadows = true;
		KeyIDVector<std::string, Entity> children;
		bool addToBVH = true;
		std::unordered_map<Button, int> buttons;
		std::unordered_map<Button, std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MousePressHandler>>>
			mousePressHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseMoveHandler>> mouseMoveHandlers;
		using MouseHoverHandler = std::function<void(bool)>;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseHoverHandler>> mouseHoverHandlers;
		size_t updateNonce;
		std::function<void(Entity&)> preUpdateFunction;
		std::function<bool(Entity&)> preRenderFunction;
		std::function<void(Entity&)> postRenderFunction;
		std::recursive_mutex handlersMutex;

	public:
		Entity(const EntityCreateInfo& info);
		~Entity();
		Entity& operator=(const Entity& other);
		void update();
		shaders::Shader* addShader(shaders::Shader* setShader = 0);
		bool isEnsured();
		void setEnsured();
		static void* getShaderData(IRenderer* iRenderer);
		void render();
		glm::mat4& getModelMatrix();
		size_t addChild(const EntityCreateInfo& childInfo);
		void removeChild(size_t& ID);
		UniqueIdentifier addMousePressHandler(const Button& button, const MousePressHandler& callback);
		void removeMousePressHandler(const Button& button, UniqueIdentifier& id);
		UniqueIdentifier addMouseMoveHandler(const MouseMoveHandler& callback);
		void removeMouseMoveHandler(UniqueIdentifier& id);
		UniqueIdentifier addMouseHoverHandler(const MouseHoverHandler& callback);
		void removeMouseHoverHandler(UniqueIdentifier& id);
		void callMousePressHandler(const Button& button, int pressed);
		void callMouseMoveHandler(glm::vec2 coords);
		void callMouseHoverHandler(bool hovered);
		template <typename T>
		void flipUVsY(std::vector<T>& uvs)
		{
			for (auto& uv : uvs)
			{
				uv.y = 1 - uv.y;
			}
		}
		void setPosition(glm::vec3 newPosition);
		void setOrientation(glm::quat newOrientation);
	};
	struct EntityCreateInfo
	{
		std::string typeName;
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		shaders::RuntimeConstants constants;
		std::string name;
		uint32_t indiceCount;
		std::vector<uint32_t> indices;
		uint32_t vertexCount;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> uV2s;
		std::vector<glm::vec3> uV3s;
		std::function<void(Entity&)> preUpdateFunction;
		std::function<bool(Entity&)> preRenderFunction;
		std::function<void(Entity&)> postRenderFunction;
		DataStorage<Entity>::DataMap dataMap;
		DataStorage<Entity>::GetDataFunctionMap getDataFunctionMap;
		DataStorage<Entity>::SetDataFunctionMap setDataFunctionMap;
		Scene* scenePointer = 0;
	};
} // namespace zg
