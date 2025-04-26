#include <zg/Registry.hpp>
#include <string>
using namespace zg;
Registry::WindowKeyIDVector Registry::windows([](auto& window) { return window.title; });
std::unique_ptr<std::map<size_t, std::vector<size_t*>>> Registry::idWindows = std::make_unique<std::map<size_t, std::vector<size_t*>>>();
std::unique_ptr<std::map<size_t, std::vector<size_t*>>> Registry::idScenes = std::make_unique<std::map<size_t, std::vector<size_t*>>>();
std::unique_ptr<std::map<size_t, std::vector<size_t*>>> Registry::idEntities = std::make_unique<std::map<size_t, std::vector<size_t*>>>();
/**
 * Gets a Window from the registry by the Window ID
 */
Window& Registry::getWindow(const std::vector<size_t*>& indexStack)
{
    auto indexStackData = indexStack.data();
    if (indexStack.size() < 1)
        throw std::runtime_error("Cannot find a Scene with a stack size of less than 1");
    auto iter = (windows.begin() + *indexStackData[0]);
    if (iter == windows.end())
        throw std::runtime_error("Window not found with indexStack[0] = " + std::to_string(*indexStackData[0]));
    return *iter;
}
Window& Registry::getWindow(size_t ID)
{
    auto& idWindowsRef = *idWindows;
    auto idIter = idWindowsRef.find(ID);
    if (idIter == idWindowsRef.end())
        throw std::runtime_error("Window not found with ID: " + std::to_string(ID));
    auto& stack = idIter->second;
    return getWindow(stack);
}
/**
 * Emplaces a Window at the back of the Windows' key/id vector, returning the tuple containing key/id/*index/*value
 * 
 * *value can become degenerate if adding multiple windows/scenes/entities, so use ID (stacks) instead
 */
Registry::WindowKeyIDVector::EmplaceBackTuple Registry::addWindow(const WindowCreateInfo& createInfo)
{
    auto window_tuple = windows.emplace_back(createInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
    window.ID = std::get<KEY_ID_VECTOR_ID_INDEX>(window_tuple);
    window.INDEX = std::get<KEY_ID_VECTOR_INDEX_INDEX>(window_tuple);
	window.INDEX_STACK = {window.INDEX};
    (*idWindows)[window.ID] = window.INDEX_STACK;
    return window_tuple;
}
/**
 * Gets a Scene from the registry by looking
 *   up window @ indexStack[0],
 *   then window.scenes[indexStack[1]]
 */
Scene& Registry::getScene(const std::vector<size_t*>& indexStack)
{
    auto indexStackData = indexStack.data();
    if (indexStack.size() < 2)
        throw std::runtime_error("Cannot find a Scene with a stack size of less than 2");
    auto& window = getWindow(indexStack);
    auto sceneIter = (window.scenes.begin() + *indexStackData[1]);
    if (sceneIter == window.scenes.end())
        throw std::runtime_error("Scene not found with indexStack[1] = " + std::to_string(*indexStackData[1]));
    return *sceneIter;
}
Scene& Registry::getScene(size_t ID)
{
    auto& idScenesRef = *idScenes;
    auto idIter = idScenesRef.find(ID);
    if (idIter == idScenesRef.end())
        throw std::runtime_error("Scene not found with ID: " + std::to_string(ID));
    auto& stack = idIter->second;
    return getScene(stack);
}
/**
 * Gets an Entity from the registry by looking up..
 *   window @ indexStack[0],
 *   then window.scenes[indexStack[1]],
 *   then scenes.entities[indexStack[3]] = &Entity
 *   of (if needbe) [...entity.children[indexStack[4..n-1]]] = &Entity
 */
Entity& Registry::getEntity(const std::vector<size_t*>& indexStack)
{
    auto& scene = getScene(indexStack);
    if (indexStack.size() < 3)
        throw std::runtime_error("Cannot find an Entity with a stack size of less than 2");
    auto indexStackData = indexStack.data();
    auto entityIter = (scene.entities.begin() + *indexStackData[2]);
    if (entityIter == scene.entities.end())
        throw std::runtime_error("Entity not found with indexStack[2] = " + std::to_string(*indexStackData[2]));
    auto& entity = *entityIter;
    auto currentEntity = &entity;
    auto indexStackSize = indexStack.size();
    for (int i = 3; i < indexStackSize; i++)
    {
        auto& currentEntityRef = *currentEntity;
        auto childIter = (currentEntityRef.children.begin() + *indexStackData[i]);
        if (childIter == currentEntityRef.children.end())
            throw std::runtime_error("Child Entity not found with indexStack[" + std::to_string(i) + "] = " + std::to_string(*indexStackData[i]));
        currentEntity = &*childIter;
    }
    return *currentEntity;
}
Entity& Registry::getEntity(size_t ID)
{
    auto& idEntitiesRef = *idEntities;
    auto idIter = idEntitiesRef.find(ID);
    if (idIter == idEntitiesRef.end())
        throw std::runtime_error("Entity not found with ID: " + std::to_string(ID));
    auto& stack = idIter->second;
    return getEntity(stack);
}