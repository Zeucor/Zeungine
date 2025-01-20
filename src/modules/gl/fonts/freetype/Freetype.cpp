#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <stdexcept>
#include <iostream>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/strings/Utf8Iterator.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/entities/Plane.hpp>
using namespace anex::modules::gl::fonts::freetype;
FT_Library FreetypeFont::freetypeLibrary;
bool FreetypeFont::freetypeLoaded = ([]()
{
	if (FT_Init_FreeType(&freetypeLibrary))
  {
    throw std::runtime_error("Failed to initialize freetype library");
  }
	return true;
})();
struct ft_error
{
	int err;
	const char *str;
};
#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s)	{ (e), (s) },
#define FT_ERROR_START_LIST
#define FT_ERROR_END_LIST	{ 0, NULL }
static const struct ft_error ft_errors[] =
{
#include FT_ERRORS_H
};
const char *ft_errorstring(int err)
{
	const struct ft_error *e;

	for (e = ft_errors; e->str != NULL; e++)
		if (e->err == err)
			return e->str;

	return "Unknown error";
};
void FreetypeFont::FT_PRINT_AND_THROW_ERROR(const FT_Error &error)
{
	if (error)
	{
		auto errorString = "Error loading font: " + std::string(ft_errorstring(error));
		std::cerr << errorString << std::endl;
		throw std::runtime_error(errorString);
	}
};
FreetypeCharacter::FreetypeCharacter(GLWindow &window, const FreetypeFont &freeTypeFont, const float &codepoint, const float &fontSize)
{
	auto &face = *freeTypeFont.facePointer.get();
	glyphIndex = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	auto loadCharCode = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL | FT_LOAD_COLOR);
	if (loadCharCode)
	{
		throw std::runtime_error("Failed to load glyph: " + std::to_string(FT_Get_Char_Index(face, glyphIndex)));
	}
	size = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
	bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
	if (size.x == 0 || size.y == 0)
	{
		goto _setAdvance;
	}
	{
		uint64_t imgSize = size.x * size.y * 4;
		std::shared_ptr<uint8_t[]> rgbaImg(new uint8_t[imgSize]);
		auto rgbaImgPointer = rgbaImg.get();
		for (int64_t imgY = size.y - 1, rgbaImgY = 0; imgY >= 0; imgY--, rgbaImgY++)
		{
			for (uint64_t imgX = 0; imgX < size.x; imgX++)
			{
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 0] = 255;
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 1] = 255;
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 2] = 255;
				rgbaImgPointer[((rgbaImgY * (uint64_t)size.x + imgX) * 4) + 3] = face->glyph->bitmap.buffer[(imgY * face->glyph->bitmap.pitch + imgX)];
				continue;
			}
		}
		texturePointer.reset(new textures::Texture(window, {size.x, size.y, 1, 0}, rgbaImgPointer, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Nearest));
	}
	_setAdvance:
		advance = face->glyph->advance.x;
};
FreetypeFont::FreetypeFont(GLWindow &window, File &fontFile):
	facePointer(new FT_Face, [](FT_Face *pointer)
	{
		FT_Done_Face(*pointer);
		delete pointer;
	}),
	window(window)
{
	fontFileBytes = fontFile.toBytes();
  auto fontFileSize = fontFile.size();
	auto actualFacePointer = facePointer.get();
	FT_PRINT_AND_THROW_ERROR(FT_New_Memory_Face(freetypeLibrary, (uint8_t *)fontFileBytes.get(), fontFileSize, 0, actualFacePointer));
	FT_PRINT_AND_THROW_ERROR(FT_Select_Charmap(*actualFacePointer, FT_ENCODING_UNICODE));
};
float textureScale = 1.f;
const glm::vec2 FreetypeFont::stringSize(const std::string &string, const float &fontSize, float &lineHeight, const glm::vec2 &bounds)
{
	strings::Utf8Iterator iterator(string, 0);
	const unsigned long &stringSize = string.size();
	auto scaledBounds = bounds;
	auto &face = *facePointer.get();
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (lineHeight == 0)
	{
		lineHeight = face->size->metrics.height / 64.f;
	}
	glm::vec2 currentPosition = {0, lineHeight};
	glm::vec2 size = {0, 0};
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
			auto &character = getCharacter(codepoint, fontSize);
			advanceX += (character.advance >> 6);
			if (FT_HAS_KERNING(face) && iterator.hasNextCodepoint())
			{
				auto nextIterator = iterator + 1;
				unsigned long nextCodepoint = *nextIterator;
				if (nextCodepoint != 10)
				{
					auto &nextCharacter = getCharacter(nextCodepoint, fontSize);
					FT_Vector kerning;
					FT_Get_Kerning(face, character.glyphIndex, nextCharacter.glyphIndex, FT_KERNING_DEFAULT, &kerning);
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
		if (scaledBounds.x > 0 && currentPosition.x + advanceX > scaledBounds.x)
		{
			advanceLine();
		}
		currentPosition.x += advanceX;
		size.x = (glm::max)(size.x, currentPosition.x);
		++iterator;
	}
	size.y = currentPosition.y;
	return size;
};
void FreetypeFont::stringToTexture(const std::string &string,
																   const glm::vec4 &color,
																   const float &fontSize,
																   float &lineHeight,
																   const glm::vec2 &textureSize,
																   std::shared_ptr<textures::Texture> &texturePointer,
																   const int64_t &cursorIndex,
																   glm::vec3 &cursorPosition)
{
	glm::ivec2 scaledSize = textureSize * textureScale;
	if (!texturePointer || texturePointer->size.x != scaledSize.x || texturePointer->size.y != scaledSize.y)
	{
		texturePointer.reset(new textures::Texture(window, glm::ivec4(scaledSize.x, scaledSize.y, 1, 0), 0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Nearest));
	}
	textures::Framebuffer framebuffer(window, *texturePointer);
	GLScene scene(window, {scaledSize.x / 2.f, scaledSize.y / 2.f, 50}, {0, 0, -1}, glm::vec2(scaledSize), &framebuffer);
	strings::Utf8Iterator iterator(string, 0);
	const uint64_t &stringSize = string.size();
	auto &face = *facePointer.get();
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (lineHeight == 0)
	{
		lineHeight = face->size->metrics.height / 64.f;
	}
	float descender = face->size->metrics.descender / 64.f;
	glm::vec3 currentPosition = {0, -descender, 25.f};
	auto advanceLine = [&]()
	{
		currentPosition.y -= lineHeight;
		currentPosition.x = 0;
	};
	FreetypeCharacter *characterPointer = 0;
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
		}
		else
		{
			characterPointer = &getCharacter(codepoint, fontSize * textureScale);
			advanceX = (characterPointer->advance >> 6);
		}
		if (currentPosition.x + advanceX > scaledSize.x)
		{
			advanceLine();
		}
		if (characterPointer)
		{
			if (characterPointer->size.x != 0 && characterPointer->size.y != 0)
			{
				glm::vec3 characterPosition = currentPosition;
				characterPosition.x = currentPosition.x + characterPointer->bearing.x + (characterPointer->size.x / 2.f);
				characterPosition.y = (currentPosition.y - (characterPointer->size.y - characterPointer->bearing.y)) + (characterPointer->size.y / 2.f);
				scene.addEntity(std::make_shared<entities::Plane>(window, scene, characterPosition, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), characterPointer->size, *characterPointer->texturePointer), false);
			}
			if (FT_HAS_KERNING(face) && iterator.hasNextCodepoint())
			{
				auto nextIterator = iterator + 1;
				uint64_t nextCodepoint = *nextIterator;
				if (nextCodepoint != 10)
				{
					auto &nextCharacter = getCharacter(nextCodepoint, fontSize * textureScale);
					FT_Vector kerning;
					FT_Get_Kerning(face, characterPointer->glyphIndex, nextCharacter.glyphIndex, FT_KERNING_DEFAULT, &kerning);
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
FreetypeCharacter &FreetypeFont::getCharacter(const float &codepoint, const float &fontSize)
{
	auto iter = codepointFontSizeCharacters[codepoint].insert({fontSize, {window, *this, codepoint, fontSize}});
	return iter.first->second;
};