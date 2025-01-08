#include <anex/modules/gl/textures/FramebufferFactory.hpp>
using namespace anex::modules::gl::textures;
void FramebufferFactory::initFramebuffer(Framebuffer &framebuffer)
{
  glGenFramebuffers(1, &framebuffer.id);
  GLcheck("glGenFramebuffers");
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
  GLcheck("glBindFramebuffer");
  GLenum frameBufferTarget;
  switch (framebuffer.texture.format)
  {
    case Texture::Depth:
      frameBufferTarget = GL_DEPTH_ATTACHMENT;

      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      break;
  }
  if (framebuffer.texture.target == GL_TEXTURE_CUBE_MAP)
  {
    glFramebufferTexture(GL_FRAMEBUFFER, frameBufferTarget, framebuffer.texture.id, 0);
    GLcheck("glFramebufferTexture");
  }
  else
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, GL_TEXTURE_2D, framebuffer.texture.id, 0);
    GLcheck("glFramebufferTexture2D");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck("glBindFramebuffer");
};