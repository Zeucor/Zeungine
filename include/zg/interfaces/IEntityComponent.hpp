#pragma once
#include "IComponent.hpp"
namespace zg
{
	struct Entity;
}
namespace zg::interfaces
{
	struct IEntityComponent : IComponent
	{
		virtual ~IEntityComponent() = default;
		virtual void onUpdate(Entity& entity) = 0;
		virtual void onAdded(Entity& entity) = 0;
		virtual void onRemoved(Entity& entity) = 0;
	};
} // namespace zg::interfaces
