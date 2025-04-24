#pragma once
namespace zg
{
	struct IRenderer;
}
#include <map>
#include <memory>
#include <vector>
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
		enum Multisampling
		{
			x1,
			x2,
			x4,
			x8,
			x16,
			x32,
			x64
		};
		IRenderer* iRenderer = 0;
		glm::ivec4 size;
		std::vector<std::pair<size_t, std::shared_ptr<char>>> datas;
		Format format;
		Type type;
		FilterType filterType;
		void* rendererData = 0;
		bool isFramebufferAttachment = false;
		Multisampling multisampling;
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const void* data, const Format& format = RGBA8,
										 const Type& type = UnsignedByte, const FilterType& filterType = Linear,
										 bool isFramebufferAttachment = false, Multisampling multisampling = x1);
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::vector<void*> datas,
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear, bool isFramebufferAttachment = false,
										 Multisampling multisampling = x1);
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::string_view path,
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear, bool isFramebufferAttachment = false,
										 Multisampling multisampling = x1);
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::vector<std::string_view>& paths,
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear, bool isFramebufferAttachment = false,
										 Multisampling multisampling = x1);
		~Texture();
		void bind() const;
		void unbind() const;
		void update(const void* data);
		void update(const std::string_view path);
		void update(const std::vector<std::string_view>& paths);
	};
#if defined(USE_GL) || defined(USE_EGL)
	struct GLTextureImpl
	{
		GLuint id = 0;
		GLenum target = 0;
	};
#endif
} // namespace zg::textures
