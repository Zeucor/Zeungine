#pragma once
#include <functional>
#include <memory>
#include <zg/glm.hpp>
namespace zg
{
    struct Window;
	struct Entity;
	using Runnable = std::function<void(Window&)>;
	using Key = uint32_t;
	using Button = uint32_t;
	using UniqueIdentifier = size_t;
	using KeyPressHandler = std::function<void(bool)>;
	using KeyUpdateHandler = std::function<void()>;
	using AnyKeyPressHandler = std::function<void(const Key&, bool)>;
	using MousePressHandler = std::function<void(bool)>;
	using MouseMoveHandler = std::function<void(glm::vec2)>;
	using ViewResizeHandler = std::function<void(glm::vec2)>;
	using FocusHandler = std::function<void(bool)>;
	using OnEntityAddedFunction = std::function<void(const Entity&)>;
	using PreSwapbuffersOnceoff = std::function<void()>;
	using ShutdownHandler = std::function<void(Window&)>;
    static constexpr unsigned int MinMouseButtonIndex = 0;
    static constexpr unsigned int MaxMouseButtonIndex = 6;
    static constexpr unsigned int MinMouseButton = 1;
    static constexpr unsigned int MaxMouseButton = 7;
} // namespace zg
