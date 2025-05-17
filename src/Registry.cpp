#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/crypto/vector.hpp>
#include <string>
using namespace zg;
std::unique_ptr<Registry::WindowKeyIDVector> Registry::windows = std::make_unique<Registry::WindowKeyIDVector>([](auto& window) { return window.title; });
std::unique_ptr<Registry::MeshKeyIDVector> Registry::meshes = std::make_unique<Registry::MeshKeyIDVector>([](auto& mesh) { return mesh.hash; });
std::unordered_map<size_t, size_t> Registry::meshIDRefCounts;
std::unique_ptr<std::map<size_t, std::vector<size_t*>>> Registry::idWindows = std::make_unique<std::map<size_t, std::vector<size_t*>>>();
std::unique_ptr<std::map<size_t, std::vector<size_t*>>> Registry::idScenes = std::make_unique<std::map<size_t, std::vector<size_t*>>>();
std::unique_ptr<std::map<size_t, std::vector<size_t*>>> Registry::idEntities = std::make_unique<std::map<size_t, std::vector<size_t*>>>();
std::map<size_t, std::vector<size_t*>> Registry::idEntityComponents;
std::map<size_t, std::vector<size_t*>> Registry::idSceneComponents;
std::map<size_t, std::vector<size_t*>> Registry::idWindowComponents;
/**
 * Gets a Window from the registry by the Window ID
 */
Window& Registry::getWindow(const std::vector<size_t*>& indexStack)
{
    auto indexStackData = indexStack.data();
    if (indexStack.size() < 1)
        throw std::runtime_error("Cannot find a Window with a stack size of less than 1");
    auto& windowsRef = *windows;
    auto iter = (windowsRef.begin() + *indexStackData[0]);
    if (iter == windowsRef.end())
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
    auto usingInfo = createInfo;
    auto& windowsRef = *windows;
    auto transaction = windowsRef.startTransaction();
    usingInfo.INDEX_STACK = {transaction.index};
    usingInfo.ID = transaction.id;
    usingInfo.INDEX = transaction.index;
    auto& window = windowsRef.commitTransaction(transaction, usingInfo);
    (*idWindows)[window.ID] = window.INDEX_STACK;
    return {transaction.key, transaction.id, transaction.index, &window};
}
components::windows::WindowComponent& Registry::getWindowComponent(size_t ID)
{
    auto indexStackIter = idWindowComponents.find(ID);
    if (indexStackIter == idWindowComponents.end())
        throw std::runtime_error("Window Component not found with ID: "+ std::to_string(ID));
    auto& host = getWindow(indexStackIter->second);
    return host.getComponentByID(ID);
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
components::scenes::SceneComponent& Registry::getSceneComponent(size_t ID)
{
    auto indexStackIter = idSceneComponents.find(ID);
    if (indexStackIter == idSceneComponents.end())
        throw std::runtime_error("Scene Component not found with ID: "+ std::to_string(ID));
    auto& host = getScene(indexStackIter->second);
    return host.getComponentByID(ID);
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
bool Registry::getNthParentEntity(const std::vector<size_t*>& INDEX_STACK, Entity*& pointer, size_t n)
{
    if (INDEX_STACK.size() < 3 + n)
        return false;
    auto parent_INDEX_STACK = INDEX_STACK;
    parent_INDEX_STACK.erase(parent_INDEX_STACK.begin() + (parent_INDEX_STACK.size() - n), parent_INDEX_STACK.end());
    pointer = &getEntity(parent_INDEX_STACK);
    return true;
}
components::entities::EntityComponent& Registry::getEntityComponent(size_t ID)
{
    auto indexStackIter = idEntityComponents.find(ID);
    if (indexStackIter == idEntityComponents.end())
        throw std::runtime_error("Entity Component not found with ID: "+ std::to_string(ID));
    auto& host = getEntity(indexStackIter->second);
    return host.getComponentByID(ID);
}
size_t Registry::hashMeshCreateInfo(const MeshCreateInfo& info, Entity& entity)
{
    size_t finalHash = 0;
    uint64_t shift = 0;
    auto mesh_info_iter = info.entity_id_mesh_infos.find(entity.ID);
    if (mesh_info_iter == info.entity_id_mesh_infos.end())
    {
        auto meshInfo = info.info(entity);
        ((MeshCreateInfo&)info).entity_id_mesh_infos[entity.ID] = meshInfo;
        mesh_info_iter = info.entity_id_mesh_infos.find(entity.ID);
    }
    auto& meshInfo = mesh_info_iter->second;
    finalHash = (std::hash<uint32_t>{}(uint32_t(info.shapeType)) + 1) << ++shift;
    if (meshInfo.indices.empty() || meshInfo.vertices.empty())
        goto _textureHash;
    {
        auto indiceHash = crypto::hashVector(meshInfo.indices);
        auto verticeHash = crypto::hashVector(meshInfo.vertices);
        finalHash ^= indiceHash << ++shift;
        finalHash ^= verticeHash << ++shift;
    }
_textureHash:
    for (auto& keyedPair : info.keyedTextures)
    {
        finalHash ^= (size_t)keyedPair.second->rendererData << ++shift;
    }
    return finalHash;
}
size_t Registry::addMesh(const MeshCreateInfo& info, Entity& entity)
{
    auto usingInfo = info;
    usingInfo.hash = hashMeshCreateInfo(usingInfo, entity);
    auto& meshesRef = *meshes;
    auto keyIter = meshesRef.find_key(usingInfo.hash);
    if (keyIter != meshesRef.end())
    {
        auto meshID = keyIter.id();
        meshIDRefCounts[meshID]++;
        return meshID;
    }
    auto transaction = meshesRef.startTransaction();
    auto& mesh = meshesRef.commitTransaction(transaction, usingInfo, entity);
    mesh.ID = transaction.id;
    mesh.INDEX = transaction.index;
    mesh.INDEX_STACK = {transaction.index};
    std::lock_guard lock(meshIDMutex);
    meshIDRefCounts[mesh.ID]++;
    return transaction.id;
}
Mesh& Registry::getMesh(size_t ID)
{
    std::lock_guard lock(meshIDMutex);
    auto& meshesRef = *meshes;
    auto idIter = meshesRef.find_id(ID);
    if (idIter == meshesRef.end())
        throw std::runtime_error("Mesh not found with ID: " + std::to_string(ID));
    return *idIter;
}
bool Registry::deRefMesh(size_t ID)
{
    std::lock_guard lock(meshIDMutex);
    auto& count = meshIDRefCounts[ID];
    if (--count)
        return false;
    auto& meshesRef = *meshes;
    auto idIter = meshesRef.find_id(ID);
    if (idIter == meshesRef.end())
        throw std::runtime_error("Mesh not found with ID: " + std::to_string(ID));
    meshesRef.erase(idIter);
    return true;
}