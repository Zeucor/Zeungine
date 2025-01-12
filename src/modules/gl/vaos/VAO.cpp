#include <anex/modules/gl/vaos/VAO.hpp>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
using namespace anex::modules::gl::vaos;
VAO::VAO(const RuntimeConstants &constants, const uint32_t &indiceCount, const uint32_t &elementCount):
  constants(constants),
  indiceCount(indiceCount),
  elementCount(elementCount),
  stride(VAOFactory::getStride(constants))
{
  VAOFactory::generateVAO(constants, vao, vbo, ebo, indiceCount, elementCount);
};
VAO::~VAO()
{
  VAOFactory::destroyVAO(*this);
};
void VAO::updateIndices(const uint32_t *indices) const
{
  glBindVertexArray(vao);
  GLcheck("glBindVertexArray");
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  GLcheck("glBindBuffer");
  auto &constantSize = VAOFactory::constantSizes["Indice"];
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceCount * std::get<1>(constantSize), indices, GL_STATIC_DRAW);
  GLcheck("glBufferData");
  glBindVertexArray(0);
  GLcheck("glBindVertexArray");
};
void VAO::updateElements(const std::string_view &constant, const void *elements) const
{
  glBindVertexArray(vao);
  GLcheck("glBindVertexArray");
  auto &constantSize = VAOFactory::constantSizes[constant];
  auto offset = VAOFactory::getOffset(constants, constant);
  auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
  auto elementsAsChar = (uint8_t *)elements;
  for (size_t index = offset, c = 1, elementIndex = 0; c <= elementCount; index += stride, c++, elementIndex += elementStride)
  {
    glBufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
    GLcheck("glBufferSubData");
  }
  glBindVertexArray(0);
  GLcheck("glBindVertexArray");
};
void VAO::vaoDraw() const
{
  glBindVertexArray(vao);
  GLcheck("glBindVertexArray");
  GLenum drawMode = GL_TRIANGLES;
  GLenum polygonMode = GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
  GLcheck("glPolygonMode");
  glDrawElements(drawMode, indiceCount, GL_UNSIGNED_INT, 0);
  GLcheck("glDrawElements");
};