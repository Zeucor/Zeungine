#include <anex/modules/gl/shaders/Shader.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
using namespace anex::modules::gl::shaders;
Shader::Shader(const RuntimeConstants &constants):
	shaders(ShaderFactory::generateShaderMap(constants))
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