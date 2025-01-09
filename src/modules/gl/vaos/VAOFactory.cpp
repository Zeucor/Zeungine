#include <stdexcept>
#include <anex/modules/gl/vaos/VAOFactory.hpp>
using namespace anex::modules::gl::vaos;
VAOFactory::ConstantSizeMap VAOFactory::constantSizes = {
	{"Indice", {3, sizeof(uint32_t), GL_UNSIGNED_INT}},
  {"Color", {4, sizeof(float), GL_FLOAT}},
	{"Position", {3, sizeof(float), GL_FLOAT}},
	{"Normal", {3, sizeof(float), GL_FLOAT}}
};
VAOFactory::VAOConstantMap VAOFactory::VAOConstants = {
	{"Indice", false},
  {"Color", true},
	{"Position", true},
	{"Normal", true},
	{"View", false},
	{"Projection", false},
	{"Model", false},
	{"CameraPosition", false},
	{"Fog", false},
	{"Lighting", false},
	{"LightSpaceMatrix", false}
};
void VAOFactory::generateVAO(const RuntimeConstants &constants, GLuint& vao, GLuint& vbo, GLuint& ebo, const uint32_t &indiceCount)
{
	glGenVertexArrays(1, &vao);
	GLcheck("glGenVertexArrays");
	glGenBuffers(1, &ebo);
	GLcheck("glGenBuffers");
	glGenBuffers(1, &vbo);
	GLcheck("glGenBuffers");
  glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	GLcheck("glBindBuffer");
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLcheck("glBindBuffer");
  auto stride = getStride(constants);
	glBufferData(GL_ARRAY_BUFFER, stride * indiceCount, 0, GL_STATIC_DRAW);
	GLcheck("glBufferData");
  size_t attribIndex = 0;
  size_t offset = 0;
	for (auto &constant: constants)
  {
    if (!isVAOConstant(constant))
      continue;
	  glEnableVertexAttribArray(attribIndex);
	  GLcheck("glEnableVertexAttribArray");
	  auto &constantSize = constantSizes[constant];
		if (std::get<2>(constantSize) == GL_FLOAT)
	  {
	    glVertexAttribPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), GL_FALSE, stride, (const void*)offset);
	    GLcheck("glVertexAttribPointer");
	  }
	  else
	  {
	    glVertexAttribIPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), stride, (const void*)offset);
	    GLcheck("glVertexAttribIPointer");
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
size_t VAOFactory::getOffset(const RuntimeConstants &constants, const std::string_view &offsetConstant)
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
bool VAOFactory::isVAOConstant(const std::string_view &constant)
{
  return VAOConstants[constant];
}
void VAOFactory::destroyVAO(VAO &vao)
{
  glDeleteVertexArrays(1, &vao.vao);
  GLcheck("glDeleteVertexArrays");
  glDeleteBuffers(1, &vao.vbo);
  GLcheck("glDeleteBuffers");
  glDeleteBuffers(1, &vao.ebo);
  GLcheck("glDeleteBuffers");
};