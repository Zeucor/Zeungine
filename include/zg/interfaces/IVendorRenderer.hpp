#pragma once
#include <memory>
#include <zg/glm.hpp>
namespace zg
{
	struct IPlatformWindow;
	struct IVendorRenderer
	{
		IPlatformWindow* platformWindowPointer = nullptr;
		virtual ~IVendorRenderer() = default;
		virtual void init(IPlatformWindow *platformWindowPointer) = 0;
		virtual void render() = 0;
		virtual void destroy() = 0;
		virtual void clearColor(glm::vec4 color) = 0;
		virtual void clear() = 0;
	};
	std::shared_ptr<IVendorRenderer> createVendorRenderer();
}