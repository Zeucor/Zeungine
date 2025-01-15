#include <anex/modules/gl/textures/Framebuffer.hpp>
#include <anex/modules/gl/textures/FramebufferFactory.hpp>
using namespace anex::modules::gl::textures;
Framebuffer::Framebuffer(GLWindow &window, Texture &texture):
  window(window),
  texture(texture)
{
  FramebufferFactory::initFramebuffer(*this);
};
Framebuffer::~Framebuffer()
{
  FramebufferFactory::destroyFramebuffer(*this);
};
void Framebuffer::bind() const
{
  window.glContext.BindFramebuffer(GL_FRAMEBUFFER, id);
  GLcheck(window, "glBindFramebuffer");
  window.glContext.Viewport(0, 0, texture.size.x, texture.size.y);
  GLcheck(window, "glViewport");
};
void Framebuffer::unbind()
{
  window.glContext.BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(window, "glBindFramebuffer");
};