#pragma once
#include <zg/interfaces/IComponent.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::entities
{
	struct EntityComponent;
    struct EntityComponentCreateInfo
    {
        std::string name;
		std::function<void(EntityComponent&)> onAttachedFunction;
		std::function<void(EntityComponent&)> onDetachedFunction;
		std::function<void(EntityComponent&)> onUpdateFunction;
		interfaces::IComponent<EntityComponent>::GetDataFunctionMap getDataFunctions;
		interfaces::IComponent<EntityComponent>::SetDataFunctionMap setDataFunctions;
    };
	struct EntityComponent : interfaces::IComponent<EntityComponent>
	{
		std::function<void(EntityComponent&)> onAttachedFunction;
		std::function<void(EntityComponent&)> onDetachedFunction;
		std::function<void(EntityComponent&)> onUpdateFunction;
		EntityComponent(const EntityComponentCreateInfo& info):
			IComponent<EntityComponent>(info.name, info.getDataFunctions, info.setDataFunctions),
			onAttachedFunction(info.onAttachedFunction),
			onDetachedFunction(info.onDetachedFunction),
			onUpdateFunction(info.onUpdateFunction)
		{};
		void onAttached()
		{
			if (onAttachedFunction)
				onAttachedFunction(*this);
		}
		void onDetached()
		{
			if (onDetachedFunction)
				onDetachedFunction(*this);
		}
		void onUpdate()
		{
			if (onUpdateFunction)
				onUpdateFunction(*this);
		}
	};
} // namespace zg::interfaces
