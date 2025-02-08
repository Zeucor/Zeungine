#include <iostream>
#include <zg/vaos/VAO.hpp>
#include <zg/vaos/VAOFactory.hpp>
#include <zg/interfaces/IEntity.hpp>
#include <zg/renderers/GLRenderer.hpp>
using namespace zg::vaos;
VAO::VAO(Window &_window, const RuntimeConstants &constants, uint32_t indiceCount, uint32_t elementCount):
  constants(constants),
  indiceCount(indiceCount),
  elementCount(elementCount),
  stride(VAOFactory::getStride(constants)),
	vaoWindow(_window)
{
	VAOFactory::generate(*this);
}
VAO::~VAO()
{
  VAOFactory::destroy(*this);
}
void VAO::updateIndices(const std::vector<uint32_t> &indices)
{
  vaoWindow.iRenderer->updateIndicesVAO(*this, indices);
}
template<typename T>
void VAO::updateElements(const std::string_view constant, const std::vector<T> &elements) const
{
  auto elementsAsChar = (uint8_t *)elements.data();
  vaoWindow.iRenderer->updateElementsVAO(*this, constant, elementsAsChar);
}
template void VAO::updateElements<glm::vec2>(const std::string_view , const std::vector<glm::vec2> &) const;
template void VAO::updateElements<glm::vec3>(const std::string_view , const std::vector<glm::vec3> &) const;
template void VAO::updateElements<glm::vec4>(const std::string_view , const std::vector<glm::vec4> &) const;
void VAO::drawVAO() const
{
  vaoWindow.iRenderer->drawVAO(*this);
}