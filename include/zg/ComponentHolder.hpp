#pragma once
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <map>
#include <memory>
#include <mutex>
#include "./Events.hpp"
namespace zg
{
	struct Entity;
	struct Scene;
	struct Window;
	template <typename HostT, typename ComponentT, typename ComponentInfoT>
	struct ComponentHolder
	{
		struct ComponentEntry
		{
			size_t ID;
			std::string NAME;
			ComponentT COMPONENT;
		};
		typedef std::vector<ComponentEntry> ComponentContainer;
		std::pair<UniqueIdentifier, ComponentContainer> m_components;
		std::shared_ptr<std::mutex> m_componentMutex;
	
		ComponentHolder():
			m_componentMutex(std::make_shared<std::mutex>())
		{}
		ComponentHolder& operator=(const ComponentHolder& other)
		{
			m_components = other.m_components;
			m_componentMutex = std::make_shared<std::mutex>();
			return *this;
		}
		virtual ~ComponentHolder() = default;
		void detachAllComponents()
		{
			std::lock_guard lock(*m_componentMutex);
			auto& container = std::get<1>(m_components);
			for (auto& entry : container)
			{
				entry.COMPONENT.onDetached();
			}
			container.clear();
		}
		/**
		 * @brief adds a component to m_components and returns it's unique id
		 */
		UniqueIdentifier attachComponent(const ComponentInfoT& info)
		{
			std::lock_guard lock(*m_componentMutex);
			auto id = ++std::get<0>(m_components);
			auto& components = std::get<1>(m_components);
			auto& component = components.emplace_back(id, info.name, info).COMPONENT;
			auto& host = *dynamic_cast<HostT*>(this);
			if constexpr (std::is_same_v<HostT, Entity>)
			{
				component.hostIDStack.push_back(host.scene.window.ID);
				component.hostIDStack.push_back(host.scene.ID);
				std::vector<size_t> parentEntityStack;
				auto currentEntity = host.parentEntity;
				while (currentEntity)
				{
					parentEntityStack.push_back(currentEntity->ID);
					currentEntity = currentEntity.parentEntity;
				}
				auto parentEntityStackData = parentEntityStack.data();
				for (int i = parentEntityStack.size(); i >= 0; --i)
				{
					component.hostIDStack.push_back(parentEntityStackData[i]);
				}
				component.hostIDStack.push_back(host.ID);
			}
			else if constexpr (std::is_same_v<HostT, Scene>)
			{
				component.hostIDStack.push_back(host.window.ID);
				component.hostIDStack.push_back(host.ID);
			}
			else
			{
				component.hostIDStack.push_back(host.ID);
			}
			component.onAttached();
			return id;
		}
		/**
		 * @brief removes a component by id, returns false if the component does not exist, sets componentID to zero on
		 * success
		 */
		bool detachComponent(UniqueIdentifier& id)
		{
			std::lock_guard lock(*m_componentMutex);
			auto& components = std::get<1>(m_components);
			auto componentsSize = components.size();
			auto componentsData = components.data();
			auto i = 0;
			for (; i < componentsSize; i++)
			{
				if (componentsData[i].ID == id)
				{
					components.erase(components.begin()+i);
					id = 0;
					return true;
				}
			}
			return false;
		}
		/**
		 * @brief removes a component by name, returns false if the component does not exist
		 */
		bool detachComponent(const std::string& name)
		{
			std::lock_guard lock(*m_componentMutex);
			auto& components = std::get<1>(m_components);
			auto componentsSize = components.size();
			auto componentsData = components.data();
			auto i = 0;
			for (; i < componentsSize; i++)
			{
				if (componentsData[i].NAME == name)
				{
					components.erase(components.begin() + i);
					return true;
				}
			}
			return false;
		}

		ComponentT& getComponentByID(size_t id)
		{
			std::lock_guard lock(*m_componentMutex);
			auto& components = std::get<1>(m_components);
			auto componentsSize = components.size();
			auto componentsData = components.data();
			auto i = 0;
			for (; i < componentsSize; i++)
			{
				auto& entry = componentsData[i];
				if (entry.ID == id)
				{
					return entry.COMPONENT;
				}
			}
			throw std::runtime_error("Component not found with identifier");
		}

		ComponentT& getComponentByName(const std::string& name)
		{
			std::lock_guard lock(*m_componentMutex);
			auto& components = std::get<1>(m_components);
			auto componentsSize = components.size();
			auto componentsData = components.data();
			auto i = 0;
			for (; i < componentsSize; i++)
			{
				auto& entry = componentsData[i];
				if (entry.NAME == name)
				{
					return entry.COMPONENT;
				}
			}
			throw std::runtime_error("Component not found with name");
		}
	};
} // namespace zg
