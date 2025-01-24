#pragma once
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/entities/Button.hpp>
#include <anex/modules/gl/entities/Console.hpp>
#include <anex/modules/gl/entities/Dialog.hpp>
#include <anex/modules/gl/entities/Input.hpp>
#include <anex/modules/gl/entities/Panel.hpp>
#include <anex/modules/gl/entities/Status.hpp>
#include <anex/modules/gl/entities/Tabs.hpp>
#include <anex/modules/gl/entities/Toolbar.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include "Hotswapper.hpp"
#include "Project.hpp"
namespace anex::editor
{
	using namespace anex::modules::gl;
	struct EditorScene : GLScene
	{
		glm::vec4 editorClearColor = {0.2, 0.2, 0.2, 1};
		float toolbarHeight;
		float bottomTabsHeight;
		filesystem::File robotoRegularFile;
		modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
		GLWindow* gameWindowPointer = nullptr;
		GLWindow* codeWindowPointer = nullptr;
		float gameWindowBorderWidth;
		float gameWindowWidth;
		float gameWindowHeight;
		float gameWindowX;
		float gameWindowY;
		float codeWindowWidth;
		float codeWindowHeight;
		float codeWindowX;
		float codeWindowY;
		glm::vec4 toolbarColor;
		std::shared_ptr<entities::Toolbar> toolbar;
		std::shared_ptr<entities::TabsBar> bottomTabsBar;
		std::shared_ptr<entities::Status> status;
		glm::vec4 gameWindowBorderColor = {0.4, 0.4, 0.7, 1};
		glm::vec4 gameWindowHoveredBorderColor = {0.7, 0.4, 0.4, 1};
		glm::vec4 gameWindowActiveBorderColor = {1, 0, 0, 1};
		std::shared_ptr<entities::Plane> gameWindowBorder;
		std::shared_ptr<entities::PanelMenu> sceneGraphPanelMenu;
		float resourcePanelMenuHeight;
		std::shared_ptr<entities::PanelMenu> resourcePanelMenu;
		std::shared_ptr<entities::Console> resourceConsole;
		std::shared_ptr<entities::TabsBar> resourcePanelTabs;
		IWindow::EventIdentifier resizeID = 0;
		IWindow::EventIdentifier gameWindowBorderHoverID = 0;
		IWindow::EventIdentifier gameWindowBorderPressID = 0;
		IWindow::EventIdentifier gameWindowESCPressID = 0;
		float dialogWidth;
		float dialogHeight;
		float closeDialogButtonWidth;
		float closeDialogButtonHeight;
		float okayDialogButtonWidth;
		float okayDialogButtonHeight;
		std::shared_ptr<entities::Button> closeDialogButton;
		std::shared_ptr<entities::Button> okayDialogButton;
    std::shared_ptr<entities::Input> projectNameInput;
    std::shared_ptr<entities::Input> projectDirectoryInput;
    std::shared_ptr<entities::Dialog> newProjectDialog;
		std::shared_ptr<entities::Dialog> activeDialog;
		std::shared_ptr<hs::Hotswapper> hotswapper;
		Project project;
		std::function<void(GLWindow &)> OnLoad;
		std::function<void(GLWindow &, hscpp::AllocationResolver &)> OnHotswapLoad;
		std::function<void(GLWindow &)> OnUnLoad;
		bool loaded = false;
		explicit EditorScene(GLWindow& window);
		~EditorScene() override;
		void onEntityAdded(const std::shared_ptr<IEntity>& entity);
		void setupGameWindow();
		void setupCodeWindow();
		void minimizeWindows();
		void setupToolbarOptions();
		void newProject(std::string_view projectName, std::string_view projectDirectory);
		void loadProject(std::string_view projectDirectory);
	};
}