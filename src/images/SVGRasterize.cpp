#include <lunasvg.h>
#include <zg/images/SVGRasterize.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image_write.h>
std::shared_ptr<int8_t> zg::images::SVGRasterize(const std::shared_ptr<int8_t>& svgBytes, glm::ivec2 size,
																								 glm::vec4 color)
{
	uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
	uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
	uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
	uint8_t a = static_cast<uint8_t>(color.a * 255.0f);
	char colorHex[10];
	snprintf(colorHex, sizeof(colorHex), "#%02X%02X%02X", r, g, b);
	std::string svgString((char*)svgBytes.get());
	size_t insertPos = svgString.find("<svg");
	if (insertPos != std::string::npos)
	{
		insertPos = svgString.find(">", insertPos);
		if (insertPos != std::string::npos)
		{
			svgString.insert(insertPos + 1, "<style>* { fill: " + std::string(colorHex) + "; }</style>");
		}
	}
	auto document = lunasvg::Document::loadFromData(svgString);
	if (!document)
	{
		throw std::runtime_error("Failed to parse SVG data.");
	}
	auto bitmap = document->renderToBitmap(size.x, size.y);
	const uint8_t* pixels = bitmap.data();
	size_t imageSize = size.x * size.y * 4;
	std::shared_ptr<int8_t> rasterizedPointer((int8_t*)malloc(imageSize), free);
	memcpy(rasterizedPointer.get(), pixels, imageSize);
	return rasterizedPointer;
}
std::shared_ptr<int8_t> zg::images::SVGRasterize(const zgfilesystem::File& svgFile, glm::ivec2 size, glm::vec4 color)
{
	return SVGRasterize(((zgfilesystem::File&)svgFile).toBytes(), size, color);
}
