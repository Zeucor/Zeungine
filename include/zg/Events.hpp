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
	using EventIdentifier = uint32_t;
	using KeyPressHandler = std::function<void(bool)>;
	using KeyUpdateHandler = std::function<void()>;
	using AnyKeyPressHandler = std::function<void(const Key&, bool)>;
	using MousePressHandler = std::function<void(bool)>;
	using MouseMoveHandler = std::function<void(glm::vec2)>;
	using ViewResizeHandler = std::function<void(glm::vec2)>;
	using FocusHandler = std::function<void(bool)>;
	using OnEntityAddedFunction = std::function<void(const std::shared_ptr<Entity>&)>;
    static constexpr unsigned int MinMouseButton = 0;
    static constexpr unsigned int MaxMouseButton = 6;
} // namespace zg
