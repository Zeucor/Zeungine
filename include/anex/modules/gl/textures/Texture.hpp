#pragma once
#include "../../../glm.hpp"
#include "../common.hpp"
namespace anex::modules::gl::textures
{
	struct Texture
	{
		enum Format
		{
			RGB8 = 1,
			RGBA8,
			RGBA32F,
			Depth,
			Stencil,
			DepthStencil,
			Integer32
		};
		enum Type
		{
			UnsignedByte = 1,
			UnsignedInt24_8,
			Float,
			Int
		};
		enum FilterType
		{
			Linear = 1,
			Nearest
		};
    glm::ivec3 size;
    GLuint id = 0;
    GLenum target;
    Format format;
    Type type;
    FilterType filterType;
    Texture(const glm::ivec3 &size, const void *data = 0, const Format &format = RGBA8, const Type &type = UnsignedByte, const FilterType &filterType = Linear);
    void bind() const;
	void unbind() const;
  };
}