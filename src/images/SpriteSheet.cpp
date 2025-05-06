#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <zg/images/SpriteSheet.hpp>
#include "stb_image.h"
using namespace zg;
using namespace zg::images;
SpriteSheet::SpriteSheet(IRenderer* iRenderer, const interfaces::IFile& file, uvec bgColor,
                        const std::vector<std::pair<std::string, glm::vec4>>& spriteKeyCoords)
{
    loadSheet(iRenderer, (interfaces::IFile&)file, bgColor);
    parseSheet(spriteKeyCoords);
};
void SpriteSheet::loadSheet(IRenderer* iRenderer, interfaces::IFile& file, uvec bgColor)
{
    auto fileSize = file.size();
    auto fileBytes = file.toBytes();
	int nrChannels = 0;
    int sheetWidth = 0, sheetHeight = 0;
	uint8_t *imageData = stbi_load_from_memory((uint8_t*)fileBytes.get(), fileSize, &sheetWidth, &sheetHeight, &nrChannels, 4);
	if (!imageData)
	{
		throw std::runtime_error("Failed to load texture from memory.");
	}
    auto n = sheetWidth * sheetHeight;
    auto colors = (uvec*)imageData;
    for (size_t i = 0; i < n; ++i)
    {
        auto& color = colors[i];
        if (color.r == bgColor.r && color.g == bgColor.g && color.b == bgColor.b && color.a == bgColor.a)
            color = uvec(0);
    }
    spriteTexture = std::make_shared<textures::Texture>(
        iRenderer,
        glm::ivec4(sheetWidth, sheetHeight, 1, 0),
        (const void*)imageData,
        textures::Texture::Format::RGBA8,
        textures::Texture::Type::UnsignedByte,
        textures::Texture::FilterType::Nearest,
        false,
        textures::Texture::Multisampling::x1,
        textures::Texture::AddressMode::ClampToEdge
    );
}
void SpriteSheet::parseSheet(const std::vector<std::pair<std::string, glm::vec4>>& spriteKeyCoords)
{
    auto& spriteTextureRef = *spriteTexture;
    auto& sheetWidth = spriteTextureRef.size.x;
    auto& sheetHeight = spriteTextureRef.size.y;
    for (auto& pair : spriteKeyCoords)
    {
        auto& key = pair.first;
        auto& pixelCoords = pair.second;
        auto tl = glm::vec2(pixelCoords.x, pixelCoords.y);
        auto wh = glm::vec2(pixelCoords.z, pixelCoords.w);
        keyedUVs[key] = glm::vec4(
            // bottom left
            tl.x / sheetWidth,
            (sheetHeight - (tl.y + wh.y)) / sheetHeight,
            // top right
            (tl.x + wh.x) / sheetWidth,
            (sheetHeight - tl.y) / sheetHeight
        );
    }
}
// Given a key, extract the UV and return 4 uvs in order BL-BR-TR-TL
std::array<glm::vec2, 4> SpriteSheet::getUVForKey(const std::string& key)
{
    auto iter = keyedUVs.find(key);
    if (iter == keyedUVs.end())
        return std::array<glm::vec2, 4>();
    auto& vec = iter->second;
    auto bl = glm::vec2(vec.x, vec.y);
    auto tr = glm::vec2(vec.z, vec.w);
    return {
        bl,
        glm::vec2(tr.x, bl.y),
        tr,
        glm::vec2(bl.x, tr.y)
    };
}
std::shared_ptr<textures::Texture> SpriteSheet::getTexture() const
{
    return spriteTexture;
}