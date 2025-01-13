#include <anex/modules/gl/textures/FramebufferFactory.hpp>
using namespace anex::modules::gl::textures;
void FramebufferFactory::initFramebuffer(Framebuffer &framebuffer)
{
  framebuffer.window.glContext.GenFramebuffers(1, &framebuffer.id);
  GLcheck(framebuffer.window, "glGenFramebuffers");
  framebuffer.window.glContext.BindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
  GLcheck(framebuffer.window, "glBindFramebuffer");
  GLenum frameBufferTarget;
  switch (framebuffer.texture.format)
  {
    case Texture::Depth:
      frameBufferTarget = GL_DEPTH_ATTACHMENT;

      framebuffer.window.glContext.DrawBuffer(GL_NONE);
      framebuffer.window.glContext.ReadBuffer(GL_NONE);
      break;
  }
  if (framebuffer.texture.target == GL_TEXTURE_CUBE_MAP)
  {
    framebuffer.window.glContext.FramebufferTexture(GL_FRAMEBUFFER, frameBufferTarget, framebuffer.texture.id, 0);
    GLcheck(framebuffer.window, "glFramebufferTexture");
  }
  else
  {
    framebuffer.window.glContext.FramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, GL_TEXTURE_2D, framebuffer.texture.id, 0);
    GLcheck(framebuffer.window, "glFramebufferTexture2D");
  }
  framebuffer.window.glContext.BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(framebuffer.window, "glBindFramebuffer");
};