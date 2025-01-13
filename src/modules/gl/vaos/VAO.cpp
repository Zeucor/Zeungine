#include <anex/modules/gl/vaos/VAO.hpp>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
using namespace anex::modules::gl::vaos;
VAO::VAO(GLWindow &window, const RuntimeConstants &constants, const uint32_t &indiceCount, const uint32_t &elementCount):
  window(window),
  constants(constants),
  indiceCount(indiceCount),
  elementCount(elementCount),
  stride(VAOFactory::getStride(constants))
{
  VAOFactory::generateVAO(constants, *this, elementCount);
};
VAO::~VAO()
{
  VAOFactory::destroyVAO(*this);
};
void VAO::updateIndices(const uint32_t *indices) const
{
  window.glContext.BindVertexArray(vao);
  GLcheck(window, "glBindVertexArray");
  window.glContext.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  GLcheck(window, "glBindBuffer");
  auto &constantSize = VAOFactory::constantSizes["Indice"];
  window.glContext.BufferData(GL_ELEMENT_ARRAY_BUFFER, indiceCount * std::get<1>(constantSize), indices, GL_STATIC_DRAW);
  GLcheck(window, "glBufferData");
  window.glContext.BindVertexArray(0);
  GLcheck(window, "glBindVertexArray");
};
void VAO::updateElements(const std::string_view &constant, const void *elements) const
{
  window.glContext.BindVertexArray(vao);
  GLcheck(window, "glBindVertexArray");
  auto &constantSize = VAOFactory::constantSizes[constant];
  auto offset = VAOFactory::getOffset(constants, constant);
  auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
  auto elementsAsChar = (uint8_t *)elements;
  for (size_t index = offset, c = 1, elementIndex = 0; c <= elementCount; index += stride, c++, elementIndex += elementStride)
  {
    window.glContext.BufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
    GLcheck(window, "glBufferSubData");
  }
  window.glContext.BindVertexArray(0);
  GLcheck(window, "glBindVertexArray");
};
void VAO::vaoDraw() const
{
  window.glContext.BindVertexArray(vao);
  GLcheck(window, "glBindVertexArray");
  GLenum drawMode = GL_TRIANGLES;
  GLenum polygonMode = GL_FILL;
  window.glContext.PolygonMode(GL_FRONT_AND_BACK, polygonMode);
  GLcheck(window, "glPolygonMode");
  window.glContext.DrawElements(drawMode, indiceCount, GL_UNSIGNED_INT, 0);
  GLcheck(window, "glDrawElements");
};