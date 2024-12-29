#include <anex/modules/gl/shaders/Shader.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
using namespace anex::modules::gl::shaders;
Shader::Shader(const RuntimeConstants &constants):
	shaders(ShaderFactory::generateShaderMap(constants, *this))
{
  ShaderFactory::compileProgram(shaders, program);
};
Shader::~Shader()
{
  ShaderFactory::deleteProgram(*this);
};
void Shader::use(const bool &useProgram)
{
  if (useProgram)
  {
    glUseProgram(program);
  }
  else
  {
    glUseProgram(0);
  }
};
void Shader::addUBO(const std::string &name, const uint32_t &size, const uint32_t &bindingIndex)
{
  GLuint uboBufferIndex;
  glGenBuffers(1, &uboBufferIndex);
  auto& uboBinding = uboBindings[name];
  std::get<0>(uboBinding) = bindingIndex;
  std::get<1>(uboBinding) = uboBufferIndex;
};