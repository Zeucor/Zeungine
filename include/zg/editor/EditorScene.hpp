#pragma once
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/AssetBrowser.hpp>
#include <zg/entities/Button.hpp>
#include <zg/entities/Console.hpp>
#include <zg/entities/Dialog.hpp>
#include <zg/entities/Input.hpp>
#include <zg/entities/Panel.hpp>
#include <zg/entities/StatusText.hpp>
#include <zg/entities/Tabs.hpp>
#include <zg/entities/Toolbar.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include "Hotswapper.hpp"
#include "projects/Project.hpp"
using namespace std;
namespace zg::editor
{
	using namespace zg;
	struct EditorScene : Scene
	{
		glm::vec4 editorClearColor = {0.2, 0.2, 0.2, 1};
		float toolbarHeight;
		float bottomTabsHeight;
		zgfilesystem::File robotoRegularFile;
		fonts::freetype::FreetypeFont robotoRegularFont;
		Window* gameWindowPointer = nullptr;
		Window* codeWindowPointer = nullptr;
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
		shared_ptr<entities::Toolbar> toolbar;
		shared_ptr<entities::TabsBar> bottomTabsBar;
		shared_ptr<entities::StatusText> status;
		glm::vec4 gameWindowBorderColor = {0.4, 0.4, 0.7, 1};
		glm::vec4 gameWindowHoveredBorderColor = {0.7, 0.4, 0.4, 1};
		glm::vec4 gameWindowActiveBorderColor = {1, 0, 0, 1};
		shared_ptr<entities::Plane> gameWindowBorder;
		shared_ptr<entities::PanelMenu> sceneGraphPanelMenu;
		float resourcePanelMenuHeight;
		shared_ptr<entities::PanelMenu> resourcePanelMenu;
		shared_ptr<entities::Console> resourceConsole;
		shared_ptr<entities::AssetBrowser> resourceAssetBrowser;
		size_t assetTabID = 0;
		shared_ptr<entities::TabsBar> resourcePanelTabs;
		size_t performanceTabID = 0;
		shared_ptr<Entity> activeResourcePanelEntity;
		EventIdentifier resizeID = 0;
		EventIdentifier gameWindowBorderHoverID = 0;
		EventIdentifier gameWindowBorderPressID = 0;
		EventIdentifier gameWindowESCPressID = 0;
		float dialogWidth;
		float dialogHeight;
		float closeNewDialogButtonWidth;
		float closeNewDialogButtonHeight;
		float okayNewDialogButtonWidth;
		float okayNewDialogButtonHeight;
		float closeOpenDialogButtonWidth;
		float closeOpenDialogButtonHeight;
		float okayOpenDialogButtonWidth;
		float okayOpenDialogButtonHeight;
		shared_ptr<entities::Button> closeNewDialogButton;
		shared_ptr<entities::Button> okayNewDialogButton;
		shared_ptr<entities::Input> newProjectNameInput;
		shared_ptr<entities::Input> newProjectDirectoryInput;
		shared_ptr<entities::Dialog> newProjectDialog;
		shared_ptr<entities::Button> closeOpenDialogButton;
		shared_ptr<entities::Button> okayOpenDialogButton;
		shared_ptr<entities::Input> openProjectDirectoryInput;
		shared_ptr<entities::Dialog> openProjectDialog;
		shared_ptr<entities::Dialog> activeDialog;
		shared_ptr<hs::Hotswapper> hotswapper;
		Project project;
		void(*OnLoad)(Window&) = 0;
		bool loaded = false;
		inline static filesystem::path programDirectoryPath = zgfilesystem::File::getProgramDirectoryPath();
		explicit EditorScene(Window& window);
		~EditorScene() override;
		void onEntityAdded(const shared_ptr<Entity>& entity);
		void setupGameWindow();
		void setupCodeWindow();
		void minimizeWindows();
		void removeActiveResourceEntity() const;
		void setupToolbarOptions();
		void newProject(string_view projectName, string_view projectDirectory);
		void openProject(string_view projectDirectory);
	};
} // namespace zg::editor
