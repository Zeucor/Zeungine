#include <zg/Window.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg::textures;
void FramebufferFactory::initFramebuffer(Framebuffer &framebuffer)
{
  framebuffer.window.iRenderer->initFramebuffer(framebuffer);
}
void FramebufferFactory::destroyFramebuffer(Framebuffer &framebuffer)
{
  framebuffer.window.iRenderer->destroyFramebuffer(framebuffer);
}