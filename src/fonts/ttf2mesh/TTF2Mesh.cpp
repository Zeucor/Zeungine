#include <zg/fonts/ttf2mesh/TTF2Mesh.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/Window.hpp>
#include <zg/utilities.hpp>
using namespace zg::fonts::ttf2mesh;
using namespace zg::fonts::freetype;
TTF2MeshGlyph::TTF2MeshGlyph(IRenderer* iRenderer, const TTF2MeshFont& ttf2MeshFont, float codepoint, float fontSize):
    codepoint(codepoint),
    meshRefCount(new size_t(1))
{
	auto& face = *ttf2MeshFont.ftFacePointer.get();
	glyphIndex = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	auto loadCharCode = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL | FT_LOAD_COLOR);
	if (loadCharCode)
	{
		throw std::runtime_error("Failed to load glyph: " + std::to_string(FT_Get_Char_Index(face, glyphIndex)));
	}
	size = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
	bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
	advance = face->glyph->advance.x;
    int index = ttf_find_glyph(ttf2MeshFont.facePointer, codepoint);
    if (index < 0) return;
    auto result = ttf_glyph2mesh3d(&ttf2MeshFont.facePointer->glyphs[index], &mesh, TTF_QUALITY_HIGH, TTF_FEATURES_DFLT, 0.5f);
    if (result != TTF_DONE)
        return;
    entityCreateInfo.typeName = "TTF2MeshGlyph";
    entityCreateInfo.name = "Glyph " + std::string(1, (char)codepoint);
    vertexCount = mesh->nvert;
	auto _vertexCount = vertexCount;
    std::vector<glm::vec3> vertices(vertexCount);
    std::vector<glm::vec4> colors(vertexCount, glm::vec4(1));
    auto indiceCount = mesh->nfaces * 3;
    std::vector<uint32_t> indices(indiceCount);
	for (size_t index = 0; index < mesh->nvert; index++)
	{
		auto &vertex = mesh->vert[index];
		vertices[index] = {vertex.x, vertex.y, vertex.z};
	}
	for (size_t index = 0; index < mesh->nfaces; index++)
	{
		auto &face = mesh->faces[index];
		if (iRenderer->frontFace == zg::FRONTFACE::CLOCKWISE)
		{
			indices[index * 3] = face.v3;
			indices[index * 3 + 1] = face.v2;
			indices[index * 3 + 2] = face.v1;
		}
		else
		{
			indices[index * 3] = face.v1;
			indices[index * 3 + 1] = face.v2;
			indices[index * 3 + 2] = face.v3;
		}
	}
    entityCreateInfo.indiceCount = [indiceCount](auto&) { return indiceCount; };
    entityCreateInfo.indices = [indices](auto&) { return indices; };
    entityCreateInfo.vertexCount = [_vertexCount](auto&) { return _vertexCount; };
    entityCreateInfo.vertices = [vertices](auto&) { return vertices; };
    entityCreateInfo.colorCount = [_vertexCount](auto&) { return _vertexCount; };
    entityCreateInfo.colors = [colors](auto&) { return colors; };
    entityCreateInfo.constants = {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}};
    entityCreateInfo.position = {0, 0, 0};
    entityCreateInfo.rotation = {1, 0, 0, 0};
    entityCreateInfo.scale = {fontSize, fontSize, 1};
};
TTF2MeshGlyph::TTF2MeshGlyph(const TTF2MeshGlyph& other):
    glyphIndex(other.glyphIndex),
    codepoint(other.codepoint),
    size(other.size),
    bearing(other.bearing),
    advance(other.advance),
    entityCreateInfo(other.entityCreateInfo),
    mesh(other.mesh),
    meshRefCount(other.meshRefCount),
    vertexCount(other.vertexCount)
{
    ++*meshRefCount;
}
TTF2MeshGlyph& TTF2MeshGlyph::operator=(const TTF2MeshGlyph& other)
{
    glyphIndex = other.glyphIndex;
    codepoint = other.codepoint;
    size = other.size;
    bearing = other.bearing;
    advance = other.advance;
    entityCreateInfo = other.entityCreateInfo;
    mesh = other.mesh;
    meshRefCount = other.meshRefCount;
    vertexCount = other.vertexCount;
    return *this;
}
TTF2MeshGlyph::~TTF2MeshGlyph()
{
    if (meshRefCount)
    {
        --*meshRefCount;
        if (!*meshRefCount && mesh)
            ttf_free_mesh3d(mesh);
    }
}
TTF2MeshFont::TTF2MeshFont(IRenderer* iRenderer, interfaces::IFile& fontFile) :
		ftFacePointer(new FT_Face,
								[](FT_Face* pointer)
								{
									FT_Done_Face(*pointer);
									delete pointer;
								}),
		iRenderer(iRenderer), fontPath(fontFile.filePath)
{
	fontFileBytes = fontFile.toBytes();
	ftFontFileBytes = fontFile.toBytes();
	auto fontFileSize = fontFile.size();
	auto actualFacePointer = ftFacePointer.get();
	FreetypeFont::FT_PRINT_AND_THROW_ERROR(
		FT_New_Memory_Face(FreetypeFont::freetypeLibrary, (uint8_t*)fontFileBytes.get(), fontFileSize, 0, actualFacePointer),
		fontPath.string());
        FreetypeFont::FT_PRINT_AND_THROW_ERROR(FT_Select_Charmap(*actualFacePointer, FT_ENCODING_UNICODE), fontPath.string());
	hasKerning = FT_HAS_KERNING(*actualFacePointer);
    ttf_load_from_mem((uint8_t*)ftFontFileBytes.get(), fontFileSize, &facePointer, false);
}
float ttf2MeshtTextureScale = 1.f;
const glm::vec2 TTF2MeshFont::stringSize(const std::string_view string, float fontSize, float& lineHeight,
																				 glm::vec2 bounds, enums::EBreakStyle breakStyle)
{
	strings::Utf8Iterator iterator(string, 0);
	const unsigned long& stringSize = string.size();
	auto scaledBounds = bounds;
	auto& face = *ftFacePointer.get();
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
void TTF2MeshFont::stringToScene(const std::string_view string, glm::vec3 position, glm::vec4 color,
								glm::quat _rotation, glm::vec3 _scale, float fontSize, float& lineHeight,
								glm::vec2 bounds, enums::EBreakStyle breakStyle, Scene& scene,
								std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex, size_t& cursor,
								const shaders::RuntimeConstants& constants)
{
	strings::Utf8Iterator iterator(string, 0);
	const uint64_t& stringSize = string.size();
	auto& face = *ftFacePointer.get();
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
	TTF2MeshGlyph* characterPointer = 0;
	float advanceX = 0;
	uint64_t codepointIndex = 0;
	if (!cursor)
	{
		// cursor = std::make_shared<entities::Plane>(window, scene, glm::vec3(0), glm::vec3(0), glm::vec3(1),
		// 																					 glm::vec2(3, lineHeight), color);
	}
	// auto& cursorRef = Registry::getEntity(cursor);
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
		characterPointer = &getCharacter(codepoint, fontSize * ttf2MeshtTextureScale);
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
					currentPosition.x + characterPointer->bearing.x * _scale.x;// + (characterPointer->size.x / 2.f)) );
				characterPosition.y =
					(currentPosition.y - (((characterPointer->size.y - characterPointer->bearing.y) / 2.f) * _scale.y));//-
					// ((characterPointer->size.y / 2.f) * _scale.y);
				if (iterator.index < existingAndUpdatedGlyphIDs.size())
				{
					auto& glyphID = existingAndUpdatedGlyphIDs[iterator.index];
					if (!glyphID)
					{
						goto _addGlyph;
					}
					else 
					{
						auto& glyph = Registry::getEntity(glyphID);
						if (glyph.VALUE != codepoint)
						{
							glyph.position = characterPosition;
							glyph.scale = glm::vec3(characterPointer->size, 1.f) * _scale;
							// glyph.keyedTextures[0].second = characterPointer->texturePointer;
							glyph.VALUE = codepoint;
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
                    auto info = characterPointer->entityCreateInfo;
                    info.position = characterPosition;
                    info.scale *= _scale;
					info.constants = zg::mergeVectors<std::string>(info.constants, constants);
					std::vector<glm::vec4> colors(characterPointer->vertexCount, color);
					info.colors = [colors](auto&)
					{
						return colors;
					};
                    auto glyph_tuple = scene.addEntity(info);
					existingAndUpdatedGlyphIDs.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(glyph_tuple));
					auto& glyph = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(glyph_tuple);
					glyph.VALUE = codepoint;
					// auto glyphCreateInfo = entities::PlaneFactory(characterPointer->texturePointer, "Glyph " + std::string(1, (char)codepoint), characterPosition, _rotation, glm::vec3(characterPointer->size, 1.f) * _scale, glm::vec2(1));
					// auto glyph_tuple = scene.addEntity(glyphCreateInfo);
				}
			}
			else if (iterator.index >= existingAndUpdatedGlyphIDs.size())
			{
				existingAndUpdatedGlyphIDs.push_back(0);
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
			existingAndUpdatedGlyphIDs.push_back(0);
		}
	_advance:
		currentPosition.x += advanceX;
		codepointIndex = iterator.getCurrentCodepointIndex();
		// if (cursorIndex == codepointIndex + 1)
		// {
		// 	cursorRef.position = currentPosition;
		// 	cursorRef.position /= ttf2MeshtTextureScale;
		// }
		++iterator;
	}
	codepointIndex = iterator.getCurrentCodepointIndex();
	for (auto i = existingAndUpdatedGlyphIDs.size() - 1; i >= codepointIndex; i--)
	{
		auto& entityID = existingAndUpdatedGlyphIDs.back();
		auto& entity = Registry::getEntity(entityID);
		scene.bvh->removeEntity(scene, entity);
		scene.removeEntity(entityID);
		existingAndUpdatedGlyphIDs.erase(existingAndUpdatedGlyphIDs.end() - 1);
	}
	// if (cursorIndex == codepointIndex + 1)
	// {
	// 	cursorRef.position = currentPosition;
	// 	cursorRef.position /= ttf2MeshtTextureScale;
	// }
	return;
}
void TTF2MeshFont::stringToEntity(const std::string_view string, glm::vec3 position, glm::vec4 color,
                                glm::quat _rotation, glm::vec3 _scale, float fontSize, float& lineHeight,
                                glm::vec2 bounds, enums::EBreakStyle breakStyle, Scene& scene, Entity& entity,
                                std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex, size_t& cursor,
								const shaders::RuntimeConstants& constants)
{
	strings::Utf8Iterator iterator(string, 0);
	const uint64_t& stringSize = string.size();
	auto& face = *ftFacePointer.get();
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
	TTF2MeshGlyph* characterPointer = 0;
	float advanceX = 0;
	uint64_t codepointIndex = 0;
	if (!cursor)
	{

		// cursor = std::make_shared<entities::Plane>(window, scene, glm::vec3(0), glm::vec3(0), glm::vec3(1),
		// 																					 glm::vec2(3, lineHeight), color);
	}
	// auto& cursorRef = Registry::getEntity(cursor);
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
		characterPointer = &getCharacter(codepoint, fontSize * ttf2MeshtTextureScale);
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
						auto& glyph = Registry::getEntity(glyphID);
						if (glyph.VALUE != codepoint)
						{
							glyph.position = characterPosition;
							glyph.scale = glm::vec3(characterPointer->size, 1.f) * _scale;
							// glyph.keyedTextures[0].second = characterPointer->texturePointer;
							glyph.VALUE = codepoint;
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
                    auto info = characterPointer->entityCreateInfo;
                    info.position = characterPosition;
                    info.scale *= _scale;
					info.constants = zg::mergeVectors<std::string>(info.constants, constants);
                    auto glyph_tuple = entity.addChild(info);
					existingAndUpdatedGlyphIDs.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(glyph_tuple));
					auto& glyph = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(glyph_tuple);
					glyph.VALUE = codepoint;
				}
			}
			else if (iterator.index >= existingAndUpdatedGlyphIDs.size())
			{
				existingAndUpdatedGlyphIDs.push_back(0);
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
			existingAndUpdatedGlyphIDs.push_back(0);
		}
	_advance:
		currentPosition.x += advanceX;
		codepointIndex = iterator.getCurrentCodepointIndex();
		// if (cursorIndex == codepointIndex + 1)
		// {
		// 	cursorRef.position = currentPosition;
		// 	cursorRef.position /= ttf2MeshtTextureScale;
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
	// 	cursorRef.position /= ttf2MeshtTextureScale;
	// }
	return;
}
TTF2MeshGlyph& TTF2MeshFont::getCharacter(float codepoint, float fontSize)
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
void TTF2MeshFont::addNextKerning(float fontSize, FT_UInt currentGlyphIndex, zg::strings::Utf8Iterator iterator,
																	float& advanceX, float scaleX)
{
	if (hasKerning && iterator.hasNextCodepoint())
	{
		auto& face = *ftFacePointer.get();
		auto nextIterator = iterator + 1;
		uint64_t nextCodepoint = *nextIterator;
		if (nextCodepoint != 10)
		{
			auto& nextCharacter = getCharacter(nextCodepoint, fontSize * ttf2MeshtTextureScale);
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
TTF2MeshFont::~TTF2MeshFont()
{
    ttf_free(facePointer);
}
float TTF2MeshFont::shouldAdvanceLine(zg::strings::Utf8Iterator iterator, glm::vec2 currentPosition, float advanceX,
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