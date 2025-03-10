#include <iostream>
#include <stdexcept>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/strings/Utf8Iterator.hpp>
using namespace std;
using namespace zg::fonts::freetype;
FT_Library FreetypeFont::freetypeLibrary;
bool FreetypeFont::freetypeLoaded = ([]()
									 {
	if (FT_Init_FreeType(&freetypeLibrary))
  {
    throw runtime_error("Failed to initialize freetype library");
  }
	return true; })();
struct ft_error
{
	int err;
	const char* str;
};
#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s) {(e), (s)},
#define FT_ERROR_START_LIST
#define FT_ERROR_END_LIST {0, NULL}
static const struct ft_error ft_errors[] = {
#include FT_ERRORS_H
};
const char* ft_errorstring(int err)
{
	const struct ft_error* e;

	for (e = ft_errors; e->str != NULL; e++)
		if (e->err == err)
			return e->str;

	return "Unknown error";
};
void FreetypeFont::FT_PRINT_AND_THROW_ERROR(const FT_Error& error, const string& fontPath)
{
	if (error)
	{
		auto errorString = "Error loading font[" + fontPath + "]" + string(ft_errorstring(error));
		cout << errorString << endl;
		throw runtime_error(errorString);
	}
};
FreetypeCharacter::FreetypeCharacter(Window& window, const FreetypeFont& freeTypeFont, float codepoint, float fontSize)
{
	auto& face = *freeTypeFont.facePointer.get();
	glyphIndex = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	auto loadCharCode = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL | FT_LOAD_COLOR);
	if (loadCharCode)
	{
		throw runtime_error("Failed to load glyph: " + to_string(FT_Get_Char_Index(face, glyphIndex)));
	}
	size = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
	bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
	auto& renderer = window.iRenderer->renderer;
	if (size.x == 0 || size.y == 0)
	{
		goto _setAdvance;
	}
	{
		auto flipY = (renderer == RENDERER_GL || renderer == RENDERER_EGL);
		uint64_t imgSize = size.x * size.y * 4;
		shared_ptr<uint8_t[]> rgbaImg(new uint8_t[imgSize]);
		auto rgbaImgPointer = rgbaImg.get();
		for (int64_t imgY = flipY ? size.y - 1 : 0, rgbaImgY = 0; flipY ? imgY >= 0 : imgY < size.y;
				 flipY ? imgY-- : imgY++, rgbaImgY++)
		{
			for (uint64_t imgX = 0; imgX < size.x; imgX++)
			{
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 0] = 255;
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 1] = 255;
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 2] = 255;
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 3] =
					face->glyph->bitmap.buffer[(imgY * face->glyph->bitmap.pitch + imgX)];
				continue;
			}
		}
		texturePointer.reset(new textures::Texture(window, {size.x, size.y, 1, 0}, rgbaImgPointer,
																							 textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte,
																							 textures::Texture::FilterType::Nearest));
	}
_setAdvance:
	advance = face->glyph->advance.x;
};
FreetypeFont::FreetypeFont(Window& window, interfaces::IFile& fontFile) :
		facePointer(new FT_Face,
								[](FT_Face* pointer)
								{
									FT_Done_Face(*pointer);
									delete pointer;
								}),
		window(window), fontPath(fontFile.filePath)
{
	fontFileBytes = fontFile.toBytes();
	auto fontFileSize = fontFile.size();
	auto actualFacePointer = facePointer.get();
	FT_PRINT_AND_THROW_ERROR(
		FT_New_Memory_Face(freetypeLibrary, (uint8_t*)fontFileBytes.get(), fontFileSize, 0, actualFacePointer),
		fontPath.string());
	FT_PRINT_AND_THROW_ERROR(FT_Select_Charmap(*actualFacePointer, FT_ENCODING_UNICODE), fontPath.string());
	hasKerning = FT_HAS_KERNING(*actualFacePointer);
};
float textureScale = 1.f;
const glm::vec2 FreetypeFont::stringSize(const string_view string, float fontSize, float& lineHeight, glm::vec2 bounds,
																				 enums::EBreakStyle breakStyle)
{
	strings::Utf8Iterator iterator(string, 0);
	const unsigned long& stringSize = string.size();
	auto scaledBounds = bounds;
	auto& face = *facePointer.get();
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (lineHeight == 0)
	{
		lineHeight = face->size->metrics.height / 64.f;
	}
	glm::vec2 currentPosition = {0, lineHeight};
	glm::vec2 size = {0, 0};
	if (bounds.x)
	{
		size.x = bounds.x;
	}
	float advanceX = 0;
	auto advanceLine = [&]()
	{
		currentPosition.y += lineHeight;
		size.y = currentPosition.y;
		currentPosition.x = 0;
		advanceX = 0;
	};
	for (; iterator.index < stringSize;)
	{
		advanceX = 0;
		unsigned long codepoint = *iterator;
		if (codepoint == 10)
		{
			advanceLine();
		}
		else
		{
			auto& character = getCharacter(codepoint, fontSize);
			advanceX = (character.advance >> 6);
			addNextKerning(fontSize, character.glyphIndex, iterator, advanceX);
		}
		if (scaledBounds.x > 0 &&
				shouldAdvanceLine(iterator, currentPosition, advanceX, breakStyle, scaledBounds.x, fontSize))
		{
			advanceLine();
		}
		currentPosition.x += advanceX;
		if (!bounds.x)
		{
			size.x = (glm::max)(size.x, currentPosition.x);
		}
		++iterator;
	}
	size.y = currentPosition.y;
	return size;
};
void FreetypeFont::stringToTexture(const string_view string, glm::vec4 color, float fontSize, float& lineHeight,
																	 glm::vec2 textureSize, shared_ptr<textures::Texture>& texturePointer,
																	 const int64_t& cursorIndex, glm::vec3& cursorPosition, enums::EBreakStyle breakStyle)
{
	glm::ivec2 scaledSize = textureSize * textureScale;
	if (!texturePointer || texturePointer->size.x != scaledSize.x || texturePointer->size.y != scaledSize.y)
	{
		texturePointer.reset(new textures::Texture(window, glm::ivec4(scaledSize.x, scaledSize.y, 1, 0), 0,
																							 textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte,
																							 textures::Texture::FilterType::Nearest));
	}
	textures::Framebuffer framebuffer(window, *texturePointer);
	Scene scene(window, {scaledSize.x / 2.f, scaledSize.y / 2.f, 50}, {0, 0, -1}, glm::vec2(scaledSize), &framebuffer,
							false);
	strings::Utf8Iterator iterator(string, 0);
	const uint64_t& stringSize = string.size();
	auto& face = *facePointer.get();
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (lineHeight == 0)
	{
		lineHeight = face->size->metrics.height / 64.f;
	}
	float descender = face->size->metrics.descender / 64.f;
	glm::vec3 currentPosition = {0, scaledSize.y - descender - lineHeight, 25.f};
	auto advanceLine = [&]()
	{
		currentPosition.y -= lineHeight;
		currentPosition.x = 0;
	};
	FreetypeCharacter* characterPointer = 0;
	float advanceX = 0;
	uint64_t codepointIndex = 0;
	if (cursorIndex == 0)
	{
		cursorPosition = currentPosition;
	}
	for (; iterator.index < stringSize;)
	{
		advanceX = 0;
		characterPointer = 0;
		uint64_t codepoint = *iterator;
		if (codepoint == 10)
		{
			advanceLine();
			++iterator;
			continue;
		}
		characterPointer = &getCharacter(codepoint, fontSize * textureScale);
		advanceX = (characterPointer->advance >> 6);
		if (shouldAdvanceLine(iterator, currentPosition, advanceX, breakStyle, scaledSize.x, fontSize))
		{
			advanceLine();
		}
		if (characterPointer)
		{
			if (characterPointer->size.x != 0 && characterPointer->size.y != 0)
			{
				glm::vec3 characterPosition = currentPosition;
				characterPosition.x = currentPosition.x + characterPointer->bearing.x + (characterPointer->size.x / 2.f);
				characterPosition.y = (currentPosition.y - (characterPointer->size.y - characterPointer->bearing.y)) +
					(characterPointer->size.y / 2.f);
				scene.addEntity(make_shared<entities::Plane>(window, scene, characterPosition, glm::vec3(0, 0, 0),
																										 glm::vec3(1, 1, 1), characterPointer->size,
																										 *characterPointer->texturePointer),
												false);
			}
			addNextKerning(fontSize, characterPointer->glyphIndex, iterator, advanceX);
		}
	_advance:
		currentPosition.x += advanceX;
		codepointIndex = iterator.getCurrentCodepointIndex();
		if (cursorIndex == codepointIndex + 1)
		{
			cursorPosition = currentPosition;
			cursorPosition /= textureScale;
		}
		++iterator;
	}
	codepointIndex = iterator.getCurrentCodepointIndex();
	if (cursorIndex == codepointIndex + 1)
	{
		cursorPosition = currentPosition;
		cursorPosition /= textureScale;
	}
	scene.clearColor = glm::vec4(0);
	framebuffer.bind();
	scene.render();
	framebuffer.unbind();
	return;
};
FreetypeCharacter& FreetypeFont::getCharacter(float codepoint, float fontSize)
{
	auto iter = codepointFontSizeCharacters[codepoint].insert({fontSize, {window, *this, codepoint, fontSize}});
	return iter.first->second;
};

void FreetypeFont::addNextKerning(float fontSize, FT_UInt currentGlyphIndex, zg::strings::Utf8Iterator iterator,
																	float& advanceX)
{
	if (hasKerning && iterator.hasNextCodepoint())
	{
		auto& face = *facePointer.get();
		auto nextIterator = iterator + 1;
		uint64_t nextCodepoint = *nextIterator;
		if (nextCodepoint != 10)
		{
			auto& nextCharacter = getCharacter(nextCodepoint, fontSize * textureScale);
			FT_Vector kerning;
			FT_Get_Kerning(face, currentGlyphIndex, nextCharacter.glyphIndex, FT_KERNING_DEFAULT, &kerning);
			if (!FT_IS_SCALABLE(face))
			{
				advanceX += (kerning.x);
			}
			else
			{
				advanceX += ((float)(kerning.x) / (float)(1 << 6));
			}
		}
	}
}
float FreetypeFont::shouldAdvanceLine(zg::strings::Utf8Iterator iterator, glm::vec2 currentPosition, float advanceX,
																			enums::EBreakStyle breakStyle, float boundsX, float fontSize)
{
	if (currentPosition.x + advanceX > boundsX)
		return true;
	switch (breakStyle)
	{
	case enums::EBreakStyle::None:
		return false;
	case enums::EBreakStyle::Word:
		{
			bool breakAt = false;
			while (true)
			{
				++iterator;
				auto codepoint = *iterator;
				if (codepoint == 0 || codepoint == 32 || codepoint == 10 || codepoint == 13 || codepoint == 9)
					break;
				auto& character = getCharacter(codepoint, fontSize);
				advanceX += (character.advance >> 6);
				if (currentPosition.x + advanceX > boundsX)
				{
					breakAt = true;
				}
			}
			if (breakAt)
			{
				return true;
			}
			return false;
		}
	case enums::EBreakStyle::WordHyphen:
		{
			bool breakAt = false;
			while (true)
			{
				++iterator;
				auto codepoint = *iterator;
				if (codepoint == 0 || codepoint == 32 || codepoint == 10 || codepoint == 13 || codepoint == 9 || codepoint == 45)
					break;
				auto& character = getCharacter(codepoint, fontSize);
				advanceX += (character.advance >> 6);
				if (currentPosition.x + advanceX > boundsX)
				{
					breakAt = true;
				}
			}
			if (breakAt)
			{
				return true;
			}
			return false;
		}
	}
	throw std::runtime_error("unsupported breakStyle");
}
