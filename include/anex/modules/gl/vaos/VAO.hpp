#pragma once
#include "../shaders/RuntimeConstants.hpp"
#include "../common.hpp"
#include "../GLWindow.hpp"
namespace anex::modules::gl::vaos
{
	using namespace shaders;
	struct VAO
  {
		GLWindow &window;
		shaders::RuntimeConstants constants;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
		uint32_t indiceCount;
		uint32_t elementCount;
		uint32_t stride;
    VAO(GLWindow &window, const RuntimeConstants &constants, uint32_t indiceCount, uint32_t elementCount);
		~VAO();
		void updateIndices(const std::vector<uint32_t> &indices) const;
		template<typename T>
		void updateElements(const std::string_view constant, const std::vector<T> &elements) const;
		void vaoDraw() const;
  };
};