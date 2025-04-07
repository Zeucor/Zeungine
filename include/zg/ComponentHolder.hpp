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
				boost::multi_index::hashed_unique<boost::multi_index::tag<component_by_id>,
																					boost::multi_index::member<ComponentEntry, size_t, &ComponentEntry::ID>>,
				boost::multi_index::hashed_unique<
					boost::multi_index::tag<component_by_name>,
					boost::multi_index::member<ComponentEntry, std::string, &ComponentEntry::NAME>>>>
			ComponentContainer;
		std::pair<UniqueIdentifier, ComponentContainer> m_components;
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
			auto& container = std::get<1>(m_components);
			auto iter = container.find(id);
			if (iter == container.end())
				return false;
			iter->second->onDetached();
			container.erase(iter);
			id = 0;
			return true;
		}

		std::shared_ptr<T> getComponentByID(size_t id)
		{
			auto& components_id_index = std::get<1>(m_components).get<component_by_id>();
			auto it_id = components_id_index.find(id);
			if (it_id != components_id_index.end())
			{
				return it_id->COMPONENT;
			}
			return {};
		}

		std::shared_ptr<T> getComponentByName(const std::string& name)
		{
			auto& components_name_index = std::get<1>(m_components).get<component_by_name>();
			auto it_name = components_name_index.find(name);
			if (it_name != components_name_index.end())
			{
				return it_name->COMPONENT;
			}
			return {};
		}
	};
} // namespace zg
