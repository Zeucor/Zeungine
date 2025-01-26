#pragma once
#include <memory>
namespace zg::modules::gl
{
	struct IPlatformWindow;
	struct IVendorRenderer
	{
		IPlatformWindow* platformWindowPointer = nullptr;
		virtual ~IVendorRenderer() = default;
		virtual void init(IPlatformWindow *platformWindowPointer) = 0;
		virtual void render() = 0;
		virtual void destroy() = 0;
	};
	std::shared_ptr<IVendorRenderer> createVendorRenderer();
}