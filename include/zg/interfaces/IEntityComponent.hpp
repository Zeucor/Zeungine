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
		friend Entity;
		virtual ~IEntityComponent() = default;

	protected:
		virtual void onUpdate(Entity& entity) = 0;
		virtual void onAdded(Entity& entity) = 0;
		virtual void onRemoved(Entity& entity) = 0;
	};
} // namespace zg::interfaces
