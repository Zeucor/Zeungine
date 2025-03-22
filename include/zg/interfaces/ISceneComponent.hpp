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
		virtual ~ISceneComponent() = default;
		virtual void onUpdate(Scene& scene) = 0;
		virtual void onAdded(Scene& scene) = 0;
		virtual void onRemoved(Scene& scene) = 0;
	};
} // namespace zg::interfaces
