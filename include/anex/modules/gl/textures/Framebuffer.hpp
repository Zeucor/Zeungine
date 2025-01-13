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
		const Texture &texture;
    Framebuffer(GLWindow &window, const Texture &texture);
		void bind() const;
		void unbind();
  };
}