#include <zg/Window.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg::textures;
void FramebufferFactory::initFramebuffer(Framebuffer &framebuffer)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(framebuffer.window.iVendorRenderer);
  if (framebuffer.depthTexturePointer)
  {
    glRenderer.glContext->GenRenderbuffers(1, &framebuffer.renderbufferID);
    GLcheck(glRenderer, "glGenRenderbuffers");
  }
  glRenderer.glContext->GenFramebuffers(1, &framebuffer.id);
  GLcheck(glRenderer, "glGenFramebuffers");
  glRenderer.glContext->BindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
  GLcheck(glRenderer, "glBindFramebuffer");
  GLenum frameBufferTarget;
  switch (framebuffer.texture.format)
  {
    case Texture::Depth:
      frameBufferTarget = GL_DEPTH_ATTACHMENT;

      glRenderer.glContext->DrawBuffer(GL_NONE);
      glRenderer.glContext->ReadBuffer(GL_NONE);
      break;
    default:
      frameBufferTarget = GL_COLOR_ATTACHMENT0;
      break;
  }
  if (framebuffer.depthTexturePointer)
  {
    glRenderer.glContext->BindRenderbuffer(GL_RENDERBUFFER, framebuffer.renderbufferID);
    GLcheck(glRenderer, "glBindRenderbuffer");
    glRenderer.glContext->RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, framebuffer.depthTexturePointer->size.x, framebuffer.depthTexturePointer->size.y);
    GLcheck(glRenderer, "glRenderbufferStorage");
    glRenderer.glContext->FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.renderbufferID);
    GLcheck(glRenderer, "glFramebufferRenderbuffer");
  }
  if (framebuffer.texture.target == GL_TEXTURE_CUBE_MAP)
  {
    glRenderer.glContext->FramebufferTexture(GL_FRAMEBUFFER, frameBufferTarget, framebuffer.texture.id, 0);
    GLcheck(glRenderer, "glFramebufferTexture");
  }
  else
  {
    glRenderer.glContext->FramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, GL_TEXTURE_2D, framebuffer.texture.id, 0);
    GLcheck(glRenderer, "glFramebufferTexture2D");
  }
  glRenderer.glContext->BindFramebuffer(GL_FRAMEBUFFER, 0);
  GLcheck(glRenderer, "glBindFramebuffer");
};
void FramebufferFactory::destroyFramebuffer(Framebuffer &framebuffer)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(framebuffer.window.iVendorRenderer);
  glRenderer.glContext->DeleteFramebuffers(1, &framebuffer.id);
  GLcheck(glRenderer, "glDeleteFramebuffers");
};