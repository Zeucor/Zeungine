#include <anex/modules/gl/textures/TextureFactory.hpp>
using namespace anex::modules::gl::textures;
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
	glGenTextures(1, &texture.id);
	GLcheck("glGenTextures");
  if (texture.size.y == 0)
    texture.target = GL_TEXTURE_1D;
  else if (texture.size.z <= 1)
    texture.target = GL_TEXTURE_2D;
  else
    texture.target = GL_TEXTURE_3D;
  texture.use(true);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  GLcheck("glPixelStorei");
  if (texture.target == GL_TEXTURE_1D)
  {
    glTexImage1D(texture.target, 0, internalFormats[texture.format], texture.size.x, 0, formats[texture.format], types[{texture.format, texture.type}], data);
    GLcheck("glTexImage1D");
  }
  else if (texture.target == GL_TEXTURE_2D)
  {
    glTexImage2D(texture.target, 0, internalFormats[texture.format], texture.size.x, texture.size.y, 0, formats[texture.format], types[{texture.format, texture.type}], data);
    GLcheck("glTexImage2D");
  }
  else if (texture.target == GL_TEXTURE_3D)
  {
    glTexImage3D(texture.target, 0, internalFormats[texture.format], texture.size.x, texture.size.y, texture.size.z, 0, formats[texture.format], types[{texture.format, texture.type}], data);
    GLcheck("glTexImage3D");
  }
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
  glTexParameteri(texture.target, GL_TEXTURE_MIN_FILTER, filterType);
  GLcheck("glTexParameteri");
  glTexParameteri(texture.target, GL_TEXTURE_MAG_FILTER, filterType);
  GLcheck("glTexParameteri");
  glTexParameteri(texture.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GLcheck("glTexParameteri");
  glTexParameteri(texture.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GLcheck("glTexParameteri");
  texture.use(false);
};