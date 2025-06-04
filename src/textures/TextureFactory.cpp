#include <zg/interfaces/IRenderer.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <stdexcept>
using namespace zg::textures;
#if defined(USE_GL) || defined(USE_EGL)
TextureFactory::InternalFormatsMap TextureFactory::internalFormats = {
	{Texture::Format::RGB8, GL_RGB8},
	{Texture::Format::RGBA8, GL_RGBA8},
	{Texture::Format::RGBA32F, GL_RGBA32F},
	{Texture::Format::Depth, GL_DEPTH_COMPONENT32F},
	{Texture::Format::Stencil, GL_STENCIL_INDEX8},
	{Texture::Format::DepthStencil, GL_DEPTH32F_STENCIL8},
	{Texture::Format::Integer32, GL_R32I}};
TextureFactory::FormatsMap TextureFactory::formats = {{Texture::Format::RGB8, GL_RGB},
																											{Texture::Format::RGBA8, GL_RGBA},
																											{Texture::Format::RGBA32F, GL_RGBA},
																											{Texture::Format::Depth, GL_DEPTH_COMPONENT},
#ifdef USE_GL
																											{Texture::Format::Stencil, GL_STENCIL_INDEX},
#endif
#ifdef USE_EGL
																											{Texture::Format::Stencil, GL_STENCIL_INDEX8},
#endif
																											{Texture::Format::DepthStencil, GL_DEPTH_STENCIL},
																											{Texture::Format::Integer32, GL_RED_INTEGER}};
TextureFactory::TypesMap TextureFactory::types = {
	{{Texture::Format::RGB8, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
	{{Texture::Format::RGBA8, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
	{{Texture::Format::RGBA8, Texture::Type::Float}, ZG_FLOAT},
	{{Texture::Format::RGBA32F, Texture::Type::Float}, ZG_FLOAT},
	{{Texture::Format::DepthStencil, Texture::Type::UnsignedInt24_8}, ZG_UNSIGNED_INT_24_8},
	{{Texture::Format::DepthStencil, Texture::Type::Float}, ZG_FLOAT_32_UNSIGNED_INT_24_8_REV},
	{{Texture::Format::Stencil, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
	{{Texture::Format::Depth, Texture::Type::Float}, ZG_FLOAT},
	{{Texture::Format::Integer32, Texture::Type::Int}, GL_INT}};
#endif
void TextureFactory::initTexture(Texture& texture, const void* data)
{
	if (!texture.iRenderer)
		return;
	preInitTexture(texture);
	if (texture.flip)
		flipTextureDataY(texture, (void*)data);
	auto imageCount = texture.size.w > 0 ? 6 : texture.size.z;
	std::vector<images::ImageLoader::ImagePair> imagePairs;
	imagePairs.reserve(imageCount);
	for (int i = 0; i < imageCount; i++)
	{
		imagePairs.push_back({{texture.size.x, texture.size.y}, {(uint8_t*)data, [](uint8_t*) {}}});
		if (data)
		{
			auto [channels, sizeoftype] = getChannelsSizeOfType(texture);
			auto bytessize = texture.size.x * texture.size.y * channels * sizeoftype;
			texture.datas.push_back(
				std::pair<size_t, std::shared_ptr<char>>(bytessize, std::shared_ptr<char>((char*)data, [](auto p){})));
		}
	}
	midInitTexture(texture, imagePairs);
	postInitTexture(texture);
}
void TextureFactory::initTexture(Texture& texture, const std::vector<void*> datas)
{
	if (!texture.iRenderer)
		return;
	preInitTexture(texture);
	auto imageCount = texture.size.w > 0 ? 6 : texture.size.z;
	std::vector<images::ImageLoader::ImagePair> imagePairs;
	imagePairs.reserve(imageCount);
	for (int i = 0; i < imageCount; i++)
	{
		if (texture.flip)
			flipTextureDataY(texture, datas[i]);
		imagePairs.push_back({{texture.size.x, texture.size.y}, {(uint8_t*)datas[i], [](uint8_t*) {}}});
		if (datas[i])
		{
			auto [channels, sizeoftype] = getChannelsSizeOfType(texture);
			auto bytessize = texture.size.x * texture.size.y * channels * sizeoftype;
			texture.datas.push_back(
				std::pair<size_t, std::shared_ptr<char>>(bytessize, std::shared_ptr<char>((char*)datas[i], [](auto p){})));
		}
	}
	midInitTexture(texture, imagePairs);
	postInitTexture(texture);
}
void TextureFactory::initTexture(Texture& texture, const std::string_view path)
{
	if (!texture.iRenderer)
		return;
	preInitTexture(texture);
	auto imagePair = images::ImageLoader::load(path);
	midInitTexture(texture, {{imagePair}});
	postInitTexture(texture);
};
void TextureFactory::initTexture(Texture& texture, const std::vector<std::string_view>& paths)
{
	if (!texture.iRenderer)
		return;
	std::vector<images::ImageLoader::ImagePair> imagePairs;
	imagePairs.reserve(paths.size());
	for (const auto& path : paths)
	{
		imagePairs.push_back(images::ImageLoader::load(path));
	}
	if (!texture.size.x || !texture.size.y && imagePairs.size())
	{
		auto firstImageSize = imagePairs[0].first;
		texture.size.x = firstImageSize.x;
		texture.size.y = firstImageSize.y;
	}
	preInitTexture(texture);
	midInitTexture(texture, imagePairs);
	postInitTexture(texture);
};
void TextureFactory::preInitTexture(Texture& texture) { texture.iRenderer->preInitTexture(texture); };
void TextureFactory::midInitTexture(const Texture& texture, const std::vector<images::ImageLoader::ImagePair>& images)
{
	texture.iRenderer->midInitTexture(texture, images);
}
void TextureFactory::postInitTexture(const Texture& texture) { texture.iRenderer->postInitTexture(texture); }
void TextureFactory::destroyTexture(Texture& texture)
{
	if (texture.iRenderer)
		texture.iRenderer->destroyTexture(texture);
}
std::pair<int, size_t> TextureFactory::getChannelsSizeOfType(const Texture& texture)
{
	int channels = 0;
	size_t sizeoftype = 0;
	switch (texture.format)
	{
	case textures::Texture::Format::RGB8:
		channels = 3;
		sizeoftype = 1;
		break;
	case textures::Texture::Format::Depth:
	case textures::Texture::Format::DepthStencil:
	case textures::Texture::Format::RGBA32F:
		channels = 4;
		sizeoftype = sizeof(float);
		break;
	case textures::Texture::Format::RGBA8:
		channels = 4;
		sizeoftype = 1;
		break;
	case textures::Texture::Format::RG8:
		channels = 2;
		sizeoftype = 1;
		break;
	case textures::Texture::Format::R8:
	case textures::Texture::Format::Stencil:
		channels = 1;
		sizeoftype = 1;
		break;
	}
	return {channels, sizeoftype};
}
void TextureFactory::flipTextureDataY(const Texture& texture, void* data)
{
	auto& size_x = texture.size.x;
	auto& size_y = texture.size.y;
	auto& size_z = texture.size.z; // 1 for 2D texture, 3D texture >= 1
	auto [channels, sizeoftype] = getChannelsSizeOfType(texture);
	// Basic validation
	if (channels == 0 || sizeoftype == 0)
	{
		throw std::runtime_error("Error: Invalid texture format or data type detected. Cannot flip data.");
	}
	// Calculate the size of a single row (or scanline) in bytes
	// For 3D textures, this is the size of a row within a single slice (Z-plane).
	const size_t row_size_bytes = static_cast<size_t>(size_x) * channels * sizeoftype;
	// Cast the void* data pointer to a byte pointer for byte-level arithmetic
	uint8_t* byte_data = static_cast<uint8_t*>(data);
	// Create a temporary buffer to hold one row of data during the swap
	// Using a vector<uint8_t> is flexible and handles memory management.
	std::vector<uint8_t> temp_row_buffer(row_size_bytes);
	// Iterate through each Z-slice for 3D textures (or just once for 2D textures where size_z is 1)
	for (unsigned int z = 0; z < size_z; ++z)
	{
		// Calculate the starting offset for the current Z-slice
		// A slice consists of size_x * size_y * channels * sizeoftype bytes
		const size_t slice_offset_bytes = static_cast<size_t>(z) * size_x * size_y * channels * sizeoftype;
		// Perform the vertical flip for the current Z-slice
		// We iterate from the top row down to the middle row (inclusive)
		// and swap it with the corresponding bottom row.
		for (unsigned int y = 0; y < size_y / 2; ++y)
		{
			// Calculate the byte offset for the current top row within the slice
			const size_t top_row_offset_in_slice = static_cast<size_t>(y) * row_size_bytes;
			const size_t top_row_global_offset = slice_offset_bytes + top_row_offset_in_slice;
			// Calculate the byte offset for the corresponding bottom row within the slice
			// The bottom row is (size_y - 1 - y)
			const size_t bottom_row_offset_in_slice = static_cast<size_t>(size_y - 1 - y) * row_size_bytes;
			const size_t bottom_row_global_offset = slice_offset_bytes + bottom_row_offset_in_slice;
			// 1. Copy the top row data to the temporary buffer
			std::copy(byte_data + top_row_global_offset, byte_data + top_row_global_offset + row_size_bytes,
								temp_row_buffer.begin());
			// 2. Copy the bottom row data to the top row's position
			std::copy(byte_data + bottom_row_global_offset, byte_data + bottom_row_global_offset + row_size_bytes,
								byte_data + top_row_global_offset);
			// 3. Copy the data from the temporary buffer (original top row) to the bottom row's position
			std::copy(temp_row_buffer.begin(), temp_row_buffer.end(), byte_data + bottom_row_global_offset);
		}
	}
}
void TextureFactory::updateTexture(Texture& texture, const void* data)
{
	if (!texture.iRenderer)
		return;
	if (texture.flip)
		flipTextureDataY(texture, (void*)data);
	auto imageCount = texture.size.w > 0 ? 6 : texture.size.z;
	std::vector<images::ImageLoader::ImagePair> imagePairs;
	imagePairs.reserve(imageCount);
	for (int i = 0; i < imageCount; i++)
	{
		imagePairs.push_back({{texture.size.x, texture.size.y}, {(uint8_t*)data, [](uint8_t*) {}}});
		auto [channels, sizeoftype] = getChannelsSizeOfType(texture);
		auto bytessize = texture.size.x * texture.size.y * channels * sizeoftype;
		texture.datas.push_back(
			std::pair<size_t, std::shared_ptr<char>>(bytessize, std::shared_ptr<char>((char*)data, [](auto p){})));
	}
	midInitTexture(texture, imagePairs);
}
void TextureFactory::updateTexture(Texture& texture, const std::string_view path)
{
	if (!texture.iRenderer)
		return;
	auto imagePair = images::ImageLoader::load(path);
	midInitTexture(texture, {{imagePair}});
}
void TextureFactory::updateTexture(Texture& texture, const std::vector<std::string_view>& paths)
{
	if (!texture.iRenderer)
		return;
	std::vector<images::ImageLoader::ImagePair> imagePairs;
	imagePairs.reserve(paths.size());
	for (const auto& path : paths)
	{
		imagePairs.push_back(images::ImageLoader::load(path));
	}
	if (!texture.size.x || !texture.size.y && imagePairs.size())
	{
		auto firstImageSize = imagePairs[0].first;
		texture.size.x = firstImageSize.x;
		texture.size.y = firstImageSize.y;
	}
	midInitTexture(texture, imagePairs);
}