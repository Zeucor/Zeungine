#pragma once
#include <zg/interfaces/IComponent.hpp>
namespace zg
{
	struct Scene;
}
namespace zg::components::windows
{
	struct WindowComponent;
    struct WindowComponentCreateInfo
    {
        std::string name;
		std::function<void(WindowComponent&)> onAttachedFunction;
		std::function<void(WindowComponent&)> onDetachedFunction;
		std::function<void(WindowComponent&)> onUpdateFunction;
		interfaces::IComponent::GetDataFunctionMap getDataFunctions;
		interfaces::IComponent::SetDataFunctionMap setDataFunctions;
    };
	struct WindowComponent : interfaces::IComponent
	{
		std::function<void(WindowComponent&)> onAttachedFunction;
		std::function<void(WindowComponent&)> onDetachedFunction;
		std::function<void(WindowComponent&)> onUpdateFunction;
		WindowComponent(const WindowComponentCreateInfo& info):
			IComponent(info.name, info.getDataFunctions, info.setDataFunctions),
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
