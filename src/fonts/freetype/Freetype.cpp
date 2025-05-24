#include <iostream>
#include <stdexcept>
#include <zg/Logger.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/strings/Utf8Iterator.hpp>
using namespace zg::fonts::freetype;
FT_Library FreetypeFont::freetypeLibrary;
bool FreetypeFont::freetypeLoaded = ([]()
									 {
	if (FT_Init_FreeType(&freetypeLibrary))
  {
    throw std::runtime_error("Failed to initialize freetype library");
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
void FreetypeFont::FT_PRINT_AND_THROW_ERROR(const FT_Error& error, const std::string& fontPath)
{
	if (error)
	{
		auto errorString = "Error loading font[" + fontPath + "]" + std::string(ft_errorstring(error));
		Logger::print(Logger::Error, errorString);
		throw std::runtime_error(errorString);
	}
};
FreetypeCharacter::FreetypeCharacter(IRenderer* iRenderer, const FreetypeFont& freeTypeFont, float codepoint, float fontSize)
{
	auto& face = *freeTypeFont.facePointer.get();
	glyphIndex = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	auto loadCharCode = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL | FT_LOAD_COLOR);
	if (loadCharCode)
	{
		throw std::runtime_error("Failed to load glyph: " + std::to_string(FT_Get_Char_Index(face, glyphIndex)));
	}
	size = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
	bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
	auto& renderer = iRenderer->renderer;
	if (size.x == 0 || size.y == 0)
	{
		goto _setAdvance;
	}
	{
		auto flipY = (renderer == RENDERER_GL || renderer == RENDERER_EGL);
		uint64_t imgSize = size.x * size.y * 4;
		std::shared_ptr<uint8_t[]> rgbaImg(new uint8_t[imgSize]);
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
		texturePointer.reset(new textures::Texture(iRenderer, {size.x, size.y, 1, 0}, rgbaImgPointer,
																							 textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte,
																							 textures::Texture::FilterType::Linear));
	}
_setAdvance:
	advance = face->glyph->advance.x;
};
FreetypeFont::FreetypeFont(IRenderer* iRenderer, interfaces::IFile& fontFile) :
		facePointer(new FT_Face,
								[](FT_Face* pointer)
								{
									FT_Done_Face(*pointer);
									delete pointer;
								}),
		iRenderer(iRenderer), fontPath(fontFile.filePath)
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
float ftTextureScale = 1.f;
const glm::vec2 FreetypeFont::stringSize(const std::string_view string, float fontSize, float& lineHeight,
																				 glm::vec2 bounds, enums::EBreakStyle breakStyle)
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
	float startX = currentPosition.x;
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
			addNextKerning(fontSize, character.glyphIndex, iterator, advanceX, 1);
		}
		if (scaledBounds.x > 0 &&
				shouldAdvanceLine(iterator, currentPosition, advanceX, breakStyle, scaledBounds.x, 1, startX, fontSize))
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
void FreetypeFont::stringToTexture(const std::string_view string, glm::vec4 color, float fontSize, float& lineHeight,
																	 glm::vec2 textureSize, std::shared_ptr<textures::Texture>& pTexture,
																	 const int64_t& cursorIndex, glm::vec3& cursorPosition, enums::EBreakStyle breakStyle,
																	 const std::shared_ptr<textures::Framebuffer>& pFramebuffer,
																	 const std::shared_ptr<Scene>& scenePointer)
{
	glm::ivec2 scaledSize = textureSize * ftTextureScale;
	if (!pTexture || pTexture->size.x != scaledSize.x || pTexture->size.y != scaledSize.y)
	{
		pTexture.reset(new textures::Texture(iRenderer, glm::ivec4(scaledSize.x, scaledSize.y, 1, 0), 0,
																							 textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte,
																							 textures::Texture::FilterType::Linear));
	}
	if (!pFramebuffer)
	{
		auto attachments = std::vector<textures::Framebuffer::TextureAttachmentPair>(
			{{pTexture, textures::Framebuffer::AttachmentType::Color}});
		((std::shared_ptr<textures::Framebuffer>&)pFramebuffer) =
			std::make_shared<textures::Framebuffer>(iRenderer, attachments);
	}
	if (!scenePointer)
	{
		SceneCreateInfo sceneCreateInfo{.name = "Text Scene",
																		.cameraPosition = glm::vec3(scaledSize.x / 2.f, scaledSize.y / 2.f, 50),
																		.cameraDirection = glm::vec3(0, 0, -1),
																		.projectionType = vp::Projection::TYPE::Orthographic,
																		.orthoSize = glm::vec2(scaledSize),
																		.framebuffer = pFramebuffer,
																		.drawColorToWindowPlane = false,
																		.useBVH = false};
		((std::shared_ptr<Scene>&)scenePointer) = std::make_shared<Scene>(sceneCreateInfo);
	}
	auto& scene = *scenePointer;
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
	float startX = currentPosition.x;
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
		characterPointer = &getCharacter(codepoint, fontSize * ftTextureScale);
		advanceX = (characterPointer->advance >> 6);
		if (shouldAdvanceLine(iterator, currentPosition, advanceX, breakStyle, scaledSize.x, 1, startX, fontSize))
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
				auto planeInfo = entities::PlaneFactory(
					characterPointer->texturePointer,
					"Glyph " + std::string(1, codepoint),
					characterPosition,
					glm::quat(1, 0, 0, 0),
					glm::vec3(characterPointer->size, 1.f)
				);
				scene.addEntity(planeInfo);
			}
			addNextKerning(fontSize, characterPointer->glyphIndex, iterator, advanceX, 1);
		}
	_advance:
		currentPosition.x += advanceX;
		codepointIndex = iterator.getCurrentCodepointIndex();
		if (cursorIndex == codepointIndex + 1)
		{
			cursorPosition = currentPosition;
			cursorPosition /= ftTextureScale;
		}
		++iterator;
	}
	codepointIndex = iterator.getCurrentCodepointIndex();
	if (cursorIndex == codepointIndex + 1)
	{
		cursorPosition = currentPosition;
		cursorPosition /= ftTextureScale;
	}
	scene.clearColor = glm::vec4(0);
	scene.render();
	return;
}
void FreetypeFont::stringToScene(const std::string_view string, glm::vec3 position, glm::vec4 color,
																 glm::quat _rotation, glm::vec3 _scale, float fontSize, float& lineHeight,
																 glm::vec2 bounds, enums::EBreakStyle breakStyle, Scene& scene,
																 std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex, size_t& cursor)
{
	strings::Utf8Iterator iterator(string, 0);
	const uint64_t& stringSize = string.size();
	auto& face = *facePointer.get();
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (lineHeight == 0)
	{
		lineHeight = face->size->metrics.height / 64.f;
	}
	float descender = face->size->metrics.descender / 64.f;
	auto currentPosition = position;
	float startX = currentPosition.x;
	auto advanceLine = [&]()
	{
		currentPosition.y -= lineHeight * _scale.y;
		currentPosition.x = startX;
	};
	FreetypeCharacter* characterPointer = 0;
	float advanceX = 0;
	uint64_t codepointIndex = 0;
	if (!cursor)
	{
		// cursor = std::make_shared<entities::Plane>(window, scene, glm::vec3(0), glm::vec3(0), glm::vec3(1),
		// 																					 glm::vec2(3, lineHeight), color);
	}
	// auto& cursorRef = Registry::GetSingleton().getEntity(cursor);
	// if (cursorIndex == 0)
	// {
	// 	cursorRef.position = currentPosition;
	// }
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
		characterPointer = &getCharacter(codepoint, fontSize * ftTextureScale);
		advanceX = (characterPointer->advance >> 6) * _scale.x;
		if (shouldAdvanceLine(iterator, currentPosition, advanceX, breakStyle, bounds.x, _scale.x, startX, fontSize))
		{
			advanceLine();
		}
		if (characterPointer)
		{
			if (characterPointer->size.x != 0 && characterPointer->size.y != 0)
			{
				glm::vec3 characterPosition = currentPosition;
				characterPosition.x =
					currentPosition.x + ((characterPointer->bearing.x + (characterPointer->size.x / 2.f)) * _scale.x);
				characterPosition.y =
					(currentPosition.y - ((characterPointer->size.y - characterPointer->bearing.y) * _scale.y)) +
					((characterPointer->size.y / 2.f) * _scale.y);
				if (iterator.index < existingAndUpdatedGlyphIDs.size())
				{
					auto& glyphID = existingAndUpdatedGlyphIDs[iterator.index];
					if (!glyphID)
					{
						goto _addGlyph;
					}
					else 
					{
						auto& glyph = Registry::GetSingleton().getEntity(glyphID);
						if (glyph.VALUE.has_value())
						{
							try
							{
								auto glyphValue = glyph.template getValue<uint64_t>();
								if (glyphValue != codepoint)
								{
									glyph.position = characterPosition;
									glyph.scale = glm::vec3(characterPointer->size, 1.f) * _scale;
									assert(glyph.meshInfos.size());
									glyph.meshInfos[0].keyedTextures[0].second = characterPointer->texturePointer;
									glyph.VALUE = codepoint;
									glyph.refreshMeshes();
								}
							}
							catch (...) {}
						}
						if (glyph.position != characterPosition)
						{
							glyph.position = characterPosition;
						}
					}
				}
				else
				{
_addGlyph:
					auto glyphCreateInfo = entities::PlaneFactory(characterPointer->texturePointer, "Glyph " + std::string(1, (char)codepoint), characterPosition, _rotation, glm::vec3(characterPointer->size, 1.f) * _scale);
					auto glyph_tuple = scene.addEntity(glyphCreateInfo);
					existingAndUpdatedGlyphIDs.insert(existingAndUpdatedGlyphIDs.begin() + iterator.getCurrentCodepointIndex(), std::get<KEY_ID_VECTOR_ID_INDEX>(glyph_tuple));
					auto& glyph = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(glyph_tuple);
					glyph.VALUE = codepoint;
				}
			}
			else if (iterator.index >= existingAndUpdatedGlyphIDs.size())
			{
				existingAndUpdatedGlyphIDs.insert(existingAndUpdatedGlyphIDs.begin() + iterator.getCurrentCodepointIndex(), 0);
			}
			else
			{
				auto& glyphID = existingAndUpdatedGlyphIDs[iterator.index];
				if (glyphID)
				{
					scene.removeEntity(glyphID);
					glyphID = 0;
				}
			}
			addNextKerning(fontSize, characterPointer->glyphIndex, iterator, advanceX, _scale.x);
		}
		else if (iterator.index >= existingAndUpdatedGlyphIDs.size())
		{
			existingAndUpdatedGlyphIDs.insert(existingAndUpdatedGlyphIDs.begin() + iterator.getCurrentCodepointIndex(), 0);
		}
	_advance:
		currentPosition.x += advanceX;
		codepointIndex = iterator.getCurrentCodepointIndex();
		// if (cursorIndex == codepointIndex + 1)
		// {
		// 	cursorRef.position = currentPosition;
		// 	cursorRef.position /= ftTextureScale;
		// }
		++iterator;
	}
	codepointIndex = iterator.getCurrentCodepointIndex();
	for (auto i = existingAndUpdatedGlyphIDs.size() - 1; i >= codepointIndex; i--)
	{
		auto& entityID = existingAndUpdatedGlyphIDs.back();
		auto& entity = Registry::GetSingleton().getEntity(entityID);
		scene.bvh->removeEntity(scene, entity);
		scene.removeEntity(entityID);
		existingAndUpdatedGlyphIDs.erase(existingAndUpdatedGlyphIDs.end() - 1);
	}
	// if (cursorIndex == codepointIndex + 1)
	// {
	// 	cursorRef.position = currentPosition;
	// 	cursorRef.position /= ftTextureScale;
	// }
	return;
}
void FreetypeFont::stringToEntity(const std::string_view string, glm::vec3 position, glm::vec4 color,
																	glm::quat _rotation, glm::vec3 _scale, float fontSize, float& lineHeight,
																	glm::vec2 bounds, enums::EBreakStyle breakStyle, Scene& scene, Entity& entity,
																	std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex, size_t& cursor)
{
	strings::Utf8Iterator iterator(string, 0);
	const uint64_t& stringSize = string.size();
	auto& face = *facePointer.get();
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (lineHeight == 0)
	{
		lineHeight = face->size->metrics.height / 64.f;
	}
	float descender = face->size->metrics.descender / 64.f;
	auto currentPosition = position;
	float startX = currentPosition.x;
	auto advanceLine = [&]()
	{
		currentPosition.y -= lineHeight;
		currentPosition.x = position.x;
	};
	FreetypeCharacter* characterPointer = 0;
	float advanceX = 0;
	uint64_t codepointIndex = 0;
	if (!cursor)
	{

		// cursor = std::make_shared<entities::Plane>(window, scene, glm::vec3(0), glm::vec3(0), glm::vec3(1),
		// 																					 glm::vec2(3, lineHeight), color);
	}
	// auto& cursorRef = Registry::GetSingleton().getEntity(cursor);
	// if (cursorIndex == 0)
	// {
	// 	cursorRef.position = currentPosition;
	// }
	for (; iterator.index < stringSize;)
	{
		codepointIndex = iterator.getCurrentCodepointIndex();
		advanceX = 0;
		characterPointer = 0;
		uint64_t codepoint = *iterator;
		if (codepoint == 10)
		{
			advanceLine();
			++iterator;
			continue;
		}
		characterPointer = &getCharacter(codepoint, fontSize * ftTextureScale);
		advanceX = (characterPointer->advance >> 6) * _scale.x;
		if (shouldAdvanceLine(iterator, currentPosition, advanceX, breakStyle, bounds.x, _scale.x, startX, fontSize))
		{
			advanceLine();
		}
		if (characterPointer)
		{
			if (characterPointer->size.x != 0 && characterPointer->size.y != 0)
			{
				glm::vec3 characterPosition = currentPosition;
				characterPosition.x =
					currentPosition.x + ((characterPointer->bearing.x + (characterPointer->size.x / 2.f)) * _scale.x);
				characterPosition.y =
					(currentPosition.y - ((characterPointer->size.y - characterPointer->bearing.y) * _scale.y)) +
					((characterPointer->size.y / 2.f) * _scale.y);
				if (iterator.index < existingAndUpdatedGlyphIDs.size())
				{
					auto& glyphID = existingAndUpdatedGlyphIDs[iterator.index];
					if (!glyphID)
					{
						goto _addGlyph;
					}
					else 
					{
						auto& glyph = Registry::GetSingleton().getEntity(glyphID);
						if (glyph.VALUE.has_value())
						{
							try
							{
								auto glyphValue = glyph.template getValue<uint64_t>();
								if (glyphValue != codepoint)
								{
									glyph.position = characterPosition;
									glyph.scale = glm::vec3(characterPointer->size, 1.f) * _scale;
									assert(glyph.meshInfos.size());
									glyph.meshInfos[0].keyedTextures[0].second = characterPointer->texturePointer;
									glyph.VALUE = codepoint;
									glyph.refreshMeshes();
								}
							}
							catch (...) {}
						}
						if (glyph.position != characterPosition)
						{
							glyph.position = characterPosition;
						}
					}
				}
				else
				{
_addGlyph:
					auto glyphCreateInfo = entities::PlaneFactory(characterPointer->texturePointer, "Glyph " + std::string(1, (char)codepoint), characterPosition, _rotation, glm::vec3(characterPointer->size, 1.f) * _scale);
					auto glyph_tuple = entity.addChild(glyphCreateInfo);
					existingAndUpdatedGlyphIDs.insert(existingAndUpdatedGlyphIDs.begin() + codepointIndex, std::get<KEY_ID_VECTOR_ID_INDEX>(glyph_tuple));
					auto& glyph = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(glyph_tuple);
					glyph.VALUE = codepoint;
				}
			}
			else if (iterator.index >= existingAndUpdatedGlyphIDs.size())
			{
				existingAndUpdatedGlyphIDs.insert(existingAndUpdatedGlyphIDs.begin() + codepointIndex, 0);
			}
			else
			{
				auto& glyphID = existingAndUpdatedGlyphIDs[iterator.index];
				if (glyphID)
				{
					entity.removeChild(glyphID);
					glyphID = 0;
				}
			}
			addNextKerning(fontSize, characterPointer->glyphIndex, iterator, advanceX, _scale.x);
		}
		else if (iterator.index >= existingAndUpdatedGlyphIDs.size())
		{
			existingAndUpdatedGlyphIDs.insert(existingAndUpdatedGlyphIDs.begin() + codepointIndex, 0);
		}
	_advance:
		currentPosition.x += advanceX;
		// if (cursorIndex == codepointIndex + 1)
		// {
		// 	cursorRef.position = currentPosition;
		// 	cursorRef.position /= ftTextureScale;
		// }
		++iterator;
	}
	codepointIndex = iterator.getCurrentCodepointIndex();
	for (auto i = existingAndUpdatedGlyphIDs.size() - 1; i >= codepointIndex; i--)
	{
		entity.removeChild(existingAndUpdatedGlyphIDs.back());
		existingAndUpdatedGlyphIDs.erase(existingAndUpdatedGlyphIDs.end() - 1);
	}
	// if (cursorIndex == codepointIndex + 1)
	// {
	// 	cursorRef.position = currentPosition;
	// 	cursorRef.position /= ftTextureScale;
	// }
	return;
}
FreetypeCharacter& FreetypeFont::getCharacter(float codepoint, float fontSize)
{
	auto& fontSizes = codepointFontSizeCharacters[codepoint];
	auto iter = fontSizes.find(fontSize);
	if (iter == fontSizes.end())
	{
		auto iter2 = fontSizes.insert({fontSize, {iRenderer, *this, codepoint, fontSize}});
		return iter2.first->second;
	}
	return iter->second;
}
void FreetypeFont::addNextKerning(float fontSize, FT_UInt currentGlyphIndex, zg::strings::Utf8Iterator iterator,
																	float& advanceX, float scaleX)
{
	if (hasKerning && iterator.hasNextCodepoint())
	{
		auto& face = *facePointer.get();
		auto nextIterator = iterator + 1;
		uint64_t nextCodepoint = *nextIterator;
		if (nextCodepoint != 10)
		{
			auto& nextCharacter = getCharacter(nextCodepoint, fontSize * ftTextureScale);
			FT_Vector kerning;
			FT_Get_Kerning(face, currentGlyphIndex, nextCharacter.glyphIndex, FT_KERNING_DEFAULT, &kerning);
			if (!FT_IS_SCALABLE(face))
			{
				advanceX += ((kerning.x)) * scaleX;
			}
			else
			{
				advanceX += (((float)(kerning.x) / (float)(1 << 6))) * scaleX;
			}
		}
	}
}
float FreetypeFont::shouldAdvanceLine(zg::strings::Utf8Iterator iterator, glm::vec2 currentPosition, float advanceX,
																			enums::EBreakStyle breakStyle, float boundsX, float scaleX, float startX,
																			float fontSize)
{
	if ((currentPosition.x + advanceX) > (boundsX + startX))
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
				advanceX += (character.advance >> 6) * scaleX;
				if ((currentPosition.x + advanceX) > (boundsX + startX))
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
				if (codepoint == 0 || codepoint == 32 || codepoint == 10 || codepoint == 13 || codepoint == 9 ||
						codepoint == 45)
					break;
				auto& character = getCharacter(codepoint, fontSize);
				advanceX += (character.advance >> 6) * scaleX;
				if ((currentPosition.x + advanceX) > (boundsX + startX))
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
