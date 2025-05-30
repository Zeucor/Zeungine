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
			R8 = 1,
			RG8,
			RGB8,
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
		enum AddressMode
		{
			ClampToEdge,
			ClampToBorder,
			Repeat
		};
		IRenderer* iRenderer = 0;
		glm::ivec4 size = glm::ivec4(0);
		std::vector<std::pair<size_t, std::shared_ptr<char>>> datas;
		Format format = RGBA8;
		Type type = UnsignedByte;
		FilterType filterType = Linear;
		void* rendererData = 0;
		bool isFramebufferAttachment = false;
		Multisampling multisampling = x1;
		bool isTransparent = false;
		AddressMode addressMode = Repeat;
		bool flip = false;
		Texture() = default;
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const void* data, const Format& format = RGBA8,
										 const Type& type = UnsignedByte, const FilterType& filterType = Linear,
										 bool isFramebufferAttachment = false, Multisampling multisampling = x1,
										 AddressMode addressMode = AddressMode::Repeat,
										bool flip = false);
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::vector<void*> datas,
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear, bool isFramebufferAttachment = false,
										 Multisampling multisampling = x1,
										 AddressMode addressMode = AddressMode::Repeat,
										bool flip = false);
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::string_view path,
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear, bool isFramebufferAttachment = false,
										 Multisampling multisampling = x1,
										 AddressMode addressMode = AddressMode::Repeat,
										bool flip = false);
		explicit Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::vector<std::string_view>& paths,
										 const Format& format = RGBA8, const Type& type = UnsignedByte,
										 const FilterType& filterType = Linear, bool isFramebufferAttachment = false,
										 Multisampling multisampling = x1,
										 AddressMode addressMode = AddressMode::Repeat,
										bool flip = false);
		~Texture();
		void bind() const;
		void unbind() const;
		void update(const void* data);
		void update(const std::string_view path);
		void update(const std::vector<std::string_view>& paths);
		bool testIsTransparent(const void* data);
	};
#if defined(USE_GL) || defined(USE_EGL)
	struct GLTextureImpl
	{
		GLuint id = 0;
		GLenum target = 0;
	};
#endif
} // namespace zg::textures
#define DEFAULT_TEXTURE_FORMAT zg::textures::Texture::Format::RGBA8
#define DEFAULT_TEXTURE_TYPE zg::textures::Texture::Type::UnsignedByte
#define DEFAULT_TEXTURE_FILTERTYPE zg::textures::Texture::FilterType::Linear
#define DEFAULT_TEXTURE_MULTISAMPLING zg::textures::Texture::Multisampling::x1
#define DEFAULT_TEXTURE_ADDRESS_MODE zg::textures::Texture::AddressMode::Repeat
#define TEXTURE_CLAMP_EDGE zg::textures::Texture::AddressMode::ClampToEdge
#define TEXTURE_REPEAT zg::textures::Texture::AddressMode::Repeat
#define TEXTURE_CLAMP_BORDER zg::textures::Texture::AddressMode::ClampToBorder