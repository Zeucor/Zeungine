#include <iostream>
#include <zg/vaos/VAO.hpp>
#include <zg/vaos/VAOFactory.hpp>
#include <zg/renderers/GLRenderer.hpp>
using namespace zg::vaos;
VAO::VAO(IRenderer* iRenderer, const RuntimeConstants &constants, uint32_t indiceCount, uint32_t vertexCount):
  constants(constants),
  indiceCount(indiceCount),
  vertexCount(vertexCount),
  stride(VAOFactory::getStride(constants)),
	vaoIRenderer(iRenderer)
{
	VAOFactory::generate(*this);
}
VAO::VAO(){};
VAO& VAO::operator=(const VAO& other)
{
  constants = other.constants;
  indiceCount = other.indiceCount;
  vertexCount = other.vertexCount;
  stride = other.stride;
  vaoIRenderer = other.vaoIRenderer;
  VAOFactory::generate(*this);
  return *this;
}
VAO::~VAO()
{
  VAOFactory::destroy(*this);
}
void VAO::updateIndices(const std::vector<uint32_t> &indices)
{
  vaoIRenderer->updateIndicesVAO(*this, indices);
}
template<typename T>
void VAO::updateElements(const std::string_view constant, const std::vector<T> &elements) const
{
  auto elementsAsChar = (uint8_t *)elements.data();
  vaoIRenderer->updateElementsVAO(*this, constant, elementsAsChar);
}
template void VAO::updateElements<glm::vec2>(const std::string_view , const std::vector<glm::vec2> &) const;
template void VAO::updateElements<glm::vec3>(const std::string_view , const std::vector<glm::vec3> &) const;
template void VAO::updateElements<glm::vec4>(const std::string_view , const std::vector<glm::vec4> &) const;
void VAO::drawVAO() const
{
  vaoIRenderer->drawVAO(*this);
}