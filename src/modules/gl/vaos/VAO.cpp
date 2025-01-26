#include <iostream>
#include <zg/modules/gl/vaos/VAO.hpp>
#include <zg/modules/gl/vaos/VAOFactory.hpp>
#include <zg/IEntity.hpp>
#include <zg/modules/gl/renderers/GLRenderer.hpp>
using namespace zg::modules::gl::vaos;
VAO::VAO(RenderWindow &_window, const RuntimeConstants &constants, uint32_t indiceCount, uint32_t elementCount):
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
  auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(vaoWindow.iVendorRenderer);
  glRenderer.glContext->BindVertexArray(vao);
  GLcheck(glRenderer, "glBindVertexArray");
  glRenderer.glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  GLcheck(glRenderer, "glBindBuffer");
  auto &constantSize = VAOFactory::constantSizes["Indice"];
  glRenderer.glContext->BufferData(GL_ELEMENT_ARRAY_BUFFER, indiceCount * std::get<1>(constantSize), indices.data(), GL_STATIC_DRAW);
  GLcheck(glRenderer, "glBufferData");
  glRenderer.glContext->BindVertexArray(0);
  GLcheck(glRenderer, "glBindVertexArray");
};
template<typename T>
void VAO::updateElements(const std::string_view constant, const std::vector<T> &elements) const
{
  auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(vaoWindow.iVendorRenderer);
  glRenderer.glContext->BindVertexArray(vao);
  GLcheck(glRenderer, "glBindVertexArray");
  glRenderer.glContext->BindBuffer(GL_ARRAY_BUFFER, vbo);
  GLcheck(glRenderer, "glBindBuffer");
  auto &constantSize = VAOFactory::constantSizes[constant];
  auto offset = VAOFactory::getOffset(constants, constant);
  auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
  auto elementsAsChar = (uint8_t *)elements.data();
  for (size_t index = offset, c = 1, elementIndex = 0; c <= elementCount; index += stride, c++, elementIndex += elementStride)
  {
    glRenderer.glContext->BufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
    GLcheck(glRenderer, "glBufferSubData");
  }
  glRenderer.glContext->BindVertexArray(0);
  GLcheck(glRenderer, "glBindVertexArray");
};
template void VAO::updateElements<glm::vec2>(const std::string_view , const std::vector<glm::vec2> &) const;
template void VAO::updateElements<glm::vec3>(const std::string_view , const std::vector<glm::vec3> &) const;
template void VAO::updateElements<glm::vec4>(const std::string_view , const std::vector<glm::vec4> &) const;
void VAO::vaoDraw() const
{
  auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(vaoWindow.iVendorRenderer);
  glRenderer.glContext->BindVertexArray(vao);
  GLcheck(glRenderer, "glBindVertexArray");
  GLenum drawMode = GL_TRIANGLES;
  GLenum polygonMode = GL_FILL;
  glRenderer.glContext->PolygonMode(GL_FRONT_AND_BACK, polygonMode);
  GLcheck(glRenderer, "glPolygonMode");
  glRenderer.glContext->DrawElements(drawMode, indiceCount, GL_UNSIGNED_INT, 0);
  GLcheck(glRenderer, "glDrawElements");
};