#include <anex/modules/gl/textures/Framebuffer.hpp>
#include <anex/modules/gl/textures/FramebufferFactory.hpp>
using namespace anex::modules::gl::textures;
Framebuffer::Framebuffer(GLWindow &window, const Texture &texture):
  window(window),
  texture(texture)
{
  FramebufferFactory::initFramebuffer(*this);
};
void Framebuffer::bind() const
{
  window.glContext.BindFramebuffer(GL_FRAMEBUFFER, id);
  GLcheck(window, "glBindFramebuffer");
  window.glContext.Viewport(0, 0, texture.size.x, texture.size.y);
  GLcheck(window, "glViewport");
  window.glContext.ClearColor(0, 0, 0, 0);
  GLcheck(window, "glClearColor");
  GLbitfield clearBitfield = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
  window.glContext.Clear(clearBitfield);
  GLcheck(window, "glClear");
};
void Framebuffer::unbind()
{
  window.glContext.BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(window, "glBindFramebuffer");
};