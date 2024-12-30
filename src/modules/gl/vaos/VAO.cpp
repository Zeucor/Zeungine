#include <anex/modules/gl/vaos/VAO.hpp>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
using namespace anex::modules::gl::vaos;
VAO::VAO(const RuntimeConstants &constants, const uint32_t &indiceCount):
  constants(constants),
  indiceCount(indiceCount),
  stride(VAOFactory::getStride(constants))
{
  VAOFactory::generateVAO(constants, vao, vbo, ebo, indiceCount);
};
VAO::~VAO()
{
  VAOFactory::destroyVAO(*this);
};
void VAO::updateIndices(const uint32_t *indices)
{
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  auto &constantSize = VAOFactory::constantSizes["Indice"];
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceCount * std::get<1>(constantSize), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);
};
void VAO::updateElements(const std::string &constant, const void *elements)
{
  glBindVertexArray(vao);
  auto &constantSize = VAOFactory::constantSizes[constant];
  auto offset = VAOFactory::getOffset(constants, constant);
  auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
  auto elementsAsChar = (uint8_t *)elements;
  for (size_t index = offset, c = 1, elementIndex = 0; c <= indiceCount; index += stride, c++, elementIndex += elementStride)
  {
    glBufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
  }
  glBindVertexArray(0);
};
void VAO::vaoDraw()
{
  glBindVertexArray(vao);
  GLenum drawMode = GL_TRIANGLES;
  GLenum polygonMode = GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
  glDrawElements(drawMode, indiceCount, GL_UNSIGNED_INT, 0);
};