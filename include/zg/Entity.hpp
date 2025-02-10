#pragma once
#include <zg/Window.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/vaos/VAO.hpp>
namespace zg
{
	struct Entity : vaos::VAO
	{
		Window &window;
		size_t ID = 0;
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> positions;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 model;
		shaders::Shader *shader = 0;
		bool affectedByShadows = true;
		size_t childrenCount = 0;
		std::map<size_t, std::shared_ptr<Entity>> children;
		Entity *parentEntity = 0;
		bool addToBVH = true;
		std::unordered_map<Button, int> buttons;
		std::unordered_map<
			Button,
			std::pair<EventIdentifier, std::map<EventIdentifier, MousePressHandler>>>
			mousePressHandlers;
		std::pair<EventIdentifier, std::map<EventIdentifier, MouseMoveHandler>>
			mouseMoveHandlers;
		using MouseHoverHandler = std::function<void(bool)>;
		std::pair<EventIdentifier, std::map<EventIdentifier, MouseHoverHandler>> mouseHoverHandlers;
		std::string name;
		bool addedShader = false;
		bool ensured = false;
		Entity(Window &_window, const shaders::RuntimeConstants &constants, uint32_t indiceCount,
			   const std::vector<uint32_t> &indices, uint32_t elementCount, const std::vector<glm::vec3> &positions,
			   glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string_view name);
		virtual void update();
		void addShader();
		virtual bool preRender();
		void render();
		virtual void postRender();
		const glm::mat4 &getModelMatrix();
		size_t addChild(const std::shared_ptr<Entity> &child);
		void removeChild(size_t &ID);
		EventIdentifier addMousePressHandler(const Button &button,
													  const MousePressHandler &callback);
		void removeMousePressHandler(const Button &button, EventIdentifier &id);
		EventIdentifier addMouseMoveHandler(const MouseMoveHandler &callback);
		void removeMouseMoveHandler(EventIdentifier &id);
		EventIdentifier addMouseHoverHandler(const MouseHoverHandler &callback);
		void removeMouseHoverHandler(EventIdentifier &id);
		void callMousePressHandler(const Button &button, int pressed);
		void callMouseMoveHandler(glm::vec2 coords);
		void callMouseHoverHandler(bool hovered);
		template <typename T>
		void flipUVsY(std::vector<T> &uvs)
		{
			for (auto &uv : uvs)
			{
				uv.y = 1 - uv.y;
			}
		}
	};
} // namespace zg