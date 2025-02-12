#pragma once
#include <memory>
#include <string>
#include <zg/filesystem/File.hpp>
#include <zg/glm.hpp>
namespace zg::images
{
	std::shared_ptr<int8_t> SVGRasterize(const std::shared_ptr<int8_t>& svgBytes, glm::ivec2 size, glm::vec4 color = glm::vec4(1));
	std::shared_ptr<int8_t> SVGRasterize(const filesystem::File& svgFile, glm::ivec2 size, glm::vec4 color = glm::vec4(1));
} // namespace zg
