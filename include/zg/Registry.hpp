#pragma once
#include <span>
#include <array>
#include <vector>
#include <memory>
#include <zg/KeyIDVector.hpp>
#include <zg/Singleton.hpp>
namespace zg
{
    struct Window;
    struct WindowCreateInfo;
    struct Scene;
    struct Entity;
    struct Mesh;
    struct MeshCreateInfo;
    namespace components
    {
        namespace windows
        {
            struct WindowComponent;
        }
        namespace scenes
        {
            struct SceneComponent;
        }
        namespace entities
        {
            struct EntityComponent;
        }
    }
    struct Registry : Singleton<Registry>
    {
        using WindowKeyIDVector = KeyIDVector<std::string, Window>;
        using MeshKeyIDVector = KeyIDVector<size_t, Mesh>;
        WindowKeyIDVector windows;
        MeshKeyIDVector meshes;
        std::unordered_map<size_t, size_t> meshIDRefCounts;
        std::mutex meshIDMutex = {};
        std::map<size_t, std::vector<size_t*>> idEntities;
        std::map<size_t, std::vector<size_t*>> idScenes;
        std::map<size_t, std::vector<size_t*>> idWindows;
        std::map<size_t, std::vector<size_t*>> idEntityComponents;
        std::map<size_t, std::vector<size_t*>> idSceneComponents;
        std::map<size_t, std::vector<size_t*>> idWindowComponents;
        /**
         * @brief no arg constructor
         */
        Registry();
        /**
         * Gets a Window from the registry using the windows index stack
         */
        Window& getWindow(const std::vector<size_t*>& INDEX_STACK);
        /**
         * Gets a Window from the registry using the windows ID
         */
        Window& getWindow(size_t ID);
        /**
         * Emplaces a Window at the back of the Windows' key/id vector, returning the tuple containing key/id/(*index)/(*value)
         * 
         * *value can become degenerate if adding multiple windows/scenes/entities, so use ID (stacks) instead
         */
        WindowKeyIDVector::EmplaceBackTuple addWindow(const WindowCreateInfo& createInfo);
        /**
         * Gets a Window component by ID
         */
        components::windows::WindowComponent& getWindowComponent(size_t ID);
        /**
         * Gets a Scene from the registry by looking
         *   up window @ INDEX_STACK[0],
         *   then window.scenes[INDEX_STACK[1]]
         */
        Scene& getScene(const std::vector<size_t*>& INDEX_STACK);
        /**
         * Gets a Scene from the registry using the scenes ID
         */
        Scene& getScene(size_t ID);
        /**
         * Gets a Scene component by ID
         */
        components::scenes::SceneComponent& getSceneComponent(size_t ID);
        /**
         * Gets an Entity from the registry by looking up..
         *   window @ INDEX_STACK[0],
         *   then window.scenes[INDEX_STACK[1]],
         *   then scenes.entities[INDEX_STACK[3]] = &Entity
         *      && (if needbe) [...entity.children[INDEX_STACK[4..n-1]]] = &Entity
         */
        Entity& getEntity(const std::vector<size_t*>& INDEX_STACK);
        /**
         * Gets an Entity from the registry using the entities ID
         */
        Entity& getEntity(size_t ID);
        /**
         * Gets the Nth parent entity given an entities INDEX_STACK
         *  bool return true if found with pointer set false if not found
         */
        bool getNthParentEntity(const std::vector<size_t*>& INDEX_STACK, Entity*& pointer, size_t n = 1);
        /**
         * Gets a Entity component by ID
         */
        components::entities::EntityComponent& getEntityComponent(size_t ID);
        /**
         * Hashes a MeshCreateInfo
         */
        size_t hashMeshCreateInfo(const MeshCreateInfo& info, Entity& entity);
        /**
         * Constructs a Mesh and returns it's ID for usage in entity.meshIDs
         */
        size_t addMesh(const MeshCreateInfo& info, Entity& entity);
        /**
         * Gets a Mesh from the registry using the meshes ID
         */
        Mesh& getMesh(size_t ID);
        /**
         * Dereferences a mesh from an entity (used internally)
         * Returns true if mesh was destroyed
         */
        bool deRefMesh(size_t ID);
    };
}