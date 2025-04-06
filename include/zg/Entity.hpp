#pragma once
#include <zg/Window.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/vaos/VAO.hpp>
#include <zg/vp/Projection.hpp>
#include <zg/vp/View.hpp>
#include <zg/entities/TypeID.hpp>
#include "./ComponentHolder.hpp"
namespace zg
{
	struct Scene;
	struct Entity : vaos::VAO, ComponentHolder<zg::interfaces::IEntityComponent>
	{
		friend Scene;
		friend Window;
		using SerializeFunction = std::function<Serial&(Serial&, const std::shared_ptr<Entity>&)>;
		using DeserializeFunction = std::function<Serial&(Serial&, std::shared_ptr<Entity>&)>;
		using SerializeMap = std::unordered_map<size_t, SerializeFunction>;
		using DeserializeMap = std::unordered_map<size_t, DeserializeFunction>;
		static void registerSerialize(size_t ID, const SerializeFunction& function);
		static void registerDeserialize(size_t ID, const DeserializeFunction& function);
		static SerializeFunction getSerialize(size_t ID);
		static DeserializeFunction getDeserialize(size_t ID);
		protected:
		static void cleanupSerialize();
		public:
		Window& window;
		Scene& scene;
		size_t ID = 0;
		virtual size_t getTypeID() = 0;
		size_t VALUE = 0;
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> positions;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 model;
		std::shared_ptr<vp::Projection> projectionPointer;
		std::shared_ptr<vp::View> viewPointer;
		std::unordered_map<void*, shaders::Shader*> shaders;
		std::unordered_map<void*, bool> ensuredBools;
		bool affectedByShadows = true;
		size_t childrenCount = 0;
		std::map<size_t, std::shared_ptr<Entity>> children;
		Entity* parentEntity = 0;
		bool addToBVH = true;
		std::unordered_map<Button, int> buttons;
		std::unordered_map<Button, std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MousePressHandler>>>
			mousePressHandlers;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseMoveHandler>> mouseMoveHandlers;
		using MouseHoverHandler = std::function<void(bool)>;
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, MouseHoverHandler>> mouseHoverHandlers;
		std::string name;
		Entity(Window& _window, Scene& _scene, const shaders::RuntimeConstants& constants, uint32_t indiceCount,
					 const std::vector<uint32_t>& indices, uint32_t elementCount, const std::vector<glm::vec3>& positions,
					 glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string_view name);
		~Entity();
		virtual void update();
		shaders::Shader* addShader(shaders::Shader* setShader = 0);
		bool isEnsured();
		void setEnsured();
		static void* getShaderData(Window& window);
		virtual bool preRender();
		void render();
		virtual void postRender();
		const glm::mat4& getModelMatrix();
		size_t addChild(const std::shared_ptr<Entity>& child);
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
	};
} // namespace zg
