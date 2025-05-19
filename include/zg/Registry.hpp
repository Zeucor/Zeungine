#pragma once
#include <span>
#include <array>
#include <vector>
#include <memory>
#include <zg/KeyIDVector.hpp>
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
    struct Registry
    {
        using WindowKeyIDVector = KeyIDVector<std::string, Window>;
        using MeshKeyIDVector = KeyIDVector<size_t, Mesh>;
        static std::unique_ptr<WindowKeyIDVector> windows;
        static std::unique_ptr<MeshKeyIDVector> meshes;
        static std::unordered_map<size_t, size_t> meshIDRefCounts;
        inline static std::mutex meshIDMutex = {};
        static std::unique_ptr<std::map<size_t, std::vector<size_t*>>> idEntities;
        static std::unique_ptr<std::map<size_t, std::vector<size_t*>>> idScenes;
        static std::unique_ptr<std::map<size_t, std::vector<size_t*>>> idWindows;
        static std::map<size_t, std::vector<size_t*>> idEntityComponents;
        static std::map<size_t, std::vector<size_t*>> idSceneComponents;
        static std::map<size_t, std::vector<size_t*>> idWindowComponents;
        /**
         * Gets a Window from the registry using the windows index stack
         */
        static Window& getWindow(const std::vector<size_t*>& INDEX_STACK);
        /**
         * Gets a Window from the registry using the windows ID
         */
        static Window& getWindow(size_t ID);
        /**
         * Emplaces a Window at the back of the Windows' key/id vector, returning the tuple containing key/id/(*index)/(*value)
         * 
         * *value can become degenerate if adding multiple windows/scenes/entities, so use ID (stacks) instead
         */
        static WindowKeyIDVector::EmplaceBackTuple addWindow(const WindowCreateInfo& createInfo);
        /**
         * Gets a Window component by ID
         */
        static components::windows::WindowComponent& getWindowComponent(size_t ID);
        /**
         * Gets a Scene from the registry by looking
         *   up window @ INDEX_STACK[0],
         *   then window.scenes[INDEX_STACK[1]]
         */
        static Scene& getScene(const std::vector<size_t*>& INDEX_STACK);
        /**
         * Gets a Scene from the registry using the scenes ID
         */
        static Scene& getScene(size_t ID);
        /**
         * Gets a Scene component by ID
         */
        static components::scenes::SceneComponent& getSceneComponent(size_t ID);
        /**
         * Gets an Entity from the registry by looking up..
         *   window @ INDEX_STACK[0],
         *   then window.scenes[INDEX_STACK[1]],
         *   then scenes.entities[INDEX_STACK[3]] = &Entity
         *      && (if needbe) [...entity.children[INDEX_STACK[4..n-1]]] = &Entity
         */
        static Entity& getEntity(const std::vector<size_t*>& INDEX_STACK);
        /**
         * Gets an Entity from the registry using the entities ID
         */
        static Entity& getEntity(size_t ID);
        /**
         * Gets the Nth parent entity given an entities INDEX_STACK
         *  bool return true if found with pointer set false if not found
         */
        static bool getNthParentEntity(const std::vector<size_t*>& INDEX_STACK, Entity*& pointer, size_t n = 1);
        /**
         * Gets a Entity component by ID
         */
        static components::entities::EntityComponent& getEntityComponent(size_t ID);
        /**
         * Hashes a MeshCreateInfo
         */
        static size_t hashMeshCreateInfo(const MeshCreateInfo& info, Entity& entity);
        /**
         * Constructs a Mesh and returns it's ID for usage in entity.meshIDs
         */
        static size_t addMesh(const MeshCreateInfo& info, Entity& entity);
        /**
         * Gets a Mesh from the registry using the meshes ID
         */
        static Mesh& getMesh(size_t ID);
        /**
         * Dereferences a mesh from an entity (used internally)
         * Returns true if mesh was destroyed
         */
        static bool deRefMesh(size_t ID);
    };
}