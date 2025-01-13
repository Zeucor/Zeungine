#pragma once
#include "../../../glm.hpp"
#include "../common.hpp"
#include <anex/modules/gl/GLWindow.hpp>
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
		GLWindow &window;
    glm::ivec4 size;
    GLuint id = 0;
    GLenum target = 0;
    Format format;
    Type type;
    FilterType filterType;
    explicit Texture(GLWindow &window, const glm::ivec4 &size, const void *data = 0, const Format &format = RGBA8, const Type &type = UnsignedByte, const FilterType &filterType = Linear);
		explicit Texture(GLWindow &window, const glm::ivec4 &size, const std::string_view &path = "", const Format &format = RGBA8, const Type &type = UnsignedByte, const FilterType &filterType = Linear);
		explicit Texture(GLWindow &window, const glm::ivec4 &size, const std::vector<std::string_view> &paths = {}, const Format &format = RGBA8, const Type &type = UnsignedByte, const FilterType &filterType = Linear);
    void bind() const;
		void unbind() const;
  };
}