#pragma once
#include "../common.hpp"
#include <zg/glm.hpp>
namespace zg
{
	struct Window;
}
namespace zg::textures
{
	struct Texture;
	struct Framebuffer
	{
		Window &window;
		Texture &texture;
		Texture *depthTexturePointer = 0;
		glm::vec4 clearColor = glm::vec4(0);
		void *rendererData = 0;
		Framebuffer(Window &window, Texture &texture);
		Framebuffer(Window &window, Texture &texture, Texture &depthTexture);
		~Framebuffer();
		void bind() const;
		void unbind();
	};
#if defined(USE_GL) || defined(USE_EGL)
	struct GLFramebufferImpl
	{
		GLuint id = 0;
		GLuint renderbufferID = 0;
	};
#endif
} // namespace zg::textures