#pragma once
#include "./Texture.hpp"
#include "../common.hpp"
#include "../GLWindow.hpp"
namespace anex::modules::gl::textures
{
	struct Framebuffer
  {
		GLWindow &window;
    GLuint id = 0;
		Texture &texture;
    Framebuffer(GLWindow &window, Texture &texture);
		~Framebuffer();
		void bind() const;
		void unbind();
  };
}