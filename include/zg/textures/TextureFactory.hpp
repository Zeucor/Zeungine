#pragma once
#include <zg/images/ImageLoader.hpp>
#include "./Texture.hpp"
namespace zg::textures
{
	struct TextureFactory
	{
		struct PairHash
		{
			std::size_t operator()(const std::pair<Texture::Format, Texture::Type>& key) const
			{
				return static_cast<std::size_t>(key.first) ^ (static_cast<std::size_t>(key.second) << 1);
			}
		};
		struct PairEqual
		{
			bool operator()(const std::pair<Texture::Format, Texture::Type>& lhs,
											const std::pair<Texture::Format, Texture::Type>& rhs) const
			{
				return lhs.first == rhs.first && lhs.second == rhs.second;
			}
		};
		using InternalFormatsMap = std::unordered_map<Texture::Format, int32_t>;
		using FormatsMap = std::unordered_map<Texture::Format, int32_t>;
		using TypesMap = std::unordered_map<std::pair<Texture::Format, Texture::Type>, int32_t, PairHash, PairEqual>;
		static InternalFormatsMap internalFormats;
		static FormatsMap formats;
		static TypesMap types;
		static void initTexture(Texture& texture, const void* data);
		static void initTexture(Texture& texture, const std::string_view path);
		static void initTexture(Texture& texture, const std::vector<std::string_view>& paths);
		static void preInitTexture(Texture& texture);
		static void midInitTexture(const Texture& texture, const std::vector<images::ImageLoader::ImagePair>& images);
		static void postInitTexture(const Texture& texture);
		static void destroyTexture(Texture& texture);
	};
} // namespace zg::textures
