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
			component.host = dynamic_cast<HostT*>(this);
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
