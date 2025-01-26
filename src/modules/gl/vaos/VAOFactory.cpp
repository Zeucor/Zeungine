#include <stdexcept>
#include <zg/IEntity.hpp>
#include <zg/modules/gl/renderers/GLRenderer.hpp>
#include <zg/modules/gl/vaos/VAOFactory.hpp>
using namespace zg::modules::gl::vaos;
VAOFactory::ConstantSizeMap VAOFactory::constantSizes = {
	{"Indice", {3, sizeof(uint32_t), GL_UNSIGNED_INT}},
  {"Color", {4, sizeof(float), GL_FLOAT}},
	{"Position", {3, sizeof(float), GL_FLOAT}},
	{"Normal", {3, sizeof(float), GL_FLOAT}},
	{"UV2", {2, sizeof(float), GL_FLOAT}},
	{"UV3", {3, sizeof(float), GL_FLOAT}}
};
VAOFactory::VAOConstantMap VAOFactory::VAOConstants = {
	{"Indice", false},
  {"Color", true},
	{"Position", true},
	{"Normal", true},
	{"UV2", true},
	{"UV3", true},
	{"View", false},
	{"Projection", false},
	{"Model", false},
	{"CameraPosition", false},
	{"Fog", false},
	{"Lighting", false},
	{"LightSpaceMatrix", false}
};
void VAOFactory::generateVAO(const RuntimeConstants &constants, VAO& vao, uint32_t elementCount)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(vao.vaoWindow.iVendorRenderer);
	glRenderer.glContext->GenVertexArrays(1, &vao.vao);
	GLcheck(glRenderer, "glGenVertexArrays");
	glRenderer.glContext->GenBuffers(1, &vao.ebo);
	GLcheck(glRenderer, "glGenBuffers");
	glRenderer.glContext->GenBuffers(1, &vao.vbo);
	GLcheck(glRenderer, "glGenBuffers");
  glRenderer.glContext->BindVertexArray(vao.vao);
	GLcheck(glRenderer, "glBindVertexArray");
	glRenderer.glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
	GLcheck(glRenderer, "glBindBuffer");
	glRenderer.glContext->BindBuffer(GL_ARRAY_BUFFER, vao.vbo);
	GLcheck(glRenderer, "glBindBuffer");
  auto stride = getStride(constants);
	glRenderer.glContext->BufferData(GL_ARRAY_BUFFER, stride * elementCount, 0, GL_STATIC_DRAW);
	GLcheck(glRenderer, "glBufferData");
  size_t attribIndex = 0;
  size_t offset = 0;
	for (auto &constant: constants)
  {
    if (!isVAOConstant(constant))
      continue;
	  glRenderer.glContext->EnableVertexAttribArray(attribIndex);
	  GLcheck(glRenderer, "glEnableVertexAttribArray");
	  auto &constantSize = constantSizes[constant];
		if (std::get<2>(constantSize) == GL_FLOAT)
	  {
			glRenderer.glContext->VertexAttribPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), GL_FALSE, stride, (const void*)offset);
			GLcheck(glRenderer, "glVertexAttribPointer");
	  }
	  else
	  {
			glRenderer.glContext->VertexAttribIPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), stride, (const void*)offset);
			GLcheck(glRenderer, "glVertexAttribIPointer");
	  }
    offset += std::get<0>(constantSize) * std::get<1>(constantSize);
		attribIndex++;
  }
};
size_t VAOFactory::getStride(const RuntimeConstants &constants)
{
  size_t stride = 0;
  for (auto &constant: constants)
  {
    if (!isVAOConstant(constant))
      continue;
    auto &constantSize = constantSizes[constant];
    stride += std::get<0>(constantSize) * std::get<1>(constantSize);
  };
  return stride;
};
size_t VAOFactory::getOffset(const RuntimeConstants &constants, const std::string_view offsetConstant)
{
	size_t stride = 0;
	for (auto &constant: constants)
	{
		if (!isVAOConstant(constant))
			continue;
		if (constant == offsetConstant)
			return stride;
		auto &constantSize = constantSizes[constant];
		stride += std::get<0>(constantSize) * std::get<1>(constantSize);
	};
	throw std::runtime_error("No such constant");
};
bool VAOFactory::isVAOConstant(const std::string_view constant)
{
  return VAOConstants[constant];
}
void VAOFactory::destroyVAO(VAO &vao)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(vao.vaoWindow.iVendorRenderer);
  glRenderer.glContext->DeleteVertexArrays(1, &vao.vao);
  vao.vao = 0;
  GLcheck(glRenderer, "glDeleteVertexArrays");
  glRenderer.glContext->DeleteBuffers(1, &vao.vbo);
  vao.vbo = 0;
  GLcheck(glRenderer, "glDeleteBuffers");
  glRenderer.glContext->DeleteBuffers(1, &vao.ebo);
  vao.ebo = 0;
  GLcheck(glRenderer, "glDeleteBuffers");
};