#pragma once
#include <anex/IWindow.hpp>
#include <anex/modules/gl/shaders/Shader.hpp>
#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <anex/modules/gl/vaos/VAO.hpp>
#include "./common.hpp"
namespace anex::modules::gl
{
	struct GLTest : IWindow
	{
		const char *title;
#ifdef _WIN32
		HINSTANCE hInstance;
		HWND hwnd;
		HDC hDeviceContext;
		HGLRC hRenderingContext;
		std::vector<HWND> childWindows;
#endif
		int windowKeys[256];
		int windowButtons[7];
		bool mouseMoved = false;
		glm::vec2 mouseCoords;
		int mod = 0;
		GLTest(const char *title, const int &windowWidth, const int &windowHeight, const int &framerate = 60);
		~GLTest();
		void startWindow() override;
		void renderInit();
		void updateKeyboard() override;
		void updateMouse() override;
		void close() override;
		void drawLine(int x0, int y0, int x1, int y1, uint32_t color) override;
		void drawRectangle(int x, int y, int w, int h, uint32_t color) override;
		void drawCircle(int x, int y, int radius, uint32_t color) override;
		void drawText(int x, int y, const char* text, int scale, uint32_t color) override;
		void warpPointer(const glm::vec2 &coords) override;
		void createChildWindow(const char *title, const uint32_t &windowWidth, const uint32_t &windowHeight) override;
	};
	template<size_t VerticesLength>
	void computeNormals(const uint32_t &indicesCount, const uint32_t *indices, const std::array<glm::vec3, VerticesLength> &positions, std::array<glm::vec3, VerticesLength> &normals)
	{
		const glm::vec3 *positionsData = positions.data();
		const glm::ivec3 *indicesData = (const glm::ivec3 *)indices;
		const int nbTriangles = indicesCount / 3;
		const int nbVertices = VerticesLength;
		const int nbNormals = VerticesLength;
		auto *normalsData = normals.data();
		memset(normalsData, 0, nbVertices * sizeof(glm::vec3));
		for (uint32_t index = 0; index < nbTriangles; index++)
		{
			const auto& indice = indicesData[index];
			const auto& vertex1 = positionsData[indice.x];
			const auto& vertex2 = positionsData[indice.y];
			const auto& vertex3 = positionsData[indice.z];
			auto& normal1 = normalsData[indice.x];
			auto& normal2 = normalsData[indice.y];
			auto& normal3 = normalsData[indice.z];
			auto triangleNormal = cross(vertex2 - vertex1, vertex3 - vertex1);
			if (!triangleNormal.x && !triangleNormal.y && !triangleNormal.z)
			{
				continue;
			}
			normal1 += triangleNormal;
			normal2 += triangleNormal;
			normal3 += triangleNormal;
		}
		for (uint32_t index = 0; index < nbVertices; index++)
		{
			auto& normal = normalsData[index];
			normal = glm::normalize(normal);
		}
	}
}