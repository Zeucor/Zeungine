#include <zg/textures/Framebuffer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/Window.hpp>
#include <zg/textures/Texture.hpp>
#include <zg/renderers/GLRenderer.hpp>
using namespace zg::textures;
Framebuffer::Framebuffer(Window &window, Texture &texture) : window(window),
                                                             texture(texture)
{
  FramebufferFactory::initFramebuffer(*this);
}
Framebuffer::Framebuffer(Window &window, Texture &texture, Texture &depthTexture) : window(window),
                                                                                    texture(texture),
                                                                                    depthTexturePointer(&depthTexture)
{
  FramebufferFactory::initFramebuffer(*this);
}
Framebuffer::~Framebuffer()
{
  FramebufferFactory::destroyFramebuffer(*this);
}
void Framebuffer::bind() const
{
  window.iRenderer->bindFramebuffer(*this);
}
void Framebuffer::unbind()
{
  window.iRenderer->unbindFramebuffer(*this);
}