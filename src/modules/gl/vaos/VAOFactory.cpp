#include <stdexcept>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
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
	vao.window.glContext->GenVertexArrays(1, &vao.vao);
	GLcheck(vao.window, "glGenVertexArrays");
	vao.window.glContext->GenBuffers(1, &vao.ebo);
	GLcheck(vao.window, "glGenBuffers");
	vao.window.glContext->GenBuffers(1, &vao.vbo);
	GLcheck(vao.window, "glGenBuffers");
  vao.window.glContext->BindVertexArray(vao.vao);
	GLcheck(vao.window, "glBindVertexArray");
	vao.window.glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
	GLcheck(vao.window, "glBindBuffer");
	vao.window.glContext->BindBuffer(GL_ARRAY_BUFFER, vao.vbo);
	GLcheck(vao.window, "glBindBuffer");
  auto stride = getStride(constants);
	vao.window.glContext->BufferData(GL_ARRAY_BUFFER, stride * elementCount, 0, GL_STATIC_DRAW);
	GLcheck(vao.window, "glBufferData");
  size_t attribIndex = 0;
  size_t offset = 0;
	for (auto &constant: constants)
  {
    if (!isVAOConstant(constant))
      continue;
	  vao.window.glContext->EnableVertexAttribArray(attribIndex);
	  GLcheck(vao.window, "glEnableVertexAttribArray");
	  auto &constantSize = constantSizes[constant];
		if (std::get<2>(constantSize) == GL_FLOAT)
	  {
	    vao.window.glContext->VertexAttribPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), GL_FALSE, stride, (const void*)offset);
	    GLcheck(vao.window, "glVertexAttribPointer");
	  }
	  else
	  {
	    vao.window.glContext->VertexAttribIPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), stride, (const void*)offset);
	    GLcheck(vao.window, "glVertexAttribIPointer");
	  }
    offset += std::get<0>(constantSize) * std::get<1>(constantSize);
		attribIndex++;
  }
  // glBindVertexArray(0);
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
  vao.window.glContext->DeleteVertexArrays(1, &vao.vao);
  GLcheck(vao.window, "glDeleteVertexArrays");
  vao.window.glContext->DeleteBuffers(1, &vao.vbo);
  GLcheck(vao.window, "glDeleteBuffers");
  vao.window.glContext->DeleteBuffers(1, &vao.ebo);
  GLcheck(vao.window, "glDeleteBuffers");
};