#pragma once
#include <zg/common.hpp>
#include <zg/shaders/RuntimeConstants.hpp>
#include <zg/interfaces/IRenderer.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/GlobalUID.hpp>
namespace zg::vaos
{
	using namespace shaders;
	struct VAO
	{
		shaders::RuntimeConstants constants;
		uint32_t indiceCount;
		uint32_t vertexCount;
		uint32_t stride;
		std::vector<size_t*> VAO_INDEX_STACK;
		IRenderer* vaoIRenderer = 0;
		void* rendererData = 0;
		size_t uid = GlobalUID::GetNew();
		std::unordered_map<void*, shaders::Shader*> shaders;
		std::unordered_map<size_t, bool> ensuredBools;
		VAO();
		VAO(const VAO& other);
		VAO(const std::vector<size_t*>& VAO_INDEX_STACK, const RuntimeConstants& constants, uint32_t indiceCount, uint32_t vertexCount);
		VAO& operator=(const VAO& other);
		virtual ~VAO();
		void updateIndices(const std::vector<uint32_t>& indices);
		template <typename T>
		void updateElements(const std::string_view constant, const std::vector<T>& elements) const;
		void drawVAO() const;
		static void* getShaderUHash(IRenderer* iRenderer);
		size_t getVAOuHash() const;
		bool isEnsured();
		void setEnsured();
		shaders::Shader* addShader(shaders::Shader* setShader = 0);
	};
#if defined(USE_GL) || defined(USE_EGL)
	struct GLVAOImpl
	{
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;
	};
#endif
}; // namespace zg::vaos
