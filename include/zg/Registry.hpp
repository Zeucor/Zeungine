#include <zg/Window.hpp>
#include <span>
#include <array>
#include <vector>
#include <memory>
#include <zg/KeyIDVector.hpp>
namespace zg
{
    struct Registry
    {
        using WindowKeyIDVector = KeyIDVector<std::string, Window>;
        static WindowKeyIDVector windows;
        static std::unique_ptr<std::map<size_t, std::vector<size_t*>>> idEntities;
        static std::unique_ptr<std::map<size_t, std::vector<size_t*>>> idScenes;
        static std::unique_ptr<std::map<size_t, std::vector<size_t*>>> idWindows;
        /**
         * Gets a Window from the registry using the windows index stack
         */
        static Window& getWindow(const std::vector<size_t*>& indexStack);
        /**
         * Gets a Window from the registry using the windows ID
         */
        static Window& getWindow(size_t ID);
        /**
         * Emplaces a Window at the back of the Windows' key/id vector, returning the tuple containing key/id/*index/*value
         * 
         * *value can become degenerate if adding multiple windows/scenes/entities, so use ID (stacks) instead
         */
        static WindowKeyIDVector::EmplaceBackTuple addWindow(const WindowCreateInfo& createInfo);
        /**
         * Gets a Scene from the registry by looking
         *   up window @ indexStack[0],
         *   then window.scenes[indexStack[1]]
         */
        static Scene& getScene(const std::vector<size_t*>& indexStack);
        /**
         * Gets a Scene from the registry using the scenes ID
         */
        static Scene& getScene(size_t ID);
        /**
         * Gets an Entity from the registry by looking up..
         *   window @ indexStack[0],
         *   then window.scenes[indexStack[1]],
         *   then scenes.entities[indexStack[3]] = &Entity
         *   of (if needbe) [...entity.children[indexStack[4..n-1]]] = &Entity
         */
        static Entity& getEntity(const std::vector<size_t*>& indexStack);
        /**
         * Gets an Entity from the registry using the entities ID
         */
        static Entity& getEntity(size_t ID);
    };
}