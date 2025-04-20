#include <zg/Window.hpp>
#include <zg/KeyIDVector.hpp>
namespace zg
{
    struct Registry
    {
        using WindowKeyIDVector = KeyIDVector<std::string, Window>;
        static WindowKeyIDVector windows;
        /**
         * Gets a Window from the registry by the Window ID
         */
        static Window& getWindow(const std::vector<size_t>& idStack);
        /**
         * Emplaces a Window at the back of the Windows' key/id vector, returning the tuple containing key/id/*index/*value
         * 
         * *value can become degenerate if adding multiple windows/scenes/entities, so use ID (stacks) instead
         */
        static WindowKeyIDVector::EmplaceBackTuple addWindow(const WindowCreateInfo& createInfo);
        /**
         * Gets a Scene from the registry by looking
         *   up window @ idStack[0],
         *   then window.scenes[idStack[1]]
         */
        static Scene& getScene(const std::vector<size_t>& idStack);
        /**
         * Gets an Entity from the registry by looking up..
         *   window @ idStack[0],
         *   then window.scenes[idStack[1]],
         *   then scenes.entities[idStack[3]] = &Entity
         *   of (if needbe) [...entity.children[idStack[4..n-1]]] = &Entity
         */
        static Entity& getEntity(const std::vector<size_t>& idStack);
    };
}