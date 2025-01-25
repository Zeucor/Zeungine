#include <zg/modules/gl/textures/Framebuffer.hpp>
#include <zg/modules/gl/textures/FramebufferFactory.hpp>
#include <zg/modules/gl/GLWindow.hpp>
#include <zg/modules/gl/textures/Texture.hpp>
using namespace zg::modules::gl::textures;
Framebuffer::Framebuffer(GLWindow &window, Texture &texture):
  window(window),
  texture(texture)
{
  FramebufferFactory::initFramebuffer(*this);
};
Framebuffer::Framebuffer(GLWindow &window, Texture &texture, Texture &depthTexture):
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
  window.glContext->BindFramebuffer(GL_FRAMEBUFFER, id);
  GLcheck(window, "glBindFramebuffer");
  window.glContext->Viewport(0, 0, texture.size.x, texture.size.y);
  GLcheck(window, "glViewport");
};
void Framebuffer::unbind()
{
  window.glContext->BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(window, "glBindFramebuffer");
};