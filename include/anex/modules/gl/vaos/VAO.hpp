#pragma once
#include "../shaders/Shader.hpp"
namespace anex::modules::gl::vaos
{
	using namespace shaders;
	struct VAO
  {
		shaders::RuntimeConstants constants;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
		uint32_t indiceCount;
		uint32_t stride;
    VAO(const RuntimeConstants &constants, const uint32_t &indiceCount);
		~VAO();
		void updateIndices(const uint32_t *indices);
		void updateElements(const std::string_view &constant, const void *elements);
		void vaoDraw();
  };
};