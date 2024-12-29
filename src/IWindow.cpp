#include <anex/IWindow.hpp>
using namespace anex;
IWindow::IWindow(const int& windowWidth, const int& windowHeight, const int &framerate):
  windowWidth(windowWidth),
  windowHeight(windowHeight),
  framerate(framerate)
{};
void IWindow::awaitWindowThread()
{
  windowThread->join();
}
void IWindow::run()
{
  windowThread = std::make_shared<std::thread>(&IWindow::startWindow, this);
}
void IWindow::render()
{
  if (scene)
    scene->render();
};
unsigned int IWindow::addKeyHandler(const unsigned int& key, const std::function<void(const bool&)>& callback)
{
  auto& handlersPair = keyHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeKeyHandler(const unsigned int& key, unsigned int& id)
{
  auto& handlersPair = keyHandlers[key];
  auto handlerIter = handlersPair.second.find(id);
  if (handlerIter == handlersPair.second.end())
  {
    return;
  }
  handlersPair.second.erase(handlerIter);
  id = 0;
};
unsigned int IWindow::addKeyUpdateHandler(const unsigned int& key, const std::function<void()>& callback)
{
  auto& handlersPair = keyUpdateHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IWindow::removeKeyUpdateHandler(const unsigned int& key, unsigned int& id)
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
void IWindow::callKeyPressHandler(const unsigned int &key, const int &pressed)
{
  keys[key] = pressed;
  {
    auto handlersIter = keyHandlers.find(key);
    if (handlersIter == keyHandlers.end())
      return;
    auto& handlersMap = handlersIter->second.second;
    std::vector<std::function<void(const bool&)>> handlersCopy;
    for (const auto& pair : handlersMap)
      handlersCopy.push_back(pair.second);
    for (auto& handler : handlersCopy)
    {
      handler(!!pressed);
    }
  }
};
void IWindow::callKeyUpdateHandler(const unsigned int &key)
{
  auto handlersIter = keyUpdateHandlers.find(key);
  if (handlersIter == keyUpdateHandlers.end())
    return;
  auto& handlersMap = handlersIter->second.second;
  std::vector<std::function<void()>> handlersCopy;
  for (const auto& pair : handlersMap)
    handlersCopy.push_back(pair.second);
  for (auto& handler : handlersCopy)
  {
    handler();
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
  while (!runnables.empty())
  {
    auto runnable = runnables.front();
    runnables.pop();
    runnable(*this);
  }
};