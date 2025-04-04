#pragma once
#include <zg/glm.hpp>
#include "../common.hpp"
namespace zg
{
	struct Window;
	struct Scene;
}
namespace zg::textures
{
	struct Texture;
	struct Framebuffer
	{
		enum class AttachmentType
		{
			Color,
			Depth,
			DepthStencil,
			Stencil
		};
		using TextureAttachmentPair = std::pair<Texture*, AttachmentType>;
		Window& window;
		std::vector<TextureAttachmentPair> textureAttachmentPairs;
		glm::vec4 clearColor = glm::vec4(0);
		Scene* scenePointer = 0;
		void* rendererData = 0;
		bool hasDepthAttachment()
		{
			return std::find_if(textureAttachmentPairs.begin(), textureAttachmentPairs.end(), [](const auto& pair)
			{
				return pair.second == AttachmentType::DepthStencil || pair.second == AttachmentType::Depth;
			}) != textureAttachmentPairs.end();
		};
		bool hasColorAttachment()
		{
			return std::find_if(textureAttachmentPairs.begin(), textureAttachmentPairs.end(), [](const auto& pair)
			{
				return pair.second == AttachmentType::Color;
			}) != textureAttachmentPairs.end();
		};
		Framebuffer(Window& window, const  std::vector<TextureAttachmentPair>& textureAttachmentPairs);
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
