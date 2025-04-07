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
		IEntityComponent(const std::string& name);
		virtual ~IEntityComponent() = default;
		virtual void onAttached() = 0;
		virtual void onUpdate() = 0;
		virtual void onDetached() = 0;
	};
} // namespace zg::interfaces
