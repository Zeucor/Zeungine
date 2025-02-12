#include <zg/editor/CodeScene.hpp>
#include <zg/editor/EditorScene.hpp>
#include <zg/filesystem/Directory.hpp>
#include <zg/strings/InFileProcessor.hpp>
using namespace zg::editor;
using namespace zg::strings;
EditorScene::EditorScene(Window& window) :
		Scene(window, {0, 0, 50}, {0, 0, -1}, {2, 2}), toolbarHeight(window.windowHeight / 14),
		bottomTabsHeight(window.windowHeight / 18),
		robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
		robotoRegularFont(window, robotoRegularFile), gameWindowBorderWidth(4.f),
		gameWindowWidth(window.windowWidth / 1.75f),
		gameWindowHeight(window.windowHeight / 1.75f), //- toolbarHeight - bottomTabsHeight),
		gameWindowX(window.windowWidth / 2.f - gameWindowWidth / 2.f), gameWindowY(toolbarHeight + gameWindowBorderWidth),
		codeWindowWidth(gameWindowWidth + gameWindowBorderWidth * 2),
		codeWindowHeight(gameWindowHeight + gameWindowBorderWidth * 2), codeWindowX(gameWindowX - gameWindowBorderWidth),
		codeWindowY(gameWindowY - gameWindowBorderWidth), toolbarColor(35 / 255.f, 33 / 255.f, 36 / 255.f, 1),
		toolbar(std::make_shared<entities::Toolbar>(window, *this, glm::vec3(-1, 1, 0), glm::vec3(0, 0, 0),
																								glm::vec3(1, 1, 1), toolbarColor, toolbarHeight, robotoRegularFont)),
		bottomTabsBar(std::make_shared<entities::TabsBar>(
			window, *this,
			glm::vec3(-1 + ((gameWindowX - gameWindowBorderWidth) / window.windowWidth / 0.5),
								((bottomTabsHeight - gameWindowBorderWidth * 2) / window.windowHeight / 0.5) - 1, 0),
			glm::vec3(0), glm::vec3(1), toolbarColor, robotoRegularFont, gameWindowWidth + gameWindowBorderWidth * 2,
			bottomTabsHeight - gameWindowBorderWidth * 2)),
		status(std::make_shared<entities::StatusText>(
			window, *this,
			glm::vec3(-1, ((bottomTabsHeight - gameWindowBorderWidth * 2) / window.windowHeight / 0.5) - 1, 0.1),
			glm::vec3(0), glm::vec3(1), toolbarColor, robotoRegularFont, (window.windowWidth - gameWindowWidth - gameWindowBorderWidth) / 2,
			bottomTabsHeight - gameWindowBorderWidth * 2, "Idle")),
		gameWindowBorder(std::make_shared<entities::Plane>(
			window, *this,
			glm::vec3(-1 + ((gameWindowX + (gameWindowWidth / 2)) / window.windowWidth / 0.5),
								1 - ((gameWindowY + (gameWindowHeight / 2)) / window.windowHeight / 0.5), 0.1),
			glm::vec3(0), glm::vec3(1),
			glm::vec2((gameWindowWidth + gameWindowBorderWidth * 2) / (window.windowWidth / 2),
								(gameWindowHeight + gameWindowBorderWidth * 2) / (window.windowHeight / 2)),
			gameWindowBorderColor)),
		sceneGraphPanelMenu(std::make_shared<entities::PanelMenu>(
			window, *this, glm::vec3(-1, 1.f - (toolbarHeight / window.windowHeight / 0.5), 0), glm::vec3(0), glm::vec3(1),
			glm::vec4(0.5, 0.5, 0.5, 1), robotoRegularFont, "Scene Graph", gameWindowX - gameWindowBorderWidth,
			gameWindowHeight + gameWindowBorderWidth * 2.f)),
		resourcePanelMenuHeight(window.windowHeight - ((toolbarHeight + gameWindowHeight + bottomTabsHeight))),
		resourcePanelMenu(std::make_shared<entities::PanelMenu>(
			window, *this,
			glm::vec3(-1, 1 - ((toolbarHeight + gameWindowHeight + gameWindowBorderWidth * 2) / window.windowHeight / 0.5),
								0.1),
			glm::vec3(0), glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1), robotoRegularFont, "", window.windowWidth,
			resourcePanelMenuHeight)),
		resourceConsole(std::make_shared<entities::Console>(
			window, *this, glm::vec3(0, 0, 0.1), glm::vec3(0), glm::vec3(1), toolbarColor, robotoRegularFont,
			window.windowWidth, resourcePanelMenuHeight - bottomTabsHeight + gameWindowBorderWidth * 2)),
		resourcePanelTabs(std::make_shared<entities::TabsBar>(
			window, *this,
			glm::vec3(
				0, (-resourcePanelMenuHeight + bottomTabsHeight - gameWindowBorderWidth * 2) / window.windowHeight / 0.5, 0.1),
			glm::vec3(0), glm::vec3(1), toolbarColor, robotoRegularFont, window.windowWidth,
			bottomTabsHeight - gameWindowBorderWidth * 2)),
		dialogWidth(window.windowWidth / 3), dialogHeight(window.windowHeight / 5),
		closeNewDialogButtonWidth(window.windowWidth / 44.f), closeNewDialogButtonHeight(window.windowWidth / 44.f),
		okayNewDialogButtonWidth(dialogWidth / 6.f), okayNewDialogButtonHeight(dialogHeight / 4.5f),
		closeOpenDialogButtonWidth(closeNewDialogButtonWidth), closeOpenDialogButtonHeight(closeNewDialogButtonHeight),
		okayOpenDialogButtonWidth(okayNewDialogButtonWidth), okayOpenDialogButtonHeight(okayNewDialogButtonHeight),
		closeNewDialogButton(std::make_shared<entities::Button>(
			window, *this, glm::vec3((dialogWidth - closeNewDialogButtonWidth) / window.windowWidth / 0.5, 0, 0.1), glm::vec3(0),
			glm::vec3(1), glm::vec4(1, 0, 0, 1), glm::vec2(closeNewDialogButtonWidth, closeNewDialogButtonHeight), "x",
			robotoRegularFont, []() {})),
		okayNewDialogButton(std::make_shared<entities::Button>(
			window, *this,
			glm::vec3((dialogWidth - okayNewDialogButtonWidth) / window.windowWidth / 0.5,
								(-dialogHeight + okayNewDialogButtonHeight) / window.windowHeight / 0.5, 0.1),
			glm::vec3(0), glm::vec3(1), glm::vec4(0.2f, 0, 0.8f, 1), glm::vec2(okayNewDialogButtonWidth, okayNewDialogButtonHeight),
			"Okay", robotoRegularFont, []() {})),
		newProjectNameInput(std::make_shared<entities::Input>(
			window, *this, glm::vec3(16 / window.windowWidth / 0.5, -30 / window.windowHeight / 0.5, 0.1), glm::vec3(0),
			glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1), robotoRegularFont, dialogWidth - 32, dialogHeight / 4.5f,
			"Project Name", 128.f)),
		newProjectDirectoryInput(std::make_shared<entities::Input>(
			window, *this,
			glm::vec3(16 / window.windowWidth / 0.5, -((dialogHeight / 4.5f) * 2) / window.windowHeight / 0.5, 0.1),
			glm::vec3(0), glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1), robotoRegularFont, dialogWidth - 32, dialogHeight / 4.5f,
			"Project Directory", 128.f)),
		newProjectDialog(std::make_shared<entities::Dialog>(
			window, *this, glm::vec3(0, 0, 5.0), glm::vec3(0), glm::vec3(1), glm::vec4(0.6, 0.6, 0.6, 1), robotoRegularFont,
			"New Project", dialogWidth, dialogHeight,
			std::vector<std::shared_ptr<Entity>>(
				{closeNewDialogButton, okayNewDialogButton, newProjectNameInput, newProjectDirectoryInput}))),
		closeOpenDialogButton(std::make_shared<entities::Button>(
			window, *this, glm::vec3((dialogWidth - closeOpenDialogButtonWidth) / window.windowWidth / 0.5, 0, 0.1), glm::vec3(0),
			glm::vec3(1), glm::vec4(1, 0, 0, 1), glm::vec2(closeOpenDialogButtonWidth, closeOpenDialogButtonHeight), "x",
			robotoRegularFont, []() {})),
		okayOpenDialogButton(std::make_shared<entities::Button>(
			window, *this,
			glm::vec3((dialogWidth - okayOpenDialogButtonWidth) / window.windowWidth / 0.5,
								(-dialogHeight + okayOpenDialogButtonHeight) / window.windowHeight / 0.5, 0.1),
			glm::vec3(0), glm::vec3(1), glm::vec4(0.2f, 0, 0.8f, 1), glm::vec2(okayOpenDialogButtonWidth, okayOpenDialogButtonHeight),
			"Okay", robotoRegularFont, []() {})),
		openProjectDirectoryInput(std::make_shared<entities::Input>(
			window, *this,
			glm::vec3(16 / window.windowWidth / 0.5, -30 / window.windowHeight / 0.5, 0.1),
			glm::vec3(0), glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1), robotoRegularFont, dialogWidth - 32, dialogHeight / 4.5f,
			"Project Directory", 128.f)),
		openProjectDialog(std::make_shared<entities::Dialog>(
			window, *this, glm::vec3(0, 0, 5.0), glm::vec3(0),
			glm::vec3(1), glm::vec4(0.6, 0.6, 0.6, 1), robotoRegularFont, "Open Project", dialogWidth, dialogHeight,
			std::vector<std::shared_ptr<Entity>>({closeOpenDialogButton, okayOpenDialogButton, openProjectDirectoryInput})))
{
	(*newProjectNameInput->textPointer) = "EditorGame";
#ifdef _WIN32
	(*newProjectDirectoryInput->textPointer) = "C:/Users/Steven/Projects/EditorGame";
	(*openProjectDirectoryInput->textPointer) = "C:/Users/Steven/Projects/EditorGame";
#elif defined(__linux__)
	(*newProjectDirectoryInput->textPointer) = "/home/zeun/Projects/EditorGame";
	(*openProjectDirectoryInput->textPointer) = "/home/zeun/Projects/EditorGame";
#endif
	// newProjectNameInput->handleKey(0, true);
	// newProjectDirectoryInput->handleKey(0, true);
	closeNewDialogButton->handler = [&]()
	{
		removeEntity(activeDialog->ID);
		newProjectNameInput->clear();
		newProjectDirectoryInput->clear();
	};
	okayNewDialogButton->handler = [&]()
	{
		removeEntity(activeDialog->ID);
		newProject(*newProjectNameInput->textPointer, *newProjectDirectoryInput->textPointer);
		newProjectNameInput->clear();
		newProjectDirectoryInput->clear();
	};
	closeOpenDialogButton->handler = [&]()
	{
		removeEntity(activeDialog->ID);
		openProjectDirectoryInput->clear();
	};
	okayOpenDialogButton->handler = [&]()
	{
		removeEntity(activeDialog->ID);
		openProject(*openProjectDirectoryInput->textPointer);
		openProjectDirectoryInput->clear();
	};
	clearColor = editorClearColor;
	sceneGraphPanelMenu->addToBVH = false;
	addEntity(sceneGraphPanelMenu);
	addEntity(resourcePanelMenu);
	filesystem::File consoleFile((std::filesystem::path(filesystem::File::getProgramDirectoryPath()) / "icons" / "Remix" / "Development" / "terminal-line.svg").string(), enums::EFileLocation::Absolute, "r");
	resourcePanelTabs->addTab(
		"Console",
		[&]
		{
			removeActiveResourceEntity();
			resourcePanelMenu->addPanelEntity(resourceConsole, false);
			activeResourcePanelEntity = std::dynamic_pointer_cast<Entity>(resourceConsole);
		},
		true,
		consoleFile);
	resourcePanelMenu->addPanelEntity(resourceConsole, false);
	activeResourcePanelEntity = std::dynamic_pointer_cast<Entity>(resourceConsole);
	resourcePanelMenu->addPanelEntity(resourcePanelTabs, false);
	setupToolbarOptions();
	addEntity(toolbar);
	bottomTabsBar->addToBVH = false;
	addEntity(bottomTabsBar);
	addEntity(status);
	addEntity(gameWindowBorder);
	setupGameWindow();
	setupCodeWindow();
	resizeID = view.addResizeHandler(
		[&](auto newSize) mutable
		{
			toolbarHeight = newSize.y / 14;
			bottomTabsHeight = newSize.y / 18;
			gameWindowWidth = window.windowWidth / 1.75f;
			gameWindowHeight = window.windowHeight - toolbarHeight - bottomTabsHeight;
			gameWindowX = window.windowWidth / 2.f - gameWindowWidth / 2.f;
			gameWindowY = toolbarHeight + gameWindowBorderWidth;
			gameWindowPointer->setWidthHeight(gameWindowWidth, gameWindowHeight);
			gameWindowPointer->setXY(gameWindowX, gameWindowY);
			codeWindowWidth = gameWindowWidth + gameWindowBorderWidth * 2;
			codeWindowHeight = gameWindowHeight + gameWindowBorderWidth * 2;
			codeWindowX = gameWindowX - gameWindowBorderWidth;
			codeWindowY = gameWindowY - gameWindowBorderWidth;
			codeWindowPointer->setWidthHeight(codeWindowWidth, codeWindowHeight);
			codeWindowPointer->setXY(codeWindowX, codeWindowY);
		});
	filesystem::File sceneFile((std::filesystem::path(filesystem::File::getProgramDirectoryPath()) / "icons" / "Remix" / "Design" / "shapes-line.svg").string(), enums::EFileLocation::Absolute, "r");
	bottomTabsBar->addTab(
		"Scene",
		[&]()
		{
			minimizeWindows();
			addEntity(gameWindowBorder);
			gameWindowPointer->restore();
		},
		true,
		sceneFile);
	filesystem::File codeEditorFile((std::filesystem::path(filesystem::File::getProgramDirectoryPath()) / "icons" / "Remix" / "Development" / "code-line.svg").string(), enums::EFileLocation::Absolute, "r");
	bottomTabsBar->addTab("Code Editor",
												[&]()
												{
													minimizeWindows();
													codeWindowPointer->restore();
												}, false, codeEditorFile);
	std::cout << "Opened ZG Editor" << std::endl;
};
EditorScene::~EditorScene()
{
	view.removeResizeHandler(resizeID);
	gameWindowBorder->removeMouseHoverHandler(gameWindowBorderHoverID);
	gameWindowBorder->removeMousePressHandler(0, gameWindowBorderPressID);
};
void EditorScene::onEntityAdded(const std::shared_ptr<Entity>& entity)
{
	auto& glEntity = *std::dynamic_pointer_cast<Entity>(entity);
	auto sizeYTotal = sceneGraphPanelMenu->getSizeYTotal();
	static const auto indent = 16.f;
	auto panelItem = std::make_shared<entities::PanelItem>(
		window, *this, glm::vec3(indent / window.windowWidth / 0.5, -sizeYTotal, 0.1), glm::vec3(0), glm::vec3(1),
		sceneGraphPanelMenu->color, glEntity.name, robotoRegularFont, glEntity, sceneGraphPanelMenu->width, indent);
	sceneGraphPanelMenu->addPanelEntity(panelItem);
};
void EditorScene::setupGameWindow()
{
	auto& gameWindow =
		window.createChildWindow("EditorChild", *this, gameWindowWidth, gameWindowHeight, gameWindowX, gameWindowY, true);
	gameWindowPointer = &gameWindow;
	// std::function<void(const std::shared_ptr<Entity>&)> entityAddedFunction =
	// 	std::bind(&EditorScene::onEntityAdded, this, std::placeholders::_1);
	// gameWindow.registerOnEntityAddedFunction(entityAddedFunction);
	gameWindowBorderHoverID = gameWindowBorder->addMouseHoverHandler(
		[&](const auto& hovered)
		{
			if (hovered)
				gameWindowBorder->setColor(gameWindowHoveredBorderColor);
			else
				gameWindowBorder->setColor(gameWindowBorderColor);
		});
	gameWindowBorderPressID =
		gameWindowBorder->addMousePressHandler(0,
																					 [&](const auto& pressed)
																					 {
																						 if (pressed)
																						 {
																							 gameWindowPointer->focused = true;
																							 if (currentHoveredEntity)
																							 {
																								 currentHoveredEntity = 0;
																							 }
																							 gameWindowBorder->setColor(gameWindowActiveBorderColor);
																						 }
																					 });
	gameWindowESCPressID = gameWindowPointer->addKeyPressHandler(27,
																															 [&](const auto& pressed)
																															 {
																																 if (!pressed)
																																 {
																																	 gameWindowPointer->focused = false;
																																	 window.callMouseMoveHandler(window.mouseCoords);
																																 }
																															 });
};
void EditorScene::setupCodeWindow()
{
	auto& codeWindow =
		window.createChildWindow("Code Editor", *this, codeWindowWidth, codeWindowHeight, codeWindowX, codeWindowY, true);
	codeWindowPointer = &codeWindow;
	codeWindow.minimize();
	codeWindow.setScene(std::make_shared<CodeScene>(codeWindow));
};
void EditorScene::minimizeWindows()
{
	for (auto& windowPointer : window.childWindows)
	{
		if (!windowPointer->minimized)
		{
			windowPointer->minimize();
			if (windowPointer == gameWindowPointer)
			{
				removeEntity(gameWindowBorder->ID);
			}
		}
	}
}
void EditorScene::removeActiveResourceEntity() const
{
	if (activeResourcePanelEntity && activeResourcePanelEntity->ID)
	{
		resourcePanelMenu->removePanelEntity(activeResourcePanelEntity);
	}
}
void EditorScene::setupToolbarOptions()
{
	auto& fileDropdown = *toolbar->fileDropdown;
	fileDropdown.addOption("New Project",
												 [&]()
												 {
													 addEntity(newProjectDialog);
													 activeDialog = newProjectDialog;
													 bvh->removeEntity(*this, *toolbar->fileDropdown);
													 toolbar->file->removeChild(toolbar->fileDropdownID);
												 });
	fileDropdown.addOption("Open Project",
												 [&]()
												 {
													 addEntity(openProjectDialog);
													 activeDialog = openProjectDialog;
													 bvh->removeEntity(*this, *toolbar->fileDropdown);
													 toolbar->file->removeChild(toolbar->fileDropdownID);
												 });
	fileDropdown.addOption("Save", []() {

	});
	fileDropdown.addOption("Save As", []() {

	});
	fileDropdown.addOption("Import Assets", []() {

	});
	fileDropdown.addOption("Export", []() {

	});
	fileDropdown.addOption("Recent Files", []() {

	});
	fileDropdown.addOption("Settings", []() {

	});
	fileDropdown.addOption("Exit", [&]() { window.close(); });
}
void EditorScene::newProject(std::string_view projectName, std::string_view projectDirectory)
{
	{
		project = {projectName, projectDirectory};
		filesystem::Directory::ensureExists(project.directory);
		std::string includePath = filesystem::File::toPlatformPath(std::string(project.directory) + "/include");
		filesystem::Directory::ensureExists(includePath);
		std::string srcPath = filesystem::File::toPlatformPath(std::string(project.directory) + "/src");
		filesystem::Directory::ensureExists(srcPath);
		std::string cmakePath = filesystem::File::toPlatformPath(std::string(project.directory) + "/cmake");
		filesystem::Directory::ensureExists(cmakePath);
		std::string zgIncInstallPrefix(TO_STRING(ZG_INC_INSTALL_PREFIX));
		InFileProcessor processor;
		std::string stringProjectName(projectName);
		auto libraryName = InFileProcessor::toKebabCase(stringProjectName);
		processor.addVariableMapping("PROJECT_NAME", stringProjectName);
		processor.addVariableMapping("LIBRARY_NAME", libraryName);
		processor.processFile({zgIncInstallPrefix + "/zg/editor/projects/templates/main.in.hpp", enums::EFileLocation::Absolute, "r"}, includePath + "/main.hpp");
		processor.processFile({zgIncInstallPrefix + "/zg/editor/projects/templates/main.in.cpp", enums::EFileLocation::Absolute, "r"}, srcPath + "/main.cpp");
		processor.processFile({zgIncInstallPrefix + "/zg/editor/projects/templates/CMakeLists.in.txt", enums::EFileLocation::Absolute, "r"}, std::string(project.directory) + "/CMakeLists.txt");
		processor.processFile({zgIncInstallPrefix + "/zg/editor/projects/templates/Zeungine.in.cmake", enums::EFileLocation::Absolute, "r"}, cmakePath + "/Zeungine.cmake");
	}
	openProject(projectDirectory);
};
void EditorScene::openProject(std::string_view projectDirectory)
{
	hotswapper = std::make_shared<hs::Hotswapper>(projectDirectory, *this);
	resourceAssetBrowser = std::make_shared<entities::AssetBrowser>(
		window, *this, glm::vec3(0, 0, 0.1), glm::vec3(0), glm::vec3(1), glm::vec4(0.1, 0.1, 0.1, 1), robotoRegularFont,
		window.windowWidth, resourcePanelMenuHeight - bottomTabsHeight, projectDirectory);
	assetTabID = resourcePanelTabs->addTab("Asset Browser",
																				 [&]
																				 {
																					 removeActiveResourceEntity();
																					 if (resourceAssetBrowser)
																					 {
																						 resourcePanelMenu->addPanelEntity(resourceAssetBrowser, false);
																						 activeResourcePanelEntity =
																							 std::dynamic_pointer_cast<Entity>(resourceAssetBrowser);
																					 }
																				 });
	performanceTabID = resourcePanelTabs->addTab("Performance", [&] { removeActiveResourceEntity(); });
};
