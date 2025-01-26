#include <zg/modules/gl/textures/Framebuffer.hpp>
#include <zg/modules/gl/textures/FramebufferFactory.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include <zg/modules/gl/textures/Texture.hpp>
#include <zg/modules/gl/renderers/GLRenderer.hpp>
using namespace zg::modules::gl::textures;
Framebuffer::Framebuffer(RenderWindow &window, Texture &texture):
  window(window),
  texture(texture)
{
  FramebufferFactory::initFramebuffer(*this);
};
Framebuffer::Framebuffer(RenderWindow &window, Texture &texture, Texture &depthTexture):
  window(window),
  texture(texture),
  depthTexturePointer(&depthTexture)
{
  FramebufferFactory::initFramebuffer(*this);
};
Framebuffer::~Framebuffer()
{
  FramebufferFactory::destroyFramebuffer(*this);
};
void Framebuffer::bind() const
{
  auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  glRenderer.glContext->BindFramebuffer(GL_FRAMEBUFFER, id);
  GLcheck(glRenderer, "glBindFramebuffer");
  glRenderer.glContext->Viewport(0, 0, texture.size.x, texture.size.y);
  GLcheck(glRenderer, "glViewport");
};
void Framebuffer::unbind()
{
  auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  glRenderer.glContext->BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(glRenderer, "glBindFramebuffer");
};