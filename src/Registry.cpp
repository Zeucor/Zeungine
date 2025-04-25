#include <zg/Registry.hpp>
#include <string>
zg::Registry::WindowKeyIDVector zg::Registry::windows([](auto& window) { return window.title; });
/**
 * Gets a Window from the registry by the Window ID
 */
zg::Window& zg::Registry::getWindow(const std::vector<size_t*>& indexStack)
{
    auto indexStackData = indexStack.data();
    if (indexStack.size() < 1)
        throw std::runtime_error("Cannot find a Scene with a stack size of less than 1");
    auto iter = (windows.begin() + *indexStackData[0]);
    if (iter == windows.end())
        throw std::runtime_error("Window not found with indexStack[0] = " + std::to_string(*indexStackData[0]));
    return *iter;
}
/**
 * Emplaces a Window at the back of the Windows' key/id vector, returning the tuple containing key/id/*index/*value
 * 
 * *value can become degenerate if adding multiple windows/scenes/entities, so use ID (stacks) instead
 */
zg::Registry::WindowKeyIDVector::EmplaceBackTuple zg::Registry::addWindow(const WindowCreateInfo& createInfo)
{
    auto window_tuple = windows.emplace_back(createInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
    window.ID = std::get<KEY_ID_VECTOR_ID_INDEX>(window_tuple);
    window.INDEX = std::get<KEY_ID_VECTOR_INDEX_INDEX>(window_tuple);
	window.INDEX_STACK = {window.INDEX};
    return window_tuple;
}
/**
 * Gets a Scene from the registry by looking
 *   up window @ indexStack[0],
 *   then window.scenes[indexStack[1]]
 */
zg::Scene& zg::Registry::getScene(const std::vector<size_t*>& indexStack)
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
/**
 * Gets an Entity from the registry by looking up..
 *   window @ indexStack[0],
 *   then window.scenes[indexStack[1]],
 *   then scenes.entities[indexStack[3]] = &Entity
 *   of (if needbe) [...entity.children[indexStack[4..n-1]]] = &Entity
 */
zg::Entity& zg::Registry::getEntity(const std::vector<size_t*>& indexStack)
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