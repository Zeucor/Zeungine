#pragma once
#include <array>
#include <unordered_map>
#include <zg/interfaces/IFile.hpp>
#include <zg/textures/Texture.hpp>
namespace zg::images
{
	struct SpriteSheet
	{
	private:
		// keyedUVs stores a vec4. x/y is bottomLeft, z/w is topRight
		std::unordered_map<std::string, glm::vec4> keyedUVs;
		std::shared_ptr<textures::Texture> spriteTexture;

	public:
		/**
		 * @brief Constructor for SpriteSheet
		 * @param bgColor is a uvec with values ranging from 0-255. Any pixel in the sheet with this color will be set to
		 * transparent
		 * @param spriteKeyCoords is a vector containing pairs of string keys and vec4 values. x/y is sprite w/h, z/w is
		 * sprite topleft
		 */
		SpriteSheet(IRenderer* iRenderer, const interfaces::IFile& file, uvec bgColor,
								const std::vector<std::pair<std::string, glm::vec4>>& spriteKeyCoords);
		void loadSheet(IRenderer* iRenderer, interfaces::IFile& file, uvec bgColor);
		void parseSheet(const std::vector<std::pair<std::string, glm::vec4>>& spriteKeyCoords);
		// Given a key, extract the UV and return 4 uvs in order BL-BR-TR-TL
		std::array<glm::vec2, 4> getUVForKey(const std::string& key);
		std::shared_ptr<textures::Texture> getTexture() const;
	};
} // namespace zg::images
