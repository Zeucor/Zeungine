#pragma once
#include "../common.hpp"
namespace zg::modules::gl
{
	struct RenderWindow;
}
namespace zg::modules::gl::textures
{
	struct Texture;
	struct Framebuffer
  {
		RenderWindow &window;
    GLuint id = 0;
		GLuint renderbufferID = 0;
		Texture &texture;
		Texture *depthTexturePointer = 0;
    Framebuffer(RenderWindow &window, Texture &texture);
    Framebuffer(RenderWindow &window, Texture &texture, Texture &depthTexture);
		~Framebuffer();
		void bind() const;
		void unbind();
  };
}