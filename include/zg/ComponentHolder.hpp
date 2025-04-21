#pragma once
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <map>
#include <memory>
#include <mutex>
#include "Events.hpp"
#include "KeyIDVector.hpp"
namespace zg
{
	struct Entity;
	struct Scene;
	struct Window;
	template <typename HostT, typename ComponentT, typename ComponentInfoT>
	struct ComponentHolder
	{
		KeyIDVector<std::string, ComponentT> m_components;
	
		ComponentHolder():
			m_components([](auto& component) { return component.NAME; })
		{}
		ComponentHolder& operator=(const ComponentHolder& other)
		{
			m_components = other.m_components;
			return *this;
		}
		virtual ~ComponentHolder() = default;
		void detachAllComponents()
		{
			std::lock_guard lock(m_components.getMutex());
			auto componentsSize = m_components.size();
			auto componentsData = m_components.data();
			for (size_t index = 0; index < componentsSize; ++index)
				componentsData[index].onDetached();
			m_components.clear();
		}
		/**
		 * @brief adds a component to m_components and returns it's unique id
		 */
		KeyIDVector<std::string, ComponentT>::EmplaceBackTuple attachComponent(const ComponentInfoT& info)
		{
			auto component_tuple = m_components.emplace_back(info);
			auto& component = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(component_tuple);
			auto& host = *dynamic_cast<HostT*>(this);
			if constexpr (std::is_same_v<HostT, Entity>)
			{
				component.hostIndexStack.push_back(host.scene.window.INDEX);
				component.hostIndexStack.push_back(host.scene.INDEX);
				std::vector<size_t*> parentEntityStack;
				auto currentEntity = host.parentEntity;
				while (currentEntity)
				{
					parentEntityStack.push_back(currentEntity->INDEX);
					currentEntity = currentEntity->parentEntity;
				}
				auto parentEntityStackData = parentEntityStack.data();
				for (int i = int(parentEntityStack.size()) - 1; i >= 0; --i)
				{
					component.hostIndexStack.push_back(parentEntityStackData[i]);
				}
				component.hostIndexStack.push_back(host.INDEX);
			}
			else if constexpr (std::is_same_v<HostT, Scene>)
			{
				component.hostIndexStack.push_back(host.window.INDEX);
				component.hostIndexStack.push_back(host.INDEX);
			}
			else
			{
				component.hostIndexStack.push_back(host.INDEX);
			}
			component.ID = std::get<KEY_ID_VECTOR_ID_INDEX>(component_tuple);
			component.onAttached();
			return component_tuple;
		}
		/**
		 * @brief removes a component by id, returns false if the component does not exist, sets componentID to zero on
		 * success
		 */
		bool detachComponent(UniqueIdentifier& id)
		{
			auto iter = m_components.find_id(id);
			if (iter == m_components.end())
				return false;
			iter->onDetached();
			m_components.erase(iter);
			id = 0;
			return true;
		}
		/**
		 * @brief removes a component by name, returns false if the component does not exist
		 */
		bool detachComponent(const std::string& name)
		{
			auto iter = m_components.find_key(name);
			if (iter == m_components.end())
				return false;
			iter->onDetached();
			m_components.erase(iter);
			return true;
		}

		ComponentT& getComponentByID(size_t id)
		{
			auto iter = m_components.find_id(id);
			if (iter == m_components.end())
				throw std::runtime_error("Component not found with identifier");
			return *iter;
		}

		ComponentT& getComponentByName(const std::string& name)
		{
			auto iter = m_components.find_key(name);
			if (iter == m_components.end())
				throw std::runtime_error("Component not found with name");
			return *iter;
		}
	};
} // namespace zg
