#include <editor/EditorScene.hpp>
#include <editor/CodeScene.hpp>
using namespace anex::editor;
static anex::SharedLibrary gameLibrary("EditorGame.dll");
EditorScene::EditorScene(GLWindow& window):
	GLScene(window, {0, 0, 50}, {0, 0, -1}, {2, 2}),
	toolbarHeight(window.windowHeight / 14),
	bottomTabsHeight(window.windowHeight / 18),
	robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
	robotoRegularFont(window, robotoRegularFile),
	gameWindowBorderWidth(4.f),
	gameWindowWidth(window.windowWidth / 1.75f),
	gameWindowHeight(window.windowHeight - toolbarHeight - bottomTabsHeight),
	gameWindowX(window.windowWidth / 2.f - gameWindowWidth / 2.f),
	gameWindowY(toolbarHeight + gameWindowBorderWidth),
	codeWindowWidth(gameWindowWidth + gameWindowBorderWidth * 2),
	codeWindowHeight(gameWindowHeight + gameWindowBorderWidth * 2),
	codeWindowX(gameWindowX - gameWindowBorderWidth),
	codeWindowY(gameWindowY - gameWindowBorderWidth),
	toolbarColor(35 / 255.f, 33 / 255.f, 36 / 255.f, 1),
	toolbar(std::make_shared<entities::Toolbar>(
		window,
		*this,
		glm::vec3(-1, 1, 0),
		glm::vec3(0, 0, 0),
		glm::vec3(1, 1, 1),
		toolbarColor,
		toolbarHeight,
		robotoRegularFont
	)),
	bottomTabsBar(std::make_shared<entities::TabsBar>(
		window,
		*this,
		glm::vec3(-1 + ((gameWindowX - gameWindowBorderWidth) / window.windowWidth / 0.5), ((bottomTabsHeight - gameWindowBorderWidth * 2) / window.windowHeight / 0.5) - 1, 0),
		glm::vec3(0),
		glm::vec3(1),
		toolbarColor,
		robotoRegularFont,
		gameWindowWidth + gameWindowBorderWidth * 2,
		bottomTabsHeight - gameWindowBorderWidth * 2)),
	gameWindowBorder(std::make_shared<entities::Plane>(
		window, *this,
		glm::vec3(-1 + ((gameWindowX + (gameWindowWidth / 2)) / window.windowWidth / 0.5),
							1 - ((gameWindowY + (gameWindowHeight / 2)) / window.windowHeight / 0.5), 0.1), glm::vec3(0),
		glm::vec3(1),
		glm::vec2((gameWindowWidth + gameWindowBorderWidth * 2) / (window.windowWidth / 2),
							(gameWindowHeight + gameWindowBorderWidth * 2) / (window.windowHeight / 2)),
		gameWindowBorderColor)),
	sceneGraphPanelMenu(std::make_shared<entities::PanelMenu>(
		window, *this,
		glm::vec3(-1, 1.f - (toolbarHeight / window.windowHeight / 0.5), 0),
		glm::vec3(0), glm::vec3(1), glm::vec4(0.5, 0.5, 0.5, 1),
		robotoRegularFont, "Scene Graph",
		gameWindowX - gameWindowBorderWidth,
		gameWindowHeight + gameWindowBorderWidth * 2.f)),
	dialogWidth(window.windowWidth / 3),
	dialogHeight(window.windowHeight / 5),
	closeDialogButtonWidth(window.windowWidth / 44.f),
	closeDialogButtonHeight(window.windowWidth / 44.f),
	okayDialogButtonWidth(dialogWidth / 6.f),
	okayDialogButtonHeight(dialogHeight / 4.5f),
	closeDialogButton(std::make_shared<entities::Button>(
		window,
		*this,
		glm::vec3(
			(dialogWidth - closeDialogButtonWidth) / window.windowWidth / 0.5,
			0,
			0.1),
		glm::vec3(0),
    	glm::vec3(1),
		glm::vec4(1, 0, 0, 1),
		glm::vec2(closeDialogButtonWidth, closeDialogButtonHeight),
		"x",
		robotoRegularFont,
    	[](){}
	)),
	okayDialogButton(std::make_shared<entities::Button>(
		window,
		*this,
		glm::vec3(
			(dialogWidth - okayDialogButtonWidth) / window.windowWidth / 0.5,
			(-dialogHeight + okayDialogButtonHeight) / window.windowHeight / 0.5,
			0.1),
		glm::vec3(0),
    	glm::vec3(1),
		glm::vec4(0.2f, 0, 0.8f, 1),
		glm::vec2(okayDialogButtonWidth, okayDialogButtonHeight),
		"Okay",
		robotoRegularFont,
    	[](){}
	)),
	projectNameInput(std::make_shared<entities::Input>(
		window,
		*this,
		glm::vec3(16 / window.windowWidth / 0.5, -30 / window.windowHeight / 0.5, 0),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(0.5, 0.5, 0.5, 1),
		robotoRegularFont,
		dialogWidth - 32,
	    dialogHeight / 4.5f,
	    "Project Name",
		128.f
	)),
	projectDirectoryInput(std::make_shared<entities::Input>(
		window,
		*this,
		glm::vec3(16 / window.windowWidth / 0.5, -((dialogHeight / 4.5f) * 2) / window.windowHeight / 0.5, 0),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(0.5, 0.5, 0.5, 1),
		robotoRegularFont,
		dialogWidth - 32,
	    dialogHeight / 4.5f,
	    "Project Directory",
		128.f
	)),
	newProjectDialog(std::make_shared<entities::Dialog>(
		window,
		*this,
		glm::vec3(0, 0, 5.0),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(0.6, 0.6, 0.6, 1),
		robotoRegularFont,
		"New Project",
		dialogWidth,
    	dialogHeight,
		std::vector<std::shared_ptr<GLEntity>>({closeDialogButton, okayDialogButton, projectNameInput, projectDirectoryInput})
  	))
{
	closeDialogButton->handler = [&]()
	{
		removeEntity(activeDialog->ID);
	};
	okayDialogButton->handler = [&]()
	{
		removeEntity(activeDialog->ID);
		project = {projectNameInput->text, projectDirectoryInput->text};
		loadProject();
	};
	clearColor = editorClearColor;
	sceneGraphPanelMenu->addToBVH = false;
	addEntity(sceneGraphPanelMenu);
	setupToolbarOptions();
	addEntity(toolbar);
	bottomTabsBar->addToBVH = false;
	addEntity(bottomTabsBar);
	addEntity(gameWindowBorder);
	setupGameWindow();
	setupCodeWindow();
	resizeID = view.addResizeHandler([&](auto newSize)mutable
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
	bottomTabsBar->addTab("Scene", [&]()
	{
		minimizeWindows();
		addEntity(gameWindowBorder);
		gameWindowPointer->restore();
	}, true);
	bottomTabsBar->addTab("Code Editor", [&]()
	{
		minimizeWindows();
		codeWindowPointer->restore();
	});
};
EditorScene::~EditorScene()
{
	view.removeResizeHandler(resizeID);
	gameWindowBorder->removeMouseHoverHandler(gameWindowBorderHoverID);
	gameWindowBorder->removeMousePressHandler(0, gameWindowBorderPressID);
};
void EditorScene::onEntityAdded(const std::shared_ptr<IEntity>& entity) const
{
	auto& glEntity = *std::dynamic_pointer_cast<GLEntity>(entity);
	sceneGraphPanelMenu->addItem(glEntity.name, glEntity);
};
void EditorScene::setupGameWindow()
{
	auto& gameWindow = window.createChildWindow(
		"EditorChild",
		*this,
		gameWindowWidth,
		gameWindowHeight,
		gameWindowX,
		gameWindowY,
		true);
  	gameWindowPointer = (GLWindow *)&gameWindow;
//	gameWindow.runOnThread([](auto &runningGameWindow)
//	{
//		auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
//		load((GLWindow&)runningGameWindow);
//	});
	std::function<void(const std::shared_ptr<IEntity>&)> entityAddedFunction = std::bind(
		&EditorScene::onEntityAdded, this, std::placeholders::_1);
	gameWindow.registerOnEntityAddedFunction(entityAddedFunction);
	gameWindowBorderHoverID = gameWindowBorder->addMouseHoverHandler([&](const auto& hovered)
	{
		if (hovered)
			gameWindowBorder->setColor(gameWindowHoveredBorderColor);
		else
			gameWindowBorder->setColor(gameWindowBorderColor);
	});
	gameWindowBorderPressID = gameWindowBorder->addMousePressHandler(0, [&](const auto& pressed)
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
	gameWindowESCPressID = gameWindowPointer->addKeyPressHandler(27, [&](const auto& pressed)
	{
		if (!pressed)
		{
			gameWindowPointer->focused = false;
			auto &glWindow = (GLWindow&)window;
			window.callMouseMoveHandler(glWindow.mouseCoords);
		}
	});
};
void EditorScene::setupCodeWindow()
{
	auto& codeWindow = window.createChildWindow(
		"Code Editor",
		*this,
		codeWindowWidth,
		codeWindowHeight,
		codeWindowX,
		codeWindowY,
		true);
  codeWindowPointer = (GLWindow *)&codeWindow;
	codeWindow.minimize();
	codeWindow.setIScene(std::make_shared<CodeScene>((GLWindow &)codeWindow));
};
void EditorScene::minimizeWindows()
{
  auto& glWindow = (GLWindow&)window;
	for (auto &windowPointer : glWindow.childWindows)
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
};
void EditorScene::setupToolbarOptions()
{
	auto &fileDropdown = *toolbar->fileDropdown;
	fileDropdown.addOption("New Project", [&]()
	{
		addEntity(newProjectDialog);
		activeDialog = newProjectDialog;
		bvh.removeEntity(*this, *toolbar->fileDropdown);
		toolbar->file->removeChild(toolbar->fileDropdownID);
	});
	fileDropdown.addOption("Open Project", []()
	{

	});
	fileDropdown.addOption("Save", []()
	{

	});
	fileDropdown.addOption("Save As", []()
	{

	});
	fileDropdown.addOption("Import Assets", []()
	{

	});
	fileDropdown.addOption("Export", []()
	{

	});
	fileDropdown.addOption("Recent Files", []()
	{

	});
	fileDropdown.addOption("Settings", []()
	{

	});
	fileDropdown.addOption("Exit", [&]()
	{
		window.close();
	});
}
void EditorScene::loadProject()
{
	std::cout << "Loading project: " << project.name << ", in directory: " << project.directory << std::endl;
};