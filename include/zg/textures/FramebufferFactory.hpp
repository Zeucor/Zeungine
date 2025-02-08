#pragma once
#include "./Framebuffer.hpp"
namespace zg::textures
{
  struct FramebufferFactory
  {
    static void initFramebuffer(Framebuffer &framebuffer);
    static void destroyFramebuffer(Framebuffer &framebuffer);
  };
}