#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <zg/entities/Plane.hpp>
#include <zg/enums/EBreakStyle.hpp>
#include <zg/glm.hpp>
#include <zg/interfaces/IFile.hpp>
#include <zg/strings/Utf8Iterator.hpp>
#include <zg/textures/Texture.hpp>
#undef min
#undef max
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
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
		std::vector<glm::vec2> uv2s;
		FreetypeCharacter(IRenderer* iRenderer, const FreetypeFont& freeTypeFont, FT_UInt glyph_index, float fontSize, bool msdf = false);
	};
	struct FreetypeFont
	{
		static FT_Library freetypeLibrary;
		static bool freetypeLoaded;
		msdfgen::FontHandle* fontHandlePointer;
    	hb_font_t* hbFont = 0;
		std::shared_ptr<int8_t> fontFileBytes;
		std::unordered_map<FT_UInt, std::unordered_map<float, FreetypeCharacter>> glyphIndexFontSizeCharacters;
		std::unordered_map<FT_UInt, std::unordered_map<float, FreetypeCharacter>> glyphIndexFontSizeMSDFCharacters;
		IRenderer* iRenderer;
		std::filesystem::path fontPath;
		bool hasKerning;
		/*add member variables in serial order*/
		FreetypeFont(IRenderer* iRenderer, interfaces::IFile& fontFile);
		~FreetypeFont();
		float calculateSegmentWidth(const std::string_view segment, float fontSize) const;
		const glm::vec2 stringSize(const std::string_view string, float fontSize, float& lineHeight, glm::vec2 bounds,
									enums::EBreakStyle breakStyle = enums::EBreakStyle::None, bool msdf = false);
		template <typename HostT>
		void stringToHost(const std::string_view string, glm::vec3 position, glm::quat _rotation,
						glm::vec3 _scale, float fontSize, float& lineHeight, glm::vec2 bounds,
						enums::EBreakStyle breakStyle, HostT& host,
						std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex,
						size_t& cursor, bool msdf = false);
		FreetypeCharacter& getCharacter(FT_UInt glyph_index, float fontSize, bool msdf = false);
		static void FT_PRINT_AND_THROW_ERROR(const FT_Error& error, const std::string& fontPath);
	};
} // namespace zg::fonts::freetype
namespace msdfgen
{
	class FontHandle
	{
	public:
		FT_Face face;
		bool ownership;
	};
} // namespace msdfgen