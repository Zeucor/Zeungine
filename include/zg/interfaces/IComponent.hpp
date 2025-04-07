#pragma once
#include <zg/Events.hpp>
#include <memory>
#include <map>
#include <string>
namespace zg::interfaces
{
    struct IComponent
    {
        size_t ID = 0;
        std::string NAME;
        IComponent(const std::string& name);
        virtual ~IComponent() = default;
		virtual void onAttached() = 0;
		virtual void onUpdate() = 0;
		virtual void onDetached() = 0;
    };
}