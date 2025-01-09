#include <anex/modules/gl/textures/Framebuffer.hpp>
#include <anex/modules/gl/textures/FramebufferFactory.hpp>
using namespace anex::modules::gl::textures;
Framebuffer::Framebuffer(const Texture &texture):
  texture(texture)
{
  FramebufferFactory::initFramebuffer(*this);
};
void Framebuffer::bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, id);
  GLcheck("glBindFramebuffer");
  glViewport(0, 0, texture.size.x, texture.size.y);
  GLcheck("glViewport");
  glClearColor(0, 0, 0, 0);
  GLcheck("glClearColor");
  GLbitfield clearBitfield = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
  glClear(clearBitfield);
  GLcheck("glClear");
};
void Framebuffer::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck("glBindFramebuffer");
};