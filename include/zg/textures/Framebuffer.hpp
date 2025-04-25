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
			Stencil,
			ColorResolve,
			DepthResolve
		};
		using TextureAttachmentPair = std::pair<Texture*, AttachmentType>;
		IRenderer* iRenderer = 0;
		std::vector<TextureAttachmentPair> textureAttachmentPairs;
		glm::vec4 clearColor = glm::vec4(0);
		Scene* scenePointer = 0;
		void* rendererData = 0;
		bool hasDepthAttachment() const
		{
			return std::find_if(textureAttachmentPairs.begin(), textureAttachmentPairs.end(), [](const auto& pair)
			{
				return pair.second == AttachmentType::DepthStencil || pair.second == AttachmentType::Depth;
			}) != textureAttachmentPairs.end();
		}
		bool hasColorAttachment() const
		{
			return std::find_if(textureAttachmentPairs.begin(), textureAttachmentPairs.end(), [](const auto& pair)
			{
				return pair.second == AttachmentType::Color;
			}) != textureAttachmentPairs.end();
		}
		bool hasColorResolveAttachment() const
		{
			return std::find_if(textureAttachmentPairs.begin(), textureAttachmentPairs.end(), [](const auto& pair)
			{
				return pair.second == AttachmentType::ColorResolve;
			}) != textureAttachmentPairs.end();
		}
		bool hasDepthResolveAttachment() const
		{
			return std::find_if(textureAttachmentPairs.begin(), textureAttachmentPairs.end(), [](const auto& pair)
			{
				return pair.second == AttachmentType::DepthResolve;
			}) != textureAttachmentPairs.end();
		}
		const Texture* getColorTexture() const
		{
			for (auto& pair : textureAttachmentPairs)
			{
				if (pair.second == AttachmentType::Color)
				{
					return pair.first;
				}
			}
			return 0;
		}
		const Texture* getColorResolveTexture() const
		{
			for (auto& pair : textureAttachmentPairs)
			{
				if (pair.second == AttachmentType::ColorResolve)
				{
					return pair.first;
				}
			}
			return 0;
		}
		const Texture* getDepthTexture() const
		{
			for (auto& pair : textureAttachmentPairs)
			{
				if (pair.second == AttachmentType::Depth)
				{
					return pair.first;
				}
			}
			return 0;
		}
		const Texture* getDepthResolveTexture() const
		{
			for (auto& pair : textureAttachmentPairs)
			{
				if (pair.second == AttachmentType::DepthResolve)
				{
					return pair.first;
				}
			}
			return 0;
		}
		Framebuffer(IRenderer* iRenderer, const std::vector<TextureAttachmentPair>& textureAttachmentPairs);
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
