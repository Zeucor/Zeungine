#pragma once
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <map>
#include <memory>
#include "./Events.hpp"
namespace zg
{
	template <typename T>
	struct ComponentHolder
	{
		struct ComponentEntry
		{
			size_t ID;
			std::string NAME;
			std::shared_ptr<T> COMPONENT;
		};
		struct component_by_id
		{
		};
		struct component_by_name
		{
		};
		typedef boost::multi_index::multi_index_container<
			ComponentEntry,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<boost::multi_index::tag<component_by_id>,
																					 boost::multi_index::member<ComponentEntry, size_t, &ComponentEntry::ID>>,
				boost::multi_index::hashed_unique<
					boost::multi_index::tag<component_by_name>,
					boost::multi_index::member<ComponentEntry, std::string, &ComponentEntry::NAME>>>>
			ComponentContainer;
		std::pair<UniqueIdentifier, ComponentContainer> m_components;

		virtual ~ComponentHolder() { unregisterAllComponents(); }
		void unregisterAllComponents()
		{
			auto& container = std::get<1>(m_components);
			for (auto& entry : container)
			{
				entry.COMPONENT->onDetached();
			}
			container.clear();
		}
		/**
		 * @brief adds a component to m_components and returns it's unique id
		 */
		UniqueIdentifier addComponent(const std::shared_ptr<T>& component)
		{
			auto id = ++std::get<0>(m_components);
			component->ID = id;
			std::get<1>(m_components).insert({id, component->NAME, component});
			component->onAttached();
			return id;
		}
		/**
		 * @brief removes a component by id, returns false if the component does not exist, sets componentID to zero on
		 * success
		 */
		bool removeComponent(UniqueIdentifier& id)
		{
			auto& components_id_index = std::get<1>(m_components).template get<component_by_id>();
			auto it_id = components_id_index.find(id);
			if (it_id == components_id_index.end())
				return false;
			it_id->COMPONENT->onDetached();
			components_id_index.erase(it_id);
			id = 0;
			return true;
		}
		/**
		 * @brief removes a component by name, returns false if the component does not exist
		 */
		bool removeComponent(const std::string& name)
		{
			auto& components_name_index = std::get<1>(m_components).template get<component_by_name>();
			auto it_name = components_name_index.find(name);
			if (it_name == components_name_index.end())
				return false;
			it_name->COMPONENT->onDetached();
			components_name_index.erase(it_name);
			return true;
		}

		std::shared_ptr<T> getComponentByID(size_t id)
		{
			auto& components_id_index = std::get<1>(m_components).template get<component_by_id>();
			auto it_id = components_id_index.find(id);
			if (it_id != components_id_index.end())
			{
				return it_id->COMPONENT;
			}
			return {};
		}

		std::shared_ptr<T> getComponentByName(const std::string& name)
		{
			auto& components_name_index = std::get<1>(m_components).template get<component_by_name>();
			auto it_name = components_name_index.find(name);
			if (it_name != components_name_index.end())
			{
				return it_name->COMPONENT;
			}
			return {};
		}
	};
} // namespace zg
