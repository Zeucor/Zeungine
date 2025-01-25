#include <zg/modules/gl/textures/TextureFactory.hpp>
using namespace zg::modules::gl::textures;
TextureFactory::InternalFormatsMap TextureFactory::internalFormats = {
  {Texture::Format::RGB8, GL_RGB8},
  {Texture::Format::RGBA8, GL_RGBA8},
  {Texture::Format::RGBA32F, GL_RGBA32F},
  {Texture::Format::Depth, GL_DEPTH_COMPONENT32F},
  {Texture::Format::Stencil, GL_STENCIL_INDEX8},
  {Texture::Format::DepthStencil, GL_DEPTH32F_STENCIL8},
  {Texture::Format::Integer32, GL_R32I}
};
TextureFactory::FormatsMap TextureFactory::formats = {
  {Texture::Format::RGB8, GL_RGB},
  {Texture::Format::RGBA8, GL_RGBA},
  {Texture::Format::RGBA32F, GL_RGBA},
  {Texture::Format::Depth, GL_DEPTH_COMPONENT},
  {Texture::Format::Stencil, GL_STENCIL_INDEX},
  {Texture::Format::DepthStencil, GL_DEPTH_STENCIL},
  {Texture::Format::Integer32, GL_RED_INTEGER}
};
TextureFactory::TypesMap TextureFactory::types = {
  {{Texture::Format::RGB8, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
  {{Texture::Format::RGBA8, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
  {{Texture::Format::RGBA8, Texture::Type::Float}, GL_FLOAT},
  {{Texture::Format::RGBA32F, Texture::Type::Float}, GL_FLOAT},
  {{Texture::Format::DepthStencil, Texture::Type::UnsignedInt24_8}, GL_UNSIGNED_INT_24_8},
  {{Texture::Format::DepthStencil, Texture::Type::Float}, GL_FLOAT_32_UNSIGNED_INT_24_8_REV},
  {{Texture::Format::Stencil, Texture::Type::UnsignedByte}, GL_UNSIGNED_BYTE},
  {{Texture::Format::Depth, Texture::Type::Float}, GL_FLOAT},
  {{Texture::Format::Integer32, Texture::Type::Int}, GL_INT}
};
void TextureFactory::initTexture(Texture &texture, const void *data)
{
  preInitTexture(texture);
  auto imageCount = texture.size.w > 0 ? 6 : texture.size.z;
  std::vector<images::ImageLoader::ImagePair> imagePairs;
  for (int i = 0; i < imageCount; i++)
  {
    imagePairs.push_back({{texture.size.x, texture.size.y}, {(uint8_t*)data, [](uint8_t*) {}}});
  }
  midInitTexture(texture, imagePairs);
  postInitTexture(texture);
};
void TextureFactory::initTexture(Texture &texture, const std::string_view path)
{
  preInitTexture(texture);
  auto imagePair = images::ImageLoader::load(path);
  midInitTexture(texture, {{imagePair}});
  postInitTexture(texture);
};
void TextureFactory::initTexture(Texture &texture, const std::vector<std::string_view> &paths)
{
  preInitTexture(texture);
  std::vector<images::ImageLoader::ImagePair> imagePairs;
  for (const auto &path : paths)
  {
    imagePairs.push_back(images::ImageLoader::load(path));
  }
  midInitTexture(texture, imagePairs);
  postInitTexture(texture);
};
void TextureFactory::preInitTexture(Texture& texture)
{
  texture.window.glContext->GenTextures(1, &texture.id);
  GLcheck(texture.window, "glGenTextures");
  if (texture.size.w > 0)
    texture.target = GL_TEXTURE_CUBE_MAP;
  else if (texture.size.y == 0)
    texture.target = GL_TEXTURE_1D;
  else if (texture.size.z <= 1)
    texture.target = GL_TEXTURE_2D;
  else
    texture.target = GL_TEXTURE_3D;
  texture.bind();
  texture.window.glContext->PixelStorei(GL_PACK_ALIGNMENT, 1);
  GLcheck(texture.window, "glPixelStorei");
};
void TextureFactory::midInitTexture(const Texture& texture, const std::vector<images::ImageLoader::ImagePair>& images)
{
  if (texture.target == GL_TEXTURE_1D)
  {
    void *data = images.size() ? std::get<1>(images[0]).get() : 0;
    texture.window.glContext->TexImage1D(texture.target, 0, internalFormats[texture.format], texture.size.x, 0, formats[texture.format], types[{texture.format, texture.type}], data);
    GLcheck(texture.window, "glTexImage1D");
  }
  else if (texture.target == GL_TEXTURE_2D)
  {
    void *data = images.size() ? std::get<1>(images[0]).get() : 0;
    texture.window.glContext->TexImage2D(texture.target, 0, internalFormats[texture.format], texture.size.x, texture.size.y, 0, formats[texture.format], types[{texture.format, texture.type}], data);
    GLcheck(texture.window, "glTexImage2D");
  }
  else if (texture.target == GL_TEXTURE_3D)
  {
    void *data = images.size() ? std::get<1>(images[0]).get() : 0;
    texture.window.glContext->TexImage3D(texture.target, 0, internalFormats[texture.format], texture.size.x, texture.size.y, texture.size.z, 0, formats[texture.format], types[{texture.format, texture.type}], data);
    GLcheck(texture.window, "glTexImage3D");
  }
  else if (texture.target == GL_TEXTURE_CUBE_MAP)
  {
    for (uint8_t face = 0; face < 6; ++face)
    {
      bool haveImage = images.size() > face;
      auto imageSize = haveImage ? std::get<0>(images[face]) : glm::uvec2(0, 0);
      void *data = haveImage ? std::get<1>(images[face]).get() : 0;
      texture.window.glContext->TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormats[texture.format], imageSize.x, imageSize.y, 0, formats[texture.format], types[{texture.format, texture.type}], data);
      GLcheck(texture.window, "glTexImage2D");
    }
  }
}
void TextureFactory::postInitTexture(const Texture& texture)
{
  GLenum filterType;
  switch (texture.filterType)
  {
  case Texture::FilterType::Nearest:
    filterType = GL_NEAREST;
    break;
  case Texture::FilterType::Linear:
    filterType = GL_LINEAR;
    break;
  }
  texture.window.glContext->TexParameteri(texture.target, GL_TEXTURE_MIN_FILTER, filterType);
  GLcheck(texture.window, "glTexParameteri");
  texture.window.glContext->TexParameteri(texture.target, GL_TEXTURE_MAG_FILTER, filterType);
  GLcheck(texture.window, "glTexParameteri");
  texture.window.glContext->TexParameteri(texture.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GLcheck(texture.window, "glTexParameteri");
  texture.window.glContext->TexParameteri(texture.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GLcheck(texture.window, "glTexParameteri");
  texture.unbind();
};
void TextureFactory::destroyTexture(Texture& texture)
{
  texture.window.glContext->DeleteTextures(1, &texture.id);
  GLcheck(texture.window, "glDeleteTextures");
};