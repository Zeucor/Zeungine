#pragma once
#include "../common.hpp"
namespace zg
{
	struct Window;
}
namespace zg::textures
{
	struct Texture;
	struct Framebuffer
  {
		Window &window;
    GLuint id = 0;
		GLuint renderbufferID = 0;
		Texture &texture;
		Texture *depthTexturePointer = 0;
    Framebuffer(Window &window, Texture &texture);
    Framebuffer(Window &window, Texture &texture, Texture &depthTexture);
		~Framebuffer();
		void bind() const;
		void unbind();
  };
}