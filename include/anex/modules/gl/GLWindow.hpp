#pragma once
#include <anex/IWindow.hpp>
#include "./common.hpp"
#include "./textures/Framebuffer.hpp"
#include <mutex>
namespace anex::modules::gl
{
	namespace shaders
	{
		struct Shader;
	}
	namespace entities
	{
		struct Plane;
	}
	struct ShaderContext
	{
		std::unordered_map<uint32_t, std::shared_ptr<shaders::Shader>> shaders;
		std::unordered_map<std::size_t, std::pair<uint32_t, std::shared_ptr<shaders::Shader>>> shadersByHash;
		uint32_t shaderCount = 0;
	};
	struct GLScene;
	struct GLWindow : IWindow
	{
		const char *title;
#ifdef _WIN32
		HINSTANCE hInstance;
		HWND hwnd;
		HDC hDeviceContext;
		HGLRC hRenderingContext;
#endif
		int windowKeys[256];
		int windowButtons[7];
		bool mouseMoved = false;
		glm::vec2 mouseCoords;
		int mod = 0;
		bool isChildWindow = false;
		GLWindow *parentWindow = 0;
		GLScene *parentScene = 0;
		std::vector<GLWindow> childWindows;
		GladGLContext *glContext = 0;
		ShaderContext *shaderContext = 0;
		std::shared_ptr<textures::Texture> framebufferTexture;
		std::shared_ptr<textures::Texture> framebufferDepthTexture;
		std::shared_ptr<textures::Framebuffer> framebuffer;
		std::shared_ptr<entities::Plane> framebufferPlane;
		static constexpr unsigned int MinMouseButton = 0;
		static constexpr unsigned int MaxMouseButton = 6;
		float dpiScale = 1.0f;
		GLWindow(const char *title,
						 const int32_t &windowWidth,
						 const int32_t &windowHeight,
						 const int32_t &windowX,
						 const int32_t &windowY,
						 const bool &borderless = false,
						 const uint32_t &framerate = 60);
		GLWindow(GLWindow &parentWindow,
						 GLScene &parentScene,
						 const char *childTitle,
						 const int32_t &childWindowWidth,
						 const int32_t &childWindowHeight,
						 const int32_t &childWindowX,
						 const int32_t &childWindowY,
						 const uint32_t &framerate = 60);
		~GLWindow();
		void startWindow() override;
		void renderInit();
		void updateKeyboard() override;
		void updateMouse() override;
		void close() override;
		void minimize() override;
		void maximize() override;
		void restore() override;
		void preRender() override;
		void postRender() override;
		void drawLine(int x0, int y0, int x1, int y1, uint32_t color) override;
		void drawRectangle(int x, int y, int w, int h, uint32_t color) override;
		void drawCircle(int x, int y, int radius, uint32_t color) override;
		void drawText(int x, int y, const char* text, int scale, uint32_t color) override;
		void warpPointer(const glm::vec2 &coords) override;
		void setXY(const int32_t &x, const int32_t &y) override;
		void setWidthHeight(const uint32_t &width, const uint32_t &height) override;
		IWindow &createChildWindow(const char *title, IScene &scene, const uint32_t &windowWidth, const uint32_t &windowHeight, const int32_t &windowX, const int32_t &windowY) override;
	};
	void computeNormals(const std::vector<uint32_t> &indices, const std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals);
}
