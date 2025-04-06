#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/Texture.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/Serial.hpp>
using namespace zg::textures;
Texture::Texture(Window &window, const glm::ivec4 &size, const void *data, const Format &format, const Type &type, const FilterType &filterType) : window(window),
                                                                                                                                                   size(size),
                                                                                                                                                   format(format),
                                                                                                                                                   type(type),
                                                                                                                                                   filterType(filterType)
{
  TextureFactory::initTexture(*this, data);
}
Texture::Texture(Window &window, const glm::ivec4 &size, const std::vector<void *> datas, const Format &format, const Type &type, const FilterType &filterType) : window(window),
                                                                                                                                                   size(size),
                                                                                                                                                   format(format),
                                                                                                                                                   type(type),
                                                                                                                                                   filterType(filterType)
{
  TextureFactory::initTexture(*this, datas);
}
Texture::Texture(Window &window, const glm::ivec4 &size, const std::string_view path, const Format &format, const Type &type, const FilterType &filterType) : window(window),
                                                                                                                                                              size(size),
                                                                                                                                                              format(format),
                                                                                                                                                              type(type),
                                                                                                                                                              filterType(filterType)
{
  TextureFactory::initTexture(*this, path);
}
Texture::Texture(Window &window, const glm::ivec4 &size, const std::vector<std::string_view> &paths, const Format &format, const Type &type, const FilterType &filterType) : window(window),
                                                                                                                                                                             size(size),
                                                                                                                                                                             format(format),
                                                                                                                                                                             type(type),
                                                                                                                                                                             filterType(filterType)
{
  TextureFactory::initTexture(*this, paths);
}
Texture::~Texture()
{
  TextureFactory::destroyTexture(*this);
}
void Texture::bind() const
{
  window.iRenderer->bindTexture(*this);
}
void Texture::unbind() const
{
  window.iRenderer->unbindTexture(*this);
}
void Texture::update(const void *data)
{
}
void Texture::update(const std::string_view path)
{
}
void Texture::update(const std::vector<std::string_view> &paths)
{
}
template<>
Serial& serialize(Serial& serial, const std::shared_ptr<zg::textures::Texture>& texturePointer)
{
  auto& texture = *texturePointer;
  serial << true;
  auto textureDatasSize = texture.datas.size();
  serial << texture.size << textureDatasSize;
  for (auto& dataPair : texture.datas)
  {
    serial << dataPair.first;
    serial.writeBytes(dataPair.second.get(), dataPair.first);
  }
  serial << texture.format << texture.type << texture.filterType;
  return serial;
}
template<>
Serial& deserialize(Serial& serial, std::shared_ptr<zg::textures::Texture>& texturePointer)
{
  bool wroteBit = false;
  serial >> wroteBit;
  if (!wroteBit)
    return serial;
  glm::ivec4 size;
  serial >> size;
  std::vector<void*> datas;
  auto textureDatasSize = datas.size();
  serial >> textureDatasSize;
  datas.resize(textureDatasSize);
  for (auto& pointer : datas)
  {
    size_t datasize = 0;
    serial >> datasize;
    char* data = (char*)malloc(datasize);
    serial.readBytes(data, datasize);
    datas.push_back(data);
  }
  zg::textures::Texture::Format format;
  zg::textures::Texture::Type type;
  zg::textures::Texture::FilterType filterType;
  serial >> format >> type >> filterType;
  auto windowPointer = (zg::Window*)serial.getContextPointer("Window");
  texturePointer = std::make_shared<zg::textures::Texture>(*windowPointer, size, datas, format, type, filterType);
  return serial;
}