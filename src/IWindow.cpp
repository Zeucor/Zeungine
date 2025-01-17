#include <stdexcept>
#include <anex/IWindow.hpp>
#include <anex/modules/gl/GLWindow.hpp>
using namespace anex;
IWindow::IWindow(const int32_t& windowWidth, const int32_t& windowHeight, const int32_t& windowX, const int32_t& windowY, const bool &borderless, const uint32_t &framerate):
  windowWidth(windowWidth),
  windowHeight(windowHeight),
  windowX(windowX),
  windowY(windowY),
  framerate(framerate),
  borderless(borderless)
{};
void IWindow::awaitWindowThread() const
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
IWindow::EventIdentifier IWindow::addKeyPressHandler(const Key& key, const KeyPressHandler& callback)
{
  auto& handlersPair = keyPressHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeKeyPressHandler(const Key& key, EventIdentifier& id)
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
IWindow::EventIdentifier IWindow::addKeyUpdateHandler(const Key& key, const KeyUpdateHandler& callback)
{
  auto& handlersPair = keyUpdateHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeKeyUpdateHandler(const Key& key, EventIdentifier& id)
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
void IWindow::callKeyPressHandler(const Key &key, const int &pressed)
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
void IWindow::callKeyUpdateHandler(const Key &key)
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
// Mouse
IWindow::EventIdentifier IWindow::addMousePressHandler(const Button& button, const MousePressHandler& callback)
{
  auto& handlersPair = mousePressHandlers[button];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeMousePressHandler(const Button& button, EventIdentifier& id)
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
void IWindow::callMousePressHandler(const Button &button, const int &pressed)
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
void IWindow::callMouseMoveHandler(const glm::vec2 &coords)
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
void IWindow::resize(const glm::vec2 &newSize)
{
  if (windowWidth != newSize.x)
    windowWidth = newSize.x;
  if (windowHeight != newSize.y)
    windowHeight = newSize.y;
  if (scene)
    scene->resize(newSize);
};
void IWindow::registerOnEntityAddedFunction(const OnEntityAddedFunction &function)
{
  onEntityAdded = function;
};
void IWindow::close(){};
void IWindow::minimize(){};
void IWindow::maximize(){};
void IWindow::restore(){};
void IWindow::warpPointer(const glm::vec2 &coords){};
void IWindow::setXY(const int32_t &x, const int32_t &y){};
void IWindow::setWidthHeight(const uint32_t &width, const uint32_t &height){};
IWindow &IWindow::createChildWindow(const char *title, IScene &scene, const uint32_t &windowWidth, const uint32_t &windowHeight, const int32_t &windowX, const int32_t &windowY)
{
  throw std::runtime_error("IWindow::createChildWindow() not implemented");
};