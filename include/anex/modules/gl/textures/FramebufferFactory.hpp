#pragma once
#include "./Framebuffer.hpp"
namespace anex::modules::gl::textures
{
	struct FramebufferFactory
  {
    static void initFramebuffer(Framebuffer &framebuffer);
  };
}