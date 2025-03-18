#pragma once
#include "./Events.hpp"
#include <map>
#include <memory>
namespace zg
{
    template <typename T>
    struct ComponentHolder
    {
		std::pair<UniqueIdentifier, std::map<UniqueIdentifier, std::shared_ptr<T>>> components;
        UniqueIdentifier addComponent(const std::shared_ptr<T> &component)
        {
            auto id = ++std::get<0>(components);
            std::get<1>(components)[id] = component;
            return id;
        }
        void removeComponent(UniqueIdentifier& id)
        {
            auto& map = std::get<1>(components);
            auto iter = map.find(id);
            if (iter == map.end())
                return;
            map.erase(iter);
            id = 0;
        }
        void updateComponent(UniqueIdentifier id, const std::shared_ptr<T>& component)
        {
            auto& map = std::get<1>(components);
            auto iter = map.find(id);
            if (iter == map.end())
                return;
            iter->second = component;
        }
    };
}