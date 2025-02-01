#pragma once
#include <zg/Window.hpp>
#include <zg/glm.hpp>
#include "../common.hpp"
namespace zg::textures
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
		Window& window;
		glm::ivec4 size;
#if defined(USE_GL) || defined(USE_EGL)
		GLuint id = 0;
		GLenum target = 0;
#endif
		Format format;
		Type type;
		FilterType filterType;
		explicit Texture(Window& window, const glm::ivec4& size, const void* data = 0, const Format& format = RGBA8,
										 const Type& type = UnsignedByte, const FilterType& filterType = Linear);
		explicit Texture(Window& window, const glm::ivec4& size, const std::string_view path = "",
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear);
		explicit Texture(Window& window, const glm::ivec4& size, const std::vector<std::string_view>& paths = {},
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear);
		~Texture();
		void bind() const;
		void unbind() const;
	};
} // namespace zg::textures
