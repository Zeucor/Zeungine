#pragma once
#include <string>
#include <memory>
#include "../glm.hpp"
namespace anex::images
{
	struct ImageLoader
  {
		using ImagePair = std::pair<glm::uvec2, std::shared_ptr<uint8_t>>;
    static ImagePair load(const std::string_view& path);
		static ImagePair loadAndResize(const std::string_view& path, const glm::uvec2 &resize);
  };
}