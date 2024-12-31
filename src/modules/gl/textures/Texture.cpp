#include <anex/modules/gl/textures/Texture.hpp>
#include <anex/modules/gl/textures/TextureFactory.hpp>
using namespace anex::modules::gl::textures;
Texture::Texture(const glm::ivec3 &size, const void *data, const Format &format, const Type &type, const FilterType &filterType):
	size(size),
  format(format),
  type(type),
  filterType(filterType)
{
  TextureFactory::initTexture(*this, data);
};
void Texture::use(const bool &useTexture)
{
  if (useTexture)
  {
    glBindTexture(target, id);
    GLcheck("glBindTexture");
  }
  else
  {
    glBindTexture(target, 0);
    GLcheck("glBindTexture");
  }
};