#pragma once
#include "IComponent.hpp"
namespace zg
{
	struct Scene;
}
namespace zg::interfaces
{
	struct ISceneComponent : IComponent
	{
		ISceneComponent(const std::string& name);
		virtual ~ISceneComponent() = default;
		virtual void onAttached() = 0;
		virtual void onUpdate() = 0;
		virtual void onDetached() = 0;
	};
} // namespace zg::interfaces
