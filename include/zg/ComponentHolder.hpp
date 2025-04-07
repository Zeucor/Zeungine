#pragma once
#include "./Events.hpp"
#include <map>
#include <memory>
namespace zg
{
    template <typename T>
    struct ComponentHolder
    {
        std::pair<UniqueIdentifier, std::map<UniqueIdentifier, std::shared_ptr<T>>> m_components;
        /**
         * @brief adds a component to m_components and returns it's unique id
         */
        UniqueIdentifier addComponent(const std::shared_ptr<T> &component)
        {
            auto id = ++std::get<0>(m_components);
            std::get<1>(m_components)[id] = component;
            component->ID = id;
            component->onAttached();
            return id;
        }
        /**
         * @brief removes a component by id, returns false if the component does not exist, sets componentID to zero on success
         */
        bool removeComponent(UniqueIdentifier& id)
        {
            auto& map = std::get<1>(m_components);
            auto iter = map.find(id);
            if (iter == map.end())
                return false;
            iter->second->onDetached();
            map.erase(iter);
            id = 0;
            return true;
        }
    };
}