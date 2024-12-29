#pragma once
#include "../common.hpp"
#include <map>
#include <string>
#include <vector>
#include <functional>
namespace anex::modules::gl::shaders
{
	using RuntimeConstants = std::vector<std::string>;
	struct Shader
  {
		enum class ShaderType
    {
      Unknown = 0,
      Vertex,
      Geometry,
      Fragment,
      TessellationControl,
      TessellationEvaluation,
      Compute
    };
    using ShaderPair = std::pair<std::string, GLuint>;
    using ShaderMap = std::map<ShaderType, ShaderPair>;
    using ShaderHook = std::function<std::string()>;
    ShaderMap shaders;
    GLuint program = 0;
		Shader(const RuntimeConstants& constants);
		~Shader();
		void use(const bool &useProgram);
  };
}