#pragma once
#include "./Texture.hpp"
#include "../common.hpp"
namespace anex::modules::gl::textures
{
	struct Framebuffer
  {
    GLuint id = 0;
	const Texture &texture;
    Framebuffer(const Texture &texture);
	void bind() const;
	void unbind() const;
  };
}