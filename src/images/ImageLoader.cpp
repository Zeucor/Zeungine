#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <anex/images/ImageLoader.hpp>
#include "stb_image.h"
#include "stb_image_resize2.h"
#include <stdexcept>
using namespace anex::images;
static bool setFlipVertically = ([]()
{
  stbi_set_flip_vertically_on_load(true);
  return true;
})();
std::pair<glm::uvec2, std::shared_ptr<uint8_t>> ImageLoader::load(const std::string_view &path)
{
  int x, y, n;
  uint8_t *data = stbi_load(path.data(), &x, &y, &n, 4);
  if (!data)
  {
    throw std::runtime_error("Failed to load image: " + std::string(path));
  }
  return {{x,y}, {data, stbi_image_free}};
};
std::pair<glm::uvec2, std::shared_ptr<uint8_t>> ImageLoader::loadAndResize(const std::string_view &path, const glm::uvec2 &resize)
{
  auto imagePair = ImageLoader::load(path);
  auto imageBytesPointer = std::get<1>(imagePair).get();
  auto &imageSize = std::get<0>(imagePair);
  auto resizedBytes = stbir_resize_uint8_srgb(imageBytesPointer, imageSize.x, imageSize.y, 0,
                                          	  0, resize.x, resize.y, 0, STBIR_RGBA);
  if (!resizedBytes)
  {
    throw std::runtime_error("Failed to resize image: " + std::string(path));
  }
  return {resize, {resizedBytes, stbi_image_free}};
};