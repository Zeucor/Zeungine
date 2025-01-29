#include <stdexcept>
#include <zg/interfaces/IWindow.hpp>
#include <zg/Window.hpp>
using namespace zg;
IWindow::IWindow(float _windowWidth,
                 float _windowHeight,
                 float _windowX,
                 float _windowY,
                 bool _borderless,
                 uint32_t _framerate):
  windowWidth(_windowWidth),
  windowHeight(_windowHeight),
  windowX(_windowX),
  windowY(_windowY),
  framerate(_framerate),
  borderless(_borderless)
{};
void IWindow::await() const
{
  windowThread->join();
}
void IWindow::run()
{
  windowThread = std::make_shared<std::thread>(&IWindow::startWindow, this);
}
void IWindow::preRender(){};
void IWindow::render()
{
  preRender();
  updateDeltaTime();
  if (scene)
  {
    scene->update();
    scene->render();
  }
  postRender();
};
void IWindow::postRender(){};
// Keyboard
IWindow::EventIdentifier IWindow::addKeyPressHandler(Key key, const KeyPressHandler& callback)
{
  auto& handlersPair = keyPressHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeKeyPressHandler(Key key, EventIdentifier& id)
{
  auto& handlersPair = keyPressHandlers[key];
  auto handlerIter = handlersPair.second.find(id);
  if (handlerIter == handlersPair.second.end())
  {
    return;
  }
  handlersPair.second.erase(handlerIter);
  id = 0;
};
IWindow::EventIdentifier IWindow::addKeyUpdateHandler(Key key, const KeyUpdateHandler& callback)
{
  auto& handlersPair = keyUpdateHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeKeyUpdateHandler(Key key, EventIdentifier& id)
{
  auto handlersIter = keyUpdateHandlers.find(key);
  if (handlersIter == keyUpdateHandlers.end())
    return;
  auto &handlers = handlersIter->second.second;
  auto handlerIter = handlers.find(id);
  if (handlerIter == handlers.end())
  {
    return;
  }
  handlers.erase(handlerIter);
  id = 0;
};
void IWindow::callKeyPressHandler(Key key, int pressed)
{
  keys[key] = pressed;
  {
    auto handlersIter = keyPressHandlers.find(key);
    if (handlersIter == keyPressHandlers.end())
      return;
    auto& handlersMap = handlersIter->second.second;
    std::vector<KeyPressHandler> handlersCopy;
    for (const auto& pair : handlersMap)
      handlersCopy.push_back(pair.second);
    for (auto& handler : handlersCopy)
    {
      handler(!!pressed);
    }
  }
};
void IWindow::callKeyUpdateHandler(Key key)
{
  auto handlersIter = keyUpdateHandlers.find(key);
  if (handlersIter == keyUpdateHandlers.end())
    return;
  auto& handlersMap = handlersIter->second.second;
  std::vector<KeyUpdateHandler> handlersCopy;
  for (const auto& pair : handlersMap)
    handlersCopy.push_back(pair.second);
  for (auto& handler : handlersCopy)
  {
    handler();
  }
};
IWindow::EventIdentifier IWindow::addAnyKeyPressHandler(const AnyKeyPressHandler& callback)
{
  auto id = ++anyKeyPressHandlers.first;
  anyKeyPressHandlers.second[id] = callback;
  return id;
};
void IWindow::removeAnyKeyPressHandler(EventIdentifier& id)
{
  auto &handlers = anyKeyPressHandlers.second;
  auto handlerIter = handlers.find(id);
  if (handlerIter == handlers.end())
  {
    return;
  }
  handlers.erase(handlerIter);
  id = 0;
};
void IWindow::callAnyKeyPressHandler(Key key, bool pressed)
{
  auto& handlersMap = anyKeyPressHandlers.second;
  std::vector<AnyKeyPressHandler> handlersCopy;
  for (const auto& pair : handlersMap)
    handlersCopy.push_back(pair.second);
  for (auto& handler : handlersCopy)
  {
    handler(key, pressed);
  }
};
// Mouse
IWindow::EventIdentifier IWindow::addMousePressHandler(Button button, const MousePressHandler& callback)
{
  auto& handlersPair = mousePressHandlers[button];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeMousePressHandler(Button button, EventIdentifier& id)
{
  auto& handlersPair = mousePressHandlers[button];
  auto handlerIter = handlersPair.second.find(id);
  if (handlerIter == handlersPair.second.end())
  {
    return;
  }
  handlersPair.second.erase(handlerIter);
  id = 0;
};
IWindow::EventIdentifier IWindow::addMouseMoveHandler(const MouseMoveHandler& callback)
{
  auto id = ++mouseMoveHandlers.first;
  mouseMoveHandlers.second[id] = callback;
  return id;
};
void IWindow::removeMouseMoveHandler(EventIdentifier& id)
{
  auto &handlers = mouseMoveHandlers.second;
  auto handlerIter = handlers.find(id);
  if (handlerIter == handlers.end())
  {
    return;
  }
  handlers.erase(handlerIter);
  id = 0;
};
void IWindow::callMousePressHandler(Button button, int pressed)
{
  buttons[button] = pressed;
  {
    auto handlersIter = mousePressHandlers.find(button);
    if (handlersIter == mousePressHandlers.end())
      return;
    auto& handlersMap = handlersIter->second.second;
    std::vector<MousePressHandler> handlersCopy;
    for (const auto& pair : handlersMap)
      handlersCopy.push_back(pair.second);
    for (auto& handler : handlersCopy)
    {
      handler(!!pressed);
    }
  }
};
void IWindow::callMouseMoveHandler(glm::vec2 coords)
{
  auto& handlersMap = mouseMoveHandlers.second;
  std::vector<MouseMoveHandler> handlersCopy;
  for (const auto& pair : handlersMap)
    handlersCopy.push_back(pair.second);
  for (auto& handler : handlersCopy)
  {
    handler(coords);
  }
};
IWindow::EventIdentifier IWindow::addResizeHandler(const ViewResizeHandler &callback)
{
	auto id = ++viewResizeHandlers.first;
	viewResizeHandlers.second[id] = callback;
	return id;
};
void IWindow::removeResizeHandler(EventIdentifier &id)
{
	auto &handlers = viewResizeHandlers.second;
	auto handlerIter = handlers.find(id);
	if (handlerIter == handlers.end())
	{
		return;
	}
	handlers.erase(handlerIter);
	id = 0;
};
void IWindow::callResizeHandler(glm::vec2 newSize)
{
	auto& handlersMap = viewResizeHandlers.second;
	std::vector<ViewResizeHandler> handlersCopy;
	for (const auto& pair : handlersMap)
		handlersCopy.push_back(pair.second);
	for (auto& handler : handlersCopy)
	{
		handler(newSize);
	}
};
std::shared_ptr<IScene> IWindow::setIScene(const std::shared_ptr<IScene>& scene)
{
  this->scene = scene;
  return scene;
};
void IWindow::runOnThread(const Runnable &runnable)
{
  runnables.push(runnable);
};
void IWindow::runRunnables()
{
  std::queue<Runnable> runnablesCopy = runnables;
  while (!runnables.empty())
    runnables.pop();
  while (!runnablesCopy.empty())
  {
    auto runnable = runnablesCopy.front();
    runnablesCopy.pop();
    runnable(*this);
  }
};
void IWindow::updateDeltaTime()
{
  auto currentTime = std::chrono::steady_clock::now();
  std::chrono::duration<float> duration = currentTime - lastFrameTime;
  deltaTime = duration.count();
  lastFrameTime = currentTime;
};
void IWindow::resize(glm::vec2 newSize)
{
  if (windowWidth != newSize.x)
    windowWidth = newSize.x;
  if (windowHeight != newSize.y)
    windowHeight = newSize.y;
  if (scene)
    scene->resize(newSize);
	callResizeHandler(newSize);
};
void IWindow::registerOnEntityAddedFunction(const OnEntityAddedFunction &function)
{
  onEntityAdded = function;
};
void IWindow::close(){};
void IWindow::minimize(){};
void IWindow::maximize(){};
void IWindow::restore(){};
void IWindow::warpPointer(glm::vec2 coords){};
void IWindow::setXY(float x, float y){};
void IWindow::setWidthHeight(float width, float height){};
IWindow &IWindow::createChildWindow(const char *title,
                                    IScene &scene,
                                    float windowWidth,
                                    float windowHeight,
                                    float windowX,
                                    float windowY,
                                    bool NDCFramebufferPlane)
{
  throw std::runtime_error("IWindow::createChildWindow() not implemented");
};