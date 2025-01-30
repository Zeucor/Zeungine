#pragma once
#include "../shaders/RuntimeConstants.hpp"
#include "../common.hpp"
#include "../Window.hpp"
namespace zg::vaos
{
	using namespace shaders;
	struct VAO
  {
	shaders::RuntimeConstants constants;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
	uint32_t indiceCount;
	uint32_t elementCount;
	uint32_t stride;
	Window &vaoWindow;
    VAO(Window &_window, const RuntimeConstants &constants, uint32_t indiceCount, uint32_t elementCount);
	virtual ~VAO();
	void updateIndices(const std::vector<uint32_t> &indices);
	template<typename T>
	void updateElements(const std::string_view constant, const std::vector<T> &elements) const;
	void drawVAO() const;
  };
};