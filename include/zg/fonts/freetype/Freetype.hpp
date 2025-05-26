#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <zg/entities/Plane.hpp>
#include <zg/enums/EBreakStyle.hpp>
#include <zg/glm.hpp>
#include <zg/interfaces/IFile.hpp>
#include <zg/strings/Utf8Iterator.hpp>
#include <zg/textures/Texture.hpp>
namespace zg
{
	struct Window;
}
namespace zg::fonts::freetype
{
	struct FreetypeFont;
	struct FreetypeCharacter
	{
		FT_UInt glyphIndex;
		std::shared_ptr<textures::Texture> texturePointer;
		glm::ivec2 size;
		glm::ivec2 bearing;
		unsigned int advance;
		FreetypeCharacter(IRenderer* iRenderer, const FreetypeFont& freeTypeFont, float codepoint, float fontSize);
	};
	struct FreetypeFont
	{
		static FT_Library freetypeLibrary;
		static bool freetypeLoaded;
		std::shared_ptr<FT_Face> facePointer;
		std::shared_ptr<int8_t> fontFileBytes;
		std::unordered_map<float, std::unordered_map<float, FreetypeCharacter>> codepointFontSizeCharacters;
		IRenderer* iRenderer;
		std::filesystem::path fontPath;
		bool hasKerning;
		/*add member variables in serial order*/
		FreetypeFont(IRenderer* iRenderer, interfaces::IFile& fontFile);
		const glm::vec2 stringSize(const std::string_view string, float fontSize, float& lineHeight, glm::vec2 bounds,
															 enums::EBreakStyle breakStyle = enums::EBreakStyle::None);
		void stringToTexture(const std::string_view string, glm::vec4 color, float fontSize, float& lineHeight,
												 glm::vec2 textureSize, std::shared_ptr<textures::Texture>& texturePointer,
												 const int64_t& cursorIndex, glm::vec3& cursorPosition,
												 enums::EBreakStyle breakStyle = enums::EBreakStyle::None,
												 const std::shared_ptr<textures::Framebuffer>& framebufferPointer = {},
												 const std::shared_ptr<Scene>& scenePointer = {});
		template <typename HostT>
		void stringToHost(const std::string_view string, glm::vec3 position, glm::vec4 color, glm::quat _rotation,
												glm::vec3 _scale, float fontSize, float& lineHeight, glm::vec2 bounds,
												enums::EBreakStyle breakStyle, HostT& host,
												std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex,
												size_t& cursor);
		FreetypeCharacter& getCharacter(float codepoint, float fontSize);
		static void FT_PRINT_AND_THROW_ERROR(const FT_Error& error, const std::string& fontPath);
		void addNextKerning(float fontSize, FT_UInt currentGlyphIndex, zg::strings::Utf8Iterator iterator, float& advanceX, float scaleX);
		float shouldAdvanceLine(zg::strings::Utf8Iterator iterator, glm::vec2 currentPosition, float advanceX,
														enums::EBreakStyle breakStyle, float boundsX, float scaleX, float startX, float fontSize);
	};
} // namespace zg::fonts::freetype
