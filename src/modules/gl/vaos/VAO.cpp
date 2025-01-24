#include <iostream>
#include <anex/modules/gl/vaos/VAO.hpp>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
#include <anex/IEntity.hpp>
using namespace anex::modules::gl::vaos;
VAO::VAO(GLWindow &_window, const RuntimeConstants &constants, uint32_t indiceCount, uint32_t elementCount):
  constants(constants),
  indiceCount(indiceCount),
  elementCount(elementCount),
  stride(VAOFactory::getStride(constants)),
	vaoWindow(_window)
{
	VAOFactory::generateVAO(constants, *this, elementCount);
};
VAO::~VAO()
{
  VAOFactory::destroyVAO(*this);
};
void VAO::updateIndices(const std::vector<uint32_t> &indices)
{
  vaoWindow.glContext->BindVertexArray(vao);
  GLcheck(vaoWindow, "glBindVertexArray");
  vaoWindow.glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  GLcheck(vaoWindow, "glBindBuffer");
  auto &constantSize = VAOFactory::constantSizes["Indice"];
  vaoWindow.glContext->BufferData(GL_ELEMENT_ARRAY_BUFFER, indiceCount * std::get<1>(constantSize), indices.data(), GL_STATIC_DRAW);
  GLcheck(vaoWindow, "glBufferData");
  vaoWindow.glContext->BindVertexArray(0);
  GLcheck(vaoWindow, "glBindVertexArray");
};
template<typename T>
void VAO::updateElements(const std::string_view constant, const std::vector<T> &elements) const
{
  vaoWindow.glContext->BindVertexArray(vao);
  GLcheck(vaoWindow, "glBindVertexArray");
  vaoWindow.glContext->BindBuffer(GL_ARRAY_BUFFER, vbo);
  GLcheck(vaoWindow, "glBindBuffer");
  auto &constantSize = VAOFactory::constantSizes[constant];
  auto offset = VAOFactory::getOffset(constants, constant);
  auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
  auto elementsAsChar = (uint8_t *)elements.data();
  for (size_t index = offset, c = 1, elementIndex = 0; c <= elementCount; index += stride, c++, elementIndex += elementStride)
  {
    vaoWindow.glContext->BufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
    GLcheck(vaoWindow, "glBufferSubData");
  }
  vaoWindow.glContext->BindVertexArray(0);
  GLcheck(vaoWindow, "glBindVertexArray");
};
template void VAO::updateElements<glm::vec2>(const std::string_view , const std::vector<glm::vec2> &) const;
template void VAO::updateElements<glm::vec3>(const std::string_view , const std::vector<glm::vec3> &) const;
template void VAO::updateElements<glm::vec4>(const std::string_view , const std::vector<glm::vec4> &) const;
void VAO::vaoDraw() const
{
  vaoWindow.glContext->BindVertexArray(vao);
  GLcheck(vaoWindow, "glBindVertexArray");
  GLenum drawMode = GL_TRIANGLES;
  GLenum polygonMode = GL_FILL;
  vaoWindow.glContext->PolygonMode(GL_FRONT_AND_BACK, polygonMode);
  GLcheck(vaoWindow, "glPolygonMode");
  vaoWindow.glContext->DrawElements(drawMode, indiceCount, GL_UNSIGNED_INT, 0);
  GLcheck(vaoWindow, "glDrawElements");
};