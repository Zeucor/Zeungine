#pragma once
#include <zg/Window.hpp>
#include <zg/interfaces/IEntity.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/vaos/VAO.hpp>
namespace zg
{
	struct Entity : zg::IEntity, vaos::VAO
	{
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> positions;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 model;
		shaders::Shader& shader;
		bool affectedByShadows = true;
		size_t childrenCount = 0;
		std::map<size_t, std::shared_ptr<Entity>> children;
		Entity* parentEntity = 0;
		bool addToBVH = true;
		std::unordered_map<IWindow::Button, int> buttons;
		std::unordered_map<
			IWindow::Button,
			std::pair<IWindow::EventIdentifier, std::map<IWindow::EventIdentifier, IWindow::MousePressHandler>>>
			mousePressHandlers;
		std::pair<IWindow::EventIdentifier, std::map<IWindow::EventIdentifier, IWindow::MouseMoveHandler>>
			mouseMoveHandlers;
		using MouseHoverHandler = std::function<void(bool)>;
		std::pair<IWindow::EventIdentifier, std::map<IWindow::EventIdentifier, MouseHoverHandler>> mouseHoverHandlers;
		std::string name;
		Entity(Window& _window, const shaders::RuntimeConstants& constants, uint32_t indiceCount,
					 const std::vector<uint32_t>& indices, uint32_t elementCount, const std::vector<glm::vec3>& positions,
					 glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string_view name);
		virtual void update() override;
		virtual void preRender();
		void render() override;
		virtual void postRender();
		const glm::mat4& getModelMatrix();
		size_t addChild(const std::shared_ptr<Entity>& child);
		void removeChild(size_t& ID);
		IWindow::EventIdentifier addMousePressHandler(const IWindow::Button& button,
																									const IWindow::MousePressHandler& callback);
		void removeMousePressHandler(const IWindow::Button& button, IWindow::EventIdentifier& id);
		IWindow::EventIdentifier addMouseMoveHandler(const IWindow::MouseMoveHandler& callback);
		void removeMouseMoveHandler(IWindow::EventIdentifier& id);
		IWindow::EventIdentifier addMouseHoverHandler(const MouseHoverHandler& callback);
		void removeMouseHoverHandler(IWindow::EventIdentifier& id);
		void callMousePressHandler(const IWindow::Button& button, int pressed);
		void callMouseMoveHandler(glm::vec2 coords);
		void callMouseHoverHandler(bool hovered);
	};
} // namespace zg
