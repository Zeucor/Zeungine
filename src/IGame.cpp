#include <anex/IGame.hpp>
using namespace anex;
IGame::IGame(const int& windowWidth, const int& windowHeight):
  windowWidth(windowWidth),
  windowHeight(windowHeight)
{};
void IGame::awaitWindowThread()
{
  windowThread->join();;
}
void IGame::run()
{
  windowThread = std::make_shared<std::thread>(&IGame::startWindow, this);
}
void IGame::render()
{
  if (scene)
    scene->render();
};
unsigned int IGame::addKeyHandler(const unsigned int& key, const std::function<void(const bool&)>& callback)
{
  auto& handlersPair = keyHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IGame::removeKeyHandler(const unsigned int& key, unsigned int& id)
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
unsigned int IGame::addKeyUpdateHandler(const unsigned int& key, const std::function<void()>& callback)
{
  auto& handlersPair = keyUpdateHandlers[key];
  auto id = ++handlersPair.first;
  handlersPair.second[id] = callback;
  return id;
};
void IGame::removeKeyUpdateHandler(const unsigned int& key, unsigned int& id)
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
void IGame::callKeyPressHandler(const unsigned int &key, const int &pressed)
{
  keys[key] = pressed;
  {
    auto handlersIter = keyHandlers.find(key);
    if (handlersIter == keyHandlers.end())
      return;
    auto& handlersMap = handlersIter->second.second;
    for (auto& handlerPair : handlersMap)
    {
      handlerPair.second(!!pressed);
    }
  }
};
void IGame::callKeyUpdateHandler(const unsigned int &key)
{
  auto handlersIter = keyUpdateHandlers.find(key);
  if (handlersIter == keyUpdateHandlers.end())
    return;
  auto& handlersMap = handlersIter->second.second;
  for (auto& handlerPair : handlersMap)
  {
    handlerPair.second();
  }
};
std::shared_ptr<IScene> IGame::setIScene(const std::shared_ptr<IScene>& scene)
{
  this->scene = scene;
  return scene;
};