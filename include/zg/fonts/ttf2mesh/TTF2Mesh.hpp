#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <zg/enums/EBreakStyle.hpp>
#include <zg/glm.hpp>
#include <zg/interfaces/IFile.hpp>
#include <zg/strings/Utf8Iterator.hpp>
#include <zg/textures/Texture.hpp>
#include <ttf2mesh.h>
#include <zg/Entity.hpp>
namespace zg
{
	struct Window;
    struct Scene;
    struct Entity;
}
namespace zg::fonts::ttf2mesh
{
	struct TTF2MeshFont;
	struct TTF2MeshGlyph
	{
		size_t glyphIndex;
        float codepoint;
		glm::ivec2 size;
		glm::ivec2 bearing;
		unsigned int advance;
        EntityCreateInfo entityCreateInfo;
        ttf_mesh3d_t* mesh = 0;
        size_t* meshRefCount = 0;
		uint32_t vertexCount;
		TTF2MeshGlyph(IRenderer* iRenderer, const TTF2MeshFont& ttf2MeshFont, float codepoint, float fontSize);
        TTF2MeshGlyph(const TTF2MeshGlyph& other);
        TTF2MeshGlyph& operator=(const TTF2MeshGlyph& other);
        ~TTF2MeshGlyph();
	};
	struct TTF2MeshFont
	{
		std::shared_ptr<FT_Face> ftFacePointer;
		ttf_t* facePointer;
		std::shared_ptr<int8_t> ftFontFileBytes;
		std::shared_ptr<int8_t> fontFileBytes;
		std::unordered_map<float, std::unordered_map<float, TTF2MeshGlyph>> codepointFontSizeCharacters;
		IRenderer* iRenderer;
		std::filesystem::path fontPath;
		bool hasKerning;
		/*add member variables in serial order*/
		TTF2MeshFont(IRenderer* iRenderer, interfaces::IFile& fontFile);
        ~TTF2MeshFont();
		const glm::vec2 stringSize(const std::string_view string, float fontSize, float& lineHeight, glm::vec2 bounds,
															 enums::EBreakStyle breakStyle = enums::EBreakStyle::None);
		void stringToScene(const std::string_view string, glm::vec3 position, glm::vec4 color, glm::quat _rotation,
											 glm::vec3 _scale, float fontSize, float& lineHeight, glm::vec2 bounds,
											 enums::EBreakStyle breakStyle, Scene& scene,
											 std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex,
											 size_t& cursor, const shaders::RuntimeConstants& constants = {});
		void stringToEntity(const std::string_view string, glm::vec3 position, glm::vec4 color, glm::quat _rotation,
												glm::vec3 _scale, float fontSize, float& lineHeight, glm::vec2 bounds,
												enums::EBreakStyle breakStyle, Scene& scene, Entity& entity,
												std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex,
												size_t& cursor, const shaders::RuntimeConstants& constants = {});
		TTF2MeshGlyph& getCharacter(float codepoint, float fontSize);
		void addNextKerning(float fontSize, FT_UInt currentGlyphIndex, zg::strings::Utf8Iterator iterator, float& advanceX, float scaleX);
		float shouldAdvanceLine(zg::strings::Utf8Iterator iterator, glm::vec2 currentPosition, float advanceX,
														enums::EBreakStyle breakStyle, float boundsX, float scaleX, float startX, float fontSize);
	};
} // namespace zg::fonts::freetype
