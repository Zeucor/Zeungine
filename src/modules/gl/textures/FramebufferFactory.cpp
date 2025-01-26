#include <zg/modules/gl/textures/FramebufferFactory.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include <zg/modules/gl/textures/Texture.hpp>
using namespace zg::modules::gl::textures;
void FramebufferFactory::initFramebuffer(Framebuffer &framebuffer)
{
  if (framebuffer.depthTexturePointer)
  {
    framebuffer.window.glContext->GenRenderbuffers(1, &framebuffer.renderbufferID);
    GLcheck(framebuffer.window, "glGenRenderbuffers");
  }
  framebuffer.window.glContext->GenFramebuffers(1, &framebuffer.id);
  GLcheck(framebuffer.window, "glGenFramebuffers");
  framebuffer.window.glContext->BindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
  GLcheck(framebuffer.window, "glBindFramebuffer");
  GLenum frameBufferTarget;
  switch (framebuffer.texture.format)
  {
    case Texture::Depth:
      frameBufferTarget = GL_DEPTH_ATTACHMENT;

      framebuffer.window.glContext->DrawBuffer(GL_NONE);
      framebuffer.window.glContext->ReadBuffer(GL_NONE);
      break;
    default:
      frameBufferTarget = GL_COLOR_ATTACHMENT0;
      break;
  }
  if (framebuffer.depthTexturePointer)
  {
    framebuffer.window.glContext->BindRenderbuffer(GL_RENDERBUFFER, framebuffer.renderbufferID);
    GLcheck(framebuffer.window, "glBindRenderbuffer");
    framebuffer.window.glContext->RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, framebuffer.depthTexturePointer->size.x, framebuffer.depthTexturePointer->size.y);
    GLcheck(framebuffer.window, "glRenderbufferStorage");
    framebuffer.window.glContext->FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.renderbufferID);
    GLcheck(framebuffer.window, "glFramebufferRenderbuffer");
  }
  if (framebuffer.texture.target == GL_TEXTURE_CUBE_MAP)
  {
    framebuffer.window.glContext->FramebufferTexture(GL_FRAMEBUFFER, frameBufferTarget, framebuffer.texture.id, 0);
    GLcheck(framebuffer.window, "glFramebufferTexture");
  }
  else
  {
    framebuffer.window.glContext->FramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, GL_TEXTURE_2D, framebuffer.texture.id, 0);
    GLcheck(framebuffer.window, "glFramebufferTexture2D");
  }
  framebuffer.window.glContext->BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(framebuffer.window, "glBindFramebuffer");
};
void FramebufferFactory::destroyFramebuffer(Framebuffer &framebuffer)
{
  framebuffer.window.glContext->DeleteFramebuffers(1, &framebuffer.id);
  GLcheck(framebuffer.window, "glDeleteFramebuffers");
};