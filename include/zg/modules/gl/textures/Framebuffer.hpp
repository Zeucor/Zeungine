#pragma once
#include "../common.hpp"
namespace zg::modules::gl
{
	struct GLWindow;
}
namespace zg::modules::gl::textures
{
	struct Texture;
	struct Framebuffer
  {
		GLWindow &window;
    GLuint id = 0;
		GLuint renderbufferID = 0;
		Texture &texture;
		Texture *depthTexturePointer = 0;
    Framebuffer(GLWindow &window, Texture &texture);
    Framebuffer(GLWindow &window, Texture &texture, Texture &depthTexture);
		~Framebuffer();
		void bind() const;
		void unbind();
  };
}