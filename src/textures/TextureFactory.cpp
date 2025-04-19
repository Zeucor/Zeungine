#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/interfaces/IRenderer.hpp>
using namespace zg::textures;
#if defined(USE_GL) || defined(USE_EGL)
TextureFactory::InternalFormatsMap TextureFactory::internalFormats = {
    {Texture::Format::RGB8, GL_RGB8},
    {Texture::Format::RGBA8, GL_RGBA8},
    {Texture::Format::RGBA32F, GL_RGBA32F},
    {Texture::Format::Depth, GL_DEPTH_COMPONENT32F},
    {Texture::Format::Stencil, GL_STENCIL_INDEX8},
    {Texture::Format::DepthStencil, GL_DEPTH32F_STENCIL8},
    {Texture::Format::Integer32, GL_R32I}};
TextureFactory::FormatsMap TextureFactory::formats = {
    {Texture::Format::RGB8, GL_RGB},
    {Texture::Format::RGBA8, GL_RGBA},
    {Texture::Format::RGBA32F, GL_RGBA},
    {Texture::Format::Depth, GL_DEPTH_COMPONENT},
#ifdef USE_GL
    {Texture::Format::Stencil, GL_STENCIL_INDEX},
#endif
#ifdef USE_EGL
    {Texture::Format::Stencil, GL_STENCIL_INDEX8},
#endif
    {Texture::Format::DepthStencil, GL_DEPTH_STENCIL},
    {Texture::Format::Integer32, GL_RED_INTEGER}};
TextureFactory::TypesMap TextureFactory::types = {
    {{Texture::Format::RGB8, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
    {{Texture::Format::RGBA8, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
    {{Texture::Format::RGBA8, Texture::Type::Float}, ZG_FLOAT},
    {{Texture::Format::RGBA32F, Texture::Type::Float}, ZG_FLOAT},
    {{Texture::Format::DepthStencil, Texture::Type::UnsignedInt24_8}, ZG_UNSIGNED_INT_24_8},
    {{Texture::Format::DepthStencil, Texture::Type::Float}, ZG_FLOAT_32_UNSIGNED_INT_24_8_REV},
    {{Texture::Format::Stencil, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
    {{Texture::Format::Depth, Texture::Type::Float}, ZG_FLOAT},
    {{Texture::Format::Integer32, Texture::Type::Int}, GL_INT}};
#endif
void TextureFactory::initTexture(Texture &texture, const void *data)
{
  preInitTexture(texture);
  auto imageCount = texture.size.w > 0 ? 6 : texture.size.z;
  std::vector<images::ImageLoader::ImagePair> imagePairs;
  for (int i = 0; i < imageCount; i++)
  {
    imagePairs.push_back({{texture.size.x, texture.size.y}, {(uint8_t *)data, [](uint8_t *) {}}});
    if (data)
    {
      auto bytessize = texture.size.x * texture.size.y * 4;
      char* chardata = (char*)malloc(bytessize);
      memcpy(chardata, data, bytessize);
      texture.datas.push_back(std::pair<size_t, std::shared_ptr<char>>(bytessize, std::shared_ptr<char>(chardata, free)));
    }
  }
  midInitTexture(texture, imagePairs);
  postInitTexture(texture);
}
void TextureFactory::initTexture(Texture &texture, const std::vector<void *> datas)
{
  preInitTexture(texture);
  auto imageCount = texture.size.w > 0 ? 6 : texture.size.z;
  std::vector<images::ImageLoader::ImagePair> imagePairs;
  for (int i = 0; i < imageCount; i++)
  {
    imagePairs.push_back({{texture.size.x, texture.size.y}, {(uint8_t *)datas[i], [](uint8_t *) {}}});
    if (datas[i])
    {
      auto bytessize = texture.size.x * texture.size.y * 4;
      char* chardata = (char*)malloc(bytessize);
      memcpy(chardata, datas[i], bytessize);
      texture.datas.push_back(std::pair<size_t, std::shared_ptr<char>>(bytessize, std::shared_ptr<char>(chardata, free)));
    }
  }
  midInitTexture(texture, imagePairs);
  postInitTexture(texture);
}
void TextureFactory::initTexture(Texture &texture, const std::string_view path)
{
  preInitTexture(texture);
  auto imagePair = images::ImageLoader::load(path);
  midInitTexture(texture, {{imagePair}});
  postInitTexture(texture);
};
void TextureFactory::initTexture(Texture &texture, const std::vector<std::string_view> &paths)
{
  std::vector<images::ImageLoader::ImagePair> imagePairs;
  for (const auto &path : paths)
  {
    imagePairs.push_back(images::ImageLoader::load(path));
  }
  if (!texture.size.x || !texture.size.y && imagePairs.size())
  {
    auto firstImageSize = imagePairs[0].first;
    texture.size.x = firstImageSize.x;
    texture.size.y = firstImageSize.y;
  }
  preInitTexture(texture);
  midInitTexture(texture, imagePairs);
  postInitTexture(texture);
};
void TextureFactory::preInitTexture(Texture &texture)
{
  texture.iRenderer->preInitTexture(texture);
};
void TextureFactory::midInitTexture(const Texture &texture, const std::vector<images::ImageLoader::ImagePair> &images)
{
  texture.iRenderer->midInitTexture(texture, images);
}
void TextureFactory::postInitTexture(const Texture &texture)
{
  texture.iRenderer->postInitTexture(texture);
};
void TextureFactory::destroyTexture(Texture &texture)
{
  texture.iRenderer->destroyTexture(texture);
};