#pragma once
#include <zg/Events.hpp>
#include <memory>
#include <map>
namespace zg::interfaces
{
    struct IComponent
    {
        virtual ~IComponent() = default;
    };
}