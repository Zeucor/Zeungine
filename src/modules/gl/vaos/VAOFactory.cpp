#include <stdexcept>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
#include <anex/IEntity.hpp>
using namespace anex::modules::gl::vaos;
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
	vao.vaoWindow.glContext->GenVertexArrays(1, &vao.vao);
	GLcheck(vao.vaoWindow, "glGenVertexArrays");
	vao.vaoWindow.glContext->GenBuffers(1, &vao.ebo);
	GLcheck(vao.vaoWindow, "glGenBuffers");
	vao.vaoWindow.glContext->GenBuffers(1, &vao.vbo);
	GLcheck(vao.vaoWindow, "glGenBuffers");
  vao.vaoWindow.glContext->BindVertexArray(vao.vao);
	GLcheck(vao.vaoWindow, "glBindVertexArray");
	vao.vaoWindow.glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
	GLcheck(vao.vaoWindow, "glBindBuffer");
	vao.vaoWindow.glContext->BindBuffer(GL_ARRAY_BUFFER, vao.vbo);
	GLcheck(vao.vaoWindow, "glBindBuffer");
  auto stride = getStride(constants);
	vao.vaoWindow.glContext->BufferData(GL_ARRAY_BUFFER, stride * elementCount, 0, GL_STATIC_DRAW);
	GLcheck(vao.vaoWindow, "glBufferData");
  size_t attribIndex = 0;
  size_t offset = 0;
	for (auto &constant: constants)
  {
    if (!isVAOConstant(constant))
      continue;
	  vao.vaoWindow.glContext->EnableVertexAttribArray(attribIndex);
	  GLcheck(vao.vaoWindow, "glEnableVertexAttribArray");
	  auto &constantSize = constantSizes[constant];
		if (std::get<2>(constantSize) == GL_FLOAT)
	  {
			vao.vaoWindow.glContext->VertexAttribPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), GL_FALSE, stride, (const void*)offset);
			GLcheck(vao.vaoWindow, "glVertexAttribPointer");
	  }
	  else
	  {
			vao.vaoWindow.glContext->VertexAttribIPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), stride, (const void*)offset);
			GLcheck(vao.vaoWindow, "glVertexAttribIPointer");
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
  vao.vaoWindow.glContext->DeleteVertexArrays(1, &vao.vao);
  vao.vao = 0;
  GLcheck(vao.vaoWindow, "glDeleteVertexArrays");
  vao.vaoWindow.glContext->DeleteBuffers(1, &vao.vbo);
  vao.vbo = 0;
  GLcheck(vao.vaoWindow, "glDeleteBuffers");
  vao.vaoWindow.glContext->DeleteBuffers(1, &vao.ebo);
  vao.ebo = 0;
  GLcheck(vao.vaoWindow, "glDeleteBuffers");
};